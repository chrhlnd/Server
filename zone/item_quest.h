#ifndef ITEM_QUEST_H
#define ITEM_QUERT_H

#include <vector>
#include <string>


#include "../common/types.h"

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
			std::string                          NameFilter;
			std::vector<ItemQuestQueryStatLimit> QStats;
		};

		struct ItemQuestResultItem {
			uint32	    ItemID;
			std::string Name;
			FLOAT       Chance;
			uint32      Occurance;
			uint32      MinLevel;
			uint32      MaxLevel;
			uint32      MinZone;
			uint32      MaxZone;
			uint32      MinExpansion;
			uint32      MaxExpansion;
		};

		extern bool Query(ItemQuestQuery &query, std::vector<ItemQuestResultItem> &results);
	}
}

#endif
