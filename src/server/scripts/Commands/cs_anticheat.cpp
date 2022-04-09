/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Language.h"
#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "Chat.h"
#include "AnticheatMgr.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"

using namespace Trinity::ChatCommands;

class anticheat_commandscript : public CommandScript
{
public:
    anticheat_commandscript() : CommandScript("anticheat_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable anticheatCommandTable =
        {
            { "global",      HandleAntiCheatGlobalCommand,   rbac::RBAC_ROLE_GAMEMASTER,              Console::Yes },
            { "player",      HandleAntiCheatPlayerCommand,   rbac::RBAC_ROLE_GAMEMASTER,              Console::Yes },
            { "delete",      HandleAntiCheatDeleteCommand,   rbac::RBAC_ROLE_ADMINISTRATOR,           Console::Yes },
            { "purge",       HandleAntiCheatPurgeCommand,    rbac::RBAC_ROLE_ADMINISTRATOR,           Console::Yes },
            { "handle",      HandleAntiCheatHandleCommand,   rbac::RBAC_ROLE_ADMINISTRATOR,           Console::Yes },
            { "jail",        HandleAnticheatJailCommand,     rbac::RBAC_ROLE_GAMEMASTER,              Console::Yes },
            { "parole",      HandleAnticheatParoleCommand,   rbac::RBAC_ROLE_GAMEMASTER,              Console::Yes },
            { "warn",        HandleAnticheatWarnCommand,     rbac::RBAC_ROLE_GAMEMASTER,              Console::Yes }
        };

        static ChatCommandTable commandTable =
        {
            { "anticheat", anticheatCommandTable },
        };

        return commandTable;
    }

    static bool HandleAnticheatWarnCommand(ChatHandler* handler, Optional<PlayerIdentifier> player)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        if (!player)
            player = PlayerIdentifier::FromTarget(handler);
        if (!player || !player->IsConnected())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ChatHandler(player->GetConnectedPlayer()->GetSession()).SendSysMessage("The anticheat system has reported several times that you may be cheating. You will be monitored to confirm if this is accurate.");
        return true;
    }

    static bool HandleAnticheatJailCommand(ChatHandler* handler, Optional<PlayerIdentifier> player)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        if (!player)
            player = PlayerIdentifier::FromTarget(handler);
        if (!player || !player->IsConnected())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* pTarget = player->GetConnectedPlayer();

        // teleport both to jail.
        handler->GetSession()->GetPlayer()->TeleportTo(1, 16226.5f, 16403.6f, -64.5f, 3.2f);

        WorldLocation loc;
        loc = WorldLocation(1, 16226.5f, 16403.6f, -64.5f, 3.2f);// GM Jail Location
        pTarget->TeleportTo(loc);
        pTarget->SetHomebind(loc, 876);// GM Jail Homebind location
        pTarget->CastSpell(pTarget, 38505);// shackle him in place to ensure no exploit happens for jail break attempt

        return true;
    }

    static bool HandleAnticheatParoleCommand(ChatHandler* handler, Optional<PlayerIdentifier> player)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        if (!player)
            player = PlayerIdentifier::FromTarget(handler);
        if (!player || !player->IsConnected())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* pTarget = player->GetConnectedPlayer();

        WorldLocation Aloc;
        WorldLocation Hloc;
        Aloc = WorldLocation(0, -8833.37f, 628.62f, 94.00f, 1.06f);// Stormwind
        Hloc = WorldLocation(1, 1569.59f, -4397.63f, 16.06f, 0.54f);// Orgrimmar

        if (pTarget->GetTeamId() == TEAM_ALLIANCE)
        {
            pTarget->TeleportTo(0, -8833.37f, 628.62f, 94.00f, 1.06f);//Stormwind
            pTarget->SetHomebind(Aloc, 1519);// Stormwind Homebind location
        }
        else
        {
            pTarget->TeleportTo(1, 1569.59f, -4397.63f, 7.7f, 0.54f);//Orgrimmar
            pTarget->SetHomebind(Hloc, 1653);// Orgrimmar Homebind location
        }
        pTarget->RemoveAura(38505);// remove shackles
        sAnticheatMgr->AnticheatDeleteCommand(pTarget->GetGUID());// deletes auto reports on player
        return true;
    }

    static bool HandleAntiCheatDeleteCommand(ChatHandler* /*handler*/, Variant<EXACT_SEQUENCE("deleteall"), PlayerIdentifier> command)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        if (command.holds_alternative<EXACT_SEQUENCE("deleteall")>())
            sAnticheatMgr->AnticheatDeleteCommand(0);
        else
            sAnticheatMgr->AnticheatDeleteCommand(command.get<PlayerIdentifier>().GetGUID().GetCounter());

        return true;
    }

    static bool HandleAntiCheatPurgeCommand(ChatHandler* handler, const char* /* args */)
    {
        sAnticheatMgr->AnticheatPurgeCommand(handler);
        handler->PSendSysMessage("The Anticheat daily_player_reports has been purged.");
        return true;
    }

    static bool HandleAntiCheatPlayerCommand(ChatHandler* handler, Optional<PlayerIdentifier> player)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        if (!player)
            player = PlayerIdentifier::FromTarget(handler);
        if (!player)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 guid = player->GetGUID().GetCounter();

        float average = sAnticheatMgr->GetAverage(guid);
        uint32 total_reports = sAnticheatMgr->GetTotalReports(guid);
        uint32 speed_reports = sAnticheatMgr->GetTypeReports(guid,0);
        uint32 fly_reports = sAnticheatMgr->GetTypeReports(guid,1);
        uint32 jump_reports = sAnticheatMgr->GetTypeReports(guid,3);
        uint32 waterwalk_reports = sAnticheatMgr->GetTypeReports(guid,2);
        uint32 teleportplane_reports = sAnticheatMgr->GetTypeReports(guid,4);
        uint32 climb_reports = sAnticheatMgr->GetTypeReports(guid,5);

        handler->PSendSysMessage("Information about player %s",player->GetName().c_str());
        handler->PSendSysMessage("Average: %f || Total Reports: %u ",average,total_reports);
        handler->PSendSysMessage("Speed Reports: %u || Fly Reports: %u || Jump Reports: %u ",speed_reports,fly_reports,jump_reports);
        handler->PSendSysMessage("Walk On Water Reports: %u  || Teleport To Plane Reports: %u",waterwalk_reports,teleportplane_reports);
        handler->PSendSysMessage("Climb Reports: %u", climb_reports);

        return true;
    }

    static bool HandleAntiCheatHandleCommand(ChatHandler* handler, bool enable)
    {
        if (enable)
        {
            sWorld->setBoolConfig(CONFIG_ANTICHEAT_ENABLE,true);
            handler->SendSysMessage("The Anticheat System is now: Enabled!");
        }
        else
        {
            sWorld->setBoolConfig(CONFIG_ANTICHEAT_ENABLE,false);
            handler->SendSysMessage("The Anticheat System is now: Disabled!");
        }

        return true;
    }

    static bool HandleAntiCheatGlobalCommand(ChatHandler* handler)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
        {
            handler->PSendSysMessage("The Anticheat System is disabled.");
            return true;
        }

        sAnticheatMgr->AnticheatGlobalCommand(handler);

        return true;
    }
};

void AddSC_anticheat_commandscript()
{
    new anticheat_commandscript();
}
