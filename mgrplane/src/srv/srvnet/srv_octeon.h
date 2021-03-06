#ifndef __SRV_OCTEON_H__
#define __SRV_OCTEON_H__

#include <common.h>

extern int octeon_show_test_command(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int octeon_show_dp_build_time(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int octeon_show_dp_pkt_stat(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
extern int octeon_show_mem_pool(uint8_t * from, uint32_t length, uint32_t fd, void *param_p);
#endif
