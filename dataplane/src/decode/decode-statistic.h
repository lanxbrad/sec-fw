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

struct vlan_stat
{
	uint64_t headerlen_err;
	uint64_t vlanlayer_exceed;
	uint64_t unsupport;
	uint64_t rx_ok;
};

struct ipv4_stat
{
	uint64_t headerlen_err;
	uint64_t version_err;
	uint64_t pktlen_err;
	uint64_t unsupport;
	uint64_t rx_ok;
};

struct frag_stat
{
	uint64_t fraglen_err;
	uint64_t fcb_no;
	uint64_t hw2sw_err;
	uint64_t defrag_err;
	uint64_t cache_ok;
	uint64_t reasm_ok;
};

struct udp_stat
{
	uint64_t headerlen_err;
	uint64_t pktlen_err;
	uint64_t rx_ok;
};

struct tcp_stat
{
	uint64_t headerlen_err;
	uint64_t pktlen_err;
	uint64_t rx_ok;
};

struct flow_stat
{
	uint64_t getnode_err;
	uint64_t proc_ok;
};

struct tx_stat
{
	uint64_t port_err;
	uint64_t hw_send_err;
	uint64_t sw_desc_err;
	uint64_t sw_send_err;
	uint64_t send_over;
};

typedef struct
{
	struct recv_count rc;
	struct rx_stat    rxstat;
	struct ether_stat l2stat;
	struct vlan_stat  vlanstat;
	struct ipv4_stat  ipv4stat;
	struct frag_stat  fragstat;
	struct udp_stat   udpstat;
	struct tcp_stat   tcpstat;
	struct flow_stat  flowstat;
	struct tx_stat    txstat;
}pkt_stat;


#define PKT_STAT_MEM_NAME "pkt-statistic"


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

#define STAT_VLAN_HEADER_ERR     do { pktstat[local_cpu_id]->vlanstat.headerlen_err++;} while(0)
#define STAT_VLAN_LAYER_EXCEED   do { pktstat[local_cpu_id]->vlanstat.vlanlayer_exceed++;} while(0)
#define STAT_VLAN_UNSUPPORT      do { pktstat[local_cpu_id]->vlanstat.unsupport++;} while(0)
#define STAT_VLAN_RECV_OK        do { pktstat[local_cpu_id]->vlanstat.rx_ok++;} while(0)


#define STAT_IPV4_HEADER_ERR     do { pktstat[local_cpu_id]->ipv4stat.headerlen_err++;} while(0)
#define STAT_IPV4_VERSION_ERR    do { pktstat[local_cpu_id]->ipv4stat.version_err++;} while(0)
#define STAT_IPV4_LEN_ERR        do { pktstat[local_cpu_id]->ipv4stat.pktlen_err++;} while(0)
#define STAT_IPV4_UNSUPPORT      do { pktstat[local_cpu_id]->ipv4stat.unsupport++;} while(0)
#define STAT_IPV4_RECV_OK        do { pktstat[local_cpu_id]->ipv4stat.rx_ok++;} while(0)

#define STAT_FRAG_LEN_ERR        do { pktstat[local_cpu_id]->fragstat.fraglen_err++; } while(0)
#define STAT_FRAG_FCB_NO         do { pktstat[local_cpu_id]->fragstat.fcb_no++; } while(0)
#define STAT_FRAG_HW2SW_ERR      do { pktstat[local_cpu_id]->fragstat.hw2sw_err++; } while(0)
#define STAT_FRAG_DEFRAG_ERR     do { pktstat[local_cpu_id]->fragstat.defrag_err++; } while(0)
#define STAT_FRAG_REASM_OK       do { pktstat[local_cpu_id]->fragstat.reasm_ok++; } while(0)
#define STAT_FRAG_CACHE_OK       do { pktstat[local_cpu_id]->fragstat.cache_ok++; } while(0)


#define STAT_UDP_HEADER_ERR      do { pktstat[local_cpu_id]->udpstat.headerlen_err++;} while(0)
#define STAT_UDP_LEN_ERR         do { pktstat[local_cpu_id]->udpstat.pktlen_err++;} while(0)
#define STAT_UDP_RECV_OK         do { pktstat[local_cpu_id]->udpstat.rx_ok++;} while(0)


#define STAT_TCP_HEADER_ERR      do { pktstat[local_cpu_id]->tcpstat.headerlen_err++;} while(0)
#define STAT_TCP_LEN_ERR         do { pktstat[local_cpu_id]->tcpstat.pktlen_err++;} while(0)
#define STAT_TCP_RECV_OK         do { pktstat[local_cpu_id]->tcpstat.rx_ok++;} while(0)


#define STAT_FLOW_GETNODE_ERR    do { pktstat[local_cpu_id]->flowstat.getnode_err++;} while(0)
#define STAT_FLOW_PROC_OK        do { pktstat[local_cpu_id]->flowstat.proc_ok++;} while(0)

#define STAT_TX_SEND_PORT_ERR    do { pktstat[local_cpu_id]->txstat.port_err++;} while(0)
#define STAT_TX_HW_SEND_ERR      do { pktstat[local_cpu_id]->txstat.hw_send_err++;} while(0)
#define STAT_TX_SW_DESC_ERR      do { pktstat[local_cpu_id]->txstat.sw_desc_err++;} while(0)
#define STAT_TX_SW_SEND_ERR      do { pktstat[local_cpu_id]->txstat.sw_send_err++;} while(0)
#define STAT_TX_SEND_OVER        do { pktstat[local_cpu_id]->txstat.send_over++;} while(0)



extern int Decode_PktStat_Init();
extern int Decode_PktStat_Get();

#endif
