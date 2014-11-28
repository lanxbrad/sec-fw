#ifndef __DECODE_TCP_H__
#define __DECODE_TCP_H__


#define TCP_HEADER_LEN                       20
#define TCP_OPTLENMAX                        40
#define TCP_OPTMAX                           20 /* every opt is at least 2 bytes
                                                 * (type + len), except EOL and NOP */


typedef struct TCPHdr_
{
	uint16_t th_sport;  /**< source port */
	uint16_t th_dport;  /**< destination port */
	uint32_t th_seq;    /**< sequence number */
	uint32_t th_ack;    /**< acknowledgement number */
	uint8_t th_offx2;   /**< offset and reserved */
	uint8_t th_flags;   /**< pkt flags */
	uint16_t th_win;    /**< pkt window */
	uint16_t th_sum;    /**< checksum */
	uint16_t th_urp;    /**< urgent pointer */
} TCPHdr;


#define TCP_GET_RAW_OFFSET(tcph)             (((tcph)->th_offx2 & 0xf0) >> 4)
#define TCP_GET_RAW_SRC_PORT(tcph)           ((tcph)->th_sport)
#define TCP_GET_RAW_DST_PORT(tcph)           ((tcph)->th_dport)


#define TCP_GET_OFFSET(p)                    TCP_GET_RAW_OFFSET((TCPHdr *)((p)->transport_header))
#define TCP_GET_HLEN(p)                      (TCP_GET_OFFSET((p)) << 2)
#define TCP_GET_SRC_PORT(p)                  TCP_GET_RAW_SRC_PORT((TCPHdr *)((p)->transport_header))
#define TCP_GET_DST_PORT(p)                  TCP_GET_RAW_DST_PORT((TCPHdr *)((p)->transport_header))

#endif