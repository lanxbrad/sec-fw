#ifndef __DECODE_H__
#define __DECODE_H__

#include <sec-common.h>
#include <sec-decode.h>


#define DECODE_OK   0  
#define DECODE_DROP 1    
#define DECODE_DONE 2






#define GET_PKT_LEN(p) ((p)->pktlen)
#define GET_PKT_DATA(p) ((p)->pktptr)




#endif
