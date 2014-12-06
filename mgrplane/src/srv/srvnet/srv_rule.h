#ifndef __SRV_RULE_H__
#define __SRV_RULE_H__

#include <common.h>
#include <message.h>


#define RULE_ENTRY_MAX 100000

#define RULE_ENTRY_STATUS_FREE 0
#define RULE_ENTRY_STATUS_USED 1


#define RULE_BUILD_UNCOMMIT  0
#define RULE_BUILD_COMMIT    1


#define RULE_OK         0
#define RULE_FULL       1
#define RULE_EXIST      2
#define RULE_NOT_EXIST  3

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







extern int Rule_list_init();
extern int Rule_add_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int Rule_del_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int Rule_commit_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int Rule_show_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int Rule_del_acl_rule_all(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int Rule_del_acl_rule_id(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);



#endif
