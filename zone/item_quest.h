#ifndef ITEM_QUEST_H
#define ITEM_QUERT_H

#include <vector>
#include <string>
#include <stdlib.h>


#include "../common/types.h"
#include "../common/string_util.h"

namespace EQEmu
{
	namespace item_quest {
		enum QStat {
			DAMAGE = 1,
			DELAY,
			AC,
			BAGSLOTS,
			FOCUS,
			HASTE,
			HP,
			REGEN,
			MANA,
			MANAREGEN,
			MR,
			PR,
			CR,
			DR,
			FR,
			RANGE,
			SKILLMODTYPE,
			PROCEFFECT,
			WORNEFFECT,
			FOCUSEFFECT,
			ITEMTYPE,
		};

		enum Operation {
			LT = 1,
			GT,
			LTE,
			GTE,
			EQ,
			NEQ
		};

		struct ItemQuestQueryStatLimit {
			QStat  stat;
			uint32 operation;
			uint32 test;
		};

		struct ItemQuestQuery {
			uint32	Slots;
			uint32	Classes;		// Bitfield of classes that can equip item (1 << class#)
			uint32	Races;			// Bitfield of races that can equip item (1 << race#)
			uint32  MinLevel;
			uint32  MaxLevel;
			uint32  MinExpansion;
			uint32  MaxExpansion;
			std::string                          Zone;
			std::string                          NameFilter;
			std::vector<ItemQuestQueryStatLimit> QStats;

			inline void Pack(std::string& ret) const {
				ret.append("s:");
				ret.append(StringFormat("%u",Slots));
				ret.append("|c:");
				ret.append(StringFormat("%u",Classes));
				ret.append("|r:");
				ret.append(StringFormat("%u",Races));
				ret.append("|ml:");
				ret.append(StringFormat("%u",MinLevel));
				ret.append("|xl:");
				ret.append(StringFormat("%u",MaxLevel));
				ret.append("|mx:");
				ret.append(StringFormat("%u",MinExpansion));
				ret.append("|xx:");
				ret.append(StringFormat("%u",MaxExpansion));
				ret.append("|z:");
				ret.append(Zone);
				ret.append("|n:");
				ret.append(NameFilter);
				for (auto it = QStats.begin(); it != QStats.end(); ++it) {
					ret.append(StringFormat("|qs%d:",it->stat));
					ret.append(StringFormat("%u:",it->operation));
					ret.append(StringFormat("%u",it->test));
				}
			}

			void UnPack(const std::string& pack);
		};

		struct ItemQuestResultItem {
			uint32	    ItemID;
			std::string Name;
			FLOAT       Chance; // this is the loot chance from a kill
			FLOAT       SpawnChance; // this is the spawn chance within the normal group.. so full drop chance is SpawnChance*Chance
			uint32      Occurance;
			uint32      MinLevel;
			uint32      MaxLevel;
			uint32      MinZone;
			uint32      MaxZone;
			uint32      MinExpansion;
			uint32      MaxExpansion;
		};


		extern bool Query(ItemQuestQuery &query, std::vector<ItemQuestResultItem> &results);

		struct ItemQuestPickResult {
			uint32      ItemId;
			FLOAT       Chance;
			FLOAT       SpawnChance;
			int32       NpcId;
			uint32      AggroRadius;
			std::string Name;
			int32       Spawn2Id;
			int32       SpawnGroup;
		    FLOAT       X;
		    FLOAT       Y;
		    FLOAT       Z;
		    FLOAT       Heading;
		    std::string Zone;
			int32       ZoneId;

			inline void Pack(std::string &into) const {
				into.append("iid:");
				into.append(StringFormat("%u",ItemId));
				into.append("|");
				into.append("npc:");
				into.append(StringFormat("%u",NpcId));
				into.append("|");
				into.append("pctd:");
				into.append(StringFormat("%.03f",Chance));
				into.append("|");
				into.append("pcts:");
				into.append(StringFormat("%.03f",SpawnChance));
				into.append("|");
				into.append("ar:");
				into.append(StringFormat("%u",AggroRadius));
				into.append("|");
				into.append("nme:");
				into.append(Name);
				into.append("|");
				into.append("s2:");
				into.append(StringFormat("%u",Spawn2Id));
				into.append("|");
				into.append("sg:");
				into.append(StringFormat("%u",SpawnGroup));
				into.append("|");
				into.append("x:");
				into.append(StringFormat("%.03f",X));
				into.append("|");
				into.append("y:");
				into.append(StringFormat("%.03f",Y));
				into.append("|");
				into.append("z:");
				into.append(StringFormat("%.03f",Z));
				into.append("|");
				into.append("h:");
				into.append(StringFormat("%.03f",Heading));
				into.append("|");
				into.append("zn:");
				into.append(Zone);
				into.append("|");
				into.append("zid:");
				into.append(StringFormat("%u",ZoneId));
			}
		};

		extern bool Pick(uint32 item_id, bool exactItem, int32 minlevel, int32 maxlevel, int32 min_exp, int32 max_exp, int32 rnd, const char* zone, ItemQuestPickResult& result);

		extern std::string GetLastPick(uint32 charid);
		extern void        SetLastPick(uint32 charid, const ItemQuestPickResult& pick, const std::string expire = "1h");
		extern long long   GetLastPickRemain(uint32 charid);

		extern std::string GetLastQuery(uint32 charid);
		extern void        SetLastQuery(uint32 charid, const ItemQuestQuery& query, const std::string expire = "1h");
		extern long long   GetLastQueryRemain(uint32 charid);
	}
}

#endif
