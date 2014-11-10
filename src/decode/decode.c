#include <sec-common.h>
#include <mbuf.h>

extern int DecodeEthernet(m_buf *mbuf, uint8_t *pkt, uint16_t len);


void
Decode(m_buf *m)
{
	if( DECODE_OK != DecodeEthernet(m, GET_PKT_DATA(m), GET_PKT_LEN(m))){
		
	}

	return;
}


