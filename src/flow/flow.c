#include <mbuf.h>
#include "flow.h"
#include "tluhash.h"

extern void l7_deliver(mbuf_t *m);





flow_table_info_t flow_table;

flow_item_t *flow_item_alloc()
{
	return NULL;
}

void session_item_free(flow_item_t *f)
{
	return;
}



int flow_item_init()
{
	return SEC_OK;
}



unsigned int flowhashfn(uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport, uint8_t prot)
{
	return flow_hashfn(prot, saddr, daddr, sport, dport) & FLOW_BUCKET_MASK;
}

static int flow_match(flow_item_t *f, mbuf_t *mbuf)
{
	return ((f->ipv4.sip == mbuf->ipv4.sip
		&& f->ipv4.dip == mbuf->ipv4.dip
		&& f->sport == mbuf->sport
		&& f->dport == mbuf->dport
		&& f->protocol == mbuf->proto) 
		|| (f->ipv4.sip == mbuf->ipv4.dip
		&& f->ipv4.dip == mbuf->ipv4.sip
		&& f->sport == mbuf->dport
		&& f->dport == mbuf->sport
		&& f->protocol == mbuf->proto));
}


flow_item_t *flow_find(flow_bucket_t *base, mbuf_t *mbuf, unsigned int hash)
{
	flow_item_t *f;
	struct hlist_node *n;

	hlist_for_each_entry(f, n, &base[hash].hash, list)
	{
		if(flow_match(f, mbuf))
		{
			return f;
		}
	}
	
	return NULL;
}

flow_item_t *flow_add(flow_bucket_t *base, unsigned int hash, mbuf_t *mbuf)
{
	flow_item_t *newf = flow_item_alloc();
	if(NULL == newf)
	{
		return NULL;
	}

	memset((void *)newf, 0, sizeof(flow_item_t));

	/*TODO: Fill flow node with useful info*/
	newf->ipv4.sip = mbuf->ipv4.sip;
	newf->ipv4.dip = mbuf->ipv4.dip;
	newf->sport = mbuf->sport;
	newf->dport = mbuf->dport;
	newf->protocol = mbuf->proto;

	hlist_add_head(&newf->list, &base[hash].hash);

	return newf;
}



flow_item_t *FlowGetFlowFromHash(mbuf_t *mbuf)
{
	unsigned int hash;
	flow_item_t * flow;
	flow_bucket_t *base;
	base = (flow_bucket_t *)flow_table.bucket_base_ptr;

	
	hash = flowhashfn(mbuf->ipv4.sip, mbuf->ipv4.dip, mbuf->sport, mbuf->dport, mbuf->proto);
	
	cvmx_spinlock_lock(&base[hash].lock);

	flow = flow_find(base, mbuf, hash);
	if(NULL == flow)
	{
		flow = flow_add(base, hash, mbuf);
	}

	if(NULL != flow)
	{
		cvmx_spinlock_lock(&flow->item_lock);
	}
	
	cvmx_spinlock_unlock(&base[hash].lock);
	
	return flow;
}



void FlowHandlePacket(mbuf_t *m)
{
	flow_item_t *f;

	f = FlowGetFlowFromHash(m);
	if(NULL == f)
	{
		/*flow failed, destroy packet*/
		packet_destroy(m);
		return;
	}

	/*TODO:  update info in the flow*/
	
	m->flow = (void *)f;

	cvmx_spinlock_unlock(&f->item_lock);

	/* set the flow in the packet */
	m->flags |= PKT_HAS_FLOW;

	l7_deliver(m);

	return;
}




static void flow_age_timeout_cb()
{
	int i = 0;
	uint64_t current_cycle;

	flow_bucket_t *base = NULL;
	
	flow_item_t *f;
	flow_item_t *tf;
	struct hlist_node *n;
	struct hlist_node *t;
	struct hlist_head timeout;

	base = (flow_bucket_t *)flow_table.bucket_base_ptr;
	
	//current_cycle = utaf_get_timer_cycles();

	for(i = 0; i < FLOW_BUCKET_NUM; i++)
	{
		INIT_HLIST_HEAD(&timeout);
		
		cvmx_spinlock_lock(&base[i].lock);

		//hlist_for_each_entry(si, n, &base[i].hash, list) 
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
			
			session_item_free(tf);
		}
		
	}

	return;
}







int flow_init(void)
{
	int i = 0;

	flow_bucket_t *base = NULL;

	flow_table.bucket_num = FLOW_BUCKET_NUM;
	flow_table.bucket_size = sizeof(flow_bucket_t);

	flow_table.item_num = FLOW_ITEM_NUM;
	flow_table.item_size = sizeof(flow_item_t);

	flow_table.bucket_base_ptr = (void *)malloc(flow_table.bucket_num * flow_table.bucket_size);
	if(flow_table.bucket_base_ptr == NULL)
	{
		printf("no memory\n");
		return SEC_NO;
	}
	printf("flow_item_init\n");
	if(SEC_NO != flow_item_init())
	{
		return SEC_NO;
	}

	base = (flow_bucket_t *)flow_table.bucket_base_ptr;
	
	for(i = 0; i < FLOW_BUCKET_NUM; i++)
	{
		INIT_HLIST_HEAD(&base[i].hash);
		cvmx_spinlock_init(&base[i].lock);
	}

	/*TODO: Timer init*/
	flow_age_timeout_cb();
	
	return SEC_OK;
}



