/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       flow.h
 *        description :    flow manage
 *
 *        created by  luoye  at  2014-11-18
 *
 ********************************************************************************/

#ifndef __FLOW_H__
#define __FLOW_H__

#include <sec-common.h>
#include <oct-common.h>
#include <oct-init.h>
#include <hlist.h>
#include <mem_pool.h>



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

	uint16_t  input_port;
	
}flow_item_t;




#define FLOW_BUCKET_NUM  65536
#define FLOW_BUCKET_MASK  (FLOW_BUCKET_NUM - 1)  /*0xFFFF*/

#define FLOW_BUCKET_SIZE sizeof(flow_bucket_t)


#define FLOW_ITEM_NUM   100000
#define FLOW_ITEM_SIZE sizeof(flow_item_t)

#define FLOW_HASH_TABLE_NAME   "Flow_Hash_Table"

#define FLOW_MAX_TIMEOUT    20*oct_cpu_rate   /* 20s */

#define FLOW_UPDATE_TIMESTAMP(m)  (m->cycle = cvmx_get_cycle())



static void flow_item_size_judge(void)
{
	BUILD_BUG_ON((sizeof(flow_item_t) + sizeof(Mem_Slice_Ctrl_B)) > 256);

	return;
}


#define FLOW_TABLE_LOCK(f)     cvmx_spinlock_lock(&f->lock)
#define FLOW_TABLE_UNLOCK(f)   cvmx_spinlock_unlock(&f->lock)



extern int FlowInit(void);
extern int FlowInfoGet();
extern void FlowAgeTimeoutCB(Oct_Timer_Threat *o, void *param);


#endif
