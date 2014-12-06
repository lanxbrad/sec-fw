#include "message.h"


int debugprint = 1;


struct msg_header_info_s cmd_msg_headers[MAX_COMMAND_TYPE + 1];
struct msg_pack_handle_s cmd_msg_handles[MAX_COMMAND_TYPE + 1];
struct cmd_process_handle_s cmd_process_handles[MAX_COMMAND_TYPE + 1];

/*
  *	get data block length
  */
static int __get_data_block_length(int block_type, int *block_length_p)
{
	switch (block_type) {
	case BLOCK_TYPE_START:
		*block_length_p = 0;
		break;
	case BLOCK_IPV4_FIVE_TUPLE:
		*block_length_p = sizeof(RCP_BLOCK_IPV4_FIVE_TUPLE);
		break;
	case BLOCK_ACL_RULE_TUPLE:
		*block_length_p = sizeof(RCP_BLOCK_ACL_RULE_TUPLE);
		break;
	case BLOCK_RESULT_CODE:
		*block_length_p = sizeof(CLI_RESULT);
		break;
	case BLOCK_ACL_RULE_ID:
		*block_length_p = sizeof(RCP_BLOCK_ACL_RULE_ID);
		break;
	default: 
		*block_length_p = 0;
	}

	if(*block_length_p > 0)
		return 0;
	else
		return 1;
}


static int __cmd_is_valid(cmd_type_t cmd)
{
	if (cmd < MAX_COMMAND_TYPE)
		return 0;

	else {
		LOG("cmd=%d is invalid\n", cmd);
		return 1;
	}
}

/*
  *	parse the message header and get the cmd type
  */
int get_cmd_type(cmd_type_t * cmd_p, MESSAGE_HEAD * header_p)
{
	uint32_t i;
	int found = 0;

	for (i = 0; i < sizeof(cmd_msg_headers) / sizeof(cmd_msg_headers[0]); i++) {
		if (header_p->flag == cmd_msg_headers[i].flag &&
			header_p->msg_type == cmd_msg_headers[i].msg_type &&
			header_p->msg_code == cmd_msg_headers[i].msg_code &&
			header_p->blocktype == cmd_msg_headers[i].msg_block_type) {

			*cmd_p = i;

			found = 1;

			LOG("i=%d,msg_type=0x%x,msg_code=0x%x,blocktype=0x%x\n", i, header_p->msg_type, header_p->msg_code, header_p->blocktype);

			break;

		}
	}
	if (found == 1)
		return 0;

	else
		return 1;
}


/*
  *	Function:	fill the message header
  */
int encap_msg_header(cmd_type_t cmd, struct rcp_msg_params_s *param_p, MESSAGE_HEAD * header_p)
{
	int block_length;
	header_p->flag = cmd_msg_headers[cmd].flag;
	header_p->msg_type = cmd_msg_headers[cmd].msg_type;

	header_p->msg_code = cmd_msg_headers[cmd].msg_code;

	header_p->blocktype = cmd_msg_headers[cmd].msg_block_type;

	header_p->msg_id = param_p->msg_id;
	header_p->more_flag = param_p->more_flag;
	header_p->data_block_num = param_p->nparam;
	__get_data_block_length(header_p->blocktype, &block_length);

	/* number of 4 bytes */
	header_p->length = ((header_p->data_block_num) * block_length + sizeof(MESSAGE_HEAD)) >> 2;
	return 0;
}

int pack_acl_rule_id(cmd_type_t cmd, void *para_p, void *sbuf, int *len_p)
{
	int rv;
	char *ptr = (char *)sbuf;
	int block_index;
	int block_length;
	MESSAGE_HEAD msg_header;
	struct rcp_msg_params_s *lpara_p = (struct rcp_msg_params_s *)para_p;

	/* make sure the cmd type is valid */
	rv = __cmd_is_valid(cmd);
	if (rv)
		return rv;

	/* fill the message header */
	encap_msg_header(cmd, lpara_p, &msg_header);

	/* copy the header to sbuf */
	memcpy(ptr, &msg_header, MESSAGE_HEADER_LENGTH);
	*len_p = MESSAGE_HEADER_LENGTH;
	ptr += MESSAGE_HEADER_LENGTH;
	
	LOG("cmd=%d, nparam=%d\n", cmd, lpara_p->nparam);

	/* copy the data block to sbuf */
	block_length = sizeof(RCP_BLOCK_ACL_RULE_ID);
	for (block_index = 0; block_index < lpara_p->nparam; block_index++) {
		memcpy(ptr, &(lpara_p->params_list.params[block_index].AclRuleId), block_length);
		ptr += block_length;
		*len_p += block_length;
	}
	
	return 0;
}

