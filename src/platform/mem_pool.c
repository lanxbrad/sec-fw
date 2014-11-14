
#include <sec-common.h>

#include "mem_pool.h"



Mem_Pool_Cfg *mem_pool[MEM_POOL_ID_MAX];




void *mem_pool_alloc(int pool_id)
{
	int index;
	Mem_Pool_Cfg *mp = mem_pool[pool_id];
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
	else
	{
		printf("invalid request pool id!\n");
		return NULL;
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

	mp = mem_pool[pool_id];

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
	Mem_Pool_Cfg *mpc = mem_pool[pool_id];
	
	if(0 != mpc->slicenum % MEM_POOL_INTERNAL_NUM)
		return SEC_NO;

	num_perchain = mpc->slicenum/MEM_POOL_INTERNAL_NUM;
	start_address = (uint64_t)mpc->start;
	for(i = 0; i < MEM_POOL_INTERNAL_NUM; i++)
	{
		INIT_LIST_HEAD(&mpc->mpc.msc[i].head);
		for(j = 0; j < num_perchain; j++)
		{
			mscb = (Mem_Slice_Ctrl_B *)start_address;
			mscb->magic = MEM_POOL_MAGIC_NUM;
			mscb->pool_id = pool_id;
			mscb->subpool_id = i;
			list_add(&mscb->list, &mpc->mpc.msc[i].head);
			start_address += mpc->slicesize;
		}
	}

	return SEC_OK;
}





int mem_pool_fpa_slice_inject(int pool_id)
{
	uint32_t i, fpa_pool_id;
	uint64_t start_address;
	Mem_Slice_Ctrl_B *mscb;
	if(MEM_POOL_ID_HOST_MBUF == pool_id)
	{
		fpa_pool_id = FPA_POOL_ID_HOST_MBUF;
	}
	else if(MEM_POOL_ID_FLOW_NODE == pool_id)
	{
		fpa_pool_id = FPA_POOL_ID_FLOW_NODE;
	}
	else
	{
		printf("invalid pool id\n");
		return SEC_NO;
	}

	start_address = (uint64_t)mem_pool[pool_id]->start;
	for (i = 0; i < mem_pool[pool_id]->slicenum; i++)
	{
		mscb = (Mem_Slice_Ctrl_B *)start_address;
		mscb->magic = MEM_POOL_MAGIC_NUM;
		mscb->pool_id = pool_id;
		cvmx_fpa_free((void *)start_address, fpa_pool_id, 0);
		start_address += mem_pool[pool_id]->slicesize;
	}
	
	return SEC_OK;
}


int Mem_Pool_Init(void)
{

	/* HOST MBUF POOL INIT*/
	Mem_Pool_Cfg *mpc = (Mem_Pool_Cfg *)cvmx_bootmem_alloc_named(MEM_POOL_TOTAL_HOST_MBUF , 128, MEM_POOL_NAME_HOST_MBUF);
	if(NULL == mpc)
		return SEC_NO;
	
	memset((void *)mpc, 0, sizeof(Mem_Pool_Cfg));
	
	mpc->slicesize = MEM_POOL_HOST_MBUF_SIZE;
	mpc->slicenum = MEM_POOL_HOST_MBUF_NUM;
	mpc->start = (uint8_t *)mpc + sizeof(Mem_Pool_Cfg);
	mpc->totalsize = MEM_POOL_HOST_MBUF_NUM * MEM_POOL_HOST_MBUF_SIZE;
	mem_pool[MEM_POOL_ID_HOST_MBUF] = mpc;
	
	if( SEC_NO == mem_pool_fpa_slice_inject(MEM_POOL_ID_HOST_MBUF))
	{
		return SEC_NO;
	}
	
	
	/* FLOW NODE POOL INIT*/
	mpc = (Mem_Pool_Cfg *)cvmx_bootmem_alloc_named(MEM_POOL_TOTAL_FLOW_NODE, 128, MEM_POOL_NAME_FLOW_NODE);
	if(NULL == mpc)
		return SEC_NO;

	memset((void *)mpc, 0, sizeof(Mem_Pool_Cfg));
	
	mpc->slicesize = MEM_POOL_FLOW_NODE_SIZE;
	mpc->slicenum = MEM_POOL_FLOW_NODE_NUM;
	mpc->start = (uint8_t *)mpc + sizeof(Mem_Pool_Cfg);
	mpc->totalsize = MEM_POOL_FLOW_NODE_NUM * MEM_POOL_FLOW_NODE_SIZE;
	mem_pool[MEM_POOL_ID_FLOW_NODE] = mpc;

	if( SEC_NO == mem_pool_fpa_slice_inject(MEM_POOL_ID_FLOW_NODE))
	{
		return SEC_NO;
	}

	/*SMALL BUF POOL INIT*/
	mpc = (Mem_Pool_Cfg *)cvmx_bootmem_alloc_named(MEM_POOL_TOTAL_SMALL_BUFFER, 128, MEM_POOL_NAME_SMALL_BUFFER);
	if(NULL == mpc)
		return SEC_NO;

	memset((void *)mpc, 0, sizeof(Mem_Pool_Cfg));
	
	mpc->slicesize = MEM_POOL_SMALL_BUFFER_SIZE;
	mpc->slicenum = MEM_POOL_SMALL_BUFFER_NUM;
	mpc->start = (uint8_t *)mpc + sizeof(Mem_Pool_Cfg);
	mpc->totalsize = MEM_POOL_SMALL_BUFFER_NUM * MEM_POOL_SMALL_BUFFER_SIZE;
	mem_pool[MEM_POOL_ID_SMALL_BUFFER] = mpc;

	if( SEC_NO == mem_pool_sw_slice_inject(MEM_POOL_ID_SMALL_BUFFER))
	{
		return SEC_NO;
	}

	/*LARGE BUF POOL INIT*/
	mpc = (Mem_Pool_Cfg *)cvmx_bootmem_alloc_named(MEM_POOL_TOTAL_LARGE_BUFFER, 128, MEM_POOL_NAME_LARGE_BUFFER);
	if(NULL == mpc)
		return SEC_NO;

	memset((void *)mpc, 0, sizeof(Mem_Pool_Cfg));
	
	mpc->slicesize = MEM_POOL_LARGE_BUFFER_SIZE;
	mpc->slicenum = MEM_POOL_LARGE_BUFFER_NUM;
	mpc->start = (uint8_t *)mpc + sizeof(Mem_Pool_Cfg);
	mpc->totalsize = MEM_POOL_LARGE_BUFFER_NUM * MEM_POOL_LARGE_BUFFER_SIZE;
	mem_pool[MEM_POOL_ID_LARGE_BUFFER] = mpc;

	if( SEC_NO == mem_pool_sw_slice_inject(MEM_POOL_ID_LARGE_BUFFER))
	{
		return SEC_NO;
	}
	
	return SEC_OK;

}




int Mem_Pool_Get()
{
	Mem_Pool_Cfg *mpc;
	
	mpc = (Mem_Pool_Cfg *)cvmx_bootmem_find_named_block(MEM_POOL_NAME_HOST_MBUF); 
	if(NULL == mpc)
		return SEC_NO;
	
	mem_pool[MEM_POOL_ID_HOST_MBUF] = mpc;

	mpc = (Mem_Pool_Cfg *)cvmx_bootmem_find_named_block(MEM_POOL_NAME_FLOW_NODE); 
	if(NULL == mpc)
		return SEC_NO;
	
	mem_pool[MEM_POOL_ID_FLOW_NODE] = mpc;

	mpc = (Mem_Pool_Cfg *)cvmx_bootmem_find_named_block(MEM_POOL_NAME_SMALL_BUFFER); 
	if(NULL == mpc)
		return SEC_NO;
	
	mem_pool[MEM_POOL_ID_SMALL_BUFFER] = mpc;

	mpc = (Mem_Pool_Cfg *)cvmx_bootmem_find_named_block(MEM_POOL_NAME_LARGE_BUFFER); 
	if(NULL == mpc)
		return SEC_NO;
	mem_pool[MEM_POOL_ID_LARGE_BUFFER] = mpc;

	return SEC_OK;
}






