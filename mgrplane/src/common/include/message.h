#ifndef __MESSAGE__H__
#define __MESSAGE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define SERV_LOCAL 0x7f000001
#define SERV_CLI_PORT  10001



#define RCP_MAX_DATA_BLOCK_PER_MESSAGE 255
#define MAX_INFO_BUF_SIZE 1000
#define BUFSIZE  2048
#define MAX_BUF	1530


#define MSG_VALID_FLAG 0x88


typedef enum
{
	TEST_COMMAND = 0,
	TEST_COMMAND_ACK,

	SHOW_DP_BUILD_TIME,
	SHOW_DP_BUILD_TIME_ACK,

	SHOW_DP_PKT_STAT,
	SHOW_DP_PKT_STAT_ACK,

	SHOW_MEM_POOL,
	SHOW_MEM_POOL_ACK,
	
	MAX_COMMAND_TYPE,
}cmd_type_t;


typedef enum _msg_block_type_e { 
	BLOCK_TYPE_START = 0x00, 
	BLOCK_IPV4_FIVE_TUPLE = 0x01, 
}msg_block_type_e;




typedef struct tag_RCP_BLOCK_IPV4_FIVE_TUPLE {
	uint8_t protocol;
	uint32_t src_ip;
	uint32_t dst_ip;
	uint16_t src_port;
	uint16_t dst_port;
}__attribute__ ((__packed__)) RCP_BLOCK_IPV4_FIVE_TUPLE;



typedef struct TAG_RCP_DATA_BLOCK {
	union {
		RCP_BLOCK_IPV4_FIVE_TUPLE Ipv4FiveTuple;
	};
}RCP_DATA_BLOCK;


typedef enum _msg_type_e {
	MSG_TYPE_CLI_OCTEON = 0x1, 
	MSG_TYPE_CLI_DEBUG = 0x2,
	MSG_TYPE_CLI_LOG = 0x3,
	MSG_TYPE_CLI_SNMP = 0x4,
} msg_type_e;


typedef enum _msg_code_e { 
	MSG_CODE_START = 0,
	
	MSG_CODE_SHOW_TEST_COMMAND = 0x101,
	MSG_CODE_SHOW_TEST_COMMAND_ACK,

	MSG_CODE_SHOW_DP_BUILD_TIME,
	MSG_CODE_SHOW_DP_BUILD_TIME_ACK,

	MSG_CODE_SHOW_DP_PKT_STAT,
	MSG_CODE_SHOW_DP_PKT_STAT_ACK,

	MSG_CODE_SHOW_MEM_POOL,
	MSG_CODE_SHOW_MEM_POOL_ACK,
}msg_code_e;




struct rcp_msg_params_s {
	uint32_t msg_id;
	uint8_t more_flag;
	uint8_t nparam;			// used by data block message
	uint16_t info_len;		//used by show info message
	union {
		RCP_DATA_BLOCK params[RCP_MAX_DATA_BLOCK_PER_MESSAGE];	// used by data block message
		char info_buf[MAX_INFO_BUF_SIZE];	//used by show info message
	} params_list;
};

/*
  *	structure for command type map to message header
  */
struct msg_header_info_s {
	cmd_type_t cmd;
	uint8_t flag;
	uint8_t msg_type;
	uint16_t msg_code;
	uint8_t msg_block_type;
};



/* Message header */
typedef struct tag_MESSAGE_HEAD {
	uint8_t flag;
	uint8_t msg_type;
	uint16_t msg_code;
	uint32_t msg_id;
	uint8_t more_flag;
	uint8_t length;
	uint8_t blocktype;
	uint8_t data_block_num;
	uint8_t payload[0];
} MESSAGE_HEAD;

#define MESSAGE_HEADER_LENGTH sizeof(MESSAGE_HEAD)


/*
  *	structure for packing/unpacking message for the comand
  */
typedef int (*msg_pack_handle_t) (cmd_type_t, void *, void *, int *);
struct msg_pack_handle_s {
	cmd_type_t cmd;
	int (*pack) (cmd_type_t, void *, void *, int *);
};


/* structure for processing  the command */
typedef int (*cmd_proc_handle_t) (uint8_t *, uint32_t, uint32_t);
struct cmd_process_handle_s {
	cmd_type_t cmd;
	int32_t (*handle) (uint8_t *, uint32_t, uint32_t);
};



extern struct msg_header_info_s cmd_msg_headers[];
extern struct msg_pack_handle_s cmd_msg_handles[];
extern struct cmd_process_handle_s cmd_process_handles[];

extern int param_to_pkt(cmd_type_t cmd, void *from, uint8_t *sbuf, int *sn_p, void *param_p);
extern int init_msg_pack_handle(void);
extern int32_t init_cmd_process_handle(void);
extern int mgmt_process_cmd(uint8_t * from, uint32_t length, uint32_t fd);

extern int register_cmd_process_handle(cmd_type_t cmd, cmd_proc_handle_t cmd_handle);
extern int init_msg_header(void);



#ifdef __cplusplus
}
#endif


#endif

