#ifndef __DECODE_DEFRAG_H__
#define __DECODE_DEFRAG_H__



#define INETFRAGS_HASHSZ		64


#define INET_FRAG_COMPLETE	4
#define INET_FRAG_FIRST_IN	2
#define INET_FRAG_LAST_IN	1








typedef struct {
	struct hlist_node	list;
	struct mbuf_t    *fragments; /* list of received fragments */
	cvmx_spinlock_t		lock;
	int             len;
	int             meat;
	uint8_t			last_in;    /* first/last segment arrived? */
}inet_frag_queue;


typedef struct 
{
	inet_frag_queue q;

	uint32_t saddr;
	uint32_t daddr;
	uint16_t sport;
	uint16_t dport;
	uint16_t id;
	uint8_t protocol;
}ipq_t;



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