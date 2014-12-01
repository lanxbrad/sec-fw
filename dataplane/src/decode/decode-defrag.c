#include <sec-common.h>
#include <jhash.h>
#include <mbuf.h>
#include <sec-debug.h>
#include "decode-defrag.h"
#include "decode-ipv4.h"
#include "decode-statistic.h"



CVMX_SHARED uint64_t new_fcb[CPU_HW_RUNNING_MAX] = {0, 0, 0, 0};
CVMX_SHARED uint64_t del_fcb[CPU_HW_RUNNING_MAX] = {0, 0, 0, 0};



frag_table_info_t *ip4_frags_table;





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
	if(FPA_POOL_ID_HOST_MBUF != mscb->pool_id)
	{
		return;
	}

	mem_pool_fpa_slice_free((void *)mscb, mscb->pool_id);

	return;
}



static inline fcb_t *fcb_create(mbuf_t *mb)
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
	
#ifdef SEC_DEFRAG_DEBUG
	printf("============>enter FragFind\n");
#endif

	hlist_for_each_entry(fcb, n, &fbucket->hash, list)
	{
		if(ip4_frags_table->match(fcb, mbuf))
		{
		#ifdef SEC_DEFRAG_DEBUG
			printf("frag match is ok\n");
		#endif
			FCB_UPDATE_TIMESTAMP(fcb);
			return fcb;
		}
	}
#ifdef SEC_DEFRAG_DEBUG
	printf("frag match is fail\n");
#endif
	return NULL;

}


void Frag_defrag_freefrags(fcb_t *fcb)
{
	mbuf_t *head;
	mbuf_t *next;
	head = fcb->fragments;
	while(head)
	{
		next = head->next;
		PACKET_DESTROY_ALL(head);
		head = next;
	}
	fcb->fragments = NULL;
	fcb->fragments_tail = NULL;
}

mbuf_t *Frag_defrag_setup(mbuf_t *head, fcb_t *fcb)
{
	mbuf_t *new_mb;
	void *packet_buffer;
	
	new_mb = MBUF_ALLOC();
	if(NULL == new_mb)
	{
		return NULL;
	}

	

	packet_buffer = MEM_8K_ALLOC(fcb->total_fraglen + 
			((uint64_t)(head->network_header) - (uint64_t)(head->pkt_ptr) + IPV4_GET_HLEN(head)));
	if(NULL == packet_buffer)
	{
		MBUF_FREE(new_mb);
		return NULL;
	}

	memset((void *)new_mb, 0, sizeof(mbuf_t));

	PKTBUF_SET_SW(new_mb);
	new_mb->pkt_ptr = packet_buffer;

	
	new_mb->ethh = head->ethh;
	new_mb->vlan_idx = head->vlan_idx;
	new_mb->vlan_id = head->vlan_id;
	new_mb->network_header = head->network_header;
	new_mb->transport_header = head->transport_header;
	new_mb->payload = head->payload;
	packet_header_ptr_adjust(new_mb, head->pkt_ptr, packet_buffer);

	new_mb->magic_flag = MBUF_MAGIC_NUM;
	new_mb->input_port = head->input_port;
	
	memcpy((void *)new_mb->eth_dst, (void *)head->eth_dst, sizeof(new_mb->eth_dst));
	memcpy((void *)new_mb->eth_src, (void *)head->eth_src, sizeof(new_mb->eth_src));
	
	new_mb->ipv4.sip = head->ipv4.sip;
	new_mb->ipv4.dip = head->ipv4.dip;

	new_mb->sport = head->sport;
	new_mb->dport = head->dport;

	new_mb->proto = head->proto;

	return new_mb;
}

