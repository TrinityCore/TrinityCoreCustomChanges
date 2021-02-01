/* Original Copyright
Original Author : Callmephil
Originally for Version : 3.3.5 / 4.3.4
Redone for TC Custom Branch 3.3.5 (4.3.4 is untested)
By Single Player Project Developer MDic
Dynamic Resurrection is a simple script that add a "Resurrection Waypoint" near the latest boss killed in dungeon or raid. for faster Resurrection.
*/

#include "Chat.h"
#include "Config.h"
#include "CreatureAI.h"
#include "DynamicResurrection.h"
#include "GameTime.h"
#include "Object.h"
#include "Player.h"

class Dynamic_Resurrections : public PlayerScript
{

public:
    Dynamic_Resurrections() : PlayerScript("Dynamic_Resurrections") {}

    void OnLogin(Player* Player, bool /*firstLogin*/) override
    {
        if (!sConfigMgr->GetBoolDefault("Dynamic.Resurrections.Announce.enable", false))
        {
            ChatHandler(Player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Trinitycore Dynamic Resurrections Announce |rmodule.");
        }

    }

    void OnCreatureKill(Player* player, Creature* boss) override
    {
        if (!sConfigMgr->GetBoolDefault("Dynamic.Resurrections.enable", false))
        {
            if (sDynRes->IsInDungeonOrRaid(player) && (boss->isWorldBoss() || boss->IsDungeonBoss()))
                player->SummonCreature(C_Resurrection_ENTRY, boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(), boss->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120min);
        }
    }
};

void AddSC_Dynamic_Resurrections()
{
    new Dynamic_Resurrections();
}