int pack_acl_rule(cmd_type_t cmd, void *para_p, void *sbuf, int *len_p)
{
	int rv;
	char *ptr = (char *)sbuf;
	int block_index;
	int block_length;
	MESSAGE_HEAD msg_header;
	struct rcp_msg_params_s *lpara_p = (struct rcp_msg_params_s *)para_p;

	/* make sure the cmd type is valid */
	rv = __cmd_is_valid(cmd);
	if (rv)
		return rv;

	/* fill the message header */
	encap_msg_header(cmd, lpara_p, &msg_header);

	/* copy the header to sbuf */
	memcpy(ptr, &msg_header, MESSAGE_HEADER_LENGTH);
	*len_p = MESSAGE_HEADER_LENGTH;
	ptr += MESSAGE_HEADER_LENGTH;
	
	LOG("cmd=%d, nparam=%d\n", cmd, lpara_p->nparam);

	/* copy the data block to sbuf */
	block_length = sizeof(RCP_BLOCK_ACL_RULE_TUPLE);
	for (block_index = 0; block_index < lpara_p->nparam; block_index++) {
		memcpy(ptr, &(lpara_p->params_list.params[block_index].Ipv4FiveTuple), block_length);
		ptr += block_length;
		*len_p += block_length;
	}
	
	return 0;
}

int pack_result_code(cmd_type_t cmd, void *para_p, void *sbuf, int *len_p)
{
	int rv;
	int block_index;
	int block_length;
	char *ptr = (char *)sbuf;
	MESSAGE_HEAD msg_header;
	
	struct rcp_msg_params_s *lpara_p = (struct rcp_msg_params_s *)para_p;
	rv = __cmd_is_valid(cmd);
	if (rv)
		return rv;
	
	encap_msg_header(cmd, lpara_p, &msg_header);
	memcpy(ptr, &msg_header, MESSAGE_HEADER_LENGTH);
	*len_p = MESSAGE_HEADER_LENGTH;
	ptr += MESSAGE_HEADER_LENGTH;
	block_length = sizeof(RCP_BLOCK_RESULT);
	for (block_index = 0; block_index < lpara_p->nparam; block_index++) {
		memcpy(ptr, &(lpara_p->params_list.params[block_index].ResultCode), block_length);
		ptr += block_length;
		*len_p += block_length;
	}
	return 0;
}


int pack_null(cmd_type_t cmd, void *para_p, void *sbuf, int *len_p)
{
	int rv;
	char *ptr = (char *)sbuf;
	MESSAGE_HEAD msg_header;
	struct rcp_msg_params_s *lpara_p = (struct rcp_msg_params_s *)para_p;

	/* make sure the cmd type is valid */
	rv = __cmd_is_valid(cmd);
	if (rv)
		return rv;

	/* fill the message header */
	encap_msg_header(cmd, lpara_p, &msg_header);

	/* copy the header to sbuf */
	memcpy(ptr, &msg_header, MESSAGE_HEADER_LENGTH);
	*len_p = MESSAGE_HEADER_LENGTH;
	ptr += MESSAGE_HEADER_LENGTH;
	return 0;
}



int pack_show_info(cmd_type_t cmd, void *para_p, void *sbuf, int *len_p)
{
	int rv;
	char *ptr = (char *)sbuf;
	MESSAGE_HEAD msg_header;
	struct rcp_msg_params_s *lpara_p = (struct rcp_msg_params_s *)para_p;
	rv = __cmd_is_valid(cmd);
	if (rv)
		return rv;
	
	encap_msg_header(cmd, lpara_p, &msg_header);
	
	msg_header.length = (uint8_t) ((MESSAGE_HEADER_LENGTH + lpara_p->info_len) >> 2);
	memcpy(ptr, &msg_header, MESSAGE_HEADER_LENGTH);
	*len_p = MESSAGE_HEADER_LENGTH;
	ptr += MESSAGE_HEADER_LENGTH;
	LOG("cmd=%d, info_len=%d\n", cmd, lpara_p->info_len);
	memcpy(ptr, lpara_p->params_list.info_buf, lpara_p->info_len);
	ptr += lpara_p->info_len;
	*len_p += lpara_p->info_len;
	return 0;
}

/*
  *	Function:		genarate the packet from parameters
  */
