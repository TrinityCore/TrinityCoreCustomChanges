#ifndef __CBATTLEGROUNDQUEUE_H
#define __CBATTLEGROUNDQUEUE_H

#include "Common.h"
#include "BattlegroundQueue.h"

class TC_GAME_API CBattlegroundQueue : public BattlegroundQueue
{
    using BattlegroundQueue::BattlegroundQueue;
public:
    bool CheckMixedMatch(Battleground* bg_template, BattlegroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers);
    bool MixPlayersToBG(Battleground* bg, BattlegroundBracketId bracket_id);
    bool CFBGGroupInserter(Battleground* bg_template, BattlegroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers, uint32 minplayers);
};

#endif // __CBATTLEGROUNDQUEUE_H

