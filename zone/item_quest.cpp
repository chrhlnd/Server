#include "item_quest.h"
#include "zonedb.h"

#include "../common/string_util.h"
#include "../common/eqemu_logsys.h"
#include "../common/seperator.h"

bool EQEmu::item_quest::Query(ItemQuestQuery &query, std::vector<ItemQuestResultItem> &found) {
	const char* SQL =
	"select cd.id, cd.name, cd.chance, cd.occurance, cd.minlevel, cd.maxlevel, cd.minzone, cd.maxzone, cd.minexpansion, cd.maxexpansion \n"
	"from classify_drops cd \n"
	"inner join items itm on itm.id = cd.id\n"
	"where cd.minlevel >= %i and cd.minlevel <= %i\n"
	;

	const char* classCheck  = " and (itm.classes & %i) > 0\n";
	const char* slotCheck   = " and (itm.slots   & %i) > 0\n";
	const char* raceCheck   = " and (itm.races   & %i) > 0\n";
	const char* minExpCheck = " and (cd.minexpansion >= %i)\n";
	const char* maxExpCheck = " and (cd.minexpansion <= %i)\n";
	const char* nameFilter  = " and (cd.name LIKE '%%%s%%')\n";

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
		sql.append(StringFormat(nameFilter, escSearchString));
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

	//Log(Logs::General, Logs::Debug, "item_quest::Query: %s", sql.c_str());
	auto results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "item_quest::Query: %s", results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		EQEmu::item_quest::ItemQuestResultItem item;
		item.ItemID       = atoul(row[0]);
		item.Name.append(row[1]);
		item.Chance       = atof(row[2]);
		item.Occurance    = atoul(row[3]);
		item.MinLevel     = atoul(row[4]);
		item.MaxLevel     = atoul(row[5]);
		item.MinZone      = atoul(row[6]);
		item.MaxZone      = atoul(row[7]);
		item.MinExpansion = atoul(row[8]);
		item.MaxExpansion = atoul(row[9]);

		found.push_back(item);
	}

	return true;
}

