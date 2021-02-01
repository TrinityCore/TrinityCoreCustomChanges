/* Original Copyright
Original Author : Callmephil
Originally for Version : 3.3.5 / 4.3.4
Redone for TC Custom Branch 3.3.5 (4.3.4 is untested)
By Single Player Project Developer MDic
Dynamic Resurrection is a simple script that add a "Resurrection Waypoint" near the latest boss killed in dungeon or raid. for faster Resurrection.
*/

#include "Config.h"
#include "DynamicResurrection.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Player.h"

bool Dynamic_Resurrection::IsInDungeonOrRaid(Player* player)
{
    if (!sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", false))
    {
        Group* group = player->GetGroup();

        if (group->GetMembersCount() >= 5)
        {
            return false;
        }

        if (sMapStore.LookupEntry(player->GetMapId())->Instanceable())
        {
            return true; // boolean need to return to a value
        }
        return false;
    }
    else
    {
        return false;
    }
}

bool Dynamic_Resurrection::CheckForSpawnPoint(Player* player)
{
    if (!sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", false))
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
    if (!sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", false))
    {
        // Find Nearest Creature And Teleport.
        if (player->FindNearestCreature(C_Resurrection_ENTRY, C_DISTANCE_CHECK_RANGE))
            player->TeleportTo(player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 1);
        // Revive Player with 70 %
        player->ResurrectPlayer(0.7f);
        player->SpawnCorpseBones();
    }
    else
    {
        return;
    }
}
