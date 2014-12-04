#ifndef __DP_CMD_H__
#define __DP_CMD_H__

#include <oct-common.h>
#include <rpc-common.h>
#include <oct-init.h>
#include <oct-rxtx.h>

#include <sos_malloc.h>
#include <mem_pool.h>

extern sos_mem_pool_region_t *sos_mem_pool;
extern void oct_rx_process_command(cvmx_wqe_t *wq);




#endif
