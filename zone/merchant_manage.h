#ifndef _MERCHANT_MANAGE_H
#define _MERCHANT_MANAGE_H

extern bool MerchantAddItem(uint32 npcid, uint32 itemid, int32 faction_req, int32 level_req, int32 alt_cost,  uint32 classesreq, int32 probability);
extern bool MerchantRemItem(uint32 npcid, uint32 itemid);

#endif
