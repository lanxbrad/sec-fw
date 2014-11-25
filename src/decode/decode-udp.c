#include <mbuf.h>
#include <sec-util.h>
#include <sec-common.h>
#include "decode-ipv4.h"
#include "decode-udp.h"
#include "decode-statistic.h"
#include "decode-defrag.h"



extern void FlowHandlePacket(mbuf_t *m);

static int DecodeUDPPacket(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	if (unlikely(len < UDP_HEADER_LEN)) {
		STAT_UDP_HEADER_ERR;
        return DECODE_DROP;
    }

    mbuf->transport_header= (void *)pkt;

	if (unlikely(len < UDP_GET_LEN(mbuf))) {
		STAT_UDP_LEN_ERR;
        return DECODE_DROP;
    }

	if (unlikely(len != UDP_GET_LEN(mbuf))) {
		STAT_UDP_LEN_ERR;
        return DECODE_DROP;
    }

	mbuf->sport = UDP_GET_SRC_PORT(mbuf);
	mbuf->dport = UDP_GET_DST_PORT(mbuf);

#ifdef SEC_UDP_DEBUG
	printf("src port is %d\n", mbuf->sport);
	printf("dst port is %d\n", mbuf->dport);
#endif

	mbuf->payload = pkt + UDP_HEADER_LEN;
    mbuf->payload_len = len - UDP_HEADER_LEN;

	mbuf->proto = PROTO_UDP;

	return DECODE_OK;
}




int DecodeUDP(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
#ifdef SEC_UDP_DEBUG
	printf("=========>enter DecodeTCP\n");
#endif

	
	if (unlikely(DECODE_OK != DecodeUDPPacket(mbuf, pkt, len))) 
	{
		return DECODE_DROP;
	}

#if 0
	if(!PKT_IS_IP_FRAG(mbuf) || PKT_IS_FIRST_FRAG(mbuf))/*own udp header, can be decoded*/
	{
		if (unlikely(DECODE_OK != DecodeUDPPacket(mbuf, pkt, len))) 
		{
			return DECODE_DROP;
		}
	}

	if(PKT_IS_IP_FRAG(mbuf))/*need be defraged */
	{
		Defrag(mbuf);	
	}
#endif
	
	
	STAT_UDP_RECV_OK;

	FlowHandlePacket(mbuf);

	return DECODE_OK;
}

