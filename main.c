/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc.y (support@cavium.com). All rights 
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc.y nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.  

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries. 

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/


/*
 * File version info: $Id: secd.c 87283 2013-08-23 18:13:43Z lrosenboim $
 *
 */

#include <stdio.h>
#include <string.h>
#ifdef __linux__
#include <unistd.h>
#endif

#include <mbuf.h>

#include <oct-common.h>
#include <mem_pool.h>
#include <decode-statistic.h>
#include <decode-defrag.h>
#include <oct-init.h>
#include <oct-rxtx.h>
#include <oct-sched.h>
#include <flow.h>
#include <dp_cmd.h>
#include <sos_malloc.h>


extern flow_item_t *flow_item_alloc();
extern void *oct_rx_process_work(cvmx_wqe_t *wq);
extern void Decode(mbuf_t *mbuf);
extern cvmx_sysinfo_t *sysinfo;

int Sec_LowLevel_Init()
{

	OCT_CPU_Init();

	OCT_UserApp_Init();

	if (cvmx_is_init_core())
	{	/* Have one core do the hardware initialization */
		OCT_Intercept_Port_Init();

		if (SEC_OK != OCT_Timer_Init())
		{
			printf("OCT_Timer_Init fail\n");
			return SEC_NO;
		}
		else
		{
			printf("OCT_Timer_Init ok\n");
		}

		if(SEC_OK != Mem_Pool_Init())
		{
			return SEC_NO;
		}
		printf("Mem_Pool_Init ok\n");

		if(SEC_OK != sos_mem_init())
		{
			return SEC_NO;
		}
		printf("sos_mem_init ok\n");

		if(SEC_OK != oct_sched_init())
		{
			return SEC_NO;
		}
		printf("oct_sched_init ok\n");

		if(SEC_OK != oct_rxtx_init())
		{
			return SEC_NO;
		}

		printf("oct_rxtx_init ok\n");

		wd_watchdog_init();

		printf("wd_watchdog_init ok\n");

	}

	OCT_RX_Group_Init();

	if (!cvmx_is_init_core())
	{
		if(SEC_OK != Mem_Pool_Get())
		{
			printf("mem pool info get failed!\n");
			return SEC_NO;
		}
		printf("mem pool info get ok!\n");

		if(SEC_OK != sos_mem_get())
		{
			printf("sos_mem_get failed!\n");
			return SEC_NO;
		}
		printf("sos_mem_get ok!\n");
		
		if(SEC_OK != oct_sched_Get())
		{
			printf("oct_sched_Get fail\n");
			return SEC_NO;
		}
		printf("oct_sched_Get ok\n");

		if(SEC_OK != oct_rxtx_get())
		{
			printf("oct_rxtx_get fail\n");
			return SEC_NO;
		}
		printf("oct_rxtx_get ok\n");
		
	}

	register_watchdog();

	return SEC_OK;

}


int Sec_HighLevel_Init()
{
	mbuf_size_judge();
	flow_item_size_judge();

	if ( cvmx_is_init_core() )
	{
		
		if(SEC_OK != Decode_PktStat_Init())
		{
			printf("Decode_PktStat_Init failed\n");
			return SEC_NO;
		}

		printf("Decode_PktStat_Init ok\n");

		if(SEC_OK != FragModule_init())
		{
			printf("FragMoudle_init failed\n");
		}
		printf("FragModule_init ok\n");
		
		if(SEC_OK != FlowInit())
		{
			printf("FlowInit failed\n");
			return SEC_NO;
		}
		printf("FlowInit ok\n");
	}

	cvmx_coremask_barrier_sync(&sysinfo->core_mask);
	
	if ( !cvmx_is_init_core() )
	{
		if(SEC_OK != Decode_PktStat_Get())
		{
			printf("Decode_PktStat_Get failed\n");
			return SEC_NO;
		}

		printf("Decode_PktStat_Get ok\n");

		if(SEC_OK != FragModuleInfo_Get())
		{
			printf("FragModuleInfo_Get failed\n");
			return SEC_NO;
		}	

		printf("FragModuleInfo_Get ok\n");

		if(SEC_OK != FlowInfoGet())
		{
			printf("FlowInfoGet failed\n");
			return SEC_NO;
		}	

		printf("FlowInfoGet ok\n");
	}

	if(SEC_OK != DP_Acl_Rule_Init())
	{
		printf("DP_Acl_Rule_Init failed\n");
		return SEC_NO;
	}
	

	return SEC_OK;
}



