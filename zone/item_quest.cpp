#include <ctime>
#include "item_quest.h"
#include "zonedb.h"

#include "../common/string_util.h"
#include "../common/eqemu_logsys.h"
#include "../common/seperator.h"

#include "data_bucket.h"

static void strsplit(const char* str, char pat, std::vector<std::string>& parts) {
	std::istringstream ss(str);
	std::string s;
	while (std::getline(ss, s, pat)) {
		parts.push_back(s);
	}
	if (str[strlen(str)-1] == pat) {
		parts.push_back("");
	}
}

void EQEmu::item_quest::ItemQuestQuery::UnPack(const std::string& pack) {
	std::vector<std::string> groups;
	std::vector<std::string> parts;

	strsplit(pack.c_str(), '|', groups);
	for (auto it = groups.begin(); it != groups.end(); ++it) {
		parts.clear();
		strsplit((*it).c_str(), ':', parts);
		if (parts.size() < 2) {
			continue;
		}

		auto key = parts[0];
		auto val = parts[1];

		if (key.compare("s") == 0) { Slots        = atoul(val.c_str()); }
		if (key.compare("c") == 0) { Classes      = atoul(val.c_str()); }
		if (key.compare("r") == 0) { Races        = atoul(val.c_str()); }
		if (key.compare("ml") == 0) { MinLevel     = atoul(val.c_str()); }
		if (key.compare("xl") == 0) { MaxLevel     = atoul(val.c_str()); }
		if (key.compare("mx") == 0) { MinExpansion = atoul(val.c_str()); }
		if (key.compare("xx") == 0) { MaxExpansion = atoul(val.c_str()); }
		if (key.compare("z") == 0) { Zone         = val; }
		if (key.compare("n") == 0) { NameFilter   = val; }
		if (key.find("qs") == 0 && parts.size() > 2) {
			auto val2 = parts[2];

			ItemQuestQueryStatLimit qs;
			qs.stat      = (QStat)atoi(key.substr(2, key.size()-2).c_str());
			qs.operation = atoul(val.c_str());
			qs.test      = atoul(val2.c_str());
			QStats.push_back(qs);
		}
	}
}


static std::string GetPickKey(uint32 charid) {
	return StringFormat("character-%u:iq-last-p", charid);
}

static std::string GetQueryKey(uint32 charid) {
	return StringFormat("character-%u:iq-last-q", charid);
}

std::string EQEmu::item_quest::GetLastPick(uint32 charid) {
	return DataBucket::GetData(GetPickKey(charid));
}

void EQEmu::item_quest::SetLastPick(uint32 charid, const ItemQuestPickResult& result, const std::string expire) {
	std::string picked;
	result.Pack(picked);
 	DataBucket::SetData(GetPickKey(charid),picked,expire);
}

long long EQEmu::item_quest::GetLastPickRemain(uint32 charid) {
	std::string expires = DataBucket::GetDataExpires(GetPickKey(charid));

	if (expires.size() == 0) {
		return 0;
	}

	long long expiresAt = atoll(expires.c_str());
	long long now = std::time(nullptr);

	return expiresAt - now;
}

std::string EQEmu::item_quest::GetLastQuery(uint32 charid) {
	return DataBucket::GetData(GetQueryKey(charid));
}

void EQEmu::item_quest::SetLastQuery(uint32 charid, const ItemQuestQuery& query, const std::string expire) {
	std::string packed;
	query.Pack(packed);
 	DataBucket::SetData(GetQueryKey(charid),packed,expire);
}

long long EQEmu::item_quest::GetLastQueryRemain(uint32 charid) {
	std::string expires = DataBucket::GetDataExpires(GetQueryKey(charid));

	if (expires.size() == 0) {
		return 0;
	}

	long long expiresAt = atoll(expires.c_str());
	long long now = std::time(nullptr);

	return expiresAt - now;
}

