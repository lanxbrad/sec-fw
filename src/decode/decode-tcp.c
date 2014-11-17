#include <mbuf.h>
#include "decode.h"
#include "decode-ipv4.h"
#include "decode-tcp.h"
#include "decode-statistic.h"

extern void FlowHandlePacket(mbuf_t *m);

static int DecodeTCPPacket(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	if (unlikely(len < TCP_HEADER_LEN)) {
		STAT_TCP_HEADER_ERR;
        return DECODE_DROP;
    }

    mbuf->transport_header = (void *)pkt;

    uint8_t hlen = TCP_GET_HLEN(mbuf);
    if (unlikely(len < hlen)) {
		STAT_TCP_LEN_ERR;
        return DECODE_DROP;
    }

	uint8_t tcp_opt_len = hlen - TCP_HEADER_LEN;
    if (unlikely(tcp_opt_len > TCP_OPTLENMAX)) {
		STAT_TCP_LEN_ERR;
        return DECODE_DROP;
    }

	mbuf->sport = TCP_GET_SRC_PORT(mbuf);
    mbuf->dport = TCP_GET_DST_PORT(mbuf);

    mbuf->proto = PROTO_TCP;

    mbuf->payload = pkt + hlen;
    mbuf->payload_len = len - hlen;


    return DECODE_OK;

}



int DecodeTCP(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{

	if (unlikely(DecodeTCPPacket(mbuf, pkt, len) != DECODE_OK)) {
        return DECODE_DROP;
    }

	FlowHandlePacket(mbuf);

	STAT_TCP_RECV_OK;
	return DECODE_OK;
}