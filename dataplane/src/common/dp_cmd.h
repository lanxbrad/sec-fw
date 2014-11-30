#ifndef __DP_CMD_H__
#define __DP_CMD_H__

#include <oct-common.h>
#include <rpc-common.h>
#include <oct-init.h>
#include <oct-rxtx.h>


extern void oct_rx_process_command(cvmx_wqe_t *wq);
extern void oct_send_response(cvmx_wqe_t *work, uint16_t opcode, void *data, uint32_t size);




#endif
