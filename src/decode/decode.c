/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       decode.c
 *        description :    decode entry
 *
 *        created by  luoye  at  2014-11-18
 *
 ********************************************************************************/



#include <sec-common.h>
#include <decode.h>
#include <mbuf.h>

extern int DecodeEthernet(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);


/*
 * decode pakcet l2->l3->l4
 * if result is DECODE_OK, not need to free anything, the packet data will pass to the next module
 * if result is DECODE_DROP, decode function must destroy the packet data,include packet buffer and mbuf
 */

void Decode(mbuf_t *m)
{
#ifdef SEC_DECODE_DEBUG
	printf("==========>enter decode()\n");
#endif

	if( DECODE_OK != DecodeEthernet(m, GET_PKT_DATA(m), GET_PKT_LEN(m))){
		PACKET_DESTROY_ALL(m);
	}

	return;
}


