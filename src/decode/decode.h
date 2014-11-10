#ifndef __DECODE_H__
#define __DECODE_H__

#include <sec-common.h>


#define DECODE_OK   0  
#define DECODE_DROP 1    
#define DECODE_DONE 2






#define GET_PKT_LEN(p) ((p)->pktlen)
#define GET_PKT_DATA(p) ((p)->pktptr)


/*Given a packet pkt offset to the start of the ip header in a packet
 *We determine the ip version. */
#define IP_GET_RAW_VER(pkt) ((((pkt)[0] & 0xf0) >> 4))





#endif
