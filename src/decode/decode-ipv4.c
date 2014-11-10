#include <sec-util.h>
#include <mbuf.h>
#include "decode-ipv4.h"



static int DecodeIPV4Packet(m_buf *mbuf, uint8_t *pkt, uint16_t len)
{
	
	if (unlikely(len < IPV4_HEADER_LEN)) {
        return DECODE_DROP;
    }

	if (unlikely(IP_GET_RAW_VER(pkt) != 4)) {
        return DECODE_DROP;
    }

	mbuf->ip4h = (IPV4Hdr *)pkt;

	if (unlikely(IPV4_GET_HLEN(mbuf) < IPV4_HEADER_LEN)) {
        return DECODE_DROP;
    }

	if (unlikely(IPV4_GET_IPLEN(mbuf) < IPV4_GET_HLEN(mbuf))) {
        return DECODE_DROP;
    }

    if (unlikely(len < IPV4_GET_IPLEN(mbuf))) {
        return DECODE_DROP;
    }

	mbuf->ipv4.sip = IPV4_GET_IPSRC(mbuf);
	mbuf->ipv4.dip = IPV4_GET_IPDST(mbuf);

	/*TODO: DecodeIPV4Options*/

	return DECODE_OK;
}


int DecodeIPV4(m_buf *mbuf, uint8_t *pkt, uint16_t len)
{
	if (unlikely(DECODE_OK != DecodeIPV4Packet (mbuf, pkt, len))) {
		return DECODE_DROP;
	}

	mbuf->proto = IPV4_GET_IPPROTO(mbuf);

	/* If a fragment, pass off for re-assembly. */
	if (unlikely(IPV4_GET_IPOFFSET(mbuf) > 0 || IPV4_GET_MF(mbuf) == 1)) {
		Defrag(mbuf);
		return DECODE_OK;
	}

	/* check what next decoder to invoke */
	switch (IPV4_GET_IPPROTO(mbuf)) {
		case PROTO_TCP:
			return DecodeTCP(mbuf, pkt + IPV4_GET_HLEN(mbuf), IPV4_GET_IPLEN(mbuf) - IPV4_GET_HLEN(mbuf));
		case PROTO_UDP:
			return DecodeUDP(mbuf, pkt + IPV4_GET_HLEN(mbuf), IPV4_GET_IPLEN(mbuf) - IPV4_GET_HLEN(mbuf));
		default:
			printf("unsupport protocol %d\n",IPV4_GET_IPPROTO(mbuf));
			return DECODE_DROP;
	}
		
	return DECODE_OK;
}

