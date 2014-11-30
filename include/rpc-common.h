#ifndef __OPCODE_H__
#define __OPCODE_H__


#define ETH_P 0x3322


#define COMMAND_INVALID    0x0
#define COMMAND_SHOW_BUILD_TIME  0xA001

#define MAX_BUF_SIZE 1000

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