mbuf_t *Frag_defrag_reasm(fcb_t *fcb)
{
	int ihlen;
	int len;
	mbuf_t *reasm_mb;
	mbuf_t *next;
	mbuf_t *head = fcb->fragments;

	/* Allocate a new buffer for the datagram. */
	ihlen = IPV4_GET_HLEN(head);
	len = ihlen + fcb->total_fraglen;

	if(len > IPV4_PKTLEN_MAX)
		goto out_oversize;

	reasm_mb = Frag_defrag_setup(head, fcb);
	if(NULL == reasm_mb)
		goto setup_err;

	memcpy((void *)reasm_mb->pkt_ptr, (void *)head->pkt_ptr, head->pkt_totallen);
	reasm_mb->pkt_totallen += head->pkt_totallen;
	next = head->next;
	while(next)
	{
		memcpy((void *)((uint8_t *)reasm_mb->pkt_ptr + reasm_mb->pkt_totallen), (void *)((uint8_t *)next->pkt_ptr + next->pkt_totallen - next->frag_len), next->frag_len);
		reasm_mb->pkt_totallen += next->frag_len;
		next = next->next;
	}


	IPV4_SET_IPLEN(reasm_mb, len);
	((IPV4Hdr *)(reasm_mb->network_header))->ip_off = 0;
	IPV4_SET_IPCSUM(reasm_mb, IPV4CalculateChecksum((uint16_t *)((reasm_mb->network_header)), ihlen));

	fcb->status |= DEFRAG_COMPLETE;

	Frag_defrag_freefrags(fcb);
	
	STAT_FRAG_REASM_OK;
	return reasm_mb;
setup_err:
out_oversize:
	
	return NULL;
}




mbuf_t *Frag_defrag_process(mbuf_t * mbuf,fcb_t * fcb)
{
	int i;
	mbuf_t *prev, *next;
	int offset;
	int end;
	
	if(fcb->last_in & DEFRAG_COMPLETE)
		goto err;
	
	/* Determine the position of this fragment. */
	offset = mbuf->frag_offset;
	end = offset + mbuf->frag_len;

	
	/* Is this the final fragment? */
	if(0 == IPV4_GET_MF(mbuf))/*final*/
	{
		if( end < fcb->total_fraglen || (fcb->last_in & DEFRAG_LAST_IN) )/*but last already in, so error*/
			goto err;
		fcb->last_in |= DEFRAG_LAST_IN;
		fcb->total_fraglen = end;
	}
	else/*not final*/
	{
		if(end > fcb->total_fraglen)/*last must be not in*/
		{
			if(fcb->last_in & DEFRAG_LAST_IN)/*but last already in, so error*/
				goto err;
			fcb->total_fraglen = end;
		}
	}

    /* 
        * Find out which fragments are in front and at the back of us
        * in the chain of fragments so far.  We must know where to put
        * this fragment, right?
        */
	prev = fcb->fragments_tail;
	if(!prev || prev->frag_offset < offset)
	{
		next = NULL;
		goto found;
	}
	prev = NULL;
	for(next = fcb->fragments; next != NULL; next = next->next)
	{
		if(next->frag_len >= offset)
			break;   /*bingo*/
		prev = next;
	}

found:
    /* 
        * We found where to put this one.  Check for overlap with
         * preceding fragment, and, if needed, align things so that
         * any overlaps are eliminated.
        */
	if(prev){
		i = (prev->frag_offset + prev->frag_len) - offset;
		if(i > 0)    /*overlap with prev*/
			goto err;
	}

	if(next){
		i = next->frag_offset - end;
		if(i < 0)  /*overlap with next*/
			goto err;
	}

	/* Insert this fragment in the chain of fragments. */
	mbuf->next = next;
	if(!next)
		fcb->fragments_tail = mbuf;
	if(prev)
		prev->next = mbuf;
	else
		fcb->fragments = mbuf;

	fcb->meat += mbuf->frag_len;
	if(offset == 0)
		fcb->last_in |= DEFRAG_FIRST_IN;

	if(fcb->last_in == (DEFRAG_FIRST_IN | DEFRAG_LAST_IN) && 
		fcb->meat == fcb->total_fraglen)
	{
	#ifdef SEC_DEFRAG_DEBUG
		printf("all in begin to reasm\n");
	#endif
		return Frag_defrag_reasm(fcb);
	}
	else
	{
		STAT_FRAG_CACHE_OK;
		return NULL;  /*cached*/
	}

err:
	PACKET_DESTROY_ALL(mbuf);
	STAT_FRAG_DEFRAG_ERR;
	return NULL;
}



/*
 * PACKET_HW TO PACKET_SW
 * Cache
 * merge
 */
