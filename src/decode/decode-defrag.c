#include <sec-common.h>
#include <jhash.h>
#include <mbuf.h>

#include "decode-defrag.h"
#include "decode-ipv4.h"





frag_table_info_t *ip4_frags_table;




#if 0


static struct inet_frag_queue *inet_frag_intern(struct inet_frag_queue *qp_in, struct inet_frags *f, unsigned int hash)
{
	cvmx_spinlock_lock(&f.bucket[hash].bkt_lock);
	
	hlist_add_head(&qp_in->list, &f->bucket[hash].hash);

	cvmx_spinlock_unlock(&f.bucket[hash].bkt_lock);

	return qp_in;
}

static struct inet_frag_queue *inet_frag_alloc(struct inet_frags *f, struct mbuf_t *mbuf)
{
	struct inet_frag_queue *q;
	struct ipq_t *qp;
	IPV4Hdr *iph;
	
	q = ipq_alloc();
	if(NULL == q)
	{
		return NULL;
	}

	memset(q, 0, sizeof(struct ipq_t));
	
	qp = container_of(q, struct ipq_t, q);

	iph = mbuf->ip4h;

	qp->protocol = iph->next_proto_id;
	qp->id = iph->packet_id;
	qp->saddr = iph->src_addr;
	qp->daddr = iph->dst_addr;

	return q;
}



static struct inet_frag_queue *inet_frag_create(struct inet_frags *f, struct mbuf_t *mbuf, unsigned int hash)
{
	struct inet_frag_queue *q;

	q = inet_frag_alloc(f, mbuf);
	if(q == NULL)
	{
		return NULL;
	}

	return inet_frag_intern(q, f, hash);
}









struct inet_frag_queue *inet_frag_find(struct inet_frags *f, 
											unsigned int hash, 
											IPV4Hdr *iph, 
											struct mbuf_t *mbuf)
{
	struct inet_frag_queue *q;
	struct hlist_node *n;

	cvmx_spinlock_lock(&ip4_frags.bucket[hash].bkt_lock);
	
	hlist_for_each_entry(q, n, &f->bucket[hash].hash, list) 
	{
		if(f->match(q, (void *)iph))
		{
			cvmx_spinlock_unlock(&ip4_frags.bucket[hash].bkt_lock);
			return q;
		}
	}

	cvmx_spinlock_unlock(&ip4_frags.bucket[hash].bkt_lock);

	return inet_frag_create(f, mbuf, hash);
}




static inline struct ipq *ip_find(IPV4Hdr *iph, 
	                                 struct mbuf_t *mbuf, 
	                                 unsigned int hash)
{
	struct inet_frag_queue *q;

	cvmx_spinlock_lock(&ip4_frags.bucket[hash].bkt_lock);

	q = inet_frag_find(&ip4_frags, hash, iph, mbuf);
	if(NULL == q)
	{
		return NULL;
	}

	return container_of(q, ipq_t, q);
}



static int ip4_frag_match(struct inet_frag_queue *q, void *arg)
{
	struct ipq *qp = NULL;
	IPV4Hdr *iph = (IPV4Hdr *)arg;

	qp = container_of(q, struct ipq, q);

	return (qp->id == iph->ip_id &&
		qp->saddr == iph->src_addr &&
		qp->daddr == iph->dst_addr &&
		qp->protocol == iph->ip_proto);

}

/* Build a new IP datagram from all its fragments. */

static int ip_frag_reasm(struct ipq *qp)
{
	return 0;
}



