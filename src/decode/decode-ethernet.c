#include "sec-common.h"
#include <mbuf.h>
#include <sec-util.h>
#include "decode.h"


#include "decode-ethernet.h"
#include "decode-vlan.h"
#include "decode-ipv4.h"




/*
  *   @mbuf:  
  *   @pkt:    start of l2 header
  *   @len:    len of l2 packet
  */
int DecodeEthernet(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	if (unlikely(len < ETHERNET_HEADER_LEN))
	{	
		return DECODE_DROP;
	}
	
	mbuf->ethh = (EthernetHdr *)pkt;
	if (unlikely(mbuf->ethh == NULL))
        return DECODE_DROP;


	switch (mbuf->ethh->eth_type) {
		case ETHERNET_TYPE_IP:
			return DecodeIPV4(mbuf, pkt + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
			
		case ETHERNET_TYPE_VLAN:
        case ETHERNET_TYPE_8021QINQ:
			return DecodeVLAN(mbuf, pkt + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);

		default:
			printf("ether type %04x not supported", mbuf->ethh->eth_type);
			return DECODE_DROP;
	}

	return DECODE_OK;

}

