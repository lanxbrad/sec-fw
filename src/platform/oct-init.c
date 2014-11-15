#include <oct-common.h>
#include "oct-init.h"


int local_cpu_id;


int OCT_CPU_Init()
{
	local_cpu_id = cvmx_get_core_num();

	return SEC_OK;
}









