
#include <vector>

#include "void.h"
#include "zonedb.h"

void Void_NewQuery(VoidQuery &query) {
	query.page = 0;
	query.pagesz = 15;
	query.slots = 0;
	query.classes = 0;
	query.races = 0;
	query.minlevel = 0;
	query.maxlevel = 0xFFF;
	query.minexpansion = 0;
	query.maxexpansion = 0xFFF;
}

void Void_Add(uint32 item_id, int quantity) {
	const char* SQL =
	"insert into void_items (id, cnt) values (%i, %i)\n"
	"on duplicate key update cnt = greatest(0, cnt + values(cnt));"
	;

	std::string sql = StringFormat(SQL, item_id, quantity);

	auto results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "Void::Add: %s", results.ErrorMessage().c_str());
		return;
	}
}

uint32 Void_Count(uint32 item_id) {
	const char* SQL =
	"select cnt "
	"from void_items "
	"WHERE id = %i;\n"
	;

	std::string sql = StringFormat(SQL, item_id);

	auto results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "Void::Count: %s", results.ErrorMessage().c_str());
		return 0;
	}

	uint32 ret = 0;

	for (auto row = results.begin(); row != results.end(); ++row) {
		ret = atoul(row[0]);
	}

	return ret;
}

void Void_Query(VoidQuery &query, std::vector<VoidQueryResult> &found) {
	const char* SQL =
	"select vi.id,    "
	"       itm.name, "
	"       vi.cnt,   "
	"       COALESCE(cd.minlevel,0)     'minlevel',"
	"       COALESCE(cd.maxlevel,0)     'maxlevel',"
	"       COALESCE(cd.minzone,0)      'minzone',"
	"       COALESCE(cd.maxzone,0)      'maxzone',"
	"       COALESCE(cd.minexpansion,0) 'minexpansion',"
	"       COALESCE(cd.maxexpansion,0) 'maxexpansion' \n"
	"from       void_items     vi \n"
	"inner join items          itm on itm.id = vi.id \n"
	"left join  classify_drops cd  on cd.id = itm.id \n"
	"where vi.cnt > 0 \n"
	;

	const char* classCheck   = " and (itm.classes & %i) > 0\n";
	const char* slotCheck    = " and (itm.slots   & %i) > 0\n";
	const char* raceCheck    = " and (itm.races   & %i) > 0\n";
	const char* minExpCheck  = " and (cd.minexpansion is NULL or cd.minexpansion >= %i)\n";
	const char* maxExpCheck  = " and (cd.minexpansion is NULL or cd.minexpansion <= %i)\n";

	const char* nameFilter1  = " and (cd.name LIKE '%";
	const char* nameFilter2  = "%')\n";

	const char* zoneFilter1  = " and (select count(1) FROM item_drop_zone WHERE item_id = itm.id AND zone LIKE '%";
	const char* zoneFilter2  = "%')\n";

	std::string sql = SQL;
	if (query.classes != 0) {
		sql.append(StringFormat(classCheck, query.classes));
	}
	if (query.slots != 0) {
		sql.append(StringFormat(slotCheck,  query.slots));
	}
	if (query.races != 0) {
		sql.append(StringFormat(raceCheck,  query.races));
	}
	if (query.minexpansion != 0) {
		sql.append(StringFormat(minExpCheck,  query.minexpansion));
	}
	if (query.maxexpansion != 0) {
		sql.append(StringFormat(maxExpCheck,  query.maxexpansion));
	}
	if (query.namefilter.size() > 0) {
		auto escSearchString = new char[query.namefilter.size()*2+1];
		database.DoEscapeString(escSearchString, query.namefilter.c_str(), query.namefilter.size());
		sql.append(nameFilter1);
		sql.append(escSearchString);
		sql.append(nameFilter2);
		safe_delete_array(escSearchString);
	}

	for (auto qstat = query.qstats.begin(); qstat != query.qstats.end(); ++qstat) {
		std::string s;
		switch ((*qstat).stat) {
			case EQ::item_quest::QStat::DAMAGE:
				s.append(" and (itm.damage ");
				break;
			case EQ::item_quest::QStat::DELAY:
				s.append(" and (itm.delay ");
				break;
			case EQ::item_quest::QStat::AC:
				s.append(" and (itm.ac ");
				break;
			case EQ::item_quest::QStat::BAGSLOTS:
				s.append(" and (itm.bagslots ");
				break;
			case EQ::item_quest::QStat::FOCUS:
				s.append(" and (itm.focus ");
				break;
			case EQ::item_quest::QStat::HASTE:
				s.append(" and (itm.haste ");
				break;
			case EQ::item_quest::QStat::HP:
				s.append(" and (itm.hp ");
				break;
			case EQ::item_quest::QStat::REGEN:
				s.append(" and (itm.regen ");
				break;
			case EQ::item_quest::QStat::MANA:
				s.append(" and (itm.mana ");
				break;
			case EQ::item_quest::QStat::MANAREGEN:
				s.append(" and (itm.manaregen ");
				break;
			case EQ::item_quest::QStat::MR:
				s.append(" and (itm.mr ");
				break;
			case EQ::item_quest::QStat::PR:
				s.append(" and (itm.pr ");
				break;
			case EQ::item_quest::QStat::CR:
				s.append(" and (itm.cr ");
				break;
			case EQ::item_quest::QStat::DR:
				s.append(" and (itm.dr ");
				break;
			case EQ::item_quest::QStat::FR:
				s.append(" and (itm.fr ");
				break;
			case EQ::item_quest::QStat::RANGE:
				s.append(" and (itm.range ");
				break;
			case EQ::item_quest::QStat::SKILLMODTYPE:
				s.append(" and (itm.skillmodtype ");
				break;
			case EQ::item_quest::QStat::PROCEFFECT:
				s.append(" and (itm.proceffect ");
				break;
			case EQ::item_quest::QStat::WORNEFFECT:
				s.append(" and (itm.worneffect ");
				break;
			case EQ::item_quest::QStat::FOCUSEFFECT:
				s.append(" and (itm.focuseffect ");
				break;
			case EQ::item_quest::QStat::ITEMTYPE:
				s.append(" and (itm.itemtype ");
				break;
		}
		switch ((*qstat).operation) {
			case EQ::item_quest::Operation::LT:
				s.append(" < ");
				break;
			case EQ::item_quest::Operation::GT:
				s.append(" > ");
				break;
			case EQ::item_quest::Operation::LTE:
				s.append(" <= ");
				break;
			case EQ::item_quest::Operation::GTE:
				s.append(" >= ");
				break;
			case EQ::item_quest::Operation::EQ:
				s.append(" = ");
				break;
			case EQ::item_quest::Operation::NEQ:
				s.append(" != ");
				break;
		}
		s.append(StringFormat("%d)",(*qstat).test));
		sql.append(s);
	}
	sql.append(StringFormat("order by itm.name limit %i, %i;\n", query.page * query.pagesz, query.pagesz));

	Log(Logs::General, Logs::Debug, "Void::Query: %s", sql.c_str());
	auto results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "Void::Query: %s", results.ErrorMessage().c_str());
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		VoidQueryResult item;

		auto i = 0;

		item.ItemID       = atoul(row[i++]);
		item.Name.append(row[i++]);
		item.Count        = atof(row[i++]);
		item.MinLevel     = atoul(row[i++]);
		item.MaxLevel     = atoul(row[i++]);
		item.MinZone      = atoul(row[i++]);
		item.MaxZone      = atoul(row[i++]);
		item.MinExpansion = atoul(row[i++]);
		item.MaxExpansion = atoul(row[i++]);

		found.push_back(item);
	}
}