uint32_t global_time = 0;


void mainloop()
{
	mbuf_t *mb;
	int grp;
	cvmx_wqe_t *work;
	while(1){

		if(unlikely(oct_tx_entries)) {
			oct_tx_done_check();
		}
		
		work = cvmx_pow_work_request_sync(CVMX_POW_WAIT);
		if (NULL != work)
		{
			grp = cvmx_wqe_get_grp(work);

			if ( FROM_INPUT_PORT_GROUP == grp )
			{
				mb = (mbuf_t *)oct_rx_process_work(work);
				if (NULL == mb)
				{
					continue;
				}
				Decode(mb);
			}
			else if ( TIMER_GROUP == grp )
			{
				global_time++;
				watchdog_ok();
				OCT_Timer_Thread_Process(work);
			}
			else if( FROM_LINUX_GROUP == grp )
			{
				printf("receive packet from linux!\n");
				oct_rx_process_command(work);
			}
			else
			{
				printf("work group error %d\n", grp);
			}
			
		}
		else
		{
			continue;
		}
	}
}





/**
 * Main entry point
 *
 * @return exit code
 */
int main(int argc, char *argv[])
{
    
    if(SEC_OK != Sec_LowLevel_Init())
	{
		printf("sec lowlevel init err!\n");
		exit(0);
	}
	else
	{
		printf("sec lowlevel init ok!\n");
	}

	if(SEC_OK != Sec_HighLevel_Init())
	{
		printf("sec HighLevel init err!\n");
		exit(0);
	}
	else
	{
		printf("sec HighLevel init ok!\n");
	}

	mainloop();
	

    return 0;
}


