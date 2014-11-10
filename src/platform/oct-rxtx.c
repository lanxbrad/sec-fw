#include "oct-rxtx.h"

#include <mbuf.h>



extern CVMX_SHARED int wqe_pool;

/*
 *  get mbuf and packet pointer
 *  if work is error , return NULL
 */
void *
oct_rx_process_work(cvmx_wqe_t *wq)
{
	m_buf *m = NULL;
	
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

	m = (m_buf *)wq->packet_data;
	m->magic_flag = SEC_MAGIC_NUM;
	m->pktlen = cvmx_wqe_get_len(wq);
	m->pktptr = (void *) cvmx_phys_to_ptr(wq->packet_ptr.s.addr);	
	
	return (void *)m;

}