int param_to_pkt(cmd_type_t cmd, void *from, uint8_t *sbuf, int *sn_p, void *param_p)
{
	int rv;	
	MESSAGE_HEAD *msg_header = (MESSAGE_HEAD *) from;
	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;

	/* first check the cmd */
	rv = __cmd_is_valid(cmd);
	if (rv) {
		LOG("cmd=%d is not valid\n", cmd);
		return rv;
	}

	/* make sure the msg id of responese is the same as the request */
	rcp_param_p->msg_id = msg_header->msg_id;

	/* then pack the rcp header and rcp data block */
	cmd_msg_handles[cmd].pack(cmd, param_p, sbuf, sn_p);
	return 0;
}





int mgmt_process_cmd(uint8_t * from, uint32_t length, uint32_t fd)
{
	int rv;
	cmd_type_t cmd = -1;
	MESSAGE_HEAD *msg_header = (MESSAGE_HEAD *) from;
	get_cmd_type(&cmd, msg_header);
	LOG("cmd=%d\n", cmd);
	rv = __cmd_is_valid(cmd);
	if (rv)
		return rv;
	
	if (cmd_process_handles[cmd].handle == NULL) {
		LOG("Error:cmd=%d has not register process handle\n", cmd);
		return 1;
	}

	/* process the command */
	cmd_process_handles[cmd].handle(from, length, fd);
	return 0;
}


/*
  *	register the command msg pack handle
  */
int register_msg_pack_handle(cmd_type_t cmd, msg_pack_handle_t pack_handle)
{
	int rv;

	/* make sure the cmd type is valid */
	rv = __cmd_is_valid(cmd);
	if (rv)
		return rv;
	cmd_msg_handles[cmd].cmd = cmd;
	cmd_msg_handles[cmd].pack = pack_handle;
	return 0;
}


/*
  *	register the command process handle
  */
int register_cmd_process_handle(cmd_type_t cmd, cmd_proc_handle_t cmd_handle)
{
	int rv;

	/* make sure the cmd type is valid */
	rv = __cmd_is_valid(cmd);
	if(rv)
		return rv;
	
	cmd_process_handles[cmd].cmd = cmd;
	cmd_process_handles[cmd].handle = cmd_handle;
	return 0;
}

/*
  *	Initialize the msg pack handles
  */
int init_msg_pack_handle(void)
{
	memset(cmd_msg_handles, 0, sizeof(struct msg_pack_handle_s) * (MAX_COMMAND_TYPE + 1));
	
	register_msg_pack_handle(TEST_COMMAND, pack_null);
	register_msg_pack_handle(TEST_COMMAND_ACK, pack_show_info);

	register_msg_pack_handle(SHOW_DP_BUILD_TIME, pack_null);
	register_msg_pack_handle(SHOW_DP_BUILD_TIME_ACK, pack_show_info);

	register_msg_pack_handle(SHOW_DP_PKT_STAT, pack_null);
	register_msg_pack_handle(SHOW_DP_PKT_STAT_ACK, pack_show_info);

	register_msg_pack_handle(SHOW_MEM_POOL, pack_null);
	register_msg_pack_handle(SHOW_MEM_POOL_ACK, pack_show_info);

	register_msg_pack_handle(SHOW_ACL_RULE, pack_null);
	register_msg_pack_handle(SHOW_ACL_RULE_ACK, pack_show_info);

	register_msg_pack_handle(ADD_ACL_RULE, pack_acl_rule);
	register_msg_pack_handle(ADD_ACL_RULE_ACK, pack_result_code);

	register_msg_pack_handle(DEL_ACL_RULE, pack_acl_rule);
	register_msg_pack_handle(DEL_ACL_RULE_ACK, pack_result_code);

	register_msg_pack_handle(DEL_ACL_RULE_ID, pack_acl_rule_id);
	register_msg_pack_handle(DEL_ACL_RULE_ID_ACK, pack_result_code);

	register_msg_pack_handle(DEL_ACL_RULE_ALL, pack_null);
	register_msg_pack_handle(DEL_ACL_RULE_ALL_ACK, pack_result_code);
	
	register_msg_pack_handle(COMMIT_ACL_RULE, pack_null);
	register_msg_pack_handle(COMMIT_ACL_RULE_ACK, pack_result_code);

	return 0;
}
/*
  *	register the command msg header info
  */
