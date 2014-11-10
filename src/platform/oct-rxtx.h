#ifndef __OCT_RXTX_H__
#define __OCT_RXTX_H__

#include <oct-common.h>



static inline void
oct_packet_free(cvmx_wqe_t * wq, int wqepool)
{
	cvmx_helper_free_packet_data(wq);
	cvmx_fpa_free(wq, wqepool, 0);
}

#endif