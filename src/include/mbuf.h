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
	
	void *ethh;           /*l2 layer header*/
	void *vlanh;
	void *network_header;        /*network layer header*/
	void *transport_header;      /*transport layer header*/

	ipv4_tuple_t ipv4;           /*sip + dip*/

	uint16_t  sport;             /*sport */
	uint16_t  dport;             /*dport*/

	uint8_t proto;               /*protocol , now only support TCP + UDP */
	uint8_t vlan_idx;            /*if vlan exist, set vlan_idx = 1*/
	uint16_t payload_len;        /*L7 payload_len */

	uint16_t vlan_id;            /*if vlan_idx support, vlan_id*/
  
    uint8_t *payload;            /*L7 payload pointer*/

	uint32_t offset;             /*offset of ip fragment packet*/
	uint32_t ip_fraglen;         /*len of ip fragment packet*/

	uint32_t flags;              /*features of packet*/

	uint32_t flow_hash;          /*hash value, used to find flow_node*/
	void    *flow;               /*flow node*/
	
}mbuf_t;


#define MBUF_MAGIC_NUM 0xab00ab00





extern mbuf_t *mbuf_alloc();
extern void mbuf_free(mbuf_t *mb);
extern void packet_destroy(mbuf_t *m);





#define MBUF_ALLOC()  mbuf_alloc()
#define MBUF_FREE(m)   mbuf_free(m)






#endif

