#ifndef REAGENT_BANK_MGR_H
#define REAGENT_BANK_MGR_H

#include "Common.h"
#include "Define.h"
#include "ObjectGuid.h"
#include <unordered_map>
#include <unordered_set>
#include <array>

constexpr uint8 REAGENT_BANK_MAX_SLOTS = 98;

struct ReagentSlotItem
{
    uint32 entry = 0;
    uint32 count = 0;
};

class Player;

class ReagentBankMgr
{
public:
    static ReagentBankMgr* instance()
    {
        static ReagentBankMgr instance;
        return &instance;
    }

    void LoadConfig();
    bool IsEnabled() const { return _enabled; }
    uint32 GetGameObjectEntry() const { return _gameObjectEntry; }

    void LoadPlayer(ObjectGuid::LowType guid);
    void SavePlayer(ObjectGuid::LowType guid);
    void UnloadPlayer(ObjectGuid::LowType guid);

    void OpenBank(Player* player, ObjectGuid bankerGuid);
    void CloseBank(ObjectGuid guid);
    bool IsBankOpen(ObjectGuid guid) const;

    void SendBankList(Player* player);
    void HandleSwapItems(Player* player, uint8 bankSlot, uint8 playerBag, uint8 playerSlot, uint32 splitedAmount, bool toBank);
    void HandleAutoStore(Player* player, uint8 bankSlot, uint8 playerBag, uint8 playerSlot, uint32 splitedAmount);

    uint32 GetItemCount(ObjectGuid::LowType guid, uint32 itemEntry) const;
    void DestroyItemCount(ObjectGuid::LowType guid, uint32 itemEntry, uint32 count);

private:
    char const* GetTabName(LocaleConstant locale) const;
    char const* GetInvalidItemMsg(LocaleConstant locale) const;
    char const* GetSoulboundItemMsg(LocaleConstant locale) const;

    bool _enabled = true;
    uint32 _gameObjectEntry = 250000;

    std::unordered_map<ObjectGuid::LowType, std::array<ReagentSlotItem, REAGENT_BANK_MAX_SLOTS>> _storage;
    std::unordered_set<ObjectGuid> _openSessions;
};

#define sReagentBankMgr ReagentBankMgr::instance()

#endif
