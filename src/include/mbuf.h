#ifndef __MBUF_H__
#define __MBUF_H__

#include "decode-ethernet.h"
#include "decode-ipv4.h"

/*packet info description
 * using wqe 96 bytes space
 * so this struct size must be less than**** 96 bytes ****
 */
typedef struct m_buf_
{
	uint32_t magic_flag;
	uint32_t pktlen;
	
	void *pktptr;  /*pointer to begin of packet*/
	
	EthernetHdr *ethh;
	IPV4Hdr *ip4h;
	void *transport_header;

	ipv4_tuple_t ipv4;

	uint16_t  sport;
   	uint16_t  dport;

	uint8_t proto;
	uint8_t res1;
	uint16_t payload_len;
  
    uint8_t *payload; /* ptr to the payload of the packet  with it's length. */
    
	
}m_buf;



#endif

