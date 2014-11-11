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
	#if 0
		cvmx_helper_free_packet_data(wq);
 		cvmx_fpa_free(wq, wqe_pool, 0);
	#endif
		return NULL;
	}
	
#ifdef SEC_RXTX_DEBUG
    printf("Received %u byte packet.\n", cvmx_wqe_get_len(wq));
    printf("Processing packet\n");
    cvmx_helper_dump_packet(wq);
#endif

	m = (m_buf *)mbuf_alloc();

	memset((void *)m, 0, sizeof(m_buf));

	m->magic_flag = MBUF_MAGIC_NUM;
	m->packet_ptr.u64 = wq->packet_ptr.u64;
	
	m->pktlen = cvmx_wqe_get_len(wq);
	m->pktptr = (void *) cvmx_phys_to_ptr(wq->packet_ptr.s.addr);

	cvmx_fpa_free(wq, wqe_pool, 0);
	
	return (void *)m;

}







