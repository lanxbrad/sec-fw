#ifndef __DECODE_H__
#define __DECODE_H__

#include <sec-common.h>


#define DECODE_OK   0  
#define DECODE_DROP 1    
#define DECODE_DONE 2


#define PKT_IP_FRAG                     (1<<1)
#define PKT_FIRST_FRAG                  (1<<2)
#define PKT_HAS_FLOW                    (1<<8)



#define PKT_SET_IP_FRAG(m) (m->flags |= PKT_IP_FRAG)
#define PKT_IS_IP_FRAG(m) (m->flags & PKT_IP_FRAG)

#define PKT_SET_FIRST_FRAG(m) (m->flags |= PKT_FIRST_FRAG)
#define PKT_IS_FIRST_FRAG(m) (m->flags & PKT_FIRST_FRAG)



#define GET_PKT_LEN(p) ((p)->pktlen)
#define GET_PKT_DATA(p) ((p)->pktptr)


/*Given a packet pkt offset to the start of the ip header in a packet
 *We determine the ip version. */
#define IP_GET_RAW_VER(pkt) ((((pkt)[0] & 0xf0) >> 4))





#endif
