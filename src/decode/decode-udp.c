#include <mbuf.h>
#include <sec-util.h>
#include "decode-udp.h"

extern void FlowHandlePacket(m_buf *m);

static int DecodeUDPPacket(m_buf *mbuf, uint8_t *pkt, uint16_t len)
{
	if (unlikely(len < UDP_HEADER_LEN)) {
        return DECODE_DROP;
    }

    mbuf->transport_header= (void *)pkt;

	if (unlikely(len < UDP_GET_LEN(mbuf))) {
        return DECODE_DROP;
    }

	if (unlikely(len != UDP_GET_LEN(mbuf))) {
        return DECODE_DROP;
    }

	mbuf->payload = pkt + UDP_HEADER_LEN;
    mbuf->payload_len = len - UDP_HEADER_LEN;

	mbuf->proto = PROTO_UDP;

	return DECODE_OK;
}




int DecodeUDP(m_buf *mbuf, uint8_t *pkt, uint16_t len)
{
	if (unlikely(DECODE_OK != DecodeUDPPacket(mbuf, pkt, len))) {
		return DECODE_DROP;
	}

	FlowHandlePacket(mbuf);

	return DECODE_OK;
}

