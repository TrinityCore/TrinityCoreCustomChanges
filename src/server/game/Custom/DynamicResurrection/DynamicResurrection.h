#ifndef DYNAMIC_RESURRECTION
#define DYNAMIC_RESURRECTION

#include "Player.h"
#include "DBCStores.h"
#include <unordered_map>

class TC_GAME_API Dynamic_Resurrection
{
public:
    static Dynamic_Resurrection* instance()
    {
        static Dynamic_Resurrection instance;
        return &instance;
    }

    bool IsInDungeonOrRaid(Player* player);
    bool CheckForSpawnPoint(Player* player);
    void SetSpawnPoint(uint32 instanceId, WorldLocation const& loc);
    void DynamicResurrection(Player* player);

private:
    std::unordered_map<uint32, WorldLocation> _checkpoints;
};
#define sDynRes Dynamic_Resurrection::instance()

#endif
