/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       oct-init.c
 *        description :    init of oct
 *
 *        created by  luoye  at  2014-11-18
 *
 ********************************************************************************/

#include <sec-common.h>
#include <oct-common.h>
#include "oct-init.h"



/* This is the Octeon hardware port number to intercept. Packets coming
    in this port are intercepted by linux-filter and processed. Packets
    received from the ethernet POW0 device are sent out this port */
CVMX_SHARED int intercept_port = 0;

/* wqe pool */
CVMX_SHARED int wqe_pool = -1;





int local_cpu_id;
uint64_t oct_cpu_rate;

cvmx_sysinfo_t *sysinfo;







int OCT_UserApp_Init()
{
	cvmx_skip_app_config_set();
	cvmx_user_app_init();
	sysinfo = cvmx_sysinfo_get();

	return SEC_OK;
}


void OCT_RX_Group_Init()
{
	/* Wait for global hardware init to complete */
    cvmx_coremask_barrier_sync(&sysinfo->core_mask);

    /* Setup scratch registers used to prefetch output queue buffers for packet output */
    cvmx_pko_initialize_local();

    /* Accept any packet except for the ones destined to the Linux group */
    cvmx_pow_set_group_mask(cvmx_get_core_num(),
                            (1<<FROM_INPUT_PORT_GROUP)|(1<<FROM_LINUX_GROUP) | (1<<TIMER_GROUP));

    /* Wait for hardware init to complete */
    cvmx_coremask_barrier_sync(&sysinfo->core_mask);

	return;
}


int OCT_Intercept_Port_Init(int argc, char *argv[])
{
	long port_override = -1;

	if (argc > 1)
    	port_override = strtol(argv[1], NULL, 0);

    printf("\n\nLoad the Linux ethernet driver with:\n"
           "\t $ modprobe octeon-ethernet\n"
           "\t $ modprobe octeon-pow-ethernet receive_group=1 broadcast_groups=4 ptp_rx_group=%d ptp_tx_group=%d\n",
           TO_LINUX_GROUP, FROM_LINUX_GROUP);

    printf("Waiting for ethernet module to complete initialization...\n\n\n");
    cvmx_ipd_ctl_status_t ipd_reg;
    do
    {
        ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
    } while (!ipd_reg.s.ipd_en);

/* Wait a second for things to really get started. */
    if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM)
    cvmx_wait_usec(1000000);
#ifdef CVMX_PKO_USE_FAU_FOR_OUTPUT_QUEUES
    #error Linux-filter cannot be built with CVMX_PKO_USE_FAU_FOR_OUTPUT_QUEUES
#endif

	if (port_override > 0)
    	intercept_port = port_override;

	__cvmx_helper_init_port_valid();

	__cvmx_import_app_config_from_named_block(CVMX_APP_CONFIG);

	__cvmx_helper_init_port_config_data_local();

	wqe_pool = cvmx_fpa_get_wqe_pool();


    /* Change the group for only the port we're interested in */
    cvmx_pip_port_tag_cfg_t tag_config;
    tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(intercept_port));
    tag_config.s.grp = FROM_INPUT_PORT_GROUP;
    cvmx_write_csr(CVMX_PIP_PRT_TAGX(intercept_port), tag_config.u64);
	
    /* We need to call cvmx_cmd_queue_initialize() to get the pointer to
        the named block. The queues are already setup by the ethernet
        driver, so we don't actually need to setup a queue. Pass some
        invalid parameters to cause the queue setup to fail */
    cvmx_cmd_queue_initialize(0, 0, -1, 0);
    printf("Waiting for packets from port %d... \n", intercept_port);

	return SEC_OK;
}


int OCT_Timer_Init()
{
	int status;
	
    status = cvmx_tim_setup(1000 , 5000);
    if (status != 0) {
        return SEC_NO;
    }
    cvmx_tim_start();

    return SEC_OK;
}

int OCT_Timer_Create(uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t qos, uint64_t grp, timer_thread_fn fn,
								void *param, uint32_t param_len, uint16_t tick)
{
	cvmx_wqe_t *wqe_p;
	cvmx_tim_status_t result;
	Oct_Timer_Threat *o;

	if( grp >= 16 || param_len > 96 - sizeof(Oct_Timer_Threat))
	{
		return SEC_NO;
	}
	
	wqe_p = cvmx_fpa_alloc(CVMX_FPA_WQE_POOL);
	if (wqe_p == NULL) 
	{
		return SEC_NO;
	}

	memset(wqe_p, 0, sizeof(cvmx_wqe_t));

	cvmx_wqe_set_unused8(wqe_p, TIMER_FLAG_OF_WORK);
	cvmx_wqe_set_tag(wqe_p, tag);
	cvmx_wqe_set_tt(wqe_p, tag_type);
	cvmx_wqe_set_qos(wqe_p, qos);
	cvmx_wqe_set_grp(wqe_p, grp);

	o = (Oct_Timer_Threat *)wqe_p->packet_data;
	o->magic = TIMER_THREAD_MAGIC;
	o->fn = fn;
	o->param = (void *)o+sizeof(Oct_Timer_Threat);
	o->tick = tick;

	result = cvmx_tim_add_entry(wqe_p, o->tick, NULL);

	CVMX_SYNCW;
	return result;
}

void OCT_Timer_Thread_Process(cvmx_wqe_t *wq)
{
	Oct_Timer_Threat *o;
	o = (Oct_Timer_Threat *)wq->packet_data;
	if( TIMER_THREAD_MAGIC != o->magic || TIMER_FLAG_OF_WORK != cvmx_wqe_get_unused8(wq))
	{
		printf("this is not a valid tim work\n");
		return;
	}

	if (o->fn != NULL) 
	{
		o->fn(o, o->param);
	}

	cvmx_tim_add_entry(wq, o->tick, NULL);
	CVMX_SYNCW;

	return;
}


int OCT_CPU_Init()
{
	local_cpu_id = cvmx_get_core_num();

	oct_cpu_rate = cvmx_clock_get_rate(CVMX_CLOCK_CORE);

	return SEC_OK;
}


















