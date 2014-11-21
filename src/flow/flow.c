/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       flow.c
 *        description :    flow manage
 *
 *        created by  luoye  at  2014-11-18
 *
 ********************************************************************************/

#include <mbuf.h>
#include <decode.h>
#include <mem_pool.h>
#include <sec-common.h>
#include "flow.h"
#include "tluhash.h"


extern void l7_deliver(mbuf_t *m);





flow_table_info_t *flow_table;


void flow_item_size_judge(void)
{
	BUILD_BUG_ON((sizeof(flow_item_t) + sizeof(Mem_Slice_Ctrl_B)) > 256);

	return;
}



static inline flow_item_t *flow_item_alloc()
{
	void *buf = mem_pool_fpa_slice_alloc(FPA_POOL_ID_FLOW_NODE);
	
	return (flow_item_t *)((uint8_t *)buf + sizeof(Mem_Slice_Ctrl_B));
}

static inline void flow_item_free(flow_item_t *f)
{
	Mem_Slice_Ctrl_B *mscb = (Mem_Slice_Ctrl_B *)((uint8_t *)f - sizeof(Mem_Slice_Ctrl_B));
	if(MEM_POOL_MAGIC_NUM != mscb->magic)
	{
		printf("magic num err %d\n", mscb->magic);
		return;
	}
	if(FPA_POOL_ID_FLOW_NODE != mscb->pool_id)
	{
		printf("pool id err %d\n", mscb->pool_id);
		return;
	}

	mem_pool_fpa_slice_free((void *)mscb, FPA_POOL_ID_FLOW_NODE);

	return;
}




unsigned int flowhashfn(uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport, uint8_t prot)
{
	return flow_hashfn(prot, saddr, daddr, sport, dport) & FLOW_BUCKET_MASK;
}

static int FlowMatch(flow_item_t *f, mbuf_t *mbuf)
{
	return ((f->ipv4.sip   == mbuf->ipv4.sip
			&& f->ipv4.dip == mbuf->ipv4.dip
			&& f->sport    == mbuf->sport
			&& f->dport    == mbuf->dport
			&& f->protocol == mbuf->proto) 
		|| (f->ipv4.sip    == mbuf->ipv4.dip
			&& f->ipv4.dip == mbuf->ipv4.sip
			&& f->sport    == mbuf->dport
			&& f->dport    == mbuf->sport
			&& f->protocol == mbuf->proto));
}


static inline flow_item_t *FlowFind(flow_bucket_t *base, mbuf_t *mbuf, unsigned int hash)
{
	flow_item_t *f;
	struct hlist_node *n;
	
#ifdef SEC_FLOW_DEBUG
	printf("============>enter FlowFind\n");
#endif

	hlist_for_each_entry(f, n, &base[hash].hash, list)
	{
		if(FlowMatch(f, mbuf))
		{
		#ifdef SEC_FLOW_DEBUG
			printf("FlowMatch is ok\n");
		#endif
			return f;
		}
	}
#ifdef SEC_FLOW_DEBUG
	printf("FlowMatch is fail\n");
#endif
	return NULL;
}

flow_item_t *FlowAdd(flow_bucket_t *base, unsigned int hash, mbuf_t *mbuf)
{
#ifdef SEC_FLOW_DEBUG
	printf("==========>enter FlowAdd\n");
#endif

	flow_item_t *newf = flow_item_alloc();
	if(NULL == newf)
	{
		return NULL;
	}

	memset((void *)newf, 0, FLOW_ITEM_SIZE);

	/*TODO: Fill flow node with useful info*/
	newf->ipv4.sip = mbuf->ipv4.sip;
	newf->ipv4.dip = mbuf->ipv4.dip;
	newf->sport    = mbuf->sport;
	newf->dport    = mbuf->dport;
	newf->protocol = mbuf->proto;

	hlist_add_head(&newf->list, &base[hash].hash);

	return newf;
}



