#include "decode.h"

extern int DecodeEthernet(PacketInfo *p, uint8_t *pkt, uint16_t len);


void
Decode(PacketInfo *pi)
{
	if( DECODE_OK != DecodeEthernet(pi, GET_PKT_DATA(pi), GET_PKT_LEN(pi))){
		
	}

	return;
}


