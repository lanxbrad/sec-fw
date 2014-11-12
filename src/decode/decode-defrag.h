#ifndef __DECODE_DEFRAG_H__
#define __DECODE_DEFRAG_H__



#define INETFRAGS_HASHSZ		64


#define INET_FRAG_COMPLETE	4
#define INET_FRAG_FIRST_IN	2
#define INET_FRAG_LAST_IN	1

struct inet_frag_queue {
	struct hlist_node	list;
	struct mbuf_t    *fragments; /* list of received fragments */
	cvmx_spinlock_t		lock;
	int             len;
	int             meat;
	uint8_t			last_in;    /* first/last segment arrived? */
};




struct frags_bucket
{
	struct hlist_head hash;
	cvmx_spinlock_t bkt_lock;
};


struct inet_frags {
	struct frags_bucket bucket[INETFRAGS_HASHSZ];


	int			(*match)(struct inet_frag_queue *q,	void *arg);
	unsigned int		(*hashfn)(struct inet_frag_queue *);
};



#endif