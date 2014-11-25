/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       oct-rxtx.c
 *        description :    
 *
 *        created by  luoye  at  2014-11-21
 *
 ********************************************************************************/

#include "oct-rxtx.h"
#include <decode-statistic.h>
#include <mbuf.h>
#include <sec-debug.h>
#include <oct-port.h>


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
	m->pkt_space = PKTBUF_HW;
	m->packet_ptr.u64 = wq->packet_ptr.u64;

	m->input_port = cvmx_wqe_get_port(wq);
	
	m->pkttotallen = cvmx_wqe_get_len(wq);
	m->pktptr = pkt_virt;

	cvmx_fpa_free(wq, wqe_pool, 0);
	
	STAT_RECV_PC_ADD;
	STAT_RECV_PB_ADD(m->pktlen);

	STAT_RECV_OK;
	return (void *)m;

}



void oct_tx_process_mbuf(mbuf_t *mbuf, uint8_t port)
{
	uint64_t queue;
	
	if(port > OCT_PHY_PORT_MAX)
	{
		printf("Send port is invalid");
		PACKET_DESTROY_ALL(mbuf);
		return;
	}

	queue = cvmx_pko_get_base_queue(port);

	cvmx_pko_send_packet_prepare(port, queue, CVMX_PKO_LOCK_CMD_QUEUE);

	/* Build a PKO pointer to this packet */
	cvmx_pko_command_word0_t pko_command;
	pko_command.u64 = 0;
	pko_command.s.segs = 1;
	pko_command.s.total_bytes = mbuf->pktlen;

	/* Send the packet */
	cvmx_pko_return_value_t send_status = cvmx_pko_send_packet_finish(port, queue, pko_command, mbuf->packet_ptr, CVMX_PKO_LOCK_CMD_QUEUE);
	if (send_status != CVMX_PKO_SUCCESS)
    {
        printf("Failed to send packet using cvmx_pko_send_packet2\n");
        PACKET_DESTROY_DATA(mbuf);
    }

	MBUF_FREE(mbuf);
}





