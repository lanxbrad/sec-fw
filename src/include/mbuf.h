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
#include <sec-util.h>


typedef struct {
	uint32_t sip;
	uint32_t dip;
}ipv4_tuple_t;


/*packet info description
 * using wqe 96 bytes space
 * so this struct size must be less than**** 96 bytes ****
 */
typedef struct m_buf_
{
	uint32_t magic_flag;         /* mbuf memory magic num*/
	uint32_t pktlen;             /*pkt total len*/

	cvmx_buf_ptr_t packet_ptr;   /*copy from wqe packet_ptr*/

	struct m_buf_ *next;         /*for cache chain*/
	struct m_buf_ *prev;
	
	void *pktptr;                /*pointer to begin of packet from wqe packet_ptr*/
	
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
  
    uint8_t *payload;            /*L7 payload pointer*/

	uint32_t offset;             /*offset of ip fragment packet*/
	uint32_t ip_fraglen;         /*len of ip fragment packet*/

	uint32_t flags;              /*features of packet*/

	uint32_t flow_hash;          /*hash value, used to find flow_node*/
	void    *flow;               /*flow node*/
	
}mbuf_t;


#define MBUF_MAGIC_NUM 0xab00ab00


#define MBUF_ALLOC()  mbuf_alloc()
#define MBUF_FREE(m)   mbuf_free(m)



extern mbuf_t *mbuf_alloc();
extern void mbuf_free(mbuf_t *mb);
extern void packet_destroy_all(mbuf_t *m);
extern void packet_destroy_data(mbuf_t *mbuf);


#define PACKET_DESTROY_ALL(m)   packet_destroy_all(m)
#define PACKET_DESTROY_DATA(m)  packet_destroy_data(m)


#endif
