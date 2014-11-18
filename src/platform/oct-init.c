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
                            (1<<FROM_INPUT_PORT_GROUP)|(1<<FROM_LINUX_GROUP));

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

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
	    cvmx_pip_prt_tagx_t tag_config;
	    cvmx_gmxx_prtx_cfg_t prt_cfg;
	    int pkind;
	    int iface = (intercept_port >> 8) - 8;
	    int iport = (intercept_port >> 4) & 0xf;
	    
	    if (iface < 0)
		iface = 0;

	    prt_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(iport, iface));
	    pkind = prt_cfg.s.pknd;

	    tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(pkind));
	    tag_config.s.grp = FROM_INPUT_PORT_GROUP & 0xf;
	    tag_config.s.grp_msb = (FROM_INPUT_PORT_GROUP >> 4) & 3;
	    cvmx_write_csr(CVMX_PIP_PRT_TAGX(pkind), tag_config.u64);
	} else {
	    /* Change the group for only the port we're interested in */
	    cvmx_pip_port_tag_cfg_t tag_config;
	    tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(intercept_port));
	    tag_config.s.grp = FROM_INPUT_PORT_GROUP;
	    cvmx_write_csr(CVMX_PIP_PRT_TAGX(intercept_port), tag_config.u64);
	}
    /* We need to call cvmx_cmd_queue_initialize() to get the pointer to
        the named block. The queues are already setup by the ethernet
        driver, so we don't actually need to setup a queue. Pass some
        invalid parameters to cause the queue setup to fail */
    cvmx_cmd_queue_initialize(0, 0, -1, 0);
    printf("Waiting for packets from port %d... \n", intercept_port);

	return SEC_OK;
}




int OCT_CPU_Init()
{
	local_cpu_id = cvmx_get_core_num();


	return SEC_OK;
}


















