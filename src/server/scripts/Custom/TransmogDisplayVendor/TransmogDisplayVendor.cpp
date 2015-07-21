#include "ScriptPCH.h"
#include "TransmogDisplayVendorConf.h"

// Config start

// Edit Transmogrification compatibility in TransmogDisplayVendorConf.h

// A multiplier for the default gold cost (change to 0.0f for no default cost)
const float TransmogDisplayVendorMgr::ScaledCostModifier = 0.0f;
// Cost added on top of other costs (can be negative)
const int32 TransmogDisplayVendorMgr::CopperCost = 0;
// For custom gold cost set ScaledCostModifier to 0.0f and CopperCost to what ever cost you want

const bool TransmogDisplayVendorMgr::RequireToken = false;
const uint32 TransmogDisplayVendorMgr::TokenEntry = 49426;
const uint32 TransmogDisplayVendorMgr::TokenAmount = 1;

const bool TransmogDisplayVendorMgr::AllowPoor = true;
const bool TransmogDisplayVendorMgr::AllowCommon = true;
const bool TransmogDisplayVendorMgr::AllowUncommon = true;
const bool TransmogDisplayVendorMgr::AllowRare = true;
const bool TransmogDisplayVendorMgr::AllowEpic = true;
const bool TransmogDisplayVendorMgr::AllowLegendary = true;
const bool TransmogDisplayVendorMgr::AllowArtifact = true;
const bool TransmogDisplayVendorMgr::AllowHeirloom = true;

const bool TransmogDisplayVendorMgr::AllowMixedArmorTypes = false;
const bool TransmogDisplayVendorMgr::AllowMixedWeaponTypes = false;
const bool TransmogDisplayVendorMgr::AllowFishingPoles = false;

const bool TransmogDisplayVendorMgr::IgnoreReqRace = true;
const bool TransmogDisplayVendorMgr::IgnoreReqClass = false;
const bool TransmogDisplayVendorMgr::IgnoreReqSkill = true;
const bool TransmogDisplayVendorMgr::IgnoreReqSpell = true;
const bool TransmogDisplayVendorMgr::IgnoreReqLevel = true;
const bool TransmogDisplayVendorMgr::IgnoreReqEvent = true;
const bool TransmogDisplayVendorMgr::IgnoreReqStats = true;

// Example AllowedItems[] = { 123, 234, 345 };
static const uint32 AllowedItems[] = { 0 };
static const uint32 NotAllowedItems[] = { 0 };

// Config end

std::set<uint32> TransmogDisplayVendorMgr::Allowed;
std::set<uint32> TransmogDisplayVendorMgr::NotAllowed;

#ifdef BOOST_VERSION
#define USING_BOOST
#endif
#ifdef USING_BOOST
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#endif

namespace
{
	class SelectionStore
	{
	public:
		typedef std::mutex LockType;
		typedef std::unique_lock<LockType> WriteGuard;

		void SetSlot(uint64 plrGuid, uint8 slot)
		{
			WriteGuard guard(lock);
			hashmap[plrGuid] = slot;
		}

		bool GetSlot(uint64 plrGuid, uint8& slot)
		{
			WriteGuard guard(lock);

			auto it = hashmap.find(plrGuid);
			if (it == hashmap.end())
				return false;

			slot = it->second;
			return true;
		}

		void RemoveData(uint64 plrGuid)
		{
			WriteGuard guard(lock);
			hashmap.erase(plrGuid);
		}

	private:
		std::mutex lock;
		std::unordered_map<uint64, uint8> hashmap; // guid to slot
	};
};

// Selection store
static SelectionStore selectionStore; // selectionStore[GUID] = Slot

// Vendor data store
struct ItemData
{
	uint32 entry;
	uint32 tworating;
};
static const std::vector<ItemData> itemList;

