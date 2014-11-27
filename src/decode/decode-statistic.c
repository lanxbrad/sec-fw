

#include "decode-statistic.h"




pkt_stat *pktstat[CPU_HW_RUNNING_MAX];



int Decode_PktStat_Init()
{
	int i;
	void *start;
	start = (void *)cvmx_bootmem_alloc_named(sizeof(pkt_stat) * CPU_HW_RUNNING_MAX, 128, "pkt-statistic");
	if( NULL == start )
	{
		return SEC_NO;
	}
	memset(start, 0, sizeof(pkt_stat) * CPU_HW_RUNNING_MAX);

	for( i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		pktstat[i] = (pkt_stat *)((uint8_t *)start + i * sizeof(pkt_stat));
	}

	return SEC_OK;
}

int Decode_PktStat_Get()
{
	int i;
	void *start;

	const cvmx_bootmem_named_block_desc_t *block_desc = cvmx_bootmem_find_named_block("pkt-statistic"); 
	if (block_desc)
	{
		start = (void *)(block_desc->base_addr);
	}
	else
	{
		printf("oct_sched_Get error \n");
		return SEC_NO;
	}
	
	
	for( i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		pktstat[i] = (pkt_stat *)((uint8_t *)start + i * sizeof(pkt_stat));
	}

	
	return SEC_OK;
}


