#ifndef __DECODE_DEFRAG_H__
#define __DECODE_DEFRAG_H__


#include <sec-common.h>
#include <oct-common.h>
#include <oct-init.h>
#include <hlist.h>


#define DEFRAG_COMPLETE	4
#define DEFRAG_FIRST_IN	2
#define DEFRAG_LAST_IN	1


typedef struct {
	struct hlist_node	list;
	struct mbuf_t    *queue;    /* list of cached fragments */
	uint64_t          cycle;
	cvmx_spinlock_t		lock;
	
	
	uint32_t sip;
	uint32_t dip;
	uint16_t id;
	
	int			len;
	int			meat;
	uint8_t		last_in;    /* first/last segment arrived? */
}fcb_t;




typedef struct 
{
	struct hlist_head hash;
	cvmx_spinlock_t bkt_lock;
}frag_bucket_t;


typedef struct {
	uint32_t bucket_num;
	uint32_t bucket_size;

	uint32_t item_size;
	uint32_t item_num;

	void *bucket_base_ptr;

	uint32_t (*match)(fcb_t * ,	mbuf_t *);
	uint32_t (*hashfn)(mbuf_t *);
}frag_table_info_t;


#define FRAG_BUCKET_NUM   1024
#define FRAG_BUCKET_MASK  (FRAG_BUCKET_NUM - 1)

#define FRAG_BUCKET_SIZE  sizeof(frag_bucket_t)


#define FRAG_ITEM_NUM     2048
#define FRAG_ITEM_SIZE    sizeof(fcb_t)


#define FRAG_HASH_TABLE_NAME   "Frag_Hash_Table"

#define FCB_TABLE_LOCK(fb)     cvmx_spinlock_lock(&fb->bkt_lock);  
#define FCB_TABLE_UNLOCK(fb)   cvmx_spinlock_unlock(&fb->bkt_lock); 



#define FCB_UPDATE_TIMESTAMP(f)  (f->cycle = cvmx_get_cycle())




static inline void fcb_size_judge(void)
{
	BUILD_BUG_ON((sizeof(fcb_t) + sizeof(Mem_Slice_Ctrl_B)) > 256);

	return;
}




extern void *Defrag(mbuf_t *mbuf);



#endif