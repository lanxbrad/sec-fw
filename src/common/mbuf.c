#include <mbuf.h>


void mbuf_init(void)
{
	BUILD_BUG_ON((sizeof(mbuf_t)) > 256);

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

	
	buffer_ptr = mbuf->packet_ptr;
	start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
	
	cvmx_fpa_free(cvmx_phys_to_ptr(start_of_buffer), buffer_ptr.s.pool, 0);

	mbuf_free(mbuf);
}









