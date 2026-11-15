#include "ScriptMgr.h"
#include "Chat.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

#define POI_NAME_0  "Nearest Innkeeper"
#define POI_NAME_6  "Posadero m\xC3\xA1s cercano"
#define POI_NAME_7  "Posadero m\xC3\xA1s cercano"

#define WARN_MSG_0  "You are not in a rested area. The nearest innkeeper is %.1f yards away (marked on your map)."
#define WARN_MSG_6  "No est\xC3\xA1s en una zona de descanso. El posadero m\xC3\xA1s cercano est\xC3\xA1 a %.1f yardas (marcado en tu mapa)."
#define WARN_MSG_7  "No est\xC3\xA1s en una zona de descanso. El posadero m\xC3\xA1s cercano est\xC3\xA1 a %.1f yardas (marcado en tu mapa)."

namespace
{
    struct InnkeeperSpawn
    {
        uint32 guid;
        float x;
        float y;
        float z;
        uint32 faction;
    };

    std::unordered_map<uint32, std::vector<InnkeeperSpawn>> InnkeeperCache;

    void LoadInnkeeperSpawns()
    {
        InnkeeperCache.clear();

        QueryResult result = WorldDatabase.PQuery(
            "SELECT c.guid, c.map, c.position_x, c.position_y, c.position_z, ct.faction "
            "FROM creature c "
            "INNER JOIN creature_template ct ON c.id = ct.entry "
            "WHERE (IF(c.npcflag <> 0, c.npcflag, ct.npcflag) & {}) <> 0",
            uint32(UNIT_NPC_FLAG_INNKEEPER));

        if (!result)
            return;

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 mapId = fields[1].GetUInt32();

            InnkeeperSpawn spawn;
            spawn.guid    = fields[0].GetUInt32();
            spawn.x       = fields[2].GetFloat();
            spawn.y       = fields[3].GetFloat();
            spawn.z       = fields[4].GetFloat();
            spawn.faction = fields[5].GetUInt32();

            InnkeeperCache[mapId].push_back(spawn);
            ++count;
        } while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded {} innkeeper spawns for logout check.", count);
    }

    InnkeeperSpawn const* GetNearestFriendlyInnkeeper(Player const* player, float& outDistance)
    {
        if (!player || !player->IsInWorld())
            return nullptr;

        auto itr = InnkeeperCache.find(player->GetMapId());
        if (itr == InnkeeperCache.end() || itr->second.empty())
            return nullptr;

        FactionTemplateEntry const* playerFaction = player->GetFactionTemplateEntry();
        if (!playerFaction)
            return nullptr;

        float minDistSq = std::numeric_limits<float>::max();
        InnkeeperSpawn const* nearest = nullptr;

        for (InnkeeperSpawn const& inn : itr->second)
        {
            FactionTemplateEntry const* innFaction = sFactionTemplateStore.LookupEntry(inn.faction);
            if (!innFaction)
                continue;

            if (playerFaction->IsHostileTo(*innFaction) || innFaction->IsHostileTo(*playerFaction))
                continue;

            float dx = player->GetPositionX() - inn.x;
            float dy = player->GetPositionY() - inn.y;
            float dz = player->GetPositionZ() - inn.z;
            float distSq = dx * dx + dy * dy + dz * dz;

            if (distSq < minDistSq)
            {
                minDistSq = distSq;
                nearest = &inn;
            }
        }

        if (nearest)
            outDistance = std::sqrt(minDistSq);

        return nearest;
    }
}

class mod_LogoutInnkeeperCheck_WorldScript : public WorldScript
{
public:
    mod_LogoutInnkeeperCheck_WorldScript() : WorldScript("mod_LogoutInnkeeperCheck_WorldScript") { }

    void OnStartup() override
    {
        LoadInnkeeperSpawns();
    }
};

class mod_LogoutInnkeeperCheck_PlayerScript : public PlayerScript
{
public:
    mod_LogoutInnkeeperCheck_PlayerScript() : PlayerScript("mod_LogoutInnkeeperCheck_PlayerScript") { }

    void OnLogoutRequest(Player* player) override
    {
        if (!player || player->GetLevel() >= 80)
            return;

        if (player->HasRestFlag(REST_FLAG_IN_TAVERN) || player->HasRestFlag(REST_FLAG_IN_CITY))
            return;

        float dist = 0.0f;
        if (InnkeeperSpawn const* inn = GetNearestFriendlyInnkeeper(player, dist))
        {
            char const* poiName = POI_NAME_0;
            char const* warnMsg = WARN_MSG_0;

            switch (player->GetSession()->GetSessionDbcLocale())
            {
                case 6: // esES
                    poiName = POI_NAME_6;
                    warnMsg = WARN_MSG_6;
                    break;
                case 7: // esMX
                    poiName = POI_NAME_7;
                    warnMsg = WARN_MSG_7;
                    break;
                default:
                    break;
            }

            WorldPacket data(SMSG_GOSSIP_POI, 4 + 4 + 4 + 4 + 4 + 32);
            data << uint32(99);
            data << float(inn->x);
            data << float(inn->y);
            data << uint32(7);
            data << uint32(0);
            data << poiName;
            player->SendDirectMessage(&data);

            ChatHandler(player->GetSession()).PSendSysMessage(warnMsg, dist);
        }
    }
};

void AddSC_Mod_LogoutInnkeeperCheck()
{
    new mod_LogoutInnkeeperCheck_WorldScript();
    new mod_LogoutInnkeeperCheck_PlayerScript();
}
