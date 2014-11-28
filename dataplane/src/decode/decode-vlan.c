#include "decode-ethernet.h"
#include "decode-vlan.h"
#include "decode-statistic.h"
#include "mbuf.h"


extern int DecodeIPV4(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);




/*
 * current support  one layer vlan
 *   @mbuf:  
 *   @pkt:    start of vlan header
 *   @len:    len of packet from vlan header
 */
int DecodeVLAN(mbuf_t *mb, uint8_t *pkt, uint16_t len)
{
	uint32_t proto;
	VLANHdr *pvh;

	if(len < VLAN_HEADER_LEN) 
	{
		STAT_VLAN_HEADER_ERR;
    	return DECODE_DROP;
	}

	if (mb->vlan_idx >= 1) 
	{
		STAT_VLAN_LAYER_EXCEED;
        return DECODE_DROP;
	}

	mb->vlanh = (void *)pkt;
	pvh = (VLANHdr *)(pkt);

	proto = GET_VLAN_PROTO(pvh);

	mb->vlan_idx = 1;

	switch (proto) {
		case ETHERNET_TYPE_IP:
			return DecodeIPV4(mb, pkt + VLAN_HEADER_LEN, len - VLAN_HEADER_LEN);
			
		case ETHERNET_TYPE_VLAN:
        case ETHERNET_TYPE_8021QINQ:
			return DecodeVLAN(mb, pkt + VLAN_HEADER_LEN, len - VLAN_HEADER_LEN);

		default:
			printf("ether type %04x not supported", proto);
			STAT_VLAN_UNSUPPORT;
			return DECODE_DROP;
	}

	STAT_VLAN_RECV_OK;
	return DECODE_OK;
}

