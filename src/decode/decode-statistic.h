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
	uint64_t addr_err;
	uint64_t rx_ok;
};

struct ether_stat
{
	uint64_t headerlen_err;
	uint64_t unsupport;
	uint64_t rx_ok;
};



typedef struct
{
	struct recv_count rc;
	struct rx_stat rxstat;
	struct ether_stat l2stat;
}pkt_stat;






extern pkt_stat *pktstat[];

extern int local_cpu_id;

#define STAT_RECV_PC_ADD 		do { pktstat[local_cpu_id]->rc.recv_packet_count++; pktstat[local_cpu_id]->rc.recv_packet_count_sum++; } while (0)
#define STAT_RECV_PB_ADD(bytes) do { pktstat[local_cpu_id]->rc.recv_packet_bytes += bytes; pktstat[local_cpu_id]->rc.recv_packet_bytes_sum += bytes; } while (0)


#define STAT_RECV_ERR      do { pktstat[local_cpu_id]->rxstat.rx_err++; } while(0)
#define STAT_RECV_ADDR_ERR do { pktstat[local_cpu_id]->rxstat.addr_err++; } while(0)
#define STAT_RECV_OK       do { pktstat[local_cpu_id]->rxstat.rx_ok++;} while(0)

#define STAT_L2_HEADER_ERR do { pktstat[local_cpu_id]->l2stat.headerlen_err++;} while(0)
#define STAT_L2_UNSUPPORT  do { pktstat[local_cpu_id]->l2stat.unsupport++;} while(0)
#define STAT_L2_RECV_OK    do { pktstat[local_cpu_id]->l2stat.rx_ok++;} while(0)





extern int Decode_PktStat_Init();
extern int Decode_PktStat_Get();

#endif
