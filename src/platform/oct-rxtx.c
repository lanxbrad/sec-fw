#include "oct-rxtx.h"
#include <decode-statistic.h>
#include <mbuf.h>
#include <sec-debug.h>


extern CVMX_SHARED int wqe_pool;


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
		STAT_RECV_ERR;
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

	m = (mbuf_t *)MBUF_ALLOC();

	memset((void *)m, 0, sizeof(mbuf_t));

	m->magic_flag = MBUF_MAGIC_NUM;
	m->packet_ptr.u64 = wq->packet_ptr.u64;

	m->input_port = cvmx_wqe_get_port(wq);
	
	m->pktlen = cvmx_wqe_get_len(wq);
	m->pktptr = pkt_virt;

	cvmx_fpa_free(wq, wqe_pool, 0);
	
	STAT_RECV_PC_ADD;
	STAT_RECV_PB_ADD(m->pktlen);

	STAT_RECV_OK;
	return (void *)m;

}







