#ifndef __ACL_RULE_H__
#define __ACL_RULE_H__

#include <rpc-common.h>

#define RULE_ENTRY_MAX 100000

#define RULE_ENTRY_STATUS_FREE 0
#define RULE_ENTRY_STATUS_USED 1


#define RULE_BUILD_UNCOMMIT  0
#define RULE_BUILD_COMMIT    1

typedef struct{
	uint8_t entry_status;
	RCP_BLOCK_ACL_RULE_TUPLE rule_tuple;
}rule_entry_t;



typedef struct 
{
	int rule_entry_free;
	int build_status;
	rule_entry_t rule_entry[RULE_ENTRY_MAX]; 
}rule_list_t;



#endif