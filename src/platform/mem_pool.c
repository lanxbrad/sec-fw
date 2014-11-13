
#include <sec-common.h>

#include "mem_pool.h"



CVMX_SHARED Mem_Pool_Cfg mem_pool[MEM_POOL_ID_MAX];




void *mem_pool_alloc(int pool_id)
{
	int index;
	Mem_Pool_Cfg *mp = &mem_pool[pool_id];
	struct list_head *l;
	Mem_Slice_Ctrl_B *mscb;

	if(MEM_POOL_ID_SMALL_BUFFER == pool_id || MEM_POOL_ID_LARGE_BUFFER == pool_id)
	{
		index = cvmx_atomic_fetch_and_add32_nosync(&mp->mpc.global_index, 1);

		cvmx_spinlock_lock(&mp->mpc.msc[index].chain_lock);
		if(list_empty(&mp->mpc.msc[index].head))
		{
			cvmx_spinlock_unlock(&mp->mpc.msc[index].chain_lock);
			return NULL;
		}
		l = mp->mpc.msc[index].head.next;
		list_del(l);
		cvmx_spinlock_unlock(&mp->mpc.msc[index].chain_lock);
		
		mscb = container_of(l, Mem_Slice_Ctrl_B, list);

		return (void *)((uint8_t *)mscb + sizeof(Mem_Slice_Ctrl_B));
	}
	else if(MEM_POOL_ID_HOST_MBUF == pool_id || MEM_POOL_ID_FLOW_NODE == pool_id)
	{
		
	}
	else
	{
		printf("invalid request pool id!\n");
	}
}

void mem_pool_free(void *buf)
{
	int pool_id;
	int subpool_id;
	Mem_Slice_Ctrl_B *mscb;
	Mem_Pool_Cfg *mp;

	mscb = (Mem_Slice_Ctrl_B *)((uint8_t *)buf - sizeof(Mem_Slice_Ctrl_B));

	if(mscb->magic != MEM_POOL_MAGIC_NUM)
	{
		printf("buf has been destroyed!\n");
		return;
	}

	pool_id = mscb->pool_id;
	if(pool_id >= MEM_POOL_ID_MAX)
	{
		printf("buf has been destroyed!\n");
	}

	subpool_id = mscb->subpool_id;
	if(subpool_id >= MEM_POOL_INTERNAL_NUM)
	{
		printf("buf has been destroyed!\n");
	}

	mp = &mem_pool[pool_id];

	cvmx_spinlock_lock(&mp->mpc.msc[subpool_id].chain_lock);
	list_add(&mscb->list, &mp->mpc.msc[subpool_id].head);
	cvmx_spinlock_unlock(&mp->mpc.msc[subpool_id].chain_lock);

	return;
}








int mem_pool_sw_slice_inject(int pool_id)
{
	int i, j;
	int num_perchain;
	uint64_t start_address;
	Mem_Slice_Ctrl_B *mscb;
	int totalsize = mem_pool[pool_id].slicesize * mem_pool[pool_id].slicenum;
	mem_pool[pool_id].start = (void *)malloc(totalsize);
	if(NULL == mem_pool[pool_id].start)
	{
		return SEC_NO;
	}
	mem_pool[pool_id].totalsize = totalsize;

	num_perchain = mem_pool[pool_id].slicenum/MEM_POOL_INTERNAL_NUM;
	start_address = (uint64_t)mem_pool[pool_id].start;
	for(i = 0; i < MEM_POOL_INTERNAL_NUM; i++)
	{
		INIT_LIST_HEAD(&mem_pool[pool_id].mpc.msc[i].head);
		for(j = 0; j < num_perchain; j++)
		{
			mscb = (Mem_Slice_Ctrl_B *)start_address;
			mscb->magic = MEM_POOL_MAGIC_NUM;
			mscb->pool_id = pool_id;
			mscb->subpool_id = i;
			list_add(&mscb->list, &mem_pool[pool_id].mpc.msc[i].head);
			start_address += mem_pool[pool_id].slicesize;
		}
	}

	return SEC_OK;
}


int mem_pool_fpa_slice_inject(int pool_id)
{
	
	int totalsize = mem_pool[pool_id].slicesize * mem_pool[pool_id].slicenum;
	mem_pool[pool_id].start = (void *)malloc(totalsize);
	if(NULL == mem_pool[pool_id].start)
	{
		return SEC_NO;
	}
	mem_pool[pool_id].totalsize = totalsize;

	return SEC_OK;
}


int mem_pool_init(void)
{
	
	memset((void *)mem_pool, 0, sizeof(Mem_Pool_Cfg) * MEM_POOL_ID_MAX);

	mem_pool[MEM_POOL_ID_HOST_MBUF].slicesize = MEM_POOL_HOST_MBUF_SIZE;
	mem_pool[MEM_POOL_ID_HOST_MBUF].slicenum  = MEM_POOL_HOST_MBUF_NUM;

	mem_pool[MEM_POOL_ID_FLOW_NODE].slicesize = MEM_POOL_FLOW_NODE_SIZE;
	mem_pool[MEM_POOL_ID_FLOW_NODE].slicenum  = MEM_POOL_FLOW_NODE_NUM;

	mem_pool[MEM_POOL_ID_SMALL_BUFFER].slicesize = MEM_POOL_SMALL_BUFFER_SIZE;
	mem_pool[MEM_POOL_ID_SMALL_BUFFER].slicenum  = MEM_POOL_SMALL_BUFFER_NUM;

	mem_pool[MEM_POOL_ID_LARGE_BUFFER].slicesize = MEM_POOL_LARGE_BUFFER_SIZE;
	mem_pool[MEM_POOL_ID_LARGE_BUFFER].slicenum  = MEM_POOL_LARGE_BUFFER_NUM;

	if( SEC_NO == mem_pool_fpa_slice_inject(MEM_POOL_ID_HOST_MBUF))
	{
		return SEC_NO;
	}
	
	if(SEC_NO == mem_pool_fpa_slice_inject(MEM_POOL_ID_FLOW_NODE))
	{
		return SEC_NO;
	}
	
	
	if(SEC_NO == mem_pool_sw_slice_inject(MEM_POOL_ID_SMALL_BUFFER))
	{
		return SEC_NO;
	}
	
	if(SEC_NO == mem_pool_sw_slice_inject(MEM_POOL_ID_LARGE_BUFFER))
	{
		return SEC_NO;
	}
	
	return SEC_OK;
}