static uint32_t ip_frag_queue(struct ipq *qp, 
									mbuf_t *mbuf)
{
	mbuf_t *prev, *next;
	int flags, offset;
	int ihl, end;

	offset = mbuf->ip4h->ip_off;

	flags = offset & ~IP_OFFSET;
	offset &= IP_OFFSET;
	offset <<= 3;		/* offset is in 8-byte chunks */

	ihl = IPV4_GET_HLEN(mbuf);

	/* Determine the position of this fragment. */
	end = offset + IPV4_GET_IPLEN(mbuf) - ihl;

	/* Is this the final fragment? */
	if ((flags & IP_MF) == 0) {
		/* If we already have some bits beyond end
		 * or have different end, the segment is corrrupted.
		 */
		if (end < qp->q.len ||
		    ((qp->q.last_in & INET_FRAG_LAST_IN) && end != qp->q.len))
			goto err;
		qp->q.last_in |= INET_FRAG_LAST_IN;
		qp->q.len = end;
	} else {
		if (end > qp->q.len) {
			/* Some bits beyond end -> corruption. */
			if (qp->q.last_in & INET_FRAG_LAST_IN)
				goto err;
			qp->q.len = end;
		}
	}
	if (end == offset)
		goto err;

	/* Find out which fragments are in front and at the back of us
	 * in the chain of fragments so far.  We must know where to put
	 * this fragment, right?
	 */

	prev = NULL;
	for (next = qp->q.fragments; next != NULL; next = next->next) {
		if (next->offset >= offset)
			break;	/* bingo! */
		prev = next;
	}

	/* We found where to put this one.  Check for overlap with
	 * preceding fragment, and, if needed, align things so that
	 * any overlaps are eliminated.
	 */
	if (prev) {
		int i = (prev->offset + prev->ip_fraglen) - offset;
		if (i > 0) {
			goto err;
		}
	}

	if (next) {
		int i = end - next->offset; /* overlap is 'i' bytes */
		if (i > 0) {
			goto err;
		}
	}

	mbuf->offset = offset;
	mbuf->ip_fraglen = IPV4_GET_IPLEN(mbuf) - ihl;

	/* Insert this fragment in the chain of fragments. */
	mbuf->next = next;
	if (prev)
		prev->next = mbuf;
	else
		qp->q.fragments = mbuf;

	qp->q.meat += mbuf->ip_fraglen;

	if (offset == 0)
		qp->q.last_in |= INET_FRAG_FIRST_IN;

	if (qp->q.last_in == (INET_FRAG_FIRST_IN | INET_FRAG_LAST_IN) &&
	    qp->q.meat == qp->q.len)
	    return ip_frag_reasm(qp, prev);
	
err:
	free(packet);
	
	return NULL;
}
#endif




static inline fcb_t *fcb_alloc()
{
	Mem_Slice_Ctrl_B *mscb;
	void *buf = mem_pool_fpa_slice_alloc(FPA_POOL_ID_HOST_MBUF);
	if(NULL == buf)
		return NULL;

	mscb = (Mem_Slice_Ctrl_B *)buf;
	mscb->magic = MEM_POOL_MAGIC_NUM;
	mscb->pool_id = FPA_POOL_ID_HOST_MBUF;

	return (fcb_t *)((uint8_t *)buf + sizeof(Mem_Slice_Ctrl_B));
}





static inline void fcb_free(fcb_t *fcb)
{
	Mem_Slice_Ctrl_B *mscb = (Mem_Slice_Ctrl_B *)((uint8_t *)fcb - sizeof(Mem_Slice_Ctrl_B));
	if(MEM_POOL_MAGIC_NUM != mscb->magic)
	{
		return;
	}
	if(MEM_POOL_ID_HOST_MBUF != mscb->pool_id)
	{
		return;
	}

	mem_pool_fpa_slice_free((void *)mscb, mscb->pool_id);

	return;
}



fcb_t *fcb_create(mbuf_t *mb)
{
	fcb_t *fcb = fcb_alloc();
	if(NULL == fcb)
	{
		return NULL;
	}

	memset((void *)fcb, 0, sizeof(fcb_t));

	fcb->sip = mb->ipv4.sip;
	fcb->dip = mb->ipv4.dip;
	fcb->id  = mb->defrag_id;


	return fcb;
}


static inline void fcb_insert(frag_bucket_t *fb, fcb_t *fcb)
{
	hlist_add_head(&fcb->list, &fb->hash);
}


