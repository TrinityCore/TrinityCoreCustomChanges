/*
** Made by Traesh https://github.com/Traesh
** Conan513 https://github.com/conan513
** Made into a module by Micrah https://github.com/milestorme/
** Converted to TrinityCore by LEO33 http://leo33.info
*/

#include "ScriptMgr.h"
#include "Player.h"
#include "Configuration/Config.h"
#include "World.h"
#include "LFGMgr.h"
#include "Chat.h"
#include "Opcodes.h"

class lfg_solo_announce : public PlayerScript
{
public:
    lfg_solo_announce() : PlayerScript("lfg_solo_announce") {}

    void OnLogin(Player* player, bool /*loginFirst*/)
    {
        // Announce Module
        if (sConfigMgr->GetBoolDefault("SoloLFG.Announce", true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Solo Dungeon Finder|r module.");
         }
    }
};

class lfg_solo : public PlayerScript
{
public:
    lfg_solo() : PlayerScript("lfg_solo") { }

   void OnLogin(Player* player, bool /*loginFirst*/)
   {
      if (sConfigMgr->GetIntDefault("SoloLFG.Enable", true))
        {
            if (!sLFGMgr->IsSoloLFG())
            {
            sLFGMgr->ToggleSoloLFG();
            }
        }
   }
};

void AddLfgSoloScripts()
{
    new lfg_solo_announce();
    new lfg_solo();
}