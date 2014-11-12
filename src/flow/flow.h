#ifndef __FLOW_H__
#define __FLOW_H__



#include <sec-common.h>
#include <oct-common.h>

#include <hlist.h>


#define FLOW_BUCKET_NUM  1024
#define FLOW_BUCKET_MASK  (FLOW_BUCKET_NUM - 1)  /*0X3FF*/


#define FLOW_ITEM_NUM   100000



#define FLOW_MAX_TIMEOUT    20   /* 60s */




typedef struct flow_table_info_tag_s
{
	uint32_t bucket_num;
	uint32_t bucket_size;

	uint32_t item_size;
	uint32_t item_num;

	void *bucket_base_ptr;
}flow_table_info_t;





typedef struct flow_bucket_tag_s
{
	struct hlist_head hash;
	cvmx_spinlock_t lock;
}flow_bucket_t;



typedef struct flow_item_tag_s
{
	struct hlist_node	list;
	cvmx_spinlock_t     item_lock;
	uint64_t cycle;

	ipv4_tuple_t ipv4;

	uint16_t  sport;
   	uint16_t  dport;
	uint16_t  protocol;

	
}flow_item_t;







#endif
