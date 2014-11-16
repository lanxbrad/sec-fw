#ifndef __DECODE_STATISTIC_H__
#define __DECODE_STATISTIC_H__

#include <sec-common.h>
#include <oct-common.h>


struct recv_count 
{
	uint64_t recv_packet_count;
	uint64_t recv_packet_bytes;

	uint64_t recv_packet_count_sum;
	uint64_t recv_packet_bytes_sum;
};

struct rx_stat
{
	uint64_t rx_err;
	uint64_t rx_ok;
};


typedef struct
{
	struct recv_count rc;
	struct rx_stat rxstat;
}pkt_stat;




extern pkt_stat *pktstat[];

extern int local_cpu_id;

#define STAT_RECV_PC_ADD 		do { pktstat[local_cpu_id]->rc.recv_packet_count++; pktstat[local_cpu_id]->rc.recv_packet_count_sum++; } while (0)
#define STAT_RECV_PB_ADD(bytes) do { pktstat[local_cpu_id]->rc.recv_packet_bytes += bytes; pktstat[local_cpu_id]->rc.recv_packet_bytes_sum += bytes; } while (0)


#define STAT_RECV_ERR  do { pktstat[local_cpu_id]->rxstat.rx_err++; } while(0)
#define STAT_RECV_OK   do { pktstat[local_cpu_id]->rxstat.rx_ok++;} while(0)





extern int Decode_PktStat_Init();
extern int Decode_PktStat_Get();

#endif