bool EQEmu::item_quest::Pick(uint32 item_id, bool exact, int32 min_level, int32 max_level, int32 min_exp, int32 max_exp, int32 rnd, const char* zone, ItemQuestPickResult& result) {
	const char* SQL =
	"select \n"
	"npc.id, cd.chance, cd.spawn_chance, npc.aggroradius, npc.name, sp2.id, sp2.spawngroupID, sp2.x, sp2.y, sp2.z, sp2.heading, sp2.zone, zone.zoneidnumber \n"
	"from lootdrop_entries le \n"
	"inner join loottable_entries lte on lte.lootdrop_id  = le.lootdrop_id \n"
	"inner join npc_types npc         on npc.loottable_id = lte.loottable_id \n"
	"inner join spawnentry se         on se.npcID         = npc.id \n"
	"inner join spawn2 sp2            on sp2.spawngroupID = se.spawngroupID \n"
	"inner join zone                  on zone.short_name  = sp2.zone \n"
	"inner join classify_drops cd     on cd.id            = le.item_id \n"
	"where \n"
	"  le.item_id = %d \n"
	"  %s " // zone filter
	"  %s " // npc filter
	"  order by rand(%d) \n"
	"  limit 1 \n"
	"  ;"
	;
	//"  and npc.level >= %d and npc.level <= %d \n"

	const char* npcLevel         = "and npc.level >= %d and npc.level <= %d \n";

	const char* zoneFilterExact  = "and sp2.zone = '%s'\n";
	const char* zoneFilterMinMax = "and zone.`expansion` >= %d and zone.`expansion` <= %d\n";

	std::string zoneFilter;
	if (zone != NULL) {
		zoneFilter = StringFormat(zoneFilterExact, zone);
	} else {
		zoneFilter = StringFormat(zoneFilterMinMax, min_exp, max_exp);
	}

	std::string levelFilter;
	if (!exact) {
		levelFilter = StringFormat(npcLevel, min_level, max_level);
	}

	std::string sql = StringFormat(SQL, item_id, zoneFilter.c_str(), levelFilter.c_str(), rnd);
	Log(Logs::General, Logs::Debug, "item_quest::Pick: %s", sql.c_str());
	auto results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "item_quest::Pick: %s", sql.c_str());
		Log(Logs::General, Logs::Error, "item_quest::Pick: %s", results.ErrorMessage().c_str());
		return false;
	}

	result.Name.clear();
	result.Zone.clear();
	result.ItemId = 0;

	auto idx = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		result.ItemId      = item_id;
		result.NpcId       = atoi(row[idx++]);
		result.Chance      = atof(row[idx++]);
		result.SpawnChance = atof(row[idx++]);
		result.AggroRadius = atoul(row[idx++]);
		result.Name.append(row[idx++]);
		result.Spawn2Id    = atoi(row[idx++]);
		result.SpawnGroup  = atoi(row[idx++]);
		result.X           = atof(row[idx++]);
		result.Y           = atof(row[idx++]);
		result.Z           = atof(row[idx++]);
		result.Heading     = atof(row[idx++]);
		result.Zone.append(row[idx++]);
		result.ZoneId      = atoi(row[idx++]);
	}

	if (result.ItemId == 0) {
		return false;
	}

	return true;
}

