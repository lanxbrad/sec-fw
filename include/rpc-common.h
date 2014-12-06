#ifndef __OPCODE_H__
#define __OPCODE_H__


#define ETH_P 0x3322


#define COMMAND_INVALID    0x0
#define COMMAND_SHOW_BUILD_TIME  0xA001
#define COMMAND_SHOW_PKT_STAT    0xA002
#define COMMAND_SHOW_MEM_POOL    0xA003
#define COMMAND_ACL_RULE_COMMIT  0xA004



#define MAX_BUF_SIZE 1000


typedef struct tag_RCP_BLOCK_ACL_RULE_TUPLE{
	uint8_t smac[6];
	uint8_t dmac[6];
	uint32_t sip;
	uint32_t sip_mask;
	uint32_t dip;
	uint32_t dip_mask;
	uint16_t sport_start;
	uint16_t sport_end;
	uint16_t dport_start;
	uint16_t dport_end;
	uint8_t protocol_start;
	uint8_t protocol_end;
	uint16_t action;
}__attribute__ ((__packed__)) RCP_BLOCK_ACL_RULE_TUPLE;


/*
  *	the uniform data struct for packing/unpacking  
  */
typedef struct {
	uint16_t opcode;
	uint16_t info_len;		//used by show info message
	char info_buf[0];	//used by show info message
}rpc_msg_t;

typedef struct {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t type;
} rpc_ether_hdr_t;





#endif