#ifdef xxxx
int old_main(int argc, char *argv[])
{
    long port_override = -1;

    cvmx_skip_app_config_set();
    cvmx_user_app_init();
    cvmx_sysinfo_t *sysinfo = cvmx_sysinfo_get();

    /* Have one core do the hardware initialization */
    if (cvmx_is_init_core())
    {
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
#if CVMX_PKO_USE_FAU_FOR_OUTPUT_QUEUES
        #error Linux-filter cannot be built with CVMX_PKO_USE_FAU_FOR_OUTPUT_QUEUES
#endif

        if (OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX))
        {
            cvmx_gmxx_inf_mode_t mode;

            /* Choose interface that is enabled and in RGMII mode. */
            mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(0));
            if (mode.s.en && mode.s.type == 0) {
	        /* Use interface 0 */
	        intercept_port = 0;
            } else {
	        /* Use interface 1 */
	        intercept_port = 16;
            }
        }

	/* Their is no interface 0 on nic_xle_4g card, use interface 1. */
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_NIC_XLE_4G)
	    intercept_port = 16;

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
    }

    /* Wait for global hardware init to complete */
    cvmx_coremask_barrier_sync(&sysinfo->core_mask);

    /* Setup scratch registers used to prefetch output queue buffers for packet output */
    cvmx_pko_initialize_local();

    /* Accept any packet except for the ones destined to the Linux group */
    cvmx_pow_set_group_mask(cvmx_get_core_num(),
                            (1<<FROM_INPUT_PORT_GROUP)|(1<<FROM_LINUX_GROUP));

    /* Wait for hardware init to complete */
    cvmx_coremask_barrier_sync(&sysinfo->core_mask);

    while (1)
    {
#ifdef __linux__
        /* Under Linux there better thing to do than halt the CPU waiting for
            work to show up. Here we use NO_WAIT so we can continue processing
            instead of stalling for work */
        cvmx_wqe_t *work = cvmx_pow_work_request_sync(CVMX_POW_NO_WAIT);
        if (work == NULL)
        {
            /* Yield to other processes since we don't have anything to do */
            usleep(0);
            continue;
        }
#else
        /* In standalone CVMX, we have nothing to do if there isn't work, so
            use the WAIT flag to reduce power usage */
        cvmx_wqe_t *work = cvmx_pow_work_request_sync(CVMX_POW_WAIT);
        if (work == NULL)
            continue;
#endif

        /* Check for errored packets, and drop.  If sender does not respond to
            backpressure or backpressure is not sent, packets may be truncated
            if the GMX fifo overflows. */
        if (work->word2.s.rcv_error)
        {
            /* Work has error, so drop */
	    printf("error is %d\n", work->word2.s.rcv_error);
            cvmx_helper_dump_packet(work);
            cvmx_helper_free_packet_data(work);
            cvmx_fpa_free(work, wqe_pool, 0);
            continue;
        }

        /* See if we should filter this packet */
        if (is_filtered_packet(work))
        {
            printf("Received %u byte packet. Filtered.\n", cvmx_wqe_get_len(work));
            cvmx_helper_free_packet_data(work);
            cvmx_fpa_free(work, wqe_pool, 0);
        }
        else if (cvmx_wqe_get_grp(work) == FROM_LINUX_GROUP)
        {
            uint64_t queue = cvmx_pko_get_base_queue(intercept_port);

            printf("Received %u byte packet from Linux. Sending to PKO\n", cvmx_wqe_get_len(work));

            cvmx_pko_send_packet_prepare(intercept_port, queue, CVMX_PKO_LOCK_CMD_QUEUE);

            /* Build a PKO pointer to this packet */
            cvmx_pko_command_word0_t pko_command;
            pko_command.u64 = 0;
            pko_command.s.segs = work->word2.s.bufs;
            pko_command.s.total_bytes = cvmx_wqe_get_len(work);

            if (work->word2.s.tcp_or_udp && !work->word2.s.is_frag)
                pko_command.s.ipoffp1 = 14 + 1;
            else
                pko_command.s.ipoffp1 = 0;

            /* Send the packet */
            cvmx_pko_return_value_t send_status = cvmx_pko_send_packet_finish(intercept_port, queue, pko_command, work->packet_ptr, CVMX_PKO_LOCK_CMD_QUEUE);
            if (send_status != CVMX_PKO_SUCCESS)
            {
                printf("Failed to send packet using cvmx_pko_send_packet2\n");
                cvmx_helper_free_packet_data(work);
            }

            cvmx_fpa_free(work, wqe_pool, 0);
        }
        else
        {
            printf("Received %u byte packet. Sending to Linux.\n", cvmx_wqe_get_len(work));

	    	cvmx_helper_dump_packet(work);
            cvmx_wqe_set_port(work, 0);
#ifdef __linux__
            /* If we're running under Linux userspace we can't desched since
                the ethernet driver might give away our tag. Use submit work
                instead */
            cvmx_pow_work_submit(work, work->word1.tag, work->word1.tag_type, cvmx_wqe_get_qos(work), TO_LINUX_GROUP);
#else
            /* Forward the packet to the linux kernel.
               It is recommented to switch explicitly to an ATOMIC tag during deschedule.
               Please see documentation of cvmx_pow_tag_sw_desched() for details. */
            cvmx_pow_tag_sw_desched(work->word1.tag, CVMX_POW_TAG_TYPE_ATOMIC, TO_LINUX_GROUP, 0);
#endif
        }
    }

    return 0;
}
#endif

