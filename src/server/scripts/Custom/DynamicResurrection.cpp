/* Original Copyright
Original Author : Callmephil
Originally for Version : 3.3.5 / 4.3.4
Redone for TC Custom Branch 3.3.5 (4.3.4 is untested)
By Single Player Project Developer MDic
Assistance by Project Nemesis Developer Jinnai
Dynamic Resurrection is a simple script that add a "Resurrection Waypoint" near the latest boss killed in dungeon or raid. for faster Resurrection.
*/

#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "DynamicResurrection.h"
#include "GameTime.h"
#include "Group.h"
#include "GroupMgr.h"
#include "InstanceScript.h"
#include "Map.h"
#include "Object.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"

class Dynamic_Resurrections : public PlayerScript
{

public:
    Dynamic_Resurrections() : PlayerScript("Dynamic_Resurrections") {}

    void OnLogin(Player* Player, bool /*firstLogin*/) override
    {
        if (sConfigMgr->GetBoolDefault("Dynamic.Resurrections.Announce.enable", true))
        {
            ChatHandler(Player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Trinitycore Dynamic Resurrections |rmodule.");
        }

    }

    void OnCreatureKill(Player* player, Creature* boss) override
    {
        if (sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", true))
        {
            if (sDynRes->IsInDungeonOrRaid(player) && (boss->isWorldBoss() || boss->IsDungeonBoss()))
            {
                std::list<Creature*> REZlist;
                GetCreatureListWithEntryInGrid(REZlist, player, C_Resurrection_ENTRY, 50000.00f);
                for (std::list<Creature*>::iterator itr = REZlist.begin(); itr != REZlist.end(); ++itr)
                {
                    if(Creature * creature = *itr)
                    {
                        creature->DespawnOrUnsummon();
                    }
                }
                player->SummonCreature(C_Resurrection_ENTRY, boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(), boss->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120min);
            }
        }
    }
    void OnMapChanged(Player* player) override
    {
        Map* map = player->GetMap();

        if (map->IsRaid() && sConfigMgr->GetBoolDefault("Raid.Entrance.Resurrection", true))
        {
            if (Creature* checkpoint = player->FindNearestCreature(C_Resurrection_ENTRY, C_DISTANCE_CHECK_RANGE))
            {
                player->TeleportTo(player->GetMapId(), checkpoint->GetPositionX(), checkpoint->GetPositionY(), checkpoint->GetPositionZ(), 1);
                player->SpawnCorpseBones();
            }
        }
    }
};

void AddSC_Dynamic_Resurrections()
{
    new Dynamic_Resurrections();
}
