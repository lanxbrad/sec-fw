/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       decode-ethernet.c
 *        description :    ethernet layer decode
 *
 *        created by  luoye  at  2014-11-18
 *
 ********************************************************************************/

#include "sec-common.h"
#include <mbuf.h>
#include <sec-util.h>
#include "decode.h"


#include "decode-ethernet.h"
#include "decode-vlan.h"
#include "decode-ipv4.h"
#include "decode-statistic.h"




/*
  *   @mbuf:  
  *   @pkt:    start of l2 header
  *   @len:    len of l2 packet
  */
int DecodeEthernet(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	EthernetHdr *pethh;

#ifdef SEC_ETHERNET_DEBUG
	printf("==============>enter DecodeEthernet\n");
#endif
	
	if (unlikely(len < ETHERNET_HEADER_LEN))
	{	
		STAT_L2_HEADER_ERR;
		return DECODE_DROP;
	}

	pethh = (EthernetHdr *)(pkt);
	mbuf->ethh = pethh;

	if(pethh->eth_dst[0] != 0x88)
		return DECODE_DROP;
	
#ifdef SEC_ETHERNET_DEBUG
	printf("dst mac is %x:%x:%x:%x:%x:%x\n", 
		pethh->eth_dst[0],pethh->eth_dst[1],
		pethh->eth_dst[2],pethh->eth_dst[3],
		pethh->eth_dst[4],pethh->eth_dst[5]);

	printf("src mac is %x:%x:%x:%x:%x:%x\n", 
		pethh->eth_src[0],pethh->eth_src[1],
		pethh->eth_src[2],pethh->eth_src[3],
		pethh->eth_src[4],pethh->eth_src[5]);

	printf("eth type is 0x%x\n", pethh->eth_type);
#endif

	memcpy(mbuf->eth_dst, pethh->eth_dst, 6);
	memcpy(mbuf->eth_src, pethh->eth_src, 6);
	
	switch (pethh->eth_type) {
		case ETHERNET_TYPE_IP:
			STAT_L2_RECV_OK;
			return DecodeIPV4(mbuf, pkt + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
			
		case ETHERNET_TYPE_VLAN:
        case ETHERNET_TYPE_8021QINQ:
			STAT_L2_RECV_OK;
			return DecodeVLAN(mbuf, pkt + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);

		default:
			printf("ether type %04x not supported", pethh->eth_type);
			STAT_L2_UNSUPPORT;
			return DECODE_DROP;
	}

	return DECODE_OK;

}

