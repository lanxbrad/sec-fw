#include "common.h"
#include "message.h"

#include <srv_octeon.h>

static struct rcp_msg_params_s rcp_param;




int process_test_command(uint8_t * from, uint32_t length, uint32_t fd)
{
	memset(&rcp_param, 0, sizeof(struct rcp_msg_params_s));
	
	LOG("process_test_command \n");

	octeon_show_test_command(from, length, fd, (void *)&rcp_param);

	return 0;
}




int32_t init_cmd_process_handle(void)
{
	memset(cmd_process_handles, 0, sizeof(struct cmd_process_handle_s) * MAX_COMMAND_TYPE);

	register_cmd_process_handle(TEST_COMMAND, process_test_command);


	return 0;
}



