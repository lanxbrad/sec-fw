#include "decode-vlan.h"
#include "mbuf.h"

extern int DecodeIPV4(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);




/*
 * current support  one layer vlan
 */
int DecodeVLAN(mbuf_t *mb, uint8_t *pkt, uint16_t len)
{
	uint32_t proto;

	if(len < VLAN_HEADER_LEN) {
    	return DECODE_DROP;
	}

	if (mb->vlan_idx >= 1) {
        return DECODE_DROP;
	}

	mb->vlanh = (VLANHdr *)pkt;

	if(mb->vlanh == NULL){
		return DECODE_DROP;
	}

	proto = GET_VLAN_PROTO(mb->vlanh);

	mb->vlan_idx = 1;

	switch (proto) {
		case ETHERNET_TYPE_IP:
			return DecodeIPV4(mb, pkt + VLAN_HEADER_LEN, len - VLAN_HEADER_LEN);
			
		case ETHERNET_TYPE_VLAN:
        case ETHERNET_TYPE_8021QINQ:
			return DecodeVLAN(mb, pkt + VLAN_HEADER_LEN, len - VLAN_HEADER_LEN);

		default:
			printf("ether type %04x not supported", mb->ethh->eth_type);
			return DECODE_DROP;
	}

	return DECODE_OK;
}

