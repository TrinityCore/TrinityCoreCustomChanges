/* Original Copyright
Original Author : Callmephil
Originally for Version : 3.3.5 / 4.3.4
Redone for TC Custom Branch 3.3.5 (4.3.4 is untested)
By Single Player Project Developer MDic
Assistance by Project Nemesis Developer Jinnai
Dynamic Resurrection is a simple script that add a "Resurrection Waypoint" near the latest boss killed in dungeon or raid. for faster Resurrection.
*/

#include "Config.h"
#include "CreatureAI.h"
#include "DatabaseEnv.h"
#include "DynamicResurrection.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Map.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptMgr.h"

uint8 combatcount = 0;

bool Dynamic_Resurrection::IsInDungeonOrRaid(Player* player)
{
    if (sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", true))
    {
        if (sMapStore.LookupEntry(player->GetMapId())->Instanceable())
        {
            return true; // boolean need to return to a value
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Dynamic_Resurrection::CheckForSpawnPoint(Player* player)
{
    if (sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", true))
    {
        // Find Nearest Creature And Teleport.
        if (player->FindNearestCreature(C_Resurrection_ENTRY, C_DISTANCE_CHECK_RANGE))
        {
            return true;
        }
        return false;
    }
    else
    {
        return false;
    }
}

void Dynamic_Resurrection::DynamicResurrection(Player* player)
{
    float DRD = sConfigMgr->GetFloatDefault("Dynamic.Ressurrection.Dungeon.Health", 0.7f);
    float DRR = sConfigMgr->GetFloatDefault("Dynamic.Ressurrection.Raid.Health", 0.7f);

    if (sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", true))
    {
        Map* map = player->GetMap();

        if (Group* group = player->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                if (Player* member = itr->GetSource())
                    if (member->IsInCombat())
                        combatcount++;
        }

        if (combatcount > 0)
        {
            if (AreaTrigger const* exit = sObjectMgr->GetGoBackTrigger(map->GetId()))
            {
                player->TeleportTo(exit->target_mapId, exit->target_X, exit->target_Y, exit->target_Z, exit->target_Orientation + M_PI);
                if (map->IsRaid())
                    {
                        player->ResurrectPlayer(DRR);
                        player->SpawnCorpseBones();
                        return;
                    }
                else
                    {
                        player->ResurrectPlayer(DRD);
                        player->SpawnCorpseBones();
                        return;
                    }
            }
        }

        if (map->IsRaid())
        {
            if (Creature* checkpoint = player->FindNearestCreature(C_Resurrection_ENTRY, C_DISTANCE_CHECK_RANGE))
            {
                player->TeleportTo(player->GetMapId(), checkpoint->GetPositionX(), checkpoint->GetPositionY(), checkpoint->GetPositionZ(), 1);
                player->ResurrectPlayer(DRR);
                player->SpawnCorpseBones();
            }
        }
        // Find Nearest Creature And Teleport.
        if (map->IsDungeon())
        {
            if (Creature* checkpoint = player->FindNearestCreature(C_Resurrection_ENTRY, C_DISTANCE_CHECK_RANGE))
            {
                player->TeleportTo(player->GetMapId(), checkpoint->GetPositionX(), checkpoint->GetPositionY(), checkpoint->GetPositionZ(), 1);
                // Revive Player with 70 %
                player->ResurrectPlayer(DRD);
                player->SpawnCorpseBones();
            }
        }
    }
    else
    {
        return;
    }
}