uint32 TransmogDisplayVendorMgr::GetFakeEntry(const Item* item)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::GetFakeEntry");

	Player* owner = item->GetOwner();

	if (!owner)
		return 0;
	if (owner->transmogMap.empty())
		return 0;

	TransmogMapType::const_iterator it = owner->transmogMap.find(item->GetGUID());
	if (it == owner->transmogMap.end())
		return 0;
	return it->second;
}
void TransmogDisplayVendorMgr::DeleteFakeEntry(Player* player, Item* item)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::DeleteFakeEntry");

	if (player->transmogMap.erase(item->GetGUID()) != 0)
		UpdateItem(player, item);
}
void TransmogDisplayVendorMgr::SetFakeEntry(Player* player, Item* item, uint32 entry)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::SetFakeEntry");

	player->transmogMap[item->GetGUID()] = entry;
	UpdateItem(player, item);
}
void TransmogDisplayVendorMgr::UpdateItem(Player* player, Item* item)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::UpdateItem");

	if (item->IsEquipped())
	{
		player->SetVisibleItemSlot(item->GetSlot(), item);
		if (player->IsInWorld())
			item->SendUpdateToPlayer(player);
	}
}
const char* TransmogDisplayVendorMgr::getSlotName(uint8 slot, WorldSession* /*session*/)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::TransmogDisplayVendorMgr::getSlotName");

	switch (slot)
	{
	case EQUIPMENT_SLOT_HEAD: return  "Head";// session->GetTrinityString(LANG_SLOT_NAME_HEAD);
	case EQUIPMENT_SLOT_SHOULDERS: return  "Shoulders";// session->GetTrinityString(LANG_SLOT_NAME_SHOULDERS);
	case EQUIPMENT_SLOT_BODY: return  "Shirt";// session->GetTrinityString(LANG_SLOT_NAME_BODY);
	case EQUIPMENT_SLOT_CHEST: return  "Chest";// session->GetTrinityString(LANG_SLOT_NAME_CHEST);
	case EQUIPMENT_SLOT_WAIST: return  "Waist";// session->GetTrinityString(LANG_SLOT_NAME_WAIST);
	case EQUIPMENT_SLOT_LEGS: return  "Legs";// session->GetTrinityString(LANG_SLOT_NAME_LEGS);
	case EQUIPMENT_SLOT_FEET: return  "Feet";// session->GetTrinityString(LANG_SLOT_NAME_FEET);
	case EQUIPMENT_SLOT_WRISTS: return  "Wrists";// session->GetTrinityString(LANG_SLOT_NAME_WRISTS);
	case EQUIPMENT_SLOT_HANDS: return  "Hands";// session->GetTrinityString(LANG_SLOT_NAME_HANDS);
	case EQUIPMENT_SLOT_BACK: return  "Back";// session->GetTrinityString(LANG_SLOT_NAME_BACK);
	case EQUIPMENT_SLOT_MAINHAND: return  "Main hand";// session->GetTrinityString(LANG_SLOT_NAME_MAINHAND);
	case EQUIPMENT_SLOT_OFFHAND: return  "Off hand";// session->GetTrinityString(LANG_SLOT_NAME_OFFHAND);
	case EQUIPMENT_SLOT_RANGED: return  "Ranged";// session->GetTrinityString(LANG_SLOT_NAME_RANGED);
	case EQUIPMENT_SLOT_TABARD: return  "Tabard";// session->GetTrinityString(LANG_SLOT_NAME_TABARD);
	default: return nullptr;
	}
}
uint32 TransmogDisplayVendorMgr::GetSpecialPrice(ItemTemplate const* proto)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::GetSpecialPrice");

	uint32 cost = proto->SellPrice < 10000 ? 10000 : proto->SellPrice;
	return cost;
}
bool TransmogDisplayVendorMgr::CanTransmogrifyItemWithItem(Player* player, ItemTemplate const* target, ItemTemplate const* source)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::CanTransmogrifyItemWithItem");

	if (!target || !source)
		return false;

	if (source->ItemId == target->ItemId)
		return false;

	if (!SuitableForTransmogrification(player, target) || !SuitableForTransmogrification(player, source)) // if (!transmogrified->CanTransmogrify() || !transmogrifier->CanBeTransmogrified())
		return false;

	if (source->InventoryType == INVTYPE_BAG ||
		source->InventoryType == INVTYPE_RELIC ||
		// source->InventoryType == INVTYPE_BODY ||
		source->InventoryType == INVTYPE_FINGER ||
		source->InventoryType == INVTYPE_TRINKET ||
		source->InventoryType == INVTYPE_AMMO ||
		source->InventoryType == INVTYPE_QUIVER)
		return false;

	// TC doesnt check this? Checked by Inventory type check.
	if (source->Class != target->Class)
		return false;

	if (source->SubClass != target->SubClass && !IsRangedWeapon(target->Class, target->SubClass))
	{
		if (source->Class == ITEM_CLASS_ARMOR && !AllowMixedArmorTypes)
			return false;
		if (source->Class == ITEM_CLASS_WEAPON && !AllowMixedWeaponTypes)
			return false;
	}

	if (source->InventoryType != target->InventoryType)
	{
		if (source->Class == ITEM_CLASS_WEAPON &&
			(IsRangedWeapon(target->Class, target->SubClass) != IsRangedWeapon(source->Class, source->SubClass) ||
			source->InventoryType == INVTYPE_WEAPONMAINHAND ||
			source->InventoryType == INVTYPE_WEAPONOFFHAND))
			return false;
		if (source->Class == ITEM_CLASS_ARMOR &&
			!((source->InventoryType == INVTYPE_CHEST && target->InventoryType == INVTYPE_ROBE) ||
			(source->InventoryType == INVTYPE_ROBE && target->InventoryType == INVTYPE_CHEST)))
			return false;
	}

	if (!IgnoreReqClass && (source->AllowableClass & player->getClassMask()) == 0)
		return false;

	return true;
}
bool TransmogDisplayVendorMgr::SuitableForTransmogrification(Player* player, ItemTemplate const* proto)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::SuitableForTransmogrification");

	// ItemTemplate const* proto = item->GetTemplate();
	if (!proto)
		return false;

	if (proto->Class != ITEM_CLASS_ARMOR &&
		proto->Class != ITEM_CLASS_WEAPON)
		return false;

	// Skip all checks for allowed items
	if (IsAllowed(proto->ItemId))
		return true;

	if (IsNotAllowed(proto->ItemId))
		return false;

	if (!AllowFishingPoles && proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
		return false;

	if (!IsAllowedQuality(proto->Quality)) // (proto->Quality == ITEM_QUALITY_LEGENDARY)
		return false;

	if (player)
	{
		//if ((proto->Flags2 & ITEM_FLAGS_EXTRA_HORDE_ONLY) && player->GetTeam() != HORDE)
		//return false;

		//if ((proto->Flags2 & ITEM_FLAGS_EXTRA_ALLIANCE_ONLY) && player->GetTeam() != ALLIANCE)
		//return false;

		//if (!IgnoreReqClass && (proto->AllowableClass & player->getClassMask()) == 0)
		//return false;

		//if (!IgnoreReqRace && (proto->AllowableRace & player->getRaceMask()) == 0)
		//return false;

		if (!IgnoreReqSkill && proto->RequiredSkill != 0)
		{
			if (player->GetSkillValue(proto->RequiredSkill) == 0)
				return false;
			else if (player->GetSkillValue(proto->RequiredSkill) < proto->RequiredSkillRank)
				return false;
		}

		if (!IgnoreReqSpell && proto->RequiredSpell != 0 && !player->HasSpell(proto->RequiredSpell))
			return false;

		if (!IgnoreReqLevel && player->getLevel() < proto->RequiredLevel)
			return false;
	}

	// If World Event is not active, prevent using event dependant items
	if (!IgnoreReqEvent && proto->HolidayId && !IsHolidayActive((HolidayIds)proto->HolidayId))
		return false;

	if (!IgnoreReqStats)
	{
		if (!proto->RandomProperty && !proto->RandomSuffix)
		{
			bool found = false;
			for (uint8 i = 0; i < proto->StatsCount; ++i)
			{
				if (proto->ItemStat[i].ItemStatValue != 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}
	}

	return true;
}

bool TransmogDisplayVendorMgr::IsRangedWeapon(uint32 Class, uint32 SubClass)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::IsRangedWeapon");

	return Class == ITEM_CLASS_WEAPON && (
		SubClass == ITEM_SUBCLASS_WEAPON_BOW ||
		SubClass == ITEM_SUBCLASS_WEAPON_GUN ||
		SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW);
}
bool TransmogDisplayVendorMgr::IsAllowed(uint32 entry)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::IsAllowed");

	return Allowed.find(entry) != Allowed.end();
}
bool TransmogDisplayVendorMgr::IsNotAllowed(uint32 entry)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::IsNotAllowed");

	return NotAllowed.find(entry) != NotAllowed.end();
}
bool TransmogDisplayVendorMgr::IsAllowedQuality(uint32 quality)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::IsAllowedQuality");

	switch (quality)
	{
	case ITEM_QUALITY_POOR: return AllowPoor;
	case ITEM_QUALITY_NORMAL: return AllowCommon;
	case ITEM_QUALITY_UNCOMMON: return AllowUncommon;
	case ITEM_QUALITY_RARE: return AllowRare;
	case ITEM_QUALITY_EPIC: return AllowEpic;
	case ITEM_QUALITY_LEGENDARY: return AllowLegendary;
	case ITEM_QUALITY_ARTIFACT: return AllowArtifact;
	case ITEM_QUALITY_HEIRLOOM: return AllowHeirloom;
	default: return false;
	}
}

