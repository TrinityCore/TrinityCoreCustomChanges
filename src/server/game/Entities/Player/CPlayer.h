#ifndef _CPLAYER_H
#define _CPLAYER_H

#include "Common.h"
#include "Player.h"

class WorldSession;

class TC_GAME_API CPlayer : public Player
{
public:
    explicit CPlayer(WorldSession* session);
    ~CPlayer();

    bool NativeTeam() { return GetTeam() == GetOTeam(); }
    uint8 GetFRace() const { return m_fRace; }
    uint8 GetORace() const { return m_oRace; }
    uint32 GetOFaction() const { return m_oFaction; }
    uint32 GetFFaction() const { return m_fFaction; }
    uint32 GetTeam() const override;
    uint32 GetOTeam() const { return m_oTeam; }
    void CFJoinBattleground();
    void CFLeaveBattleground();
    void ReplaceRacials(bool native);
    void InitializeCFData();
    void FakeDisplayID();

private:
    uint8 m_fRace;
    uint8 m_oRace;
    uint32 m_fFaction;
    uint32 m_oFaction;
    uint32 m_fTeam;
    uint32 m_oTeam;
};

#endif // _CPLAYER_H
