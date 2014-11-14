#include <sec-common.h>
#include <jhash.h>
#include <mbuf.h>

#include "decode-defrag.h"
#include "decode-ipv4.h"





static struct inet_frags ip4_frags;


inet_frag_queue *ipq_alloc()
{
	void *buf = mem_pool_fpa_slice_alloc(MEM_POOL_ID_FLOW_NODE);

	return (inet_frag_queue *)((uint8_t *)buf + sizeof(Mem_Slice_Ctrl_B));
}


void ipq_free(ipq_t *ipq)
{
	Mem_Slice_Ctrl_B *mscb = (Mem_Slice_Ctrl_B *)((uint8_t *)ipq - sizeof(Mem_Slice_Ctrl_B));
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






static unsigned int ipqhashfn(uint16_t id, uint32_t saddr, uint32_t daddr, uint8_t prot)
{
	return jhash_3words(id << 16 | prot,
		    			saddr, 
		    			daddr,
		    			0) & (INETFRAGS_HASHSZ - 1);
	
}

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


/*
 *   if cache, return NULL
 *   if drop, packet_destroy internal and return NULL
 *   if frag reassemble success ,return defrag_mbuf which point to the whole packet.
 */
mbuf_t *Defrag(mbuf_t *mbuf)
{
	unsigned int hash;

	IPV4Hdr *iph;
	struct ipq *qp;

	iph = mbuf->ip4h;

	hash = ipqhashfn(IPV4_GET_IPID(mbuf), mbuf->ipv4.sip, mbuf->ipv4.dip, IPV4_GET_IPPROTO(mbuf));

	if((qp = ip_find(iph, mbuf, hash)) != NULL)
	{
		int ret;

		cvmx_spinlock_lock(&qp->q.lock);
		
		ret = ip_frag_queue(qp, mbuf);

		cvmx_spinlock_unlock(&qp->q.lock);

		return ret;
	}
	else /*find fail*/
	{
		packet_destroy(mbuf);
	}
	
	return 0;
}













uint32_t ipfrag_init()
{
	int i;
	
	for (i = 0; i < INETFRAGS_HASHSZ; i++)
	{
		INIT_HLIST_HEAD(&ip4_frags.bucket[i].hash);
		cvmx_spinlock_init(&ip4_frags.bucket[i].bkt_lock);
	}

	ip4_frags.match = ip4_frag_match;

	return 0;
}