static const char* getQualityName(uint32 quality)
{
	switch (quality)
	{
	case ITEM_QUALITY_POOR: return "|CFF9d9d9d[Poor]";
	case ITEM_QUALITY_NORMAL: return "|CFFffffff[Common]";
	case ITEM_QUALITY_UNCOMMON: return "|CFF1eff00[Uncommon]";
	case ITEM_QUALITY_RARE: return "|CFF0070dd[Rare]";
	case ITEM_QUALITY_EPIC: return "|CFFa335ee[Epic]";
	case ITEM_QUALITY_LEGENDARY: return "|CFFff8000[Legendary]";
	case ITEM_QUALITY_ARTIFACT: return "|CFFe6cc80[Artifact]";
	case ITEM_QUALITY_HEIRLOOM: return "|CFFe5cc80[Heirloom]";
	default: return "[Unknown]";
	}
}

static std::string getItemName(const ItemTemplate* itemTemplate, WorldSession* session)
{
	std::string name = itemTemplate->Name1;
	int loc_idx = session->GetSessionDbLocaleIndex();
	if (loc_idx >= 0)
		if (ItemLocale const* il = sObjectMgr->GetItemLocale(itemTemplate->ItemId))
			sObjectMgr->GetLocaleString(il->Name, loc_idx, name);
	return name;
}

