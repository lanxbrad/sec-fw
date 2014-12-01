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


uint32_t oct_tx_entries = 0;
oct_softx_stat_t *oct_stx[CPU_HW_RUNNING_MAX];





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
	PKTBUF_SET_HW(m);
	
	m->packet_ptr.u64 = wq->packet_ptr.u64;

	m->input_port = cvmx_wqe_get_port(wq);
	
	m->pkt_totallen = cvmx_wqe_get_len(wq);
	m->pkt_ptr = pkt_virt;

	cvmx_fpa_free(wq, wqe_pool, 0);
	
	STAT_RECV_PC_ADD;
	STAT_RECV_PB_ADD(m->pkt_totallen);

	STAT_RECV_OK;
	return (void *)m;

}


void oct_tx_done_check()
{
	int port;
	uint16_t consumer;
	uint16_t producer;
	oct_pko_pend_tx_done_t *pend_tx_done;
	oct_softx_stat_t *oct_stx_local = oct_stx[local_cpu_id];
	
	for( port = 0; port < OCT_PHY_PORT_MAX; port++ )
	{
		if(oct_stx_local->tx_done[port].tx_entries)
		{
			consumer = oct_stx_local->tx_done[port].consumer;
			producer = oct_stx_local->tx_done[port].producer;

			while(consumer != producer)
			{
				pend_tx_done = &(oct_stx_local->tx_done[port].pend_tx_done[consumer]);
				if( 0xFF == pend_tx_done->mem_ref ) {
					break;
				}

				/*Free the packet*/
				PACKET_DESTROY_ALL(pend_tx_done->mb);
				
				consumer = (consumer + 1) & (OCT_PKO_TX_DESC_NUM - 1);
				oct_stx_local->tx_done[port].tx_entries--;
				oct_tx_entries--;
			}
			oct_stx_local->tx_done[port].consumer = consumer;	
		}
	}
	
	return;
}



static inline uint8_t *
oct_pend_tx_done_add(tx_done_t *tdt, void *mb)
{
	uint8_t *mem_ref = NULL;
	uint16_t producer = tdt->producer;

	mem_ref = &tdt->pend_tx_done[producer].mem_ref;

	*mem_ref = 0xFF;
	tdt->pend_tx_done[producer].mb = mb;

	producer = (producer + 1) & (OCT_PKO_TX_DESC_NUM - 1);

	tdt->tx_entries++;
	oct_tx_entries++;
	tdt->producer = producer;

	return mem_ref;
}


static inline void
oct_pend_tx_done_remove(tx_done_t *tdt)
{
	tdt->producer = (tdt->producer - 1) & (OCT_PKO_TX_DESC_NUM - 1);
	tdt->tx_entries--;
	oct_tx_entries--;
	return;
}


void oct_tx_process_mbuf(mbuf_t *mbuf, uint8_t port)
{
	uint64_t queue;
	cvmx_pko_return_value_t send_status;
	
	if(port > OCT_PHY_PORT_MAX)
	{
		printf("Send port is invalid");
		PACKET_DESTROY_ALL(mbuf);
		STAT_TX_SEND_PORT_ERR;
		return;
	}

	queue = cvmx_pko_get_base_queue(port);

	cvmx_pko_send_packet_prepare(port, queue, CVMX_PKO_LOCK_CMD_QUEUE);


	if(PKTBUF_IS_HW(mbuf))
	{
		/* Build a PKO pointer to this packet */
		cvmx_pko_command_word0_t pko_command;
		pko_command.u64 = 0;
		pko_command.s.segs = 1;
		pko_command.s.total_bytes = mbuf->pkt_totallen;

		/* Send the packet */
		send_status = cvmx_pko_send_packet_finish(port, queue, pko_command, mbuf->packet_ptr, CVMX_PKO_LOCK_CMD_QUEUE);
		if (send_status != CVMX_PKO_SUCCESS)
	    {
	        printf("Failed to send packet using cvmx_pko_send_packet2\n");
			STAT_TX_HW_SEND_ERR;
	        PACKET_DESTROY_DATA(mbuf);
	    }

		MBUF_FREE(mbuf);
	}
	else if(PKTBUF_IS_SW(mbuf))
	{
		uint8_t *dont_free_cookie = NULL;
		tx_done_t *tx_done = &(oct_stx[local_cpu_id]->tx_done[port]);
		if(tx_done->tx_entries < (OCT_PKO_TX_DESC_NUM - 1))
		{
			dont_free_cookie = oct_pend_tx_done_add(tx_done, (void *)mbuf);			
		}
		else
		{
			PACKET_DESTROY_ALL(mbuf);
			STAT_TX_SW_DESC_ERR;
			return;
		}
		/*command word0*/
		cvmx_pko_command_word0_t pko_command;
		pko_command.u64 = 0;
		
		pko_command.s.segs = 1;
		pko_command.s.total_bytes = mbuf->pkt_totallen;

		pko_command.s.rsp = 1;
		pko_command.s.dontfree = 1;

		/*command word1*/
		cvmx_buf_ptr_t packet;
		packet.u64 = 0;
		packet.s.size = mbuf->pkt_totallen;
		packet.s.addr = (uint64_t)mbuf->pkt_ptr;

		/*command word2*/
		cvmx_pko_command_word2_t tx_ptr_word;
		tx_ptr_word.u64 = 0;
		tx_ptr_word.s.ptr = (uint64_t)cvmx_ptr_to_phys(dont_free_cookie);

		/* Send the packet */
		send_status = cvmx_pko_send_packet_finish3(port, queue, pko_command, packet, tx_ptr_word.u64, CVMX_PKO_LOCK_CMD_QUEUE);
		if(send_status != CVMX_PKO_SUCCESS)
		{
			if(dont_free_cookie)
			{
				oct_pend_tx_done_remove(tx_done);
			}

			printf("Failed to send packet using cvmx_pko_send_packet3\n");
			
	        PACKET_DESTROY_ALL(mbuf);
			STAT_TX_SW_SEND_ERR;
			return;
		}
	}
	else
	{
		printf("pkt space %d is wrong, please check it\n", PKTBUF_SPACE_GET(mbuf));
	}

	STAT_TX_SEND_OVER;
	
}






int oct_rxtx_init(void)
{
	int i;
	
	void *ptr = cvmx_bootmem_alloc_named(sizeof(oct_softx_stat_t) * CPU_HW_RUNNING_MAX, 
										CACHE_LINE_SIZE, 
										OCT_TX_DESC_NAME);
	if(NULL == ptr)
	{
		return SEC_NO;
	}

	memset(ptr, 0, sizeof(oct_softx_stat_t) * CPU_HW_RUNNING_MAX);

	for( i = 0; i < CPU_HW_RUNNING_MAX; i++ )
	{
		oct_stx[i] = (oct_softx_stat_t *)((uint8_t *)ptr + sizeof(oct_softx_stat_t) * i);
	}

	return SEC_OK;
}



int oct_rxtx_get(void)
{
	int i;
	void *ptr;
	const cvmx_bootmem_named_block_desc_t *block_desc = cvmx_bootmem_find_named_block(OCT_TX_DESC_NAME);
	if (block_desc)
	{
		ptr = cvmx_phys_to_ptr(block_desc->base_addr);
	}
	else
	{
		return SEC_NO;
	}

	for( i = 0; i < CPU_HW_RUNNING_MAX; i++ )
	{
		oct_stx[i] = (oct_softx_stat_t *)((uint8_t *)ptr + sizeof(oct_softx_stat_t) * i);
	}

	return SEC_OK;
}

