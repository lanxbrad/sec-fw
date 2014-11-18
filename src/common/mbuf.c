#include <mbuf.h>
#include <mem_pool.h>

void mbuf_init(void)
{
	BUILD_BUG_ON((sizeof(mbuf_t)) > 256);

	return;
}


mbuf_t *mbuf_alloc()
{
	void *buf = mem_pool_fpa_slice_alloc(FPA_POOL_ID_HOST_MBUF);
	printf("mbuf magic is 0x%x\n", ((Mem_Slice_Ctrl_B *)buf)->magic);

	return (mbuf_t *)((uint8_t *)buf + sizeof(Mem_Slice_Ctrl_B));
}


void mbuf_free(mbuf_t *mb)
{
	Mem_Slice_Ctrl_B *mscb = (Mem_Slice_Ctrl_B *)((uint8_t *)mb - sizeof(Mem_Slice_Ctrl_B));
	if(MEM_POOL_MAGIC_NUM != mscb->magic)
	{
		return;
	}
	if(MEM_POOL_ID_HOST_MBUF != mscb->pool_id)
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
void packet_destroy(mbuf_t *mbuf)
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









