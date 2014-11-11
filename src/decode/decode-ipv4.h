#ifndef __DECODE_IPV4_H__
#define __DECODE_IPV4_H__


#include "decode.h"


/* IP flags. */
#define IP_CE		0x8000		/* Flag: "Congestion"		*/
#define IP_DF		0x4000		/* Flag: "Don't Fragment"	*/
#define IP_MF		0x2000		/* Flag: "More Fragments"	*/
#define IP_OFFSET	0x1FFF		/* "Fragment Offset" part	*/


#define IPV4_HEADER_LEN           20    /**< Header length */

typedef struct {
	uint32_t sip;
	uint32_t dip;
}ipv4_tuple_t;


typedef struct IPV4Hdr_
{
	uint8_t ip_verhl;     /**< version & header length */
	uint8_t ip_tos;       /**< type of service */
	uint16_t ip_len;      /**< length */
	uint16_t ip_id;       /**< id */
	uint16_t ip_off;      /**< frag offset */
	uint8_t ip_ttl;       /**< time to live */
	uint8_t ip_proto;     /**< protocol (tcp, udp, etc) */
	uint16_t ip_csum;     /**< checksum */
	uint32_t src_addr;		/**< source address */
	uint32_t dst_addr;		/**< destination address */
} IPV4Hdr;


#define IPV4_GET_RAW_VER(ip4h)            (((ip4h)->ip_verhl & 0xf0) >> 4)
#define IPV4_GET_RAW_HLEN(ip4h)           ((ip4h)->ip_verhl & 0x0f)
#define IPV4_GET_RAW_IPTOS(ip4h)          ((ip4h)->ip_tos)
#define IPV4_GET_RAW_IPLEN(ip4h)          ((ip4h)->ip_len)
#define IPV4_GET_RAW_IPID(ip4h)           ((ip4h)->ip_id)
#define IPV4_GET_RAW_IPOFFSET(ip4h)       ((ip4h)->ip_off)
#define IPV4_GET_RAW_IPTTL(ip4h)          ((ip4h)->ip_ttl)
#define IPV4_GET_RAW_IPPROTO(ip4h)        ((ip4h)->ip_proto)
#define IPV4_GET_RAW_IPSRC(ip4h)          ((ip4h)->src_addr)
#define IPV4_GET_RAW_IPDST(ip4h)          ((ip4h)->dst_addr)

/* ONLY call these functions after making sure that:
 * 1. p->ip4h is set
 * 2. p->ip4h is valid (len is correct)
 */
#define IPV4_GET_VER(p) \
    IPV4_GET_RAW_VER((p)->ip4h)
#define IPV4_GET_HLEN(p) \
    (IPV4_GET_RAW_HLEN((p)->ip4h) << 2)
#define IPV4_GET_IPTOS(p) \
    IPV4_GET_RAW_IPTOS((p)->ip4h)
#define IPV4_GET_IPLEN(p) \
    IPV4_GET_RAW_IPLEN((p)->ip4h)
#define IPV4_GET_IPSRC(p) \
	(IPV4_GET_RAW_IPSRC((p)->ip4h))
#define IPV4_GET_IPDST(p) \
	(IPV4_GET_RAW_IPDST((p)->ip4h))
#define IPV4_GET_IPID(p) \
   (IPV4_GET_RAW_IPID((p)->ip4h))
/* _IPV4_GET_IPOFFSET: get the content of the offset header field in host order */
#define _IPV4_GET_IPOFFSET(p) \
    IPV4_GET_RAW_IPOFFSET((p)->ip4h)
/* IPV4_GET_IPOFFSET: get the final offset */
#define IPV4_GET_IPOFFSET(p) \
    (_IPV4_GET_IPOFFSET(p) & 0x1fff)
/* IPV4_GET_RF: get the RF flag. Use _IPV4_GET_IPOFFSET to save a ntohs call. */
#define IPV4_GET_RF(p) \
    (uint8_t)((_IPV4_GET_IPOFFSET((p)) & 0x8000) >> 15)
/* IPV4_GET_DF: get the DF flag. Use _IPV4_GET_IPOFFSET to save a ntohs call. */
#define IPV4_GET_DF(p) \
    (uint8_t)((_IPV4_GET_IPOFFSET((p)) & 0x4000) >> 14)
/* IPV4_GET_MF: get the MF flag. Use _IPV4_GET_IPOFFSET to save a ntohs call. */
#define IPV4_GET_MF(p) \
    (uint8_t)((_IPV4_GET_IPOFFSET((p)) & 0x2000) >> 13)
#define IPV4_GET_IPTTL(p) \
     IPV4_GET_RAW_IPTTL(p->ip4h)
#define IPV4_GET_IPPROTO(p) \
    IPV4_GET_RAW_IPPROTO((p)->ip4h)





#define PROTO_TCP        6  /**< tcp */

#define PROTO_UDP       17  /**< user datagram protocol */












#endif