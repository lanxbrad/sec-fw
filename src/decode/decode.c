#include <sec-common.h>
#include <mbuf.h>

extern int DecodeEthernet(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);


/*
 * decode pakcet l2->l3->l4
 * if result is DECODE_OK, not need to free anything, the packet data will pass to the next module
 * if result is DECODE_DROP, decode function must destroy the packet data,include packet buffer and mbuf
 */

void Decode(mbuf_t *m)
{
	if( DECODE_OK != DecodeEthernet(m, GET_PKT_DATA(m), GET_PKT_LEN(m))){
		packet_destroy(m);
	}

	return;
}


