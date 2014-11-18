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
	
	if (unlikely(len < ETHERNET_HEADER_LEN))
	{	
		STAT_L2_HEADER_ERR;
		return DECODE_DROP;
	}

	pethh = (EthernetHdr *)(pkt);
	mbuf->ethh = pethh;
	
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