int register_msg_header(uint8_t flag, cmd_type_t cmd, uint8_t msg_type, uint16_t msg_code, uint8_t msg_block_type)
{
	int rv;

	/* make sure the cmd type is valid */
	rv = __cmd_is_valid(cmd);
	if (rv)
		return rv;
	
	cmd_msg_headers[cmd].flag = flag;
	cmd_msg_headers[cmd].cmd = cmd;
	cmd_msg_headers[cmd].msg_type = msg_type;
	cmd_msg_headers[cmd].msg_code = msg_code;
	cmd_msg_headers[cmd].msg_block_type = msg_block_type;
	
	return 0;
}

/*
  *	Initialize the msg header info
  */
int init_msg_header(void)
{
	memset(cmd_msg_headers, 0, sizeof(struct msg_header_info_s) * (MAX_COMMAND_TYPE + 1));

	register_msg_header(MSG_VALID_FLAG, TEST_COMMAND, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_TEST_COMMAND, BLOCK_TYPE_START);
	register_msg_header(MSG_VALID_FLAG, TEST_COMMAND_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_TEST_COMMAND_ACK, BLOCK_TYPE_START);

	register_msg_header(MSG_VALID_FLAG, SHOW_DP_BUILD_TIME, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_DP_BUILD_TIME, BLOCK_TYPE_START);
	register_msg_header(MSG_VALID_FLAG, SHOW_DP_BUILD_TIME_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_DP_BUILD_TIME_ACK, BLOCK_TYPE_START);

	register_msg_header(MSG_VALID_FLAG, SHOW_DP_PKT_STAT, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_DP_PKT_STAT, BLOCK_TYPE_START);
	register_msg_header(MSG_VALID_FLAG, SHOW_DP_PKT_STAT_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_DP_PKT_STAT_ACK, BLOCK_TYPE_START);

	register_msg_header(MSG_VALID_FLAG, SHOW_MEM_POOL, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_MEM_POOL, BLOCK_TYPE_START);
	register_msg_header(MSG_VALID_FLAG, SHOW_MEM_POOL_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_MEM_POOL_ACK, BLOCK_TYPE_START);
	
	register_msg_header(MSG_VALID_FLAG, SHOW_ACL_RULE, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_ACL_RULE, BLOCK_TYPE_START);
	register_msg_header(MSG_VALID_FLAG, SHOW_ACL_RULE_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_SHOW_ACL_RULE_ACK, BLOCK_RESULT_CODE);
	
	register_msg_header(MSG_VALID_FLAG, ADD_ACL_RULE, MSG_TYPE_CLI_OCTEON, MSG_CODE_ADD_ACL_RULE, BLOCK_ACL_RULE_TUPLE);
	register_msg_header(MSG_VALID_FLAG, ADD_ACL_RULE_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_ADD_ACL_RULE_ACK, BLOCK_RESULT_CODE);
	
	register_msg_header(MSG_VALID_FLAG, DEL_ACL_RULE, MSG_TYPE_CLI_OCTEON, MSG_CODE_DEL_ACL_RULE, BLOCK_ACL_RULE_TUPLE);	
	register_msg_header(MSG_VALID_FLAG, DEL_ACL_RULE_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_DEL_ACL_RULE_ACK, BLOCK_RESULT_CODE);
	
	register_msg_header(MSG_VALID_FLAG, DEL_ACL_RULE_ID, MSG_TYPE_CLI_OCTEON, MSG_CODE_DEL_ACL_RULE_ID, BLOCK_ACL_RULE_ID);
	register_msg_header(MSG_VALID_FLAG, DEL_ACL_RULE_ID_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_DEL_ACL_RULE_ID_ACK, BLOCK_RESULT_CODE);
	
	register_msg_header(MSG_VALID_FLAG, DEL_ACL_RULE_ALL, MSG_TYPE_CLI_OCTEON, MSG_CODE_DEL_ACL_RULE_ALL, BLOCK_TYPE_START);
	register_msg_header(MSG_VALID_FLAG, DEL_ACL_RULE_ALL_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_DEL_ACL_RULE_ALL_ACK, BLOCK_RESULT_CODE);

	register_msg_header(MSG_VALID_FLAG, COMMIT_ACL_RULE, MSG_TYPE_CLI_OCTEON, MSG_CODE_COMMIT_ACL_RULE, BLOCK_TYPE_START);
	register_msg_header(MSG_VALID_FLAG, COMMIT_ACL_RULE_ACK, MSG_TYPE_CLI_OCTEON, MSG_CODE_COMMIT_ACL_RULE_ACK, BLOCK_RESULT_CODE);

	return 0;
}





