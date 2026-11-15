#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "DynamicResurrection.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"

namespace
{
    // ICC Gunship Battle commanders (Map 631)
    constexpr uint32 MAP_ICECROWN_CITADEL     = 631;
    constexpr uint32 NPC_IGB_MURADIN          = 36948;
    constexpr uint32 NPC_IGB_SAURFANG         = 36939;
}

class Dynamic_Resurrections : public PlayerScript
{
public:
    Dynamic_Resurrections() : PlayerScript("Dynamic_Resurrections") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (sConfigMgr->GetBoolDefault("Dynamic.Resurrections.Announce.enable", true))
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Trinitycore Dynamic Resurrections |rmodule.");
    }

    void OnCreatureKill(Player* player, Creature* boss) override
    {
        if (!player || !boss || !sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", true))
            return;

        if (!sDynRes->IsInDungeonOrRaid(player))
            return;

        // Hardcoded landing pad in front of the Gunship teleporter (Rise of the Deathbringer)
        if (player->GetMapId() == MAP_ICECROWN_CITADEL &&
            (boss->GetEntry() == NPC_IGB_MURADIN || boss->GetEntry() == NPC_IGB_SAURFANG || boss->GetTransport()))
        {
            sDynRes->SetSpawnPoint(player->GetInstanceId(), WorldLocation(MAP_ICECROWN_CITADEL, -536.0f, 2211.0f, 539.3f, 0.0f));
            return;
        }

        if (boss->isWorldBoss() || boss->IsDungeonBoss())
            sDynRes->SetSpawnPoint(player->GetInstanceId(), boss->GetWorldLocation());
    }

    void OnMapChanged(Player* player) override
    {
        if (!player || !sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", true))
            return;

        Map* map = player->GetMap();
        if (map && map->IsRaid() && sConfigMgr->GetBoolDefault("Raid.Entrance.Resurrection", true) && sDynRes->CheckForSpawnPoint(player))
            sDynRes->DynamicResurrection(player);
    }
};

void AddSC_Dynamic_Resurrections()
{
    new Dynamic_Resurrections();
}
