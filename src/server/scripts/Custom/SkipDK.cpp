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
*
* Skip Death Knight Module
* Original Module From Single Player Project Consolidated Skip Module
* Rewritten for TC 335 By Single Player Project Developer MDic
* Original Concept from conanhun513
* Assistance and Review by JinnaiX
*/

#include "AccountMgr.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Common.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SharedDefines.h"
#include "World.h"
#include "WorldSession.h"

constexpr auto OPTIONSKIPDK = 0;
constexpr auto YESSKIPDK = 1;
constexpr auto NOSKIPDK = 2;

class Trinitycore_skip_deathknight_announce : public PlayerScript
{
public:
    Trinitycore_skip_deathknight_announce() : PlayerScript("Trinitycore_skip_deathknight_announce") { }

    void OnLogin(Player* Player, bool /*firstLogin*/) override
    {
        if (sConfigMgr->GetBoolDefault("Skip.Deathknight.Starter.Announce.enable", true))
        {
            ChatHandler(Player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Trinitycore Skip Deathknight Starter |rmodule.");
        }
    }
};

class Trinitycore_skip_deathknight : public PlayerScript
{
public:
    Trinitycore_skip_deathknight() : PlayerScript("Trinitycore_skip_deathknight") { }

    void OnLogin(Player* player, bool firstLogin) override
    {
        if (firstLogin && player->GetAreaId() == 4342)
        {
            //These changes make it so user mistakes in the configuration file don't cause this to run 2x
            if ((sConfigMgr->GetBoolDefault("Skip.Deathknight.Starter.Enable", true) && player->GetSession()->GetSecurity() == SEC_PLAYER)
                || (sConfigMgr->GetBoolDefault("GM.Skip.Deathknight.Starter.Enable", true) && player->GetSession()->GetSecurity() >= SEC_MODERATOR))
            {
                Trinitycore_skip_deathknight_HandleSkip(player);
            }
        }
    }
};

#define LOCALE_LICHKING_0 "I wish to skip the Death Knight starter questline."
#define LOCALE_LICHKING_1 "죽음의 기사 스타터 퀘스트 라인을 건너뛰고 싶습니다."
#define LOCALE_LICHKING_2 "Je souhaite sauter la série de quêtes de démarrage du Chevalier de la mort."
#define LOCALE_LICHKING_3 "Ich möchte die Todesritter-Starter-Questreihe überspringen."
#define LOCALE_LICHKING_4 "我想跳過死亡騎士新手任務線。"
#define LOCALE_LICHKING_5 "我想跳過死亡騎士新手任務線。"
#define LOCALE_LICHKING_6 "Deseo saltarme la línea de misiones de inicio del Caballero de la Muerte."
#define LOCALE_LICHKING_7 "Deseo saltarme la línea de misiones de inicio del Caballero de la Muerte."
#define LOCALE_LICHKING_8 "Я хочу пропустить начальную цепочку заданий Рыцаря Смерти."

class Trinitycore_optional_deathknight_skip : public CreatureScript
{
public:
    Trinitycore_optional_deathknight_skip() : CreatureScript("npc_tc_skip_lich") { }

    struct npc_SkipLichAI : public ScriptedAI
    {
        npc_SkipLichAI(Creature* creature) : ScriptedAI(creature) { }

        bool OnGossipHello(Player* player) override
        {
            if (me->IsQuestGiver())
            {
                player->PrepareQuestMenu(me->GetGUID());
            }

            if (sConfigMgr->GetBoolDefault("Skip.Deathknight.Optional.Enable", true))
            {
                char const* localizedEntry;
                switch (player->GetSession()->GetSessionDbcLocale())
                {
                    case LOCALE_koKR: localizedEntry = LOCALE_LICHKING_1; break;
                    case LOCALE_frFR: localizedEntry = LOCALE_LICHKING_2; break;
                    case LOCALE_deDE: localizedEntry = LOCALE_LICHKING_3; break;
                    case LOCALE_zhCN: localizedEntry = LOCALE_LICHKING_4; break;
                    case LOCALE_zhTW: localizedEntry = LOCALE_LICHKING_5; break;
                    case LOCALE_esES: localizedEntry = LOCALE_LICHKING_6; break;
                    case LOCALE_esMX: localizedEntry = LOCALE_LICHKING_7; break;
                    case LOCALE_ruRU: localizedEntry = LOCALE_LICHKING_8; break;
                    case LOCALE_enUS: localizedEntry = LOCALE_LICHKING_0; break;
                    default: localizedEntry = LOCALE_LICHKING_0;
                }
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, localizedEntry, GOSSIP_SENDER_MAIN, OPTIONSKIPDK);
            }

            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);

            switch (action)
            {
                case OPTIONSKIPDK:
                    //Would love for this to become a confirm popup, but not sure how to do yet
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Yes", GOSSIP_SENDER_MAIN, YESSKIPDK);
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "No", GOSSIP_SENDER_MAIN, NOSKIPDK);
                    SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                    break;
                case YESSKIPDK:
                    Trinitycore_skip_deathknight_HandleSkip(player);
                    CloseGossipMenuFor(player);
                    break;
                case NOSKIPDK:
                    CloseGossipMenuFor(player);
                    break;
            }
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_SkipLichAI(creature);
    }
};

void Trinitycore_skip_deathknight_HandleSkip(Player* player)
{
    //Not sure where DKs were supposed to pick this up from, leaving as the one manual add
    player->AddItem(6948, true); //Hearthstone

    // these are all the starter quests that award talent points, quest items, or spells
    int STARTER_QUESTS[39] = { 12593, 12619, 12842, 12848, 12636, 12641, 12657, 12678, 12679, 12680, 12687, 12698, 12701, 12706, 12716, 12719, 12720, 12722, 12724, 12725, 12727, 12733, 12739, 12742, 12743, 12744, 12745, 12746, 12747, 12748, 12749, 12750, 12751, 12754, 12755, 12756, 12757, 12779, 12801 };

    for (auto questId : STARTER_QUESTS)
    {
        if (player->GetQuestStatus(questId) == QUEST_STATUS_NONE)
        {
            player->AddQuest(sObjectMgr->GetQuestTemplate(questId), nullptr);
            player->RewardQuest(sObjectMgr->GetQuestTemplate(questId), false, player);
        }
    }

    int DKL = sConfigMgr->GetFloatDefault("Skip.Deathknight.Start.Level", 58);
    if (player->GetLevel() <= DKL)
    {
        //GiveLevel updates character properties more thoroughly than SetLevel
        player->GiveLevel(DKL);
    }

    //Don't need to save all players, just current
    player->SaveToDB();

    if (player->GetTeam() == ALLIANCE)
    {
        player->TeleportTo(0, -8833.37f, 628.62f, 94.00f, 1.06f);//Stormwind
    }
    else
    {
        player->TeleportTo(1, 1569.59f, -4397.63f, 16.06f, 0.54f);//Orgrimmar
    }
}

void AddSC_skip_StarterArea()
{
    new Trinitycore_skip_deathknight_announce;
    new Trinitycore_skip_deathknight;
    new Trinitycore_optional_deathknight_skip;
}