static uint32 getCorrectInvType(uint32 inventorytype)
{
	switch (inventorytype)
	{
	case INVTYPE_WEAPONMAINHAND:
	case INVTYPE_WEAPONOFFHAND:
		return INVTYPE_WEAPON;
	case INVTYPE_RANGEDRIGHT:
		return INVTYPE_RANGED;
	case INVTYPE_ROBE:
		return INVTYPE_CHEST;
	default:
		return inventorytype;
	}
}

void TransmogDisplayVendorMgr::HandleTransmogrify(Player* player, Creature* /*creature*/, uint32 vendorslot, uint32 itemEntry, bool no_cost)
{
	TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::HandleTransmogrify");

	uint8 slot;
	if (!selectionStore.GetSlot(player->GetGUID(), slot))
		return; // cheat, no slot selected

	const char* slotname = TransmogDisplayVendorMgr::getSlotName(slot, player->GetSession());
	if (!slotname)
		return;

	// slot of the transmogrified item
	if (slot >= EQUIPMENT_SLOT_END)
	{
		TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::HandleTransmogrify - %s (%s) tried to transmogrify item %u with a wrong slot (%u) when transmogrifying items.", player->GetName().c_str(), player->GetGUID().ToString().c_str(), itemEntry, slot);
		return; // LANG_ERR_TRANSMOG_INVALID_SLOT
	}

	const ItemTemplate* itemTransmogrifier = nullptr;
	// guid of the transmogrifier item, if it's not 0
	if (itemEntry)
	{
		itemTransmogrifier = sObjectMgr->GetItemTemplate(itemEntry);
		if (!itemTransmogrifier)
		{
			TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::HandleTransmogrify - %s (%s) tried to transmogrify with an invalid item entry %u.", player->GetName().c_str(), player->GetGUID().ToString().c_str(), itemEntry);
			return; // LANG_ERR_TRANSMOG_MISSING_SRC_ITEM
		}
	}

	// transmogrified item
	Item* itemTransmogrified = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
	if (!itemTransmogrified)
	{
		TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::HandleTransmogrify - %s (%s) tried to transmogrify an invalid item in a valid slot (slot: %u).", player->GetName().c_str(), player->GetGUID().ToString().c_str(), slot);
		player->GetSession()->SendNotification("No item in %s slot", slotname);
		return; // LANG_ERR_TRANSMOG_MISSING_DEST_ITEM
	}

	if (!itemTransmogrifier) // reset look newEntry
	{
		DeleteFakeEntry(player, itemTransmogrified);
	}
	else
	{
		if (!CanTransmogrifyItemWithItem(player, itemTransmogrified->GetTemplate(), itemTransmogrifier))
		{
			TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::HandleTransmogrify - %s (%s) failed CanTransmogrifyItemWithItem (%u with %u).", player->GetName().c_str(), player->GetGUID().ToString().c_str(), itemTransmogrified->GetEntry(), itemTransmogrifier->ItemId);
			player->GetSession()->SendNotification("Equipped item is not suitable for selected transmogrification");
			return; // LANG_ERR_TRANSMOG_INVALID_ITEMS
		}

		if (uint32 fakeEntry = GetFakeEntry(itemTransmogrified))
		{
			if (const ItemTemplate* fakeItemTemplate = sObjectMgr->GetItemTemplate(fakeEntry))
			{
				if (fakeItemTemplate->DisplayInfoID == itemTransmogrifier->DisplayInfoID)
				{
					return;
				}
			}
		}

		const ItemData* item_data = nullptr;
		for (auto&& data : itemList)
		{
			if (data.entry == itemEntry)
				item_data = &data;
		}

		if (!item_data)
		{
			player->GetSession()->SendNotification("Equipped item is not suitable for selected transmogrification");
			return; // either cheat or changed items (not found in correct place in transmog vendor view)
		}

		auto Q = CharacterDatabase.PQuery("SELECT counter FROM character_achievement_progress WHERE criteria=451 AND guid=%u", player->GetGUIDLow());
		auto W = CharacterDatabase.PQuery("SELECT counter FROM character_achievement_progress WHERE criteria=447 AND guid=%u", player->GetGUIDLow());
		auto E = WorldDatabase.PQuery("SELECT 3v3_rating FROM transmog_vendor_items WHERE entry=%u", item_data->entry);
		uint32 twohighest = 0;
		uint32 threehighest = 0;
		uint32 threerating = 5000;


		if (Q)
		{
			Field* qfield = Q->Fetch();
			twohighest = qfield[0].GetUInt32();
		}

		if (W)
		{
			Field* wfield = W->Fetch();
			threehighest = wfield[0].GetUInt32();
		}

		if (E)
		{
			Field* efield = E->Fetch();
			threerating = efield[0].GetUInt32();
		}

		if (twohighest < item_data->tworating || threehighest < threerating)
		{
			if (item_data->tworating == 0)
				ChatHandler(player->GetSession()).PSendSysMessage("You need to have achieved %u 3v3 rating", threerating);
			else if (threerating == 0)
				ChatHandler(player->GetSession()).PSendSysMessage("You need to have achieved %u 2v2 rating", item_data->tworating);
			else
				ChatHandler(player->GetSession()).PSendSysMessage("You need to have achieved %u 2v2 rating or %u 3v3 rating", item_data->tworating, threerating);
			return; // LANG_ERR_TRANSMOG_NOT_ENOUGH_RATING
		}

		if (!no_cost)
		{
			if (RequireToken)
			{
				if (player->HasItemCount(TokenEntry, TokenAmount))
				{
					player->DestroyItemCount(TokenEntry, TokenAmount, true);
				}
				else
				{
					player->GetSession()->SendNotification("You do not have enough %ss", getItemName(sObjectMgr->GetItemTemplate(TransmogDisplayVendorMgr::TokenEntry), player->GetSession()).c_str());
					return; // LANG_ERR_TRANSMOG_NOT_ENOUGH_TOKENS
				}
			}

			int32 cost = 0;
			cost = GetSpecialPrice(itemTransmogrified->GetTemplate());
			cost *= ScaledCostModifier;
			cost += CopperCost;

			if (cost) // 0 cost if reverting look
			{
				if (cost < 0)
				{
					TC_LOG_DEBUG("custom.transmog", "TransmogDisplayVendorMgr::HandleTransmogrify - %s (%s) transmogrification invalid cost (non negative, amount %i). Transmogrified %u with %u", player->GetName().c_str(), player->GetGUID().ToString().c_str(), -cost, itemTransmogrified->GetEntry(), itemTransmogrifier->ItemId);
				}
				else
				{
					if (!player->HasEnoughMoney(cost))
					{
						player->GetSession()->SendNotification("You do not have enough money");
						return; // LANG_ERR_TRANSMOG_NOT_ENOUGH_MONEY
					}
					player->ModifyMoney(-cost, false);
				}
			}

			SetFakeEntry(player, itemTransmogrified, itemTransmogrifier->ItemId);

			itemTransmogrified->UpdatePlayedTime(player);

			itemTransmogrified->SetOwnerGUID(player->GetGUID());
			itemTransmogrified->SetNotRefundable(player);
			itemTransmogrified->ClearSoulboundTradeable(player);

			//if (itemTransmogrifier->GetTemplate()->Bonding == BIND_WHEN_EQUIPED || itemTransmogrifier->GetTemplate()->Bonding == BIND_WHEN_USE)
			//    itemTransmogrifier->SetBinding(true);

			//itemTransmogrifier->SetOwnerGUID(player->GetGUID());
			//itemTransmogrifier->SetNotRefundable(player);
			//itemTransmogrifier->ClearSoulboundTradeable(player);
		}

		player->PlayDirectSound(3337);
		player->GetSession()->SendAreaTriggerMessage("%s transmogrified", slotname);
		//return LANG_ERR_TRANSMOG_OK;
	}
}

