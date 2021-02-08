/* Original Copyright
Original Author : Callmephil
Originally for Version : 3.3.5 / 4.3.4
Redone for TC Custom Branch 3.3.5 (4.3.4 is untested)
By Single Player Project Developer MDic
Assistance by Project Nemesis Developer Jinnai
Dynamic Resurrection is a simple script that add a "Resurrection Waypoint" near the latest boss killed in dungeon or raid. for faster Resurrection.
*/

#ifndef DYNAMIC_RESURRECTION
#define DYNAMIC_RESURRECTION

#include "Player.h"
#include "DBCStores.h"

class Player;

enum WAYPOINT_CREATURE
{
    C_Resurrection_ENTRY = 1, // change this as you wishes
    C_DISTANCE_CHECK_RANGE = 50000, // change this (in yards)
};

class TC_GAME_API Dynamic_Resurrection
{
public:
    static Dynamic_Resurrection* instance()
    {
        static Dynamic_Resurrection instance;
        return &instance;
    }

    void OnMapChanged(Player* player);
    bool IsInDungeonOrRaid(Player* player);
    bool CheckForSpawnPoint(Player* player);
    void DynamicResurrection(Player* player);
};
#define sDynRes Dynamic_Resurrection::instance()

#endif
