#include "srv_octeon.h"
#include <message.h>
#include <trans.h>

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
