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
cparser_cmd_show_test_info (cparser_context_t *context)
{
	assert(context);

	int sn;
	cmd_type_t cmd;
	struct rcp_msg_params_s rcp_para;

	memset(send_buf, 0, sizeof(send_buf));
	memset(recv_buf, 0, sizeof(recv_buf));
	cmd = TEST_COMMAND;
	rcp_para.nparam = 0;
	rcp_para.more_flag = 0;
	rcp_para.msg_id = g_msg_id;
	g_msg_id++;
	LOG("cmd=%d\n", cmd);

	cmd_msg_handles[cmd].pack(cmd, &rcp_para, send_buf, &sn);
	LOG("after pack the message\n");

	process_cli_show_cmd(recv_buf, send_buf, sn);


	return CPARSER_OK;
}


