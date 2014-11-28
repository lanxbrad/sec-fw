#include <sec-common.h>
#include <sec-util.h>
#include <mbuf.h>
#include "decode-ethernet.h"
#include "decode-vlan.h"
#include "decode-ipv4.h"
#include "decode-statistic.h"
#include "decode-defrag.h"

extern int DecodeTCP(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);
extern int DecodeUDP(mbuf_t *mbuf, uint8_t *pkt, uint16_t len);







static int DecodeIPV4Packet(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	
	if (unlikely(len < IPV4_HEADER_LEN)) {
		STAT_IPV4_HEADER_ERR;
        return DECODE_DROP;
    }

	if (unlikely(IP_GET_RAW_VER(pkt) != 4)) {
		STAT_IPV4_VERSION_ERR;
        return DECODE_DROP;
    }

	mbuf->network_header = (void *)pkt;

	if (unlikely(IPV4_GET_HLEN(mbuf) < IPV4_HEADER_LEN)) {
		STAT_IPV4_HEADER_ERR;
        return DECODE_DROP;
    }

	if (unlikely(IPV4_GET_IPLEN(mbuf) < IPV4_GET_HLEN(mbuf))) {
		STAT_IPV4_LEN_ERR;
        return DECODE_DROP;
    }

    if (unlikely(len < IPV4_GET_IPLEN(mbuf))) {
		STAT_IPV4_LEN_ERR;
        return DECODE_DROP;
    }

	mbuf->ipv4.sip = IPV4_GET_IPSRC(mbuf);
	mbuf->ipv4.dip = IPV4_GET_IPDST(mbuf);

#ifdef SEC_IPV4_DEBUG
	printf("sip is 0x%x\n", mbuf->ipv4.sip);
	printf("dip is 0x%x\n", mbuf->ipv4.dip);
#endif

	/*TODO: DecodeIPV4Options*/

	return DECODE_OK;
}


int DecodeIPV4(mbuf_t *mbuf, uint8_t *pkt, uint16_t len)
{
	uint8_t protocol;
	int ihl;
	mbuf_t *nmbuf;

#ifdef SEC_IPV4_DEBUG
	printf("=========>enter DecodeIPV4()\n");
#endif

	if (unlikely(DECODE_OK != DecodeIPV4Packet (mbuf, pkt, len))) {
		return DECODE_DROP;
	}

	protocol = IPV4_GET_IPPROTO(mbuf);
	mbuf->proto = protocol;
	
	nmbuf = mbuf; /*maybe cache or not , so switch it*/

	/* If a fragment, pass off for re-assembly. */
	if(IPV4_IS_FRAGMENT(mbuf))
	{
	
	#ifdef SEC_DEFRAG_DEBUG
		printf("this is a fragment\n");
	#endif
	
		ihl = IPV4_GET_HLEN(mbuf);
		mbuf->defrag_id = IPV4_GET_IPID(mbuf);
		mbuf->frag_offset = IPV4_GET_IPOFFSET(mbuf) << 3;
		mbuf->frag_len = len - ihl;
		
	#ifdef SEC_DEFRAG_DEBUG
		printf("frag offset is %d, frag len is %d\n", mbuf->frag_offset, mbuf->frag_len);
	#endif
	
		if(0 == mbuf->frag_len)
		{
			return DECODE_DROP;
		}
		
		nmbuf = Defrag(mbuf);	
		if(NULL == nmbuf)
		{
			return DECODE_OK;
		}
	}

#ifdef SEC_IPV4_DEBUG
	printf("protocol is %d\n", nmbuf->proto);
#endif
	

	/* check what next decoder to invoke */
	switch (nmbuf->proto) {
		case PROTO_TCP:
			STAT_IPV4_RECV_OK;
			if(DECODE_OK != DecodeTCP(nmbuf, 
									(void *)((uint8_t *)(nmbuf->network_header)+ IPV4_GET_HLEN(nmbuf)), 
									IPV4_GET_IPLEN(nmbuf) - IPV4_GET_HLEN(nmbuf))){
				PACKET_DESTROY_ALL(nmbuf);
			}
			return DECODE_OK;	
		case PROTO_UDP:
			STAT_IPV4_RECV_OK;
			if(DECODE_OK != DecodeUDP(nmbuf, 
								(void *)((uint8_t *)(nmbuf->network_header)+ IPV4_GET_HLEN(nmbuf)), 
								IPV4_GET_IPLEN(nmbuf) - IPV4_GET_HLEN(nmbuf))){
				PACKET_DESTROY_ALL(nmbuf);
			}
			return DECODE_OK;
		default:
		#ifdef SEC_IPV4_DEBUG
			printf("unsupport protocol %d\n",IPV4_GET_IPPROTO(nmbuf));
		#endif
			STAT_IPV4_UNSUPPORT;
			PACKET_DESTROY_ALL(nmbuf);
			return DECODE_OK;
	}

	return DECODE_OK;
}

