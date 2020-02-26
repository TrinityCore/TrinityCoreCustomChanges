#include "ScriptMgr.h"
#include "CFBGData.h"
#include "Chat.h"
#include "RBAC.h"
#include "WorldSession.h"
#include "Battleground.h"
#include "World.h"
#include "Config.h"

class CrossFactionBattlegroundPlayerScript : public PlayerScript
{
public:
    CrossFactionBattlegroundPlayerScript() : PlayerScript("CrossfactionBattlegroundPlayerScript") { }

    void OnLogin(Player* player, bool /*firstLogin*/)
    {
        if (!player->cfbgdata->NativeTeam())
        {
            player->SetRace(player->cfbgdata->GetFRace());
            player->SetFactionForRace(player->GetRace());
            player->cfbgdata->SetRaceDisplayID();
        }
    }
};

class CrossFactionBattlegroundCommandScript : public CommandScript
{
public:
    CrossFactionBattlegroundCommandScript() : CommandScript("CrossFactionBattlegroundCommandScript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> debugCommandTable =
        {
            { "battleground", rbac::RBAC_PERM_COMMAND_DEBUG, false, &HandleDebugBattlegroundCommand, "" },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "debug",	rbac::RBAC_PERM_COMMAND_DEBUG, true, nullptr, "", debugCommandTable },
        };
        return commandTable;
    }
    static bool HandleDebugBattlegroundCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (!handler->GetSession())
            return false;

        auto bg = handler->GetSession()->GetPlayer()->GetBattleground();
        if (bg)
        {
            bg->SetStartDelayTime(-1);
            handler->SendSysMessage("Battleground started");
        }
        return true;
    }
};

class CrossFactionBattlegroundWorldScript : public WorldScript
{
public:
    CrossFactionBattlegroundWorldScript() : WorldScript("CrossFactionBattlegroundWorldScript") { }

    void OnConfigLoad(bool /*reload*/)
    {
        sWorld->setBoolConfig(CONFIG_CFBG_ENABLED, sConfigMgr->GetBoolDefault("CrossFactionBattlegrounds.Enable", true));
        sWorld->setBoolConfig(CONFIG_CFBG_REPLACELANGUAGES, sConfigMgr->GetBoolDefault("CrossFactionBattlegrounds.ReplaceLanguages", true));
        sWorld->setBoolConfig(CONFIG_CFBG_REPLACERACIALS, sConfigMgr->GetBoolDefault("CrossFactionBattlegrounds.ReplaceRacials", false));
    }
};

void AddSC_CrossfactionBattlegrounds()
{
    new CrossFactionBattlegroundPlayerScript();
    new CrossFactionBattlegroundCommandScript();
    new CrossFactionBattlegroundWorldScript();
}
