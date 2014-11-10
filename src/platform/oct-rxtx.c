#include "oct-rxtx.h"

#include <sec-decode.h>



extern CVMX_SHARED int wqe_pool;

/*
 *  get packetinfo and packet pointer
 *  if work is error , return NULL
 */
void *
oct_rx_process_work(cvmx_wqe_t *wq)
{
	PacketInfo *pi = NULL;
	
	if (wq->word2.s.rcv_error){
		/* Work has error, so drop */
		oct_packet_free(wq, wqe_pool);
		//cvmx_helper_free_packet_data(wq);
 		//cvmx_fpa_free(wq, wqe_pool, 0);
		return NULL;
	}
	
#ifdef SEC_RXTX_DEBUG
    printf("Received %u byte packet.\n", cvmx_wqe_get_len(wq));
    printf("Processing packet\n");
    cvmx_helper_dump_packet(wq);
#endif

	pi = (PacketInfo *)wq->packet_data;
	pi->magic_flag = SEC_MAGIC_NUM;
	pi->pktlen = cvmx_wqe_get_len(wq);
	pi->pktptr = (void *) cvmx_phys_to_ptr(wq->packet_ptr.s.addr);	
	
	return (void *)pi;

}







