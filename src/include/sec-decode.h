#ifndef __SEC_DECODE_H__
#define __SEC_DECODE_H__

#include "sec-common.h"
#include "decode-ethernet.h"

/*packet info description
 * using wqe 96 bytes space
 * so this struct size must be less than**** 96 bytes ****
 */
typedef struct Packet_
{
	uint32_t magic_flag;
	uint32_t pktlen;
	
	void *pktptr;  /*pointer to begin of packet*/
	
	EthernetHdr *ethh;
	
}PacketInfo;



#endif
