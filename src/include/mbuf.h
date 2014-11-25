/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       mbuf.h
 *        description :    
 *
 *        created by  luoye  at  2014-11-21
 *
 ********************************************************************************/

#ifndef __MBUF_H__
#define __MBUF_H__

#include <oct-common.h>
#include <sec-common.h>
#include <sec-util.h>
#include <mem_pool.h>


typedef struct {
	uint32_t sip;
	uint32_t dip;
}ipv4_tuple_t;


/*packet info description
 */
typedef struct m_buf_
{
	uint32_t magic_flag;         /* mbuf memory magic num*/

	uint16_t pkt_space;          /*pkt is hw or sw buffer*/
	uint16_t pkt_totallen;             /*pkt total len*/

	cvmx_buf_ptr_t packet_ptr;   /*copy from wqe packet_ptr*/

	struct m_buf_ *next;         /*for cache chain*/
	//struct m_buf_ *prev;
	
	void *pkt_ptr;                /*pointer to begin of packet from wqe packet_ptr*/
	
	void *ethh;                  /*l2 layer header*/
	void *vlanh;
	void *network_header;        /*network layer header*/
	void *transport_header;      /*transport layer header*/
	
	uint32_t input_port;         /*input port of phy*/
	
	uint8_t eth_dst[6];          /*DMAC*/
    uint8_t eth_src[6];          /*SMAC*/
	
	ipv4_tuple_t ipv4;           /*sip + dip*/

	uint16_t  sport;             /*sport */
	uint16_t  dport;             /*dport*/

	uint8_t proto;               /*protocol , now only support TCP + UDP */
	uint8_t vlan_idx;            /*if vlan exist, set vlan_idx = 1*/
	uint16_t payload_len;        /*L7 payload_len */

	uint16_t vlan_id;            /*if vlan_idx support, vlan_id*/
	uint16_t defrag_id;

	//uint16_t len;               /*move with decode process*/
	//void *   pkt;                  
  
	void *payload;            /*L7 payload pointer*/

	int frag_offset;             /*offset of ip fragment packet*/
	int frag_len;         /*len of ip fragment packet*/

	uint32_t flags;              /*features of packet*/

	uint32_t flow_hash;          /*hash value, used to find flow_node*/
	void    *flow;               /*flow node*/
	
}mbuf_t;


#define MBUF_MAGIC_NUM 0xab00ab00


#define PKTBUF_HW    1
#define PKTBUF_SW    2


#define MBUF_ALLOC()  mbuf_alloc()
#define MBUF_FREE(m)   mbuf_free(m)


static inline void mbuf_size_judge(void)
{
	BUILD_BUG_ON((sizeof(mbuf_t) + sizeof(Mem_Slice_Ctrl_B)) > MEM_POOL_HOST_MBUF_SIZE);

	return;
}



extern mbuf_t *mbuf_alloc();
extern void mbuf_free(mbuf_t *mb);
extern void packet_destroy_all(mbuf_t *m);
extern void packet_destroy_data(mbuf_t *mbuf);
extern uint32_t packet_hw2sw(mbuf_t *mbuf);


#define PACKET_DESTROY_ALL(m)   packet_destroy_all(m)
#define PACKET_DESTROY_DATA(m)  packet_destroy_data(m)


#endif