uint32_t ip4_frag_hashfn(mbuf_t *mb)
{
	return jhash_3words(mb->ipv4.sip, mb->ipv4.dip, mb->defrag_id, 0) & FRAG_BUCKET_MASK;
}



uint32_t ip4_frag_match(fcb_t *fcb, mbuf_t *mb)
{
	return (fcb->id == mb->defrag_id 
		&& fcb->sip == mb->ipv4.sip 
		&& fcb->dip == mb->ipv4.dip);
}



fcb_t *FragFind(frag_bucket_t *fbucket, mbuf_t *mbuf, uint32_t hash)
{
	fcb_t *fcb;
	struct hlist_node *n;
	
#ifdef SEC_FRAG_DEBUG
	printf("============>enter FragFind\n");
#endif

	hlist_for_each_entry(fcb, n, &fbucket->hash, list)
	{
		if(ip4_frags_table->match(fcb, mbuf))
		{
		#ifdef SEC_FRAG_DEBUG
			printf("frag match is ok\n");
		#endif
			FCB_UPDATE_TIMESTAMP(fcb);
			return fcb;
		}
	}
#ifdef SEC_FRAG_DEBUG
	printf("frag match is fail\n");
#endif
	return NULL;

}

uint32_t Frag_defrag_begin(mbuf_t *mbuf, fcb_t *fcb)
{
	/*for first fragment packet */
	
	


	/*for nofirst fragment packet*/

	/*frag reassemble or frag session*/
}



void *Defrag(mbuf_t *mb)
{
	uint32_t hash;
	frag_bucket_t *base;
	frag_bucket_t *fb;
	fcb_t *fcb;
	
	hash = ip4_frags_table->hashfn(mb);
	
	base = (frag_bucket_t *)ip4_frags_table->bucket_base_ptr;
	fb = &base[hash];

	FCB_TABLE_LOCK(fb);
	
	fcb = FragFind(fb, mb, hash);
	
	if(NULL == fcb) /*not find , create a new one and add it into table*/
	{
		fcb = fcb_create(mb);
		if(NULL == fcb)
		{
			FCB_TABLE_UNLOCK(fb);
			return NULL;
		}

		fcb_insert(fb, fcb);		
	}

	FCB_TABLE_UNLOCK(fb);

	

	Frag_defrag_begin(mb, fcb);
	
	
	return NULL;
}


uint32_t FragModule_init()
{
	int i;
	frag_bucket_t *base;

	ip4_frags_table = (frag_table_info_t *)cvmx_bootmem_alloc_named((sizeof(frag_table_info_t) + FRAG_BUCKET_NUM * FRAG_BUCKET_SIZE), CACHE_LINE_SIZE, FRAG_HASH_TABLE_NAME);
	if(NULL == ip4_frags_table)
	{
		printf("ipfrag_init: no memory\n");
		return SEC_NO;
	}
	
	ip4_frags_table->bucket_num = FRAG_BUCKET_NUM;
	ip4_frags_table->bucket_size = FRAG_BUCKET_SIZE;
	
	ip4_frags_table->item_num = FRAG_ITEM_NUM;
	ip4_frags_table->item_size = FRAG_ITEM_SIZE;
	
	ip4_frags_table->bucket_base_ptr = (void *)((uint8_t *)ip4_frags_table + sizeof(frag_table_info_t));
		
	base = (frag_bucket_t *)ip4_frags_table->bucket_base_ptr;
	
	for (i = 0; i < FRAG_BUCKET_NUM; i++)
	{
		INIT_HLIST_HEAD(&base[i].hash);
		cvmx_spinlock_init(&base[i].bkt_lock);
	}

	ip4_frags_table->match = ip4_frag_match;
	ip4_frags_table->hashfn = ip4_frag_hashfn;

	return SEC_OK;
}


int FragModuleInfo_Get()
{
	ip4_frags_table = (frag_table_info_t *)cvmx_bootmem_find_named_block(FRAG_HASH_TABLE_NAME);
	if(NULL == ip4_frags_table)
	{
		printf("FlowInfoGet fail\n");
		return SEC_NO;
	}

	return SEC_OK;
}






