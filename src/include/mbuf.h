#ifndef __MBUF_H__
#define __MBUF_H__

#include <decode-ethernet.h>
#include <decode-ipv4.h>
#include <decode-vlan.h>
#include <sec-util.h>




/*packet info description
 * using wqe 96 bytes space
 * so this struct size must be less than**** 96 bytes ****
 */
typedef struct m_buf_
{
	uint32_t magic_flag;
	uint32_t pktlen;

	struct m_buf_ *next;
	struct m_buf_ *prev;
	
	void *pktptr;  /*pointer to begin of packet*/
	
	EthernetHdr *ethh;
	VLANHdr *vlanh;
	IPV4Hdr *ip4h;
	void *transport_header;

	ipv4_tuple_t ipv4;

	uint16_t  sport;
   	uint16_t  dport;

	uint8_t proto;
	uint8_t vlan_idx;
	uint16_t payload_len;

	uint16_t vlan_id;
  
    uint8_t *payload; /* ptr to the payload of the packet  with it's length. */

	uint32_t offset;
	uint32_t ip_fraglen;

	uint32_t flags;
	
}m_buf;





#endif

