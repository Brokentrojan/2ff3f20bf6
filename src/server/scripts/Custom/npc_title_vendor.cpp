#include "ScriptPCH.h"
#include "ItemEnchantmentMgr.h"
#include "Language.h"

#define H_X             -1258.48f
#define H_Y             63.53f
#define H_Z             127.9f
#define A_X             -1208.98f
#define A_Y             -88.27f
#define A_Z             161.50f
#define TOKEN           20558
#define TOKEN_COUNT     8
#define TITLE_REWARDED  47 // title "The Conqueror"
#define ELEM_COUNT      25 // Amount of enchants showed per page
#define TXT_PREV_PG     "<- Previous page"
#define TXT_GO_BACK     "<-- back to item list"
#define TXT_NEXT_PG     "Next page ->"
#define TXT_LAST_PAGE   "you reach the last page !"
#define TXT_KTHXBY      "Nevermind.."
#define TXT_BAD_SKILL   "Profession NPC: received non-valid skill ID (LearnAllRecipesInProfession)"
#define TXT_ERR_SKILL   "you already have that skill"
#define TXT_ERR_MAX     "you already know two professions!"
#define TXT_PROBLEM     "Internal error occured!"
#define TXT_SPRINT      "Make me the fastest turtle ever!"
#define TXT_GET_TITLE   "Gimi my Title !"
#define TXT_VENDOR_LIST "Show me your goodies..."
#define TXT_ERR_TITLE   "you already have the Title :)"
#define TXT_NO_FLAG     "Gimi Vendor flags plz."
#define FORMAT_END      ":20:20|t "
#define TXT_NEXT_TITLE  "Next title : |TInterface/PvPRankBadges/PvPRank"
#define TXT_REQ_TITLE   "you need to be at least : |TInterface/PvPRankBadges/PvPRank"
#define TXT_MAX_RANK    "you already are max rank !"
#define TXT_WSG_MARK    " Warsong Gulch Mark of Ranking to go! We sell the same items as the Arena Vendors do."

enum Gossip_Option_Custom
{
        CUSTOM_OPTION_NONE = 20,
        CUSTOM_OPTION_UNLEARN = 21,
        CUSTOM_OPTION_EXIT = 22,
        CUSTOM_OPTION_TITLE_PVP = 23,
        CUSTOM_OPTION_TITLE_CLIMB = 24,
        CUSTOM_OPTION_SPRINT = 25,
        GOSSIP_OPTION_HELLO = 26,
        CUSTOM_OPTION_ITEM_MENU = 27,
        CUSTOM_OPTION_ITEM_MENU_P2 = 28,
        CUSTOM_OPTION_ITEM_MENU_P3 = 29,
        CUSTOM_OPTION_ITEM_MENU_P4 = 30,
        CUSTOM_OPTION_ITEM_MENU_P5 = 31,
        CUSTOM_OPTION_ITEM_MENU_P6 = 32,
        CUSTOM_OPTION_ITEM_MENU_P7 = 33,
        CUSTOM_OPTION_ITEM_MENU_MAX = 34,
        CUSTOM_OPTION_SUFFIX = 35,
        CUSTOM_OPTION_PROPERTY = 36,
        CUSTOM_OPTION_VENDOR = 37,
        CUSTOM_OPTION_MAX
};

enum NPCs {
        NPC_ALLIANCE_LOOK_BACK = 500113,
        NPC_ALLIANCE_LOOK_ARROUND = 500114,
        NPC_HORDE_LOOK_BACK = 500115,
        NPC_HORDE_LOOK_ARROUND = 500116
};

static const char   *titlesNames[28] =
{
        "Private",
        "Corporal",
        "Sergeant",
        "Master Sergeant",
        "Sergeant Major",
        "Knight",
        "Knight Lieutenant",
        "Knight Captain",
        "Knight Champion",
        "Lieutenant Commander",
        "Commander",
        "Marshal",
        "Field Marshal",
        "Grand Marshal",
        "Scout",
        "Grunt",
        "Sergeant",
        "Senior Sergeant",
        "First Sergeant",
        "Stone Guard",
        "Blood Guard",
        "Legionnaire",
        "Centurion",
        "Champion",
        "Lieutenant General",
        "General",
        "Warlord",
        "High Warlord"
};


class npc_title_giver : public CreatureScript
{
public:
        npc_title_giver() : CreatureScript("npc_title_giver") {}

        void RewardTitles(Player *player, uint8 *nextTitle, uint16 *reqTokens, const uint16 totalTokens, const uint8 faction)
        {
                while ((*nextTitle < 14) && (((*reqTokens += (1 + *nextTitle) * TOKEN_COUNT)) <= totalTokens))
                {
                        (*nextTitle)++;
                        player->SetTitle(sCharTitlesStore.LookupEntry(*nextTitle + faction));
                }
        }

        const char *GetNextTitleName(const uint8 nextTitle, const uint16 reqTokens, const uint16 totalTokens, const uint8 faction)
        {
                std::ostringstream  ss;

                if (nextTitle > 13)
                        ss << TXT_MAX_RANK;
                else
                {
                        ss << TXT_NEXT_TITLE;
                        if (nextTitle < 9)
                                ss << '0';
                        ss << 1 + nextTitle << FORMAT_END << titlesNames[nextTitle + faction];
                        ss << " in " << reqTokens - totalTokens << " tokens.";
                }
                return (ss.str().c_str());
        }

        bool OnGossipHello(Player* player, Creature* me)
        {
			const uint16  totalTokens = player->GetTotalTokens(player); // lelelelele
                const uint8   faction = (player->GetTeam() == ALLIANCE) ? 0 : 14;
                uint8         nextTitle = 0;
                uint16        reqTokens = 0;

                RewardTitles(player, &nextTitle, &reqTokens, totalTokens, faction);
                const char *gossipText = GetNextTitleName(nextTitle, reqTokens, totalTokens, faction);
                player->PlayerTalkClass->ClearMenus();
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, gossipText, GOSSIP_SENDER_MAIN, CUSTOM_OPTION_EXIT);
                player->ADD_GOSSIP_ITEM(1, TXT_KTHXBY, GOSSIP_SENDER_MAIN, CUSTOM_OPTION_EXIT);
                player->PlayerTalkClass->SendGossipMenu(9425, me->GetGUID());
                return (true);
        }

        bool OnGossipSelect(Player* player, Creature* /*me*/, uint32 /*uiSender*/, uint32 /*uiAction*/)
        {
                player->PlayerTalkClass->SendCloseGossip();
                return (true);
        }
};
void AddSC_title_system()
{
        new npc_title_giver();

}