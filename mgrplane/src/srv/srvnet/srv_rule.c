#include "srv_rule.h"
#include <message.h>
#include <trans.h>
#include <pow.h>
#include <rpc-common.h>


int Rule_show_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	LOG("Rule_show_acl_rule\n");

	return 0;
}



int Rule_add_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	LOG("Rule_add_acl_rule\n");
	uint8_t s_buf[MAX_BUF];
	cmd_type_t cmd_ack = ADD_ACL_RULE_ACK;

	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;

	RCP_BLOCK_ACL_RULE_TUPLE *blocks = (RCP_BLOCK_ACL_RULE_TUPLE *)(from + MESSAGE_HEADER_LENGTH);

	/*ADD RULE INFO INTO LOCAL MANAGER*/

	printf("sip is %x\n", blocks->sip);

	rcp_param_p->nparam = 1;
	rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_OK;

	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);
	
	return 0;
}


int Rule_del_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	LOG("Rule_del_acl_rule\n");
	
	return 0;
}


int Rule_commit_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	LOG("Rule_commit_acl_rule\n");
	
	return 0;
}




