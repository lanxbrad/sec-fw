#include "common.h"
#include "message.h"

#include <srv_octeon.h>
#include <srv_rule.h>

static struct rcp_msg_params_s rcp_param;




int process_test_command(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));
	
	LOG("process_test_command \n");

	octeon_show_test_command(from, length, fd, (void *)&rcp_param);

	return 0;
}


int process_show_dp_build_time(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));
	LOG("process_show_dp_build_time \n");

	octeon_show_dp_build_time(from, length, fd, (void *)&rcp_param);

	return 0;
}

int process_show_dp_pkt_stat(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));
	LOG("process_show_dp_pkt_stat \n");

	octeon_show_dp_pkt_stat(from, length, fd, (void *)&rcp_param);

	return 0;
}

int process_show_mem_pool(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));

	LOG("process_show_mem_pool \n");

	octeon_show_mem_pool(from, length, fd, (void *)&rcp_param);

	return 0;
}

int process_show_acl_rule(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));
	LOG("process_show_acl_rule \n");

	Rule_show_acl_rule(from, length, fd, (void *)&rcp_param);
	
	return 0;
}

int process_add_acl_rule(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));

	LOG("process_add_acl_rule \n");

	Rule_add_acl_rule(from, length, fd, (void *)&rcp_param);

	return 0;
}

int process_del_acl_rule(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));

	LOG("process_del_acl_rule \n");

	Rule_del_acl_rule(from, length, fd, (void *)&rcp_param);

	return 0;
}

int process_del_acl_rule_id(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));

	LOG("process_del_acl_rule_id \n");

	Rule_del_acl_rule_id(from, length, fd, (void *)&rcp_param);

	return 0;
}

int process_del_acl_rule_all(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));

	LOG("process_del_acl_rule_all \n");

	Rule_del_acl_rule_all(from, length, fd, (void *)&rcp_param);

	return 0;
}


int process_commit_acl_rule(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));

	LOG("process_commit_acl_rule \n");

	Rule_commit_acl_rule(from, length, fd, (void *)&rcp_param);

	return 0;
}





int32_t init_cmd_process_handle(void)
{
	memset(cmd_process_handles, 0, sizeof(struct cmd_process_handle_s) * MAX_COMMAND_TYPE);

	register_cmd_process_handle(TEST_COMMAND, process_test_command);
	register_cmd_process_handle(SHOW_DP_BUILD_TIME, process_show_dp_build_time);
	register_cmd_process_handle(SHOW_DP_PKT_STAT, process_show_dp_pkt_stat);
	register_cmd_process_handle(SHOW_MEM_POOL, process_show_mem_pool);

	register_cmd_process_handle(SHOW_ACL_RULE, process_show_acl_rule);
	register_cmd_process_handle(ADD_ACL_RULE, process_add_acl_rule);
	register_cmd_process_handle(DEL_ACL_RULE, process_del_acl_rule);
	register_cmd_process_handle(DEL_ACL_RULE_ID, process_del_acl_rule_id);
	register_cmd_process_handle(DEL_ACL_RULE_ALL, process_del_acl_rule_all);
	register_cmd_process_handle(COMMIT_ACL_RULE, process_commit_acl_rule);
	
	


	return 0;
}



