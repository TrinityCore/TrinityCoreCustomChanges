#include "Config.h"
#include "DynamicResurrection.h"
#include "Group.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Player.h"

bool Dynamic_Resurrection::IsInDungeonOrRaid(Player* player)
{
    if (!player)
        return false;

    if (MapEntry const* mapEntry = sMapStore.LookupEntry(player->GetMapId()))
        return mapEntry->Instanceable();

    return false;
}

void Dynamic_Resurrection::SetSpawnPoint(uint32 instanceId, WorldLocation const& loc)
{
    if (instanceId > 0)
        _checkpoints[instanceId] = loc;
}

bool Dynamic_Resurrection::CheckForSpawnPoint(Player* player)
{
    if (!player || !sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", true))
        return false;

    auto itr = _checkpoints.find(player->GetInstanceId());
    return itr != _checkpoints.end() && itr->second.GetMapId() == player->GetMapId();
}

void Dynamic_Resurrection::DynamicResurrection(Player* player)
{
    auto itr = _checkpoints.find(player->GetInstanceId());
    if (itr == _checkpoints.end())
        return;

    WorldLocation const& loc = itr->second;
    float DRD = sConfigMgr->GetFloatDefault("Dynamic.Ressurrection.Dungeon.Health", 0.7f);
    float DRR = sConfigMgr->GetFloatDefault("Dynamic.Ressurrection.Raid.Health", 0.7f);

    uint8 combatcount = 0;
    if (Group* group = player->GetGroup())
    {
        for (GroupReference* itrGrp = group->GetFirstMember(); itrGrp != nullptr; itrGrp = itrGrp->next())
            if (Player* member = itrGrp->GetSource())
                if (member != player && member->IsAlive() && member->IsInCombat() && member->GetInstanceId() == player->GetInstanceId())
                    ++combatcount;
    }

    Map* map = player->GetMap();
    if (map->IsRaid() && sConfigMgr->GetBoolDefault("Raid.Entrance.Resurrection", true))
    {
        if (combatcount >= 1)
        {
            if (AreaTriggerTeleport const* exit = sObjectMgr->GetGoBackTrigger(map->GetId()))
            {
                player->TeleportTo(exit->target_mapId, exit->target_X, exit->target_Y, exit->target_Z, exit->target_Orientation + float(M_PI));
                player->ResurrectPlayer(DRR);
            }
            return;
        }

        player->TeleportTo(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ() + 50.0f, loc.GetOrientation());
        player->ResurrectPlayer(DRR);
        player->SpawnCorpseBones();
        return;
    }

    if (map->IsDungeon())
    {
        player->TeleportTo(loc);
        player->ResurrectPlayer(DRD);
        player->SpawnCorpseBones();
    }
}
