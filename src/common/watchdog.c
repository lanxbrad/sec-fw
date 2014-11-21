#include <oct-sched.h>
#include <pthread.h>

int wd_is_watchdog_registered(int cpuid)
{
	return (sched_tbl->data[cpuid].watchdog_enabled);
}


void wd_disable_watchdog(void)
{
	sched_tbl->watchdog_disabled = 1;
}


void wd_enable_watchdog(void)
{
	sched_tbl->watchdog_disabled = 0;
}


static uint32_t wd_is_watchdog_disabled(void)
{
	return sched_tbl->watchdog_disabled;
}


void wd_set_watchdog_timeout(uint32_t sec)
{
	if(sec < WD_WATCHDOG_TIMEOUT)
	{
		sec = WD_WATCHDOG_TIMEOUT;
	}

	sched_tbl->watchdog_retry = sec/WD_WATCHDOG_CHECK_INTERVAL;
}

void wd_check_watchdog()
{
	int i;
	int watchdog_fired = 0;
	
	if(wd_is_watchdog_disabled())
	{
		return;
	}

	for (i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		if(!wd_is_watchdog_registered(i))
		{
			continue;
		}

		if(sched_tbl->data[i].watchdog_ok > sched_tbl->watchdog_retry)
		{
			watchdog_fired = 1;
		}
		++sched_tbl->data[i].watchdog_ok;
	}

	if( watchdog_fired)
	{
		abort();
	}
}



static pthread_t wd_watchdog_thread;


static void *wd_watchdog_func(void *arg)
{
	int rc;
	while(1)
	{
		rc = sleep(WD_WATCHDOG_CHECK_INTERVAL);

		if(0 == rc)
		{
			wd_check_watchdog(NULL);
		}
	}
	return NULL;
}

void sec_watchdog_init(void)
{
	wd_enable_watchdog();

	wd_set_watchdog_timeout(WD_WATCHDOG_TIMEOUT);

	pthread_create(&wd_watchdog_thread, NULL, wd_watchdog_func, NULL);
	return;
}