class NPC_TransmogDisplayVendor : public CreatureScript
{
public:
	NPC_TransmogDisplayVendor() : CreatureScript("NPC_TransmogDisplayVendor")
	{
	} // If you change this, also change in Player.cpp: if (creature->GetScriptName() == "NPC_TransmogDisplayVendor")

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		player->PlayerTalkClass->ClearMenus();
		WorldSession* session = player->GetSession();
		for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
		{
			if (slot == 0 || slot == 1 || slot == 10 || slot == 11 || slot == 12 || slot == 13 
				|| slot == 14 || slot == 15 || slot == 16 || slot == 17 || slot == 18)
				continue;

			// if (player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
			if (const char* slotName = TransmogDisplayVendorMgr::getSlotName(slot, session))
				player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, slotName, SENDER_SELECT_VENDOR, slot);
		}
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, "Remove transmogrifications", SENDER_REMOVE_MENU, 0);
		player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
	{
		WorldSession* session = player->GetSession();
		player->PlayerTalkClass->ClearMenus();
		switch (sender)
		{
		case SENDER_SELECT_VENDOR: // action = slot
		{
			Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, action);
			if (!item)
			{
				if (const char* slotname = TransmogDisplayVendorMgr::getSlotName(action, player->GetSession()))
					session->SendNotification("No item equipped in %s slot", slotname);
				OnGossipHello(player, creature);
				return true;
			}

			// items to show in vendor
			std::vector< std::pair<const ItemTemplate*, uint32> > vendorItems;

			ItemTemplate const* itemTemplate = item->GetTemplate();
			for (auto&& data : itemList)
			{
				ItemTemplate const* curtemp = sObjectMgr->GetItemTemplate(data.entry);
				if (!curtemp)
					continue;

				if (!TransmogDisplayVendorMgr::CanTransmogrifyItemWithItem(player, itemTemplate, curtemp))
					continue;

				vendorItems.push_back(std::make_pair(curtemp, data.tworating));
			}

			player->CLOSE_GOSSIP_MENU();

			TC_LOG_DEBUG("network", "WORLD: Sent SMSG_LIST_INVENTORY");

			Creature* vendor = player->GetNPCIfCanInteractWith(creature->GetGUID(), UNIT_NPC_FLAG_VENDOR);
			if (!vendor)
			{
				TC_LOG_DEBUG("network", "WORLD: SendListInventory - Unit (GUID: %u) not found or you can not interact with him.", creature->GetGUIDLow());
				player->SendSellError(SELL_ERR_CANT_FIND_VENDOR, nullptr, ObjectGuid::Empty, 0);
				return true;
			}

			if (player->HasUnitState(UNIT_STATE_DIED))
				player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

			if (vendor->HasUnitState(UNIT_STATE_MOVING))
				vendor->StopMoving();

			uint8 count = 0;

			WorldPacket data(SMSG_LIST_INVENTORY, 8 + 1 + vendorItems.size() * 8 * 4);
			data << uint64(creature->GetGUID());

			size_t countPos = data.wpos();
			data << uint8(count);

			uint32 item_amount = 0;
			for (auto&& vendorItem : vendorItems)
			{
				if (item_amount >= MAX_VENDOR_ITEMS)
				{
					TC_LOG_ERROR("custom.transmog", "transmog_vendor_items has too many items for slot %u, showing only %u", static_cast<uint32>(action), static_cast<uint32>(MAX_VENDOR_ITEMS));
					break;
				}

				auto Q = CharacterDatabase.PQuery("SELECT counter FROM character_achievement_progress WHERE criteria=451 AND guid=%u", player->GetGUIDLow());
				auto W = CharacterDatabase.PQuery("SELECT counter FROM character_achievement_progress WHERE criteria=447 AND guid=%u", player->GetGUIDLow());
				auto E = WorldDatabase.PQuery("SELECT 3v3_rating FROM transmog_vendor_items WHERE entry=%u", vendorItem.first->ItemId);
				uint32 twohighest = 0;
				uint32 threehighest = 0;
				uint32 threerating = 5000;

				if (Q)
				{
					Field* qfield = Q->Fetch();
					twohighest = qfield[0].GetUInt32();
				}

				if (E)
				{
					Field* efield = E->Fetch();
					threerating = efield[0].GetUInt32();
				}


				bool grey = false;
				if (twohighest < vendorItem.second && threehighest < threerating)
					grey = true;

				data << uint32(count + 1);
				data << uint32(vendorItem.first->ItemId);
				data << uint32(vendorItem.first->DisplayInfoID);
				if (!grey)
					data << int32(0xFFFFFFFF);
				else
					data << int32(0);
				data << uint32(0);
				data << uint32(vendorItem.first->MaxDurability);
				data << uint32(vendorItem.first->BuyCount);
				data << uint32(0);
				++item_amount;
			}

			if (!item_amount)
			{
				session->SendAreaTriggerMessage("No transmogrifications found for equipped item");
				OnGossipHello(player, creature);
				return true;
			}
			else
			{
				data.put<uint8>(countPos, item_amount);
				selectionStore.SetSlot(player->GetGUID(), action);
				session->SendPacket(&data);
			}
		} break;
		case SENDER_BACK: // Back
		{
			OnGossipHello(player, creature);
		} break;
		case SENDER_REMOVE_ALL: // Remove TransmogDisplayVendorMgrs
		{
			bool removed = false;
			for (uint8 Slot = EQUIPMENT_SLOT_START; Slot < EQUIPMENT_SLOT_END; Slot++)
			{
				if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, Slot))
				{
					if (!TransmogDisplayVendorMgr::GetFakeEntry(newItem))
						continue;
					TransmogDisplayVendorMgr::DeleteFakeEntry(player, newItem);
					removed = true;
				}
			}
			if (removed)
			{
				session->SendAreaTriggerMessage("Transmogrifications removed from equipped items");
				player->PlayDirectSound(3337);
			}
			else
			{
				session->SendNotification("You have no transmogrified items equipped");
			}
			OnGossipSelect(player, creature, SENDER_REMOVE_MENU, 0);
		} break;
		case SENDER_REMOVE_ONE: // Remove TransmogDisplayVendorMgr from single item
		{
			const char* slotname = TransmogDisplayVendorMgr::getSlotName(action, player->GetSession());
			if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, action))
			{
				if (TransmogDisplayVendorMgr::GetFakeEntry(newItem))
				{
					TransmogDisplayVendorMgr::DeleteFakeEntry(player, newItem);
					if (slotname)
						session->SendAreaTriggerMessage("%s transmogrification removed", slotname);
					player->PlayDirectSound(3337);
				}
				else if (slotname)
				{
					session->SendNotification("No transmogrification on %s slot", slotname);
				}
			}
			else if (slotname)
			{
				session->SendNotification("No item equipped in %s slot", slotname);
			}
			OnGossipSelect(player, creature, SENDER_REMOVE_MENU, 0);
		} break;
		case SENDER_REMOVE_MENU:
		{
			for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
			{
				const char* slotname = TransmogDisplayVendorMgr::getSlotName(slot, player->GetSession());
				if (!slotname)
					continue;
				std::ostringstream ss;
				ss << "Remove transmogrification from " << slotname << "?";
				player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, (std::string)"Remove from " + slotname, SENDER_REMOVE_ONE, slot, ss.str().c_str(), 0, false);
			}
			player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, "Remove all transmogrifications", SENDER_REMOVE_ALL, 0, "Are you sure you want to remove all transmogrifications?", 0, false);
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "Back..", SENDER_BACK, 0);
			player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
		} break;
		}
		return true;
	}
};
class Player_Transmogrify : public PlayerScript
{
public:
	Player_Transmogrify() : PlayerScript("Player_Transmogrify")
	{
	}

