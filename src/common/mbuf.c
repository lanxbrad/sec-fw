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
 *   secode: free mbuf
 */
void packet_destroy_all(mbuf_t *mbuf)
{
	cvmx_buf_ptr_t buffer_ptr;
	uint64_t start_of_buffer;

	/*free packet, find start of packet buffer*/
	buffer_ptr = mbuf->packet_ptr;
	start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
	
	cvmx_fpa_free(cvmx_phys_to_ptr(start_of_buffer), buffer_ptr.s.pool, 0);

	/*free mbuf*/
	MBUF_FREE(mbuf);
}


/*only free packet*/
void packet_destroy_data(mbuf_t *mbuf)
{
	cvmx_buf_ptr_t buffer_ptr;
	uint64_t start_of_buffer;

	/*free packet, find start of packet buffer*/
	buffer_ptr = mbuf->packet_ptr;
	start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
	
	cvmx_fpa_free(cvmx_phys_to_ptr(start_of_buffer), buffer_ptr.s.pool, 0);
}



uint32_t packet_hw2sw(mbuf_t *mbuf)
{
	return SEC_OK;
}




