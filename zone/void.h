#ifndef _VOID_INC
#define _VOID_INC

#include <string>

#include "item_quest.h"

struct VoidQueryResult {
	uint32	    ItemID;
	std::string Name;
	uint32      Count;
	uint32      MinLevel;
	uint32      MaxLevel;
	uint32      MinZone;
	uint32      MaxZone;
	uint32      MinExpansion;
	uint32      MaxExpansion;
};

struct VoidQuery {
	uint32      page;
	uint32      pagesz;
	uint32	    slots;
	uint32	    classes;		// bitfield of classes that can equip item (1 << class#)
	uint32    	races;			// bitfield of races that can equip item (1 << race#)
	uint32      minlevel;
	uint32      maxlevel;
	uint32      minexpansion;
	uint32      maxexpansion;
	std::string namefilter;
	std::vector<EQ::item_quest::ItemQuestQueryStatLimit> qstats;
};

extern void   Void_NewQuery(VoidQuery &query);
extern void   Void_Query(VoidQuery &query, std::vector<VoidQueryResult> &found);
extern uint32 Void_Count(uint32 item_id);
extern void   Void_Add(uint32 item_id, int quantity);

#endif
