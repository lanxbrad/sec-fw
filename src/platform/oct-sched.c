/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       oct-sched.c
 *        description :    
 *
 *        created by  luoye  at  2014-11-18
 *
 ********************************************************************************/

#include "oct-sched.h"
#include <sec-util.h>


oct_sched_t *sched_tbl;


int oct_sched_init(void)
{
	sched_tbl = (oct_sched_t *)cvmx_bootmem_alloc_named(sizeof(oct_sched_t) , CACHE_LINE_SIZE, OCT_SCHED_TABLE_NAME);
	if(NULL == sched_tbl)
	{
		printf("oct_sched_init no mem\n");
		return SEC_NO;
	}

	memset((void *)sched_tbl, 0, sizeof(oct_sched_t));

	return SEC_OK;
}

int oct_sched_Get(void)
{
	sched_tbl = (oct_sched_t *)cvmx_bootmem_find_named_block(OCT_SCHED_TABLE_NAME); 
	if(NULL == sched_tbl)
	{
		printf("oct_sched_Get error \n");
		return SEC_NO;
	}

	return SEC_OK;
}





