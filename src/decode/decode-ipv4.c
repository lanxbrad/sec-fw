#include <sec-common.h>
#include <sec-util.h>
#include <mbuf.h>
#include "decode-ethernet.h"
#include "decode-vlan.h"
#include "decode-ipv4.h"
#include "decode-statistic.h"
#include "decode-defrag.h"


extern int DecodeTCP(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);
extern int DecodeUDP(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);







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

#ifdef SEC_IPV4_DEBUG
	printf("sip is 0x%x\n", mbuf->ipv4.sip);
	printf("dip is 0x%x\n", mbuf->ipv4.dip);
#endif

	/*TODO: DecodeIPV4Options*/

	return DECODE_OK;
}


int DecodeIPV4(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	uint8_t protocol;
	uint16_t ipoffset;
	mbuf_t *mb;

#ifdef SEC_IPV4_DEBUG
	printf("=========>enter DecodeIPV4()\n");
#endif

	mbuf->pkt = pkt; /*ip header start*/
	mbuf->len = len; /*from ip header to pkt end*/

	if (unlikely(DECODE_OK != DecodeIPV4Packet (mbuf, pkt, len))) {
		return DECODE_DROP;
	}

	protocol = IPV4_GET_IPPROTO(mbuf);
	mbuf->proto = protocol;

#ifdef SEC_IPV4_DEBUG
	printf("protocol is %d\n", protocol);
#endif

	/* If a fragment, pass off for re-assembly. */

	if(0 == IPV4_GET_IPOFFSET(mbuf) && IPV4_GET_MF(mbuf) == 1)/*first frag packet*/
	{
		PKT_SET_IP_FRAG(mbuf);
		PKT_SET_FIRST_FRAG(mbuf);
	}

	if(IPV4_GET_IPOFFSET(mbuf) > 0)
	{
		PKT_SET_IP_FRAG(mbuf);/*nofirst frag packet*/
	}

	if(PKT_IS_IP_FRAG(mbuf))
	{
		mb = Defrag(mbuf);	
		if(NULL == mb)
		{
			return DECODE_OK;
		}
	}		

	/* check what next decoder to invoke */
	switch (protocol) {
		case PROTO_TCP:
			STAT_IPV4_RECV_OK;
			return DecodeTCP(mbuf, pkt + IPV4_GET_HLEN(mbuf), IPV4_GET_IPLEN(mbuf) - IPV4_GET_HLEN(mbuf));
		case PROTO_UDP:
			STAT_IPV4_RECV_OK;
			return DecodeUDP(mbuf, pkt + IPV4_GET_HLEN(mbuf), IPV4_GET_IPLEN(mbuf) - IPV4_GET_HLEN(mbuf));
		default:
			printf("unsupport protocol %d\n",IPV4_GET_IPPROTO(mbuf));
			STAT_IPV4_UNSUPPORT;
			return DECODE_DROP;
	}

	return DECODE_OK;
}

