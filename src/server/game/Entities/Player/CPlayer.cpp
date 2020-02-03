#include "CPlayer.h"
#include "Player.h"
#include "WorldSession.h"
#include "World.h"
#include "MiscPackets.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "Battleground.h"
#include "CharacterCache.h"

CPlayer::CPlayer(WorldSession* session) : Player(session)
{
}

CPlayer::~CPlayer()
{
}

void CPlayer::CFJoinBattleground()
{
    if (!sWorld->getBoolConfig(CONFIG_CFBG_ENABLED))
        return;

    if (NativeTeam())
        return;

    SetByteValue(UNIT_FIELD_BYTES_0, 0, GetFRace());
    SetFaction(GetFFaction());
    ReplaceRacials(NativeTeam());
    FakeDisplayID();

    // Calling this in BattleGround::AddPlayer fixes scoreboard
    sCharacterCache->UpdateCharacterData(GetGUID(), GetName(), boost::none, GetRace());
}

void CPlayer::CFLeaveBattleground()
{
    if (!sWorld->getBoolConfig(CONFIG_CFBG_ENABLED))
        return;

    SetByteValue(UNIT_FIELD_BYTES_0, 0, GetORace());
    SetFaction(GetOFaction());
    ReplaceRacials(true);
    InitDisplayIds();

    sCharacterCache->UpdateCharacterData(GetGUID(), GetName(), boost::none, GetRace());
}

void CPlayer::FakeDisplayID()
{
    if (!sWorld->getBoolConfig(CONFIG_CFBG_ENABLED))
        return;

    PlayerInfo const* info = sObjectMgr->GetPlayerInfo(GetRace(), GetClass());

    if (!info)
        return;

    SetObjectScale(1.f);

    uint8 gender = GetGender();
    switch (gender)
    {
    case GENDER_FEMALE:
        SetDisplayId(info->displayId_f);
        SetNativeDisplayId(info->displayId_f);
        break;
    case GENDER_MALE:
        SetDisplayId(info->displayId_m);
        SetNativeDisplayId(info->displayId_m);
        break;
    default:
        return;
    }
}

void CPlayer::ReplaceRacials(bool native)
{
    if (!sWorld->getBoolConfig(CONFIG_CFBG_ENABLED))
        return;

    std::unordered_map<uint16, bool> skills;

    auto updateskills = [&] (uint32 prace, uint32 pclass, bool add) -> void
    {
        if (auto pinfo = sObjectMgr->GetPlayerInfo(prace, pclass))
        {
            for (auto& i : pinfo->skills)
            {
                if (SkillLineEntry const* skill = sSkillLineStore.LookupEntry(i.SkillId))
                {
                    if ((skill->categoryId == SKILL_CATEGORY_LANGUAGES &&
                         sWorld->getBoolConfig(CONFIG_CFBG_REPLACELANGUAGES)) ||
                        (skill->categoryId != SKILL_CATEGORY_LANGUAGES &&
                         sWorld->getBoolConfig(CONFIG_CFBG_REPLACERACIALS)))
                    {
                        skills[i.SkillId] = add;
                    }
                }
            }
        }
    };

    for (uint8 i = 0; i < MAX_RACES; ++i)
        updateskills(i, GetClass(), false);

    updateskills(native ? GetORace() : GetFRace(), GetClass(), true);

    for (auto& i : skills)
    {
        if (i.second)
            LearnDefaultSkill(i.first, 0);
        else
        {
            SetSkill(i.first, 0, 0, 0);
            SkillStatusMap::iterator itr = mSkillStatus.find(i.first);
            if (itr != mSkillStatus.end())
                itr->second.DBDelete = false;
        }
    }
    SendUpdateToPlayer(this);
}

void CPlayer::InitializeCFData()
{
    m_oRace = GetRace();
    Player::SetFactionForRace(GetRace());
    m_oFaction = GetFaction();
    m_oTeam = Player::TeamForRace(m_oRace);

    m_fRace = 0;

    while (m_fRace == 0)
        for (uint8 i = 0; i < MAX_RACES; ++i)
            if (sObjectMgr->GetPlayerInfo(i, GetClass()) && Player::TeamForRace(i) != GetOTeam() && urand(0, 5) == 0)
                m_fRace = i;

    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(m_fRace);
    m_fFaction = rEntry ? rEntry->FactionID : 0;
    m_fTeam = Player::TeamForRace(m_fRace);
}

uint32 CPlayer::GetTeam() const
{
    return m_bgData.bgTeam && GetBattleground() ? m_bgData.bgTeam : GetOTeam();
}
