#include "ReagentBankMgr.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GuildPackets.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <algorithm>

#define REAGENT_TAB_NAME_0        "Reagent Bank"
#define REAGENT_TAB_NAME_6        "Banco de Componentes"
#define REAGENT_TAB_NAME_7        "Banco de Componentes"

#define REAGENT_INVALID_ITEM_0    "You can only store crafting reagents, gems, or recipes in this bank."
#define REAGENT_INVALID_ITEM_6    "Solo puedes guardar materiales de profesi\xC3\xB3n, gemas o recetas en este banco."
#define REAGENT_INVALID_ITEM_7    "Solo puedes guardar materiales de profesi\xC3\xB3n, gemas o recetas en este banco."

#define REAGENT_SOULBOUND_0       "You cannot store soulbound or enchanted items in the Reagent Bank."
#define REAGENT_SOULBOUND_6       "No puedes guardar objetos ligados o encantados en el Banco de Componentes."
#define REAGENT_SOULBOUND_7       "No puedes guardar objetos ligados o encantados en el Banco de Componentes."

void ReagentBankMgr::LoadConfig()
{
    _enabled = sConfigMgr->GetBoolDefault("ReagentBank.Enable", true);
    _gameObjectEntry = sConfigMgr->GetIntDefault("ReagentBank.GameObjectEntry", 250000);
}

void ReagentBankMgr::LoadPlayer(ObjectGuid::LowType guid)
{
    auto& slots = _storage[guid];
    slots.fill(ReagentSlotItem{ 0, 0 });

    QueryResult result = CharacterDatabase.PQuery(
        "SELECT slot, item_entry, count FROM character_reagent_bank WHERE guid = {}", guid);

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint8 slot = fields[0].GetUInt8();
        uint32 entry = fields[1].GetUInt32();
        uint32 count = fields[2].GetUInt32();

        if (slot < REAGENT_BANK_MAX_SLOTS && sObjectMgr->GetItemTemplate(entry))
            slots[slot] = { entry, count };

    } while (result->NextRow());
}

void ReagentBankMgr::SavePlayer(ObjectGuid::LowType guid)
{
    auto it = _storage.find(guid);
    if (it == _storage.end())
        return;

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM character_reagent_bank WHERE guid = {}", guid);

    for (uint8 slot = 0; slot < REAGENT_BANK_MAX_SLOTS; ++slot)
    {
        ReagentSlotItem const& item = it->second[slot];
        if (item.entry > 0 && item.count > 0)
            trans->PAppend("INSERT INTO character_reagent_bank (guid, slot, item_entry, count) VALUES ({}, {}, {}, {})",
                guid, slot, item.entry, item.count);
    }

    CharacterDatabase.CommitTransaction(trans);
}

void ReagentBankMgr::UnloadPlayer(ObjectGuid::LowType guid)
{
    SavePlayer(guid);
    _storage.erase(guid);
}

void ReagentBankMgr::OpenBank(Player* player, ObjectGuid /*bankerGuid*/)
{
    if (!player || !_enabled)
        return;

    _openSessions.insert(player->GetGUID());

    WorldPackets::Guild::GuildPermissionsQueryResults permPacket;
    permPacket.RankID = 0;
    permPacket.WithdrawGoldLimit = 0;
    permPacket.Flags = 0x03;
    permPacket.NumTabs = 1;
        for (uint8 i = 0; i < GUILD_BANK_MAX_TABS; ++i)
            permPacket.Tab[i] = { 0x03, 98 };
    player->SendDirectMessage(permPacket.Write());

    SendBankList(player);
}

void ReagentBankMgr::CloseBank(ObjectGuid guid)
{
    _openSessions.erase(guid);
}

bool ReagentBankMgr::IsBankOpen(ObjectGuid guid) const
{
    return _enabled && (_openSessions.find(guid) != _openSessions.end());
}

