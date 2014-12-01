#include "srv_octeon.h"
#include <message.h>
#include <trans.h>
#include <pow.h>
#include <rpc-common.h>

int octeon_show_test_command(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	int len;
	uint8_t s_buf[MAX_BUF];
	cmd_type_t cmd_ack = TEST_COMMAND_ACK;
	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;
	char *ptr = rcp_param_p->params_list.info_buf + rcp_param_p->info_len;

	len = sprintf(ptr, "show test command.\n");
	ptr += len;
	rcp_param_p->info_len += len;

	
	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);
	LOG("show test command\n");
	return 0;
}


int octeon_show_dp_build_time(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	int ret;
	uint8_t recv_buf[MAX_RECV_LEN];
	uint32_t recv_len;
	uint16_t info_len;
	uint8_t s_buf[MAX_BUF];
	rpc_msg_t rpcmsg;

	cmd_type_t cmd_ack = SHOW_DP_BUILD_TIME_ACK;
	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;
	char *ptr = rcp_param_p->params_list.info_buf + rcp_param_p->info_len;
	
	rpcmsg.opcode = COMMAND_SHOW_BUILD_TIME;
	rpcmsg.info_len = 0;
	
	ret = pow_rpc_syncall2dp(&comm_pow, (void *)&rpcmsg, sizeof(rpc_msg_t), recv_buf, &recv_len);
	if(ret < 0)
	{
		return -1;
	}

	info_len = ((rpc_msg_t *)&recv_buf)->info_len;
	if((info_len + sizeof(rpc_msg_t)) != recv_len)
		return -1;

	memcpy((void *)ptr, (void *)((rpc_msg_t *)&recv_buf)->info_buf, info_len);
	ptr[info_len] = 0;
	ptr += info_len;
	rcp_param_p->info_len += info_len;
	
	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);
	
	LOG("show test command\n");
	
	return 0;
}