	std::vector<ObjectGuid> GetItemList(const Player* player) const
	{
		std::vector<ObjectGuid> itemlist;

		// Copy paste from Player::GetItemByGuid(guid)

		for (uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
			if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
				itemlist.push_back(pItem->GetGUID());

		for (uint8 i = KEYRING_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
			if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
				itemlist.push_back(pItem->GetGUID());

		for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_BAG_END; ++i)
			if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
				itemlist.push_back(pItem->GetGUID());

		for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
			if (Bag* pBag = player->GetBagByPos(i))
				for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
					if (Item* pItem = pBag->GetItemByPos(j))
						itemlist.push_back(pItem->GetGUID());

		for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
			if (Bag* pBag = player->GetBagByPos(i))
				for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
					if (Item* pItem = pBag->GetItemByPos(j))
						itemlist.push_back(pItem->GetGUID());

		return itemlist;
	}

	void OnSave(Player* player) override
	{
		uint32 lowguid = player->GetGUIDLow();
		SQLTransaction trans = CharacterDatabase.BeginTransaction();
		trans->PAppend("DELETE FROM `custom_transmogrification` WHERE `Owner` = %u", lowguid);

		if (!player->transmogMap.empty())
		{
			// Only save items that are in inventory / bank / etc
			std::vector<ObjectGuid> items = GetItemList(player);
			for (std::vector<ObjectGuid>::const_iterator it = items.begin(); it != items.end(); ++it)
			{
				TransmogMapType::const_iterator it2 = player->transmogMap.find(*it);
				if (it2 == player->transmogMap.end())
					continue;

				trans->PAppend("REPLACE INTO custom_transmogrification (GUID, FakeEntry, Owner) VALUES (%u, %u, %u)", it2->first.GetCounter(), it2->second, lowguid);
			}
		}

		if (trans->GetSize()) // basically never false
			CharacterDatabase.CommitTransaction(trans);
	}

	void OnLogin(Player* player, bool /*firstLogin*/) override
	{
		QueryResult result = CharacterDatabase.PQuery("SELECT GUID, FakeEntry FROM custom_transmogrification WHERE Owner = %u", player->GetGUIDLow());

		if (result)
		{
			do
			{
				Field* field = result->Fetch();
				ObjectGuid itemGUID(HIGHGUID_ITEM, 0, field[0].GetUInt32());
				uint32 fakeEntry = field[1].GetUInt32();
				// Only load items that are in inventory / bank / etc
				if (sObjectMgr->GetItemTemplate(fakeEntry) && player->GetItemByGuid(itemGUID))
				{
					player->transmogMap[itemGUID] = fakeEntry;
				}
				else
				{
					// Ignore, will be erased on next save.
					// Additionally this can happen if an item was deleted from DB but still exists for the player
					// TC_LOG_ERROR("custom.transmog", "Item entry (Entry: %u, itemGUID: %u, playerGUID: %u) does not exist, ignoring.", fakeEntry, GUID_LOPART(itemGUID), player->GetGUIDLow());
					// CharacterDatabase.PExecute("DELETE FROM custom_transmogrification WHERE FakeEntry = %u", fakeEntry);
				}
			} while (result->NextRow());

			if (!player->transmogMap.empty())
			{
				for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
				{
					if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
					{
						player->SetVisibleItemSlot(slot, item);
						if (player->IsInWorld())
							item->SendUpdateToPlayer(player);
					}
				}
			}
		}
	}

	void OnLogout(Player* player) override
	{
		selectionStore.RemoveData(player->GetGUID());
	}
};

class PREP_TransmogDisplayVendor : public WorldScript
{
public:
	PREP_TransmogDisplayVendor() : WorldScript("PREP_TransmogDisplayVendor")
	{
	}

