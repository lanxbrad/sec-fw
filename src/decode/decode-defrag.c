#include <sec-common.h>
#include <jhash.h>
#include <mbuf.h>

#include "decode-defrag.h"
#include "decode-ipv4.h"

#define frag_q_alloc()

struct ipq 
{
	struct inet_frag_queue q;

	uint32_t saddr;
	uint32_t daddr;
	uint16_t id;
	uint8_t protocol;
	uint16_t sport;
	uint16_t dport;
};



static struct inet_frags ip4_frags;


static unsigned int ipqhashfn(uint16_t id, uint32_t saddr, uint32_t daddr, uint8_t prot)
{
	return jhash_3words(id << 16 | prot,
		    			saddr, 
		    			daddr,
		    			0) & (INETFRAGS_HASHSZ - 1);
	
}

static struct inet_frag_queue *inet_frag_intern(struct inet_frag_queue *qp_in, struct inet_frags *f, unsigned int hash)
{
	hlist_add_head(&qp_in->list, &f->bucket[hash].hash);

	return qp_in;
}


static struct inet_frag_queue *inet_frag_create(struct inet_frags *f, struct m_buf *mbuf, unsigned int hash)
{
	struct inet_frag_queue *q;

	q = inet_frag_alloc(f, mbuf);
	if(q == NULL)
	{
		return NULL;
	}

	return inet_frag_intern(q, f, hash);
}






static struct inet_frag_queue *inet_frag_alloc(struct inet_frags *f, struct m_buf *mbuf)
{
	struct inet_frag_queue *q;
	struct ipq *qp = NULL;

	IPV4Hdr *iph = mbuf->ip4h;
	
	q = frag_q_alloc();
	if(NULL == q)
	{
		return NULL;
	}

	memset(q, 0, sizeof(struct ipq));
	
	qp = container_of(q, struct ipq, q);

	qp->protocol = iph->next_proto_id;
	qp->id = iph->packet_id;
	qp->saddr = iph->src_addr;
	qp->daddr = iph->dst_addr;

	return q;
}


struct inet_frag_queue *inet_frag_find(struct inet_frags *f, 
											unsigned int hash, 
											IPV4Hdr *iph, 
											struct m_buf *mbuf)
{
	struct inet_frag_queue *q;
	struct hlist_node *n;
	
	hlist_for_each_entry(q, n, &f->bucket[hash].hash, list) 
	{
		if(f->match(q, (void *)iph))
		{
			return q;
		}
	}


	return inet_frag_create(f, mbuf, hash);
}




static inline struct ipq *ip_find(IPV4Hdr *iph, 
	                                 struct m_buf *mbuf, 
	                                 unsigned int hash)
{
	struct inet_frag_queue *q;

	q = inet_frag_find(&ip4_frags, hash, iph, mbuf);
	if(NULL == q)
	{
		return NULL;
	}

	return container_of(q, struct ipq, q);
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
									m_buf *mbuf)
{
	m_buf *prev, *next;
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

	/* Insert this fragment in the chain of fragments. */
	mbuf->next = next;
	if (prev)
		prev->next = mbuf;
	else
		qp->q.fragments = mbuf;

	if (offset == 0)
		qp->q.last_in |= INET_FRAG_FIRST_IN;

	if (qp->q.last_in == (INET_FRAG_FIRST_IN | INET_FRAG_LAST_IN) &&
	    qp->q.meat == qp->q.len)
	    return ip_frag_reasm(qp, prev);
	
err:
	

}




int Defrag(m_buf *mbuf)
{
	unsigned int hash;

	IPV4Hdr *iph;

	struct ipq *qp;

	iph = mbuf->ip4h;

	hash = ipqhashfn(IPV4_GET_IPID(mbuf), mbuf->ipv4.sip, mbuf->ipv4.dip, IPV4_GET_IPPROTO(mbuf));

	cvmx_spinlock_lock(&ip4_frags.bucket[hash].bkt_lock);

	if((qp = ip_find(iph, mbuf, hash)) != NULL)
	{
		ret = ip_frag_queue(qp, mbuf);
	}
	else
	{
		ret = UTAF_RX_DROP;
	}
	
	cvmx_spinlock_unlock(&ip4_frags.bucket[hash].bkt_lock);
	
	return 0;
}













uint32_t ipfrag_init()
{
	int i;
	//uint64_t hz;
	
	for (i = 0; i < INETFRAGS_HASHSZ; i++)
	{
		INIT_HLIST_HEAD(&ip4_frags.bucket[i].hash);
	}

	ip4_frags.hashfn = ip4_hashfn;
	ip4_frags.match = ip4_frag_match;


	timer_init(&frag_age);
	
	/*10ms*/
	//timer_reset(&frag_age, FRAG_SYS_HZ/100000, PERIODICAL, mtconfig.age_lcore, frag_age_timeout_cb, NULL);
	return timer_reset(&frag_age, UTAF_SYS_HZ, PERIODICAL, mtconfig.age_lcore, frag_age_timeout_cb, NULL);

}






