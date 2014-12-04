#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "cparser.h"
#include "cparser_token.h"


#include "common.h"
#include "message.h"
#include "cli_trans.h"


cparser_result_t
cparser_cmd_commit_rule_all(cparser_context_t *context)
{
	assert(context);

	int sn,rn;
	cmd_type_t cmd;
	CLI_RESULT *blocks;
	int rv = 0;
	struct rcp_msg_params_s rcp_para;
	memset(&rcp_para, 0, sizeof(struct rcp_msg_params_s));

	memset(send_buf, 0, sizeof(send_buf));
	memset(recv_buf, 0, sizeof(recv_buf));
	cmd = COMMIT_ACL_RULE;
	rcp_para.nparam = 0;
	rcp_para.more_flag = 0;
	rcp_para.msg_id = g_msg_id;
	g_msg_id++;
	LOG("cmd=%d\n", cmd);

	cmd_msg_handles[cmd].pack(cmd, &rcp_para, send_buf, &sn);
	LOG("after pack the message\n");

	process_message(sn, &rn);
	if (rn <= 0) {
		LOG("%s error\n", __FUNCTION__);
	}

	blocks = (CLI_RESULT *) (recv_buf + MESSAGE_HEADER_LENGTH);
	rv = blocks[0].result_code;
	sec_error_print(rv, NULL);


	return CPARSER_OK;
}


cparser_result_t
cparser_cmd_delete_rule(cparser_context_t *context)
{
	return CPARSER_OK;
}


cparser_result_t
cparser_cmd_delete_rule_all(cparser_context_t *context)
{
	return CPARSER_OK;
}



cparser_result_t
cparser_cmd_show_rule(cparser_context_t *context)
{
	return CPARSER_OK;
}


cparser_result_t
cparser_cmd_add_rule_smac_smac_dmac_dmac_sip_sip_mask_sip_mask_dip_dip_mask_dip_mask_sport_start_sport_start_sport_end_sport_end_dport_start_dport_start_dport_end_dport_end_proto_start_proto_start_proto_end_proto_end_action
		(cparser_context_t *context, 
			cparser_macaddr_t *smac,
			cparser_macaddr_t *dmac,
			uint32_t *sip,
			uint32_t *sip_mask,
			uint32_t *dip,
			uint32_t *dip_mask,
			uint32_t *sport_start,
			uint32_t *sport_end,
			uint32_t *dport_start,
			uint32_t *dport_end,
			uint32_t *proto_start,
			uint32_t *proto_end,
			char **action)
{
	assert(context);

	int sn, rn;
	cmd_type_t cmd;
	struct rcp_msg_params_s rcp_para;
	CLI_RESULT *blocks;
	int rv = 0;
	
	memset(&rcp_para, 0, sizeof(struct rcp_msg_params_s));

	memset(send_buf, 0, sizeof(send_buf));
	memset(recv_buf, 0, sizeof(recv_buf));
	cmd = ADD_ACL_RULE;
	
	rcp_para.nparam = 1;
	rcp_para.more_flag = 0;
	rcp_para.msg_id = g_msg_id;
	g_msg_id++;
	LOG("cmd=%d\n", cmd);

	memcpy(rcp_para.params_list.params[0].AclRuleTuple.smac, smac, 6);
	memcpy(rcp_para.params_list.params[0].AclRuleTuple.dmac, dmac, 6);

	if( 0 == *sip && 0 != *sip_mask)
	{
		printf("sip mask invalid\n");
		return CPARSER_NOT_OK;
	}
	if( 0 != *sip && *sip_mask > 32)
	{
		printf("sip mask invalid\n");
		return CPARSER_NOT_OK;
	}
		
	rcp_para.params_list.params[0].AclRuleTuple.sip = *sip;
	rcp_para.params_list.params[0].AclRuleTuple.sip_mask = *sip_mask;

	if( 0 == *dip && 0 != *dip_mask)
	{
		printf("dip mask invalid\n");
		return CPARSER_NOT_OK;
	}
	if( 0 != *dip && *dip_mask > 32)
	{
		printf("dip mask invalid\n");
		return CPARSER_NOT_OK;
	}
	
	rcp_para.params_list.params[0].AclRuleTuple.dip = *dip;
	rcp_para.params_list.params[0].AclRuleTuple.dip_mask = *dip_mask;	


	if( *sport_start > *sport_end || *sport_end > 0xffff )
	{
		printf("sport invalid\n");
		return CPARSER_NOT_OK;
	}

	rcp_para.params_list.params[0].AclRuleTuple.sport_start = *sport_start;
	rcp_para.params_list.params[0].AclRuleTuple.sport_end = *sport_end;
	
	if( *dport_start > *dport_end || *dport_end > 0xffff )
	{
		printf("dport invalid\n");
		return CPARSER_NOT_OK;
	}

	rcp_para.params_list.params[0].AclRuleTuple.dport_start = *dport_start;
	rcp_para.params_list.params[0].AclRuleTuple.dport_end = *dport_end;
	
	if( *proto_start > *proto_end || *proto_end > 0xff )
	{
		printf("dport invalid\n");
		return CPARSER_NOT_OK;
	}

	rcp_para.params_list.params[0].AclRuleTuple.protocol_start = *proto_start;
	rcp_para.params_list.params[0].AclRuleTuple.protocol_end = *proto_end;

	if (!strcmp(*action, "fw")) {
        LOG("action is fw\n");
		rcp_para.params_list.params[0].AclRuleTuple.action = ACL_RULE_ACTION_FW;
    }else if (!strcmp(*action, "drop")) {
        LOG("action is drop\n");
		rcp_para.params_list.params[0].AclRuleTuple.action = ACL_RULE_ACTION_DROP;	
    }

	cmd_msg_handles[cmd].pack(cmd, &rcp_para, send_buf, &sn);
	LOG("after pack the message\n");

	process_message(sn, &rn);
	if (rn <= 0) {
		LOG("%s error\n", __FUNCTION__);
	}

	blocks = (CLI_RESULT *) (recv_buf + MESSAGE_HEADER_LENGTH);
	rv = blocks[0].result_code;
	sec_error_print(rv, NULL);

	return rv;
	
}