flow_item_t *FlowGetFlowFromHash(mbuf_t *mbuf)
{
	unsigned int hash;
	flow_item_t * flow;
	flow_bucket_t *base;

	
	base = (flow_bucket_t *)flow_table->bucket_base_ptr;

	hash = flowhashfn(mbuf->ipv4.sip, mbuf->ipv4.dip, mbuf->sport, mbuf->dport, mbuf->proto);

#ifdef SEC_FLOW_DEBUG
	printf("hash value is %d\n", hash);
#endif

	cvmx_spinlock_lock(&base[hash].lock);

	flow = FlowFind(base, mbuf, hash);
	if(NULL == flow)
	{
	#ifdef SEC_FLOW_DEBUG
		printf("flow has not been found\n");
	#endif
		flow = FlowAdd(base, hash, mbuf);
	}

	if(NULL != flow)
	{
		cvmx_spinlock_lock(&flow->item_lock);
	}
	
	cvmx_spinlock_unlock(&base[hash].lock);

	flow->cycle = cvmx_get_cycle();
	
	return flow;
}



void FlowHandlePacket(mbuf_t *m)
{
	flow_item_t *f;

#ifdef SEC_FLOW_DEBUG
	printf("=========>enter FlowHandlePacket\n");
#endif
	f = FlowGetFlowFromHash(m);
	if(NULL == f)
	{
		/*flow failed, destroy packet*/
		packet_destroy(m);
		return;
	}

	/*TODO:  update info in the flow*/
	
	m->flow = (void *)f;
	f->input_port = m->input_port;

	cvmx_spinlock_unlock(&f->item_lock);

	/* set the flow in the packet */
	m->flags |= PKT_HAS_FLOW;

	l7_deliver(m);

	return;
}




void FlowAgeTimeoutCB(Oct_Timer_Threat *o, void *param)
{
	int i = 0;
	uint64_t current_cycle;

	flow_bucket_t *base = NULL;
	
	flow_item_t *f;
	flow_item_t *tf;
	struct hlist_node *n;
	struct hlist_node *t;
	struct hlist_head timeout;

	base = (flow_bucket_t *)flow_table->bucket_base_ptr;
	
	current_cycle = cvmx_get_cycle();

	for(i = 0; i < FLOW_BUCKET_NUM; i++)
	{
		INIT_HLIST_HEAD(&timeout);
		
		cvmx_spinlock_lock(&base[i].lock);

		hlist_for_each_entry_safe(f, t, n, &base[i].hash, list)	
		{
			if((current_cycle > f->cycle) && ((current_cycle - f->cycle) > FLOW_MAX_TIMEOUT))
			{
				hlist_del(&f->list);
				
				hlist_add_head(&f->list, &timeout);
			}
		}
		
		cvmx_spinlock_unlock(&base[i].lock);

		hlist_for_each_entry_safe(tf, t, n, &timeout, list)	
		{	
			hlist_del(&tf->list);

			/*TODO: session ageing do something*/
			
			flow_item_free(tf);
		}
		
	}

	return;
}




int FlowInit(void)
{
	int i = 0;

	flow_bucket_t *base = NULL;

	flow_table = (flow_table_info_t *)cvmx_bootmem_alloc_named((sizeof(flow_table_info_t) + FLOW_BUCKET_NUM * FLOW_BUCKET_SIZE), CACHE_LINE_SIZE, FLOW_HASH_TABLE_NAME);
	if(NULL == flow_table)
	{
		printf("flow init: no memory\n");
		return SEC_NO;
	}

	flow_table->bucket_num = FLOW_BUCKET_NUM;
	flow_table->bucket_size = FLOW_BUCKET_SIZE;

	flow_table->item_num = FLOW_ITEM_NUM;
	flow_table->item_size = FLOW_ITEM_SIZE;

	flow_table->bucket_base_ptr = (void *)((uint8_t *)flow_table + sizeof(flow_table_info_t));
	

	base = (flow_bucket_t *)flow_table->bucket_base_ptr;
	
	for(i = 0; i < FLOW_BUCKET_NUM; i++)
	{
		INIT_HLIST_HEAD(&base[i].hash);
		cvmx_spinlock_init(&base[i].lock);
	}

	if(OCT_Timer_Create(0xFFFFFF, 0, 2, TIMER_GROUP, FlowAgeTimeoutCB, NULL, 0, 1000))/*1s*/
	{
		printf("timer create fail\n");
		return SEC_NO;
	}

	printf("flow age timer create ok\n");
	
	return SEC_OK;
}


int FlowInfoGet()
{
	flow_table = (flow_table_info_t *)cvmx_bootmem_find_named_block(FLOW_HASH_TABLE_NAME);
	if(NULL == flow_table)
	{
		printf("FlowInfoGet fail\n");
		return SEC_NO;
	}

	return SEC_OK;
}