	void OnStartup() override
	{
		// perform a const cast on the item list so we can modify it in this function
		// otherwise it should be read only data!
		std::vector<ItemData>& mod_itemList = const_cast<std::vector<ItemData>&>(itemList);

		for (size_t i = 0; i < sizeof(AllowedItems) / sizeof(*AllowedItems); ++i)
			TransmogDisplayVendorMgr::Allowed.insert(AllowedItems[i]);
		for (size_t i = 0; i < sizeof(NotAllowedItems) / sizeof(*NotAllowedItems); ++i)
			TransmogDisplayVendorMgr::NotAllowed.insert(NotAllowedItems[i]);

		TC_LOG_INFO("server.loading", "Creating a list of usable transmogrification entries...");
		// clear for reload
		mod_itemList.clear();

		if (auto Q = WorldDatabase.PQuery("SELECT entry, 2v2_rating FROM transmog_vendor_items ORDER BY rating"))
		{
			do
			{
				ItemData data;
				data.entry = Q->Fetch()[0].GetUInt32();
				data.tworating = Q->Fetch()[1].GetUInt32();
				if (auto itrsecond = sObjectMgr->GetItemTemplate(data.entry))
					mod_itemList.push_back(data);
				else
					TC_LOG_ERROR("custom.transmog", "transmog_vendor_items has a non-existing item entry %u", data.entry);
			} while (Q->NextRow());
		}

		// resize entry list
		mod_itemList.shrink_to_fit();

		TC_LOG_INFO("custom.transmog", "Deleting non-existing transmogrification entries...");
		CharacterDatabase.DirectExecute("DELETE FROM custom_transmogrification WHERE NOT EXISTS (SELECT 1 FROM item_instance WHERE item_instance.guid = custom_transmogrification.GUID)");
	}
};

class transmogcommands : public CommandScript
{
public:
	transmogcommands() : CommandScript("transmogcommands")
	{
	}

