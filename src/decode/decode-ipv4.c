#include <sec-util.h>
#include <mbuf.h>
#include "decode-ethernet.h"
#include "decode-vlan.h"
#include "decode-ipv4.h"
#include "decode-statistic.h"


extern int DecodeTCP(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);
extern int DecodeUDP(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);



mbuf_t *Defrag(mbuf_t *mbuf)
{
	return NULL;
}



static int DecodeIPV4Packet(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	
	if (unlikely(len < IPV4_HEADER_LEN)) {
		STAT_IPV4_HEADER_ERR;
        return DECODE_DROP;
    }

	if (unlikely(IP_GET_RAW_VER(pkt) != 4)) {
		STAT_IPV4_VERSION_ERR;
        return DECODE_DROP;
    }

	mbuf->network_header = (void *)pkt;

	if (unlikely(IPV4_GET_HLEN(mbuf) < IPV4_HEADER_LEN)) {
		STAT_IPV4_HEADER_ERR;
        return DECODE_DROP;
    }

	if (unlikely(IPV4_GET_IPLEN(mbuf) < IPV4_GET_HLEN(mbuf))) {
		STAT_IPV4_LEN_ERR;
        return DECODE_DROP;
    }

    if (unlikely(len < IPV4_GET_IPLEN(mbuf))) {
		STAT_IPV4_LEN_ERR;
        return DECODE_DROP;
    }

	mbuf->ipv4.sip = IPV4_GET_IPSRC(mbuf);
	mbuf->ipv4.dip = IPV4_GET_IPDST(mbuf);

	/*TODO: DecodeIPV4Options*/

	return DECODE_OK;
}


int DecodeIPV4(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	mbuf_t *m_defrag;
	uint8_t protocol;

	printf("enter DecodeIPV4()\n");
	
	if (unlikely(DECODE_OK != DecodeIPV4Packet (mbuf, pkt, len))) {
		return DECODE_DROP;
	}

	protocol = IPV4_GET_IPPROTO(mbuf);
	mbuf->proto = protocol;

	printf("protocol is %d\n", protocol);
	printf("total len is %d\n", IPV4_GET_IPLEN(mbuf));
	

	/* If a fragment, pass off for re-assembly. */
	if (unlikely(IPV4_GET_IPOFFSET(mbuf) > 0 || IPV4_GET_MF(mbuf) == 1)) {
		m_defrag = Defrag(mbuf);
		if(NULL != m_defrag)
		{
			return DecodeIPV4(m_defrag, m_defrag->network_header, 
				m_defrag->pktlen - ETHERNET_HEADER_LEN - m_defrag->vlan_idx * VLAN_HEADER_LEN);
		}
		return DECODE_OK;
	}

	/* check what next decoder to invoke */
	switch (protocol) {
		case PROTO_TCP:
			return DecodeTCP(mbuf, pkt + IPV4_GET_HLEN(mbuf), IPV4_GET_IPLEN(mbuf) - IPV4_GET_HLEN(mbuf));
		case PROTO_UDP:
			return DecodeUDP(mbuf, pkt + IPV4_GET_HLEN(mbuf), IPV4_GET_IPLEN(mbuf) - IPV4_GET_HLEN(mbuf));
		default:
			printf("unsupport protocol %d\n",IPV4_GET_IPPROTO(mbuf));
			STAT_IPV4_UNSUPPORT;
			return DECODE_DROP;
	}

	STAT_IPV4_RECV_OK;
	return DECODE_OK;
}

