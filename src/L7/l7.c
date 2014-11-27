#include <mbuf.h>
#include <oct-rxtx.h>
#include <sec-debug.h>
void l7_deliver(mbuf_t *m)
{
#ifdef SEC_L7_DEBUG
	printf("l7 enter\n");
#endif

	if(PKTBUF_IS_SW(m))
		oct_tx_process_mbuf(m, m->input_port);
		//PACKET_DESTROY_ALL(m);
	else if(PKTBUF_IS_HW(m))
		oct_tx_process_mbuf(m, m->input_port);
	else
		printf("pkt space error\n");
	

	return; 
}

