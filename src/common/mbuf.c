/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       mbuf.c
 *        description :    
 *
 *        created by  luoye  at  2014-11-21
 *
 ********************************************************************************/
#include <mbuf.h>
#include <flow.h>


static void inline packet_recycle(mbuf_t *mbuf)
{
	FlowDeReference((flow_item_t **)&(mbuf->flow));
}


mbuf_t *mbuf_alloc()
{
	Mem_Slice_Ctrl_B *mscb;
	void *buf = mem_pool_fpa_slice_alloc(FPA_POOL_ID_HOST_MBUF);
	if(NULL == buf)
		return NULL;

	mscb = (Mem_Slice_Ctrl_B *)buf;
	mscb->magic = MEM_POOL_MAGIC_NUM;
	mscb->pool_id = FPA_POOL_ID_HOST_MBUF;
	

	return (mbuf_t *)((uint8_t *)buf + sizeof(Mem_Slice_Ctrl_B));
}


void mbuf_free(mbuf_t *mb)
{

	packet_recycle(mb);

	Mem_Slice_Ctrl_B *mscb = (Mem_Slice_Ctrl_B *)((uint8_t *)mb - sizeof(Mem_Slice_Ctrl_B));
	if(MEM_POOL_MAGIC_NUM != mscb->magic)
	{
		printf("mbuf has been destroyed\n");
		return;
	}
	if(FPA_POOL_ID_HOST_MBUF != mscb->pool_id)
	{
		return;
	}

	mem_pool_fpa_slice_free((void *)mscb, mscb->pool_id);

	return;
}




/*
 *   first: free packet
 *   second: free mbuf
 */
void packet_destroy_all(mbuf_t *mbuf)
{
	cvmx_buf_ptr_t buffer_ptr;
	uint64_t start_of_buffer;

	
	/*free packet, find start of packet buffer*/
	if(PKTBUF_IS_HW(mbuf))
	{
		buffer_ptr = mbuf->packet_ptr;
		start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
	
		cvmx_fpa_free(cvmx_phys_to_ptr(start_of_buffer), buffer_ptr.s.pool, 0);
	}
	else if(PKTBUF_IS_SW(mbuf))
	{
		MEM_2OR8K_FREE(mbuf->pkt_ptr);
	}
	else
	{
		printf("pkt buffer region error\n");
	}
	
	/*free mbuf*/
	MBUF_FREE(mbuf);
}


/*only free packet*/
void packet_destroy_data(mbuf_t *mbuf)
{
	cvmx_buf_ptr_t buffer_ptr;
	uint64_t start_of_buffer;

	/*free packet, find start of packet buffer*/
	if(PKTBUF_IS_HW(mbuf))
	{
		buffer_ptr = mbuf->packet_ptr;
		start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
	
		cvmx_fpa_free(cvmx_phys_to_ptr(start_of_buffer), buffer_ptr.s.pool, 0);
	}
}


void packet_destory_rawdata(cvmx_buf_ptr_t buffer_ptr)
{
	uint64_t start_of_buffer;

	start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
	
	cvmx_fpa_free(cvmx_phys_to_ptr(start_of_buffer), buffer_ptr.s.pool, 0);
}






uint32_t packet_hw2sw(mbuf_t *mbuf)
{
	void *pkt_buf_sw;
	void *pkt_buf_hw;
	cvmx_buf_ptr_t cvmx_buffer_ptr;


	if(PKTBUF_IS_SW(mbuf))
	{
		return SEC_OK;
	}
	
	pkt_buf_sw = MEM_2K_ALLOC(mbuf->pkt_totallen);
	if(NULL == pkt_buf_sw)
	{
		return SEC_NO;
	}
	pkt_buf_hw = mbuf->pkt_ptr;

	memcpy((void *)pkt_buf_sw, (void *)pkt_buf_hw, mbuf->pkt_totallen);

	cvmx_buffer_ptr.u64 = mbuf->packet_ptr.u64;

	/*need adjuest the mbuf from hw2sw*/
	PKTBUF_SET_SW(mbuf);
	
	mbuf->pkt_ptr = pkt_buf_sw;

	packet_header_ptr_adjust(mbuf, pkt_buf_hw, pkt_buf_sw);

	mbuf->packet_ptr.u64 = 0;

	packet_destory_rawdata(cvmx_buffer_ptr);
	
	return SEC_OK;
}