void ReagentBankMgr::SendBankList(Player* player)
{
    if (!player)
        return;

    ObjectGuid::LowType guid = player->GetGUID().GetCounter();
    auto const& slots = _storage[guid];
    LocaleConstant loc = player->GetSession()->GetSessionDbcLocale();

    char const* tabName = REAGENT_TAB_NAME_0;
    switch (loc)
    {
        case 6: tabName = REAGENT_TAB_NAME_6; break;
        case 7: tabName = REAGENT_TAB_NAME_7; break;
        default: break;
    }

    WorldPackets::Guild::GuildBankQueryResults packet;
    packet.Money = 0;
    packet.Tab = 0;
    packet.WithdrawalsRemaining = REAGENT_BANK_MAX_SLOTS;
    packet.FullUpdate = true;

    WorldPackets::Guild::GuildBankTabInfo tabInfo;
    tabInfo.Name = tabName;
    tabInfo.Icon = "inv_misc_bag_10_blue";
    packet.TabInfo.push_back(tabInfo);

    for (uint8 slot = 0; slot < REAGENT_BANK_MAX_SLOTS; ++slot)
    {
        ReagentSlotItem const& item = slots[slot];
        if (item.entry == 0 || item.count == 0)
            continue;

        WorldPackets::Guild::GuildBankItemInfo itemInfo;
        itemInfo.Slot = slot;
        itemInfo.ItemID = item.entry;
        itemInfo.Count = item.count;
        packet.ItemInfo.push_back(itemInfo);
    }

    player->SendDirectMessage(packet.Write());
}

void ReagentBankMgr::HandleSwapItems(Player* player, uint8 bankSlot, uint8 playerBag, uint8 playerSlot, uint32 splitedAmount, bool toBank)
{
    if (!player)
        return;

    // Deteccion inteligente: si hay un objeto valido en la mochila (playerBag != NULL_BAG), la intencion es depositar
    Item* bagItemCheck = (playerBag != NULL_BAG) ? player->GetItemByPos(playerBag, playerSlot) : nullptr;
    if (bagItemCheck)
    {
        toBank = true;
        if (bankSlot == NULL_SLOT || bankSlot >= REAGENT_BANK_MAX_SLOTS)
        {
            HandleAutoStore(player, NULL_SLOT, playerBag, playerSlot, splitedAmount);
            return;
        }
    }

    if (bankSlot >= REAGENT_BANK_MAX_SLOTS)
        return;

    LocaleConstant loc = player->GetSession()->GetSessionDbcLocale();
    char const* msgInvalid   = REAGENT_INVALID_ITEM_0;
    char const* msgSoulbound = REAGENT_SOULBOUND_0;

    switch (loc)
    {
        case 6:
            msgInvalid   = REAGENT_INVALID_ITEM_6;
            msgSoulbound = REAGENT_SOULBOUND_6;
            break;
        case 7:
            msgInvalid   = REAGENT_INVALID_ITEM_7;
            msgSoulbound = REAGENT_SOULBOUND_7;
            break;
        default:
            break;
    }

    auto& slots = _storage[player->GetGUID().GetCounter()];
    ReagentSlotItem& bItem = slots[bankSlot];

    if (toBank)
    {
        Item* pItem = player->GetItemByPos(playerBag, playerSlot);
        if (!pItem)
        {
            SendBankList(player);
            return;
        }

        ItemTemplate const* proto = pItem->GetTemplate();
        if (proto->Class != ITEM_CLASS_TRADE_GOODS && proto->Class != ITEM_CLASS_GEM && proto->Class != ITEM_CLASS_RECIPE)
        {
            ChatHandler(player->GetSession()).SendSysMessage(msgInvalid);
            SendBankList(player);
            return;
        }

        if (pItem->IsSoulBound() || pItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) != 0)
        {
            ChatHandler(player->GetSession()).SendSysMessage(msgSoulbound);
            SendBankList(player);
            return;
        }

        uint32 moveCount = (splitedAmount > 0 && splitedAmount < pItem->GetCount()) ? splitedAmount : pItem->GetCount();

        if (bItem.entry == 0)
        {
            bItem.entry = pItem->GetEntry();
            bItem.count = moveCount;
            player->DestroyItemCount(pItem, moveCount, true);
        }
        else if (bItem.entry == pItem->GetEntry())
        {
            uint32 maxStack = proto->GetMaxStackSize();
            uint32 canAdd = (bItem.count < maxStack) ? (maxStack - bItem.count) : 0;
            uint32 actualAdd = std::min(moveCount, canAdd);
            if (actualAdd > 0)
            {
                bItem.count += actualAdd;
                player->DestroyItemCount(pItem, actualAdd, true);
            }
        }
    }
    else
    {
        if (bItem.entry == 0 || bItem.count == 0)
        {
            SendBankList(player);
            return;
        }

        uint32 takeCount = (splitedAmount > 0 && splitedAmount < bItem.count) ? splitedAmount : bItem.count;

        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, bItem.entry, takeCount);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, nullptr, nullptr, bItem.entry);
            SendBankList(player);
            return;
        }

        Item* newItem = player->StoreNewItem(dest, bItem.entry, true);
        if (newItem)
        {
            player->SendNewItem(newItem, takeCount, true, false);
            bItem.count -= takeCount;
            if (bItem.count == 0)
                bItem.entry = 0;
        }
    }

    SendBankList(player);
}