	ChatCommand* GetCommands() const
	{
		static ChatCommand transmogCommandTable[] =
		{
			{ "reload", rbac::RBAC_PERM_COMMAND_RELOAD, true, &HandleReloadTransmog, "", NULL },
			{ NULL, 0, false, NULL, "", NULL }
		};
		static ChatCommand commandTable[] =
		{
			{ "transmog", rbac::RBAC_PERM_COMMAND_RELOAD, false, NULL, "", transmogCommandTable },
			{ NULL, 0, false, NULL, "", NULL }
		};
		return commandTable;
	}

	static bool HandleReloadTransmog(ChatHandler* handler, const char* args)
	{

		// perform a const cast on the item list so we can modify it in this function
		// otherwise it should be read only data!
		std::vector<ItemData>& mod_itemList = const_cast<std::vector<ItemData>&>(itemList);

		for (size_t i = 0; i < sizeof(AllowedItems) / sizeof(*AllowedItems); ++i)
			TransmogDisplayVendorMgr::Allowed.insert(AllowedItems[i]);
		for (size_t i = 0; i < sizeof(NotAllowedItems) / sizeof(*NotAllowedItems); ++i)
			TransmogDisplayVendorMgr::NotAllowed.insert(NotAllowedItems[i]);

		TC_LOG_INFO("server.loading", "Creating a list of usable transmogrification entries...");
		// clear for reload
		mod_itemList.clear();

		if (auto Q = WorldDatabase.PQuery("SELECT entry, 2v2_rating FROM transmog_vendor_items ORDER BY rating"))
		{
			do
			{
				ItemData data;
				data.entry = Q->Fetch()[0].GetUInt32();
				data.tworating = Q->Fetch()[1].GetUInt32();
				if (auto itrsecond = sObjectMgr->GetItemTemplate(data.entry))
					mod_itemList.push_back(data);
				else
					TC_LOG_ERROR("custom.transmog", "transmog_vendor_items has a non-existing item entry %u", data.entry);
			} while (Q->NextRow());
		}

		// resize entry list
		mod_itemList.shrink_to_fit();

		TC_LOG_INFO("custom.transmog", "Deleting non-existing transmogrification entries...");
		CharacterDatabase.DirectExecute("DELETE FROM custom_transmogrification WHERE NOT EXISTS (SELECT 1 FROM item_instance WHERE item_instance.guid = custom_transmogrification.GUID)");
		return true;
	}
};

void AddSC_NPC_TransmogDisplayVendor()
{
	new NPC_TransmogDisplayVendor();
	new PREP_TransmogDisplayVendor();
	new Player_Transmogrify();
	new transmogcommands();
}