bool EQEmu::item_quest::Query(ItemQuestQuery &query, std::vector<ItemQuestResultItem> &found) {
	const char* SQL =
	"select cd.id, cd.name, cd.chance, cd.spawn_chance, cd.occurance, cd.minlevel, cd.maxlevel, cd.minzone, cd.maxzone, cd.minexpansion, cd.maxexpansion \n"
	"from classify_drops cd \n"
	"inner join items itm on itm.id = cd.id\n"
	"where cd.minlevel >= %i and cd.minlevel <= %i\n"
	;

	const char* classCheck  = " and (itm.classes & %i) > 0\n";
	const char* slotCheck   = " and (itm.slots   & %i) > 0\n";
	const char* raceCheck   = " and (itm.races   & %i) > 0\n";
	const char* minExpCheck = " and (cd.minexpansion >= %i)\n";
	const char* maxExpCheck = " and (cd.minexpansion <= %i)\n";
	const char* nameFilter1  = " and (cd.name LIKE '%";
	const char* nameFilter2 = "%')\n";
	const char* zoneFilter1  = " and (select count(1) FROM item_drop_zone WHERE item_id = itm.id AND zone LIKE '%";
	const char* zoneFilter2  = "%')\n";

	std::string sql = StringFormat(SQL, query.MinLevel, query.MaxLevel);
	if (query.Classes != 0) {
		sql.append(StringFormat(classCheck, query.Classes));
	}
	if (query.Slots != 0) {
		sql.append(StringFormat(slotCheck,  query.Slots));
	}
	if (query.Races != 0) {
		sql.append(StringFormat(raceCheck,  query.Races));
	}
	if (query.MinExpansion != 0) {
		sql.append(StringFormat(minExpCheck,  query.MinExpansion));
	}
	if (query.MaxExpansion != 0) {
		sql.append(StringFormat(maxExpCheck,  query.MaxExpansion));
	}
	if (query.NameFilter.size() > 0) {
		auto escSearchString = new char[query.NameFilter.size()*2+1];
		database.DoEscapeString(escSearchString, query.NameFilter.c_str(), query.NameFilter.size());
		sql.append(nameFilter1);
		sql.append(escSearchString);
		sql.append(nameFilter2);
		safe_delete_array(escSearchString);
	}
	if (query.Zone.size() > 0) {
		auto escSearchString = new char[query.Zone.size()*2+1];
		database.DoEscapeString(escSearchString, query.Zone.c_str(), query.Zone.size());
		sql.append(zoneFilter1);
		sql.append(escSearchString);
		sql.append(zoneFilter2);
		safe_delete_array(escSearchString);
	}

	for (auto qstat = query.QStats.begin(); qstat != query.QStats.end(); ++qstat) {
		std::string s;
		switch ((*qstat).stat) {
			case EQEmu::item_quest::QStat::DAMAGE:
				s.append(" and (itm.damage ");
				break;
			case EQEmu::item_quest::QStat::DELAY:
				s.append(" and (itm.delay ");
				break;
			case EQEmu::item_quest::QStat::AC:
				s.append(" and (itm.ac ");
				break;
			case EQEmu::item_quest::QStat::BAGSLOTS:
				s.append(" and (itm.bagslots ");
				break;
			case EQEmu::item_quest::QStat::FOCUS:
				s.append(" and (itm.focus ");
				break;
			case EQEmu::item_quest::QStat::HASTE:
				s.append(" and (itm.haste ");
				break;
			case EQEmu::item_quest::QStat::HP:
				s.append(" and (itm.hp ");
				break;
			case EQEmu::item_quest::QStat::REGEN:
				s.append(" and (itm.regen ");
				break;
			case EQEmu::item_quest::QStat::MANA:
				s.append(" and (itm.mana ");
				break;
			case EQEmu::item_quest::QStat::MANAREGEN:
				s.append(" and (itm.manaregen ");
				break;
			case EQEmu::item_quest::QStat::MR:
				s.append(" and (itm.mr ");
				break;
			case EQEmu::item_quest::QStat::PR:
				s.append(" and (itm.pr ");
				break;
			case EQEmu::item_quest::QStat::CR:
				s.append(" and (itm.cr ");
				break;
			case EQEmu::item_quest::QStat::DR:
				s.append(" and (itm.dr ");
				break;
			case EQEmu::item_quest::QStat::FR:
				s.append(" and (itm.fr ");
				break;
			case EQEmu::item_quest::QStat::RANGE:
				s.append(" and (itm.range ");
				break;
			case EQEmu::item_quest::QStat::SKILLMODTYPE:
				s.append(" and (itm.skillmodtype ");
				break;
			case EQEmu::item_quest::QStat::PROCEFFECT:
				s.append(" and (itm.proceffect ");
				break;
			case EQEmu::item_quest::QStat::WORNEFFECT:
				s.append(" and (itm.worneffect ");
				break;
			case EQEmu::item_quest::QStat::FOCUSEFFECT:
				s.append(" and (itm.focuseffect ");
				break;
			case EQEmu::item_quest::QStat::ITEMTYPE:
				s.append(" and (itm.itemtype ");
				break;
		}
		switch ((*qstat).operation) {
			case EQEmu::item_quest::Operation::LT:
				s.append(" < ");
				break;
			case EQEmu::item_quest::Operation::GT:
				s.append(" > ");
				break;
			case EQEmu::item_quest::Operation::LTE:
				s.append(" <= ");
				break;
			case EQEmu::item_quest::Operation::GTE:
				s.append(" >= ");
				break;
			case EQEmu::item_quest::Operation::EQ:
				s.append(" = ");
				break;
			case EQEmu::item_quest::Operation::NEQ:
				s.append(" != ");
				break;
		}
		s.append(StringFormat("%d)",(*qstat).test));
		sql.append(s);
	}

	Log(Logs::General, Logs::Debug, "item_quest::Query: %s", sql.c_str());
	auto results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "item_quest::Query: %s", results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		EQEmu::item_quest::ItemQuestResultItem item;

		auto i = 0;

		item.ItemID       = atoul(row[i++]);
		item.Name.append(row[i++]);
		item.Chance       = atof(row[i++]);
		item.SpawnChance  = atof(row[i++]);
		item.Occurance    = atoul(row[i++]);
		item.MinLevel     = atoul(row[i++]);
		item.MaxLevel     = atoul(row[i++]);
		item.MinZone      = atoul(row[i++]);
		item.MaxZone      = atoul(row[i++]);
		item.MinExpansion = atoul(row[i++]);
		item.MaxExpansion = atoul(row[i++]);

		found.push_back(item);
	}

	return true;
}

