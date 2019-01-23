#include "zone.h"
#include "zonedb.h"

#include "../common/string_util.h"
#include "../common/eqemu_logsys.h"

extern Zone* zone;

bool MerchantAddItem(uint32 npcid, uint32 itemid, int32 faction_req, int32 level_req, int32 alt_cur_cost, uint32 classesreq, int32 probability) {
	const char* SQL =
	"insert into merchantlist (merchantid, slot, item, faction_required, level_required, alt_currency_cost, classes_required, probability) \n"
	"select merchant_id \n"
	"      ,(select max(slot+1) from merchantlist where merchantid = merchant_id) \n"
	"      ,%d \n" // item id
	"      ,%d \n" // faction_req
	"      ,%d \n" // level_req
	"      ,%d \n" // alt_cur_cost
	"      ,%d \n" // classes
	"      ,%d \n" // probability
	"from npc_types \n"
	"where id = %d and COALESCE(merchant_id,0) > 0; \n"
	;

	const char* SQL2 = "SELECT merchant_id FROM npc_types WHERE id = %d;";

	std::string sql = StringFormat(SQL, itemid, faction_req, level_req, alt_cur_cost, classesreq, probability, npcid);
	Log(Logs::General, Logs::Debug, "merchant_manager::AddItem %s", sql.c_str());
	auto results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "merchant_manager::AddItem %s", sql.c_str());
		Log(Logs::General, Logs::Error, "merchant_manager::AddItem %s", results.ErrorMessage().c_str());
		return false;
	}

	sql = StringFormat(SQL2, npcid);
	uint32 merchantid = 0;
	results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "merchant_manager::AddItem %s", sql.c_str());
		Log(Logs::General, Logs::Error, "merchant_manager::AddItem %s", results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		merchantid = atoul(row[0]);
	}

	if (merchantid != 0) {
		Log(Logs::General, Logs::Debug, "merchant_manager::AddItem reloaded merchant %d npc_type %d", merchantid, npcid);
		zone->LoadNewMerchantData(merchantid);
	} else {
		Log(Logs::General, Logs::Debug, "merchant_manager::AddItem npc_type %d didn't have a valid merchantid", npcid);
	}
	return true;
}

bool MerchantRemItem(uint32 npcid, uint32 itemid) {
	const char* SQL =
	"delete from merchantlist where merchantid = (select merchant_id from npc_types where id = %d) and item = %d;\n"
	;

	std::string sql = StringFormat(SQL, npcid, itemid, npcid);
	Log(Logs::General, Logs::Debug, "merchant_manager::RemItem %s", sql.c_str());
	auto results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "merchant_manager::RemItem %s", sql.c_str());
		Log(Logs::General, Logs::Error, "merchant_manager::RemItem %s", results.ErrorMessage().c_str());
		return false;
	}

	bool didWork = results.RowsAffected() != 0;

	const char* SQL2 = "SELECT merchant_id FROM npc_types WHERE id = %d;";
	sql = StringFormat(SQL2, npcid);
	results = database.QueryDatabase(sql);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "merchant_manager::RemItem %s", sql.c_str());
		Log(Logs::General, Logs::Error, "merchant_manager::RemItem %s", results.ErrorMessage().c_str());
		return false;
	}


	uint32 merchantid = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		merchantid = atoul(row[0]);
	}

	if (didWork) {
		Log(Logs::General, Logs::Debug, "merchant_manager::RemItem reloaded merchant %d npc_type %d", merchantid, npcid);
		zone->LoadNewMerchantData(merchantid);
	} else {
		Log(Logs::General, Logs::Debug, "merchant_manager::RemItem fialed, merchant %d didn't have item %d ?", merchantid, itemid);
	}

	return true;
}

