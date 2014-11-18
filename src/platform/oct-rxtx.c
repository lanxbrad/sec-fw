#include "oct-rxtx.h"
#include <decode-statistic.h>
#include <mbuf.h>
#include <sec-debug.h>


extern CVMX_SHARED int wqe_pool;

/**
 * Determine if the supplied work queue entry and packet
 * should be filtered.
 *
 * @param work   Work queue entry to check
 * @return Non zero if packet should be filtered out
 */
static int is_filtered_packet(cvmx_wqe_t *work)
{
    /* As an example, block all IP broadcasts */
    return (work->word2.s.is_bcast && !work->word2.s.not_IP);
}




/*
 *  alloc a mbuf which can be used to describe the packet
 *  if work is error , return NULL
 *  then free wqe, reurn mbuf
 */
void *
oct_rx_process_work(cvmx_wqe_t *wq)
{
	void *pkt_virt;
	mbuf_t *m;

	
	if (wq->word2.s.rcv_error || cvmx_wqe_get_bufs(wq) > 1){
		/* 
		  *  Work has error, so drop
		  *  and now do not support jumbo packet
		  */
		printf("recv error\n");
		oct_packet_free(wq, wqe_pool);
	#if 0
		cvmx_helper_free_packet_data(wq);
 		cvmx_fpa_free(wq, wqe_pool, 0);
	#endif
		STAT_RECV_ERR;
		return NULL;
	}

	if (is_filtered_packet(wq))
    {
        printf("Received %u byte packet. Filtered.\n", cvmx_wqe_get_len(wq));
        oct_packet_free(wq, wqe_pool);
		return NULL;
    }

	pkt_virt = (void *) cvmx_phys_to_ptr(wq->packet_ptr.s.addr);
	if(NULL == pkt_virt)
	{
		STAT_RECV_ADDR_ERR;
		return NULL;
	}
	
#ifdef SEC_RXTX_DEBUG
    printf("Received %u byte packet.\n", cvmx_wqe_get_len(wq));
    printf("Processing packet\n");
    cvmx_helper_dump_packet(wq);
#endif

	oct_packet_free(wq, wqe_pool);
	return NULL;

	m = (mbuf_t *)MBUF_ALLOC();

	memset((void *)m, 0, sizeof(mbuf_t));

	m->magic_flag = MBUF_MAGIC_NUM;
	m->packet_ptr.u64 = wq->packet_ptr.u64;
	
	m->pktlen = cvmx_wqe_get_len(wq);
	m->pktptr = pkt_virt;

	cvmx_fpa_free(wq, wqe_pool, 0);
	
	STAT_RECV_PC_ADD;
	STAT_RECV_PB_ADD(m->pktlen);

	STAT_RECV_OK;
	return (void *)m;

}







