#include <mbuf.h>
#include <oct-rxtx.h>
#include <sec-debug.h>
void l7_deliver(mbuf_t *m)
{
#ifdef SEC_L7_DEBUG
	printf("l7 enter\n");
#endif
	//PACKET_DESTROY_ALL(m);
	//oct_tx_process_mbuf(m, m->input_port);
	
	//packet_destroy_all(m);

	if(m->pkt_space == PKTBUF_SW)
		packet_destroy_all(m);
	else if(m->pkt_space == PKTBUF_HW)
		oct_tx_process_mbuf(m, m->input_port);
	else
		printf("pkt space error\n");
	

	return; 
}

