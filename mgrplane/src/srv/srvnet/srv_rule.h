#ifndef __SRV_RULE_H__
#define __SRV_RULE_H__

#include <common.h>

extern int Rule_set_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int Rule_del_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int Rule_commit_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int Rule_show_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);

#endif
