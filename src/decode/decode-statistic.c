

#include "decode-statistic.h"




pkt_stat *pktstat;



int Decode_PktStat_Init()
{
	pktstat = (pkt_stat *)cvmx_bootmem_alloc_named(sizeof(pkt_stat) * CPU_RUNNING_MAX, 128, "pkt-statistic");
	if( NULL == pktstat )
	{
		return SEC_NO;
	}

	return SEC_OK;
}

int Decode_PktStat_Get()
{
	pktstat = (pkt_stat *)cvmx_bootmem_find_named_block("pkt-statistic");
	if( NULL == pktstat )
	{
		return SEC_NO;
	}

	return SEC_OK;
}