mbuf_t *Frag_defrag_begin(mbuf_t *mbuf, fcb_t *fcb)
{
	mbuf_t *mb;
	if(SEC_OK != PACKET_HW2SW(mbuf))
	{
		PACKET_DESTROY_ALL(mbuf);
		STAT_FRAG_HW2SW_ERR;
		return NULL;
	}
	
	FCB_LOCK(fcb);

	mb = Frag_defrag_process(mbuf, fcb);
	
	/*frag reassemble or frag session*/
	FCB_UNLOCK(fcb);
	
	return mb;
}



mbuf_t *Defrag(mbuf_t *mb)
{
	uint32_t hash;
	frag_bucket_t *base;
	frag_bucket_t *fb;
	fcb_t *fcb;
	
	hash = ip4_frags_table->hashfn(mb);

#ifdef SEC_DEFRAG_DEBUG
	printf("frag hash is %d\n", hash);
#endif

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
			PACKET_DESTROY_ALL(mb);
			STAT_FRAG_FCB_NO;
			return NULL;
		}

		FCB_UPDATE_TIMESTAMP(fcb);
		fcb_insert(fb, fcb);
	}

	FCB_TABLE_UNLOCK(fb);

	return Frag_defrag_begin(mb, fcb);
}


void Frag_defrag_timeout(Oct_Timer_Threat *o, void *param)
{
	int i;
	uint64_t current_cycle;

	frag_bucket_t *base;
	frag_bucket_t *fb;
	fcb_t *fcb;
	fcb_t *tfcb;
	struct hlist_node *n;
	struct hlist_node *t;
	struct hlist_head timeout;

	base = (frag_bucket_t *)ip4_frags_table->bucket_base_ptr;
	
	current_cycle = cvmx_get_cycle();

	for(i = 0; i < FRAG_BUCKET_NUM; i++)
	{
		INIT_HLIST_HEAD(&timeout);
		fb = &base[i];
		
		if(FCB_TABLE_TRYLOCK(fb) != 0)
			continue;

		hlist_for_each_entry_safe(fcb, t, n, &base[i].hash, list)	
		{
			if((current_cycle > fcb->cycle) && ((current_cycle - fcb->cycle) > FRAG_MAX_TIMEOUT))
			{
				hlist_del(&fcb->list);
			#ifdef SEC_DEFRAG_DEBUG
				printf("delete one fcb %p\n", fcb);
			#endif
				del_fcb[local_cpu_id]++;
				hlist_add_head(&fcb->list, &timeout);
			}
		}
		
		FCB_TABLE_UNLOCK(fb);

		hlist_for_each_entry_safe(tfcb, t, n, &timeout, list)	
		{	
			hlist_del(&tfcb->list);

			/*TODO: session ageing do something*/
			Frag_defrag_freefrags(tfcb);
			fcb_free(tfcb);
		}
		
	}

	return;
}



uint32_t FragModule_init()
{
	int i;
	frag_bucket_t *base;
	frag_bucket_t *f;

	ip4_frags_table = (frag_table_info_t *)cvmx_bootmem_alloc_named((sizeof(frag_table_info_t) + FRAG_BUCKET_NUM * FRAG_BUCKET_SIZE), 																												CACHE_LINE_SIZE, 
																  FRAG_HASH_TABLE_NAME);
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
		f = &base[i];
		FCB_TABLE_INITLOCK(f);
	}

	ip4_frags_table->match = ip4_frag_match;
	ip4_frags_table->hashfn = ip4_frag_hashfn;

	if(OCT_Timer_Create(0xFFFFFF, 0, 2, TIMER_GROUP, Frag_defrag_timeout, NULL, 0, 1000))/*1s*/
	{
		printf("timer create fail\n");
		return SEC_NO;
	}

	printf("frag age timer create ok\n");
	
	return SEC_OK;
}


uint32_t FragModuleInfo_Get()
{
	
	const cvmx_bootmem_named_block_desc_t *block_desc = cvmx_bootmem_find_named_block(FRAG_HASH_TABLE_NAME); 
	if (block_desc)
	{
		ip4_frags_table = (frag_table_info_t *)(block_desc->base_addr);
	}
	else
	{
		printf("FragModuleInfo_Get error \n");
		return SEC_NO;
	}


	return SEC_OK;
}

