#include "sec-common.h"
#include <sec-decode.h>
#include <sec-util.h>
#include "decode.h"

#include "decode-ethernet.h"
#include "decode-vlan.h"

extern int DecodeIPV4(PacketInfo *p, uint8_t *pkt, uint16_t len);
extern int DecodeVLAN(PacketInfo *p, uint8_t *pkt, uint16_t len);


int DecodeEthernet(PacketInfo *p, uint8_t *pkt, uint16_t len)
{
	if (unlikely(len < ETHERNET_HEADER_LEN))
		return DECODE_DROP;
    
	p->ethh = (EthernetHdr *)pkt;
	if (unlikely(p->ethh == NULL))
        return DECODE_DROP;


	switch (p->ethh->eth_type) {
		case ETHERNET_TYPE_IP:
			return DecodeIPV4(p, pkt + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
			
		case ETHERNET_TYPE_VLAN:
        case ETHERNET_TYPE_8021QINQ:
			return DecodeVLAN(p, pkt + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);

		default:
			printf("ether type %04x not supported", p->ethh->eth_type);
			return DECODE_DROP;
	}

	return DECODE_OK;

}