void ReagentBankMgr::HandleAutoStore(Player* player, uint8 bankSlot, uint8 playerBag, uint8 playerSlot, uint32 splitedAmount)
{
    if (!player)
        return;

    auto& slots = _storage[player->GetGUID().GetCounter()];

    // Caso 1: Clic derecho sobre una casilla del Banco de Componentes -> Retirar a la mochila
    if (bankSlot < REAGENT_BANK_MAX_SLOTS && slots[bankSlot].entry > 0 && slots[bankSlot].count > 0 && playerBag == NULL_BAG)
    {
        HandleSwapItems(player, bankSlot, NULL_BAG, NULL_SLOT, splitedAmount, false);
        return;
    }

    // Caso 2: Clic derecho sobre un objeto de la mochila -> Depositar automaticamente en el primer stack/hueco libre
    Item* pItem = player->GetItemByPos(playerBag, playerSlot);
    if (!pItem)
    {
        SendBankList(player);
        return;
    }

    ItemTemplate const* proto = pItem->GetTemplate();
    uint32 maxStack = proto->GetMaxStackSize();

    // Primero intentar apilar en un stack existente incompleto del mismo objeto
    for (uint8 i = 0; i < REAGENT_BANK_MAX_SLOTS; ++i)
    {
        if (slots[i].entry == pItem->GetEntry() && slots[i].count < maxStack)
        {
            HandleSwapItems(player, i, playerBag, playerSlot, splitedAmount, true);
            return;
        }
    }

    // Si no hay stack incompleto, buscar la primera casilla vacia (0..97)
    for (uint8 i = 0; i < REAGENT_BANK_MAX_SLOTS; ++i)
    {
        if (slots[i].entry == 0)
        {
            HandleSwapItems(player, i, playerBag, playerSlot, splitedAmount, true);
            return;
        }
    }

    SendBankList(player);
}

uint32 ReagentBankMgr::GetItemCount(ObjectGuid::LowType guid, uint32 itemEntry) const
{
    if (!_enabled)
        return 0;

    auto it = _storage.find(guid);
    if (it == _storage.end())
        return 0;

    uint32 total = 0;
    for (auto const& slot : it->second)
        if (slot.entry == itemEntry)
            total += slot.count;
    return total;
}

void ReagentBankMgr::DestroyItemCount(ObjectGuid::LowType guid, uint32 itemEntry, uint32 count)
{
    if (!_enabled)
        return;

    auto it = _storage.find(guid);
    if (it == _storage.end() || count == 0)
        return;

    for (auto& slot : it->second)
    {
        if (slot.entry != itemEntry || slot.count == 0)
            continue;

        uint32 take = std::min(slot.count, count);
        slot.count -= take;
        count -= take;
        if (slot.count == 0)
            slot.entry = 0;

        if (count == 0)
            break;
    }
}

class ReagentBank_WorldScript : public WorldScript
{
public:
    ReagentBank_WorldScript() : WorldScript("ReagentBank_WorldScript") { }

    void OnConfigLoad(bool /*reload*/) override
    {
        sReagentBankMgr->LoadConfig();
    }
};

class ReagentBank_PlayerScript : public PlayerScript
{
public:
    ReagentBank_PlayerScript() : PlayerScript("ReagentBank_PlayerScript") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (sReagentBankMgr->IsEnabled())
            sReagentBankMgr->LoadPlayer(player->GetGUID().GetCounter());
    }

    void OnSave(Player* player) override
    {
        if (sReagentBankMgr->IsEnabled())
            sReagentBankMgr->SavePlayer(player->GetGUID().GetCounter());
    }

    void OnLogout(Player* player) override
    {
        sReagentBankMgr->CloseBank(player->GetGUID());
        sReagentBankMgr->UnloadPlayer(player->GetGUID().GetCounter());
    }
};

void AddSC_reagent_bank()
{
    new ReagentBank_WorldScript();
    new ReagentBank_PlayerScript();
}
