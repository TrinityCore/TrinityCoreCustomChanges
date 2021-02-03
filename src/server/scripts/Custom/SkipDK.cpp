/* Skip Death Knight Module
   Original Module From Single Player Project Consolidated Skip Module
   Rewritten for TC 335 By Single Player Project Developer MDic
   Original Concept from conanhun513
*/

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "SharedDefines.h"
#include "World.h"

class Trinitycore_skip_deathknight : public PlayerScript
{
public:
    Trinitycore_skip_deathknight() : PlayerScript("Trinitycore_skip_deathknight") { }

    void OnLogin(Player* Player, bool /*firstLogin*/) override
    {
        if (sConfigMgr->GetBoolDefault("Skip.Deathknight.Starter.Announce.enable", true))
        {
            ChatHandler(Player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Trinitycore Skip Deathknight Starter |rmodule.");
        }
    }

    void OnLogin(Player* Player, bool firstLogin) override
    {
        if (sConfigMgr->GetBoolDefault("Skip.Deathknight.Starter.Enable", true))
        {
            if (Player->GetAreaId() == 4342)
            {
                if (!firstLogin)
                    return;
                Player->SetLevel(58);
                Player->LearnSpell(53428, false);//runeforging
                Player->LearnSpell(53441, false);//runeforging
                Player->LearnSpell(53344, false);//runeforging
                Player->LearnSpell(62158, false);//runeforging
                Player->LearnSpell(33391, false);//journeyman riding
                Player->LearnSpell(54586, false);//runeforging credit
                Player->LearnSpell(48778, false);//acherus deathcharger
                Player->LearnSkillRewardedSpells(776, 375);//Runeforging
                Player->LearnSkillRewardedSpells(960, 375);//Runeforging
                Player->EquipNewItem(EQUIPMENT_SLOT_HEAD, 38661, true);//Greathelm of the Scourge Champion
                Player->EquipNewItem(EQUIPMENT_SLOT_WRISTS, 38666, true);//Plated Saronite Bracers
                Player->EquipNewItem(EQUIPMENT_SLOT_WAIST, 38668, true);//The Plaguebringer's Girdle
                Player->EquipNewItem(EQUIPMENT_SLOT_HANDS, 38667, true);//Bloodbane's Gauntlets of Command
                Player->EquipNewItem(EQUIPMENT_SLOT_CHEST, 38665, true);//Saronite War Plate
                Player->EquipNewItem(EQUIPMENT_SLOT_LEGS, 38669, true);//Engraved Saronite Legplates
                Player->EquipNewItem(EQUIPMENT_SLOT_SHOULDERS, 38663, true);// Blood-Soaked Saronite Plated Spaulders
                Player->EquipNewItem(EQUIPMENT_SLOT_FEET, 38670, true);//Greaves of the Slaughter
                Player->EquipNewItem(EQUIPMENT_SLOT_TRINKET1, 38675, true);//Signet of the Dark Brotherhood
                Player->EquipNewItem(EQUIPMENT_SLOT_TRINKET2, 38674, true);//Soul Harvester's Charm
                Player->EquipNewItem(EQUIPMENT_SLOT_FINGER1, 38671, true);//Valanar's Signet Ring
                Player->EquipNewItem(EQUIPMENT_SLOT_FINGER2, 38672, true);// Keleseth's Signet Ring
                Player->AddItem(39320, true);//Sky Darkener's Shroud of Blood
                Player->AddItem(38664, true);//Sky Darkener's Shroud of the Unholy
                Player->AddItem(39322, true);//Shroud of the North Wind
                Player->AddItem(38632, true);//Greatsword of the Ebon Blade
                Player->AddItem(6948, true);//Hearthstone
                Player->AddItem(38707, true);//Runed Soulblade
                Player->AddItem(40483, true);//Insignia of the Scourge

                if (Player->GetQuestStatus(12657) == QUEST_STATUS_NONE)//The Might Of The Scourge
                {
                    Player->AddQuest(sObjectMgr->GetQuestTemplate(12657), nullptr);
                    Player->RewardQuest(sObjectMgr->GetQuestTemplate(12657), false, Player);
                }
                if (Player->GetQuestStatus(12801) == QUEST_STATUS_NONE)//The Light of Dawn
                {
                    Player->AddQuest(sObjectMgr->GetQuestTemplate(12801), nullptr);
                    Player->RewardQuest(sObjectMgr->GetQuestTemplate(12801), false, Player);
                }
                if (Player->GetTeam() == ALLIANCE && Player->GetQuestStatus(13188) == QUEST_STATUS_NONE)//Where Kings Walk
                    Player->AddQuest(sObjectMgr->GetQuestTemplate(13188), nullptr);
                else if (Player->GetTeam() == HORDE && Player->GetQuestStatus(13189) == QUEST_STATUS_NONE)//Saurfang's Blessing
                    Player->AddQuest(sObjectMgr->GetQuestTemplate(13189), nullptr);
                if (Player->GetTeam() == ALLIANCE)
                    Player->TeleportTo(0, -8833.37f, 628.62f, 94.00f, 1.06f);//Stormwind
                else
                    Player->TeleportTo(1, 1569.59f, -4397.63f, 16.06f, 0.54f);//Orgrimmar
                ObjectAccessor::SaveAllPlayers();//Save
            }
        }
    }
};

void AddSC_skip_StarterArea()
{
    new Trinitycore_skip_deathknight;
}
