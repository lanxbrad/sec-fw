#include "dp_cmd.h"
#include "decode-statistic.h"

void oct_send_response(cvmx_wqe_t *work, uint16_t opcode, void *data, uint32_t size)
{
	void *resp = NULL;
	rpc_ether_hdr_t *hdr;
	rpc_msg_t *rpcmsg;

	resp = (void *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);

	hdr = (rpc_ether_hdr_t *)resp;

	hdr->type = ETH_P;

	rpcmsg = (rpc_msg_t *)((uint8_t *)resp + sizeof(rpc_ether_hdr_t));
	rpcmsg->opcode = opcode;
	rpcmsg->info_len = size;
	memcpy((void *)rpcmsg->info_buf, data, size);

	work->packet_ptr.s.size = sizeof(rpc_ether_hdr_t) + sizeof(rpc_msg_t) + rpcmsg->info_len;

	cvmx_wqe_set_len(work, work->packet_ptr.s.size);
	cvmx_wqe_set_port(work, 0);
	cvmx_wqe_set_grp(work, TO_LINUX_GROUP);
	
	cvmx_pow_work_submit(work, work->word1.tag, work->word1.tag_type, cvmx_wqe_get_qos(work), TO_LINUX_GROUP);
}

uint16_t oct_rx_command_get(cvmx_wqe_t *work)
{
	uint8_t *data;
	rpc_msg_t *rpcmsg;
	
	if(cvmx_wqe_get_bufs(work))
	{ 
        data = cvmx_phys_to_ptr(work->packet_ptr.s.addr);
		if(NULL == data)
			return COMMAND_INVALID;
    } 
	else 
	{
        return COMMAND_INVALID;
    }

	rpcmsg = (rpc_msg_t *)data;

	return rpcmsg->opcode;
}

void dp_show_build_time(cvmx_wqe_t *wq, void *data)
{
	char out[1024];
	uint32_t len;
	memset((void *)out, 0, sizeof(out));

	sprintf(out, "%s, %s\n", __DATE__, __TIME__);
	len = strlen(out);

	oct_send_response(wq, ((rpc_msg_t *)data)->opcode, out, len);
}

void dp_show_pkt_stat(cvmx_wqe_t *wq, void *data)
{
	char out[1024];
	uint32_t len = 0;
	int i;
	uint32_t totallen = 0;
	uint8_t *ptr;
	memset((void *)out, 0, sizeof(out));

	ptr = (uint8_t *)&out;

	len = sprintf((void *)ptr, "packet statistic:\n");
	ptr += len;
	totallen += len;
	
	len = sprintf((void *)ptr, "----------------------------------\n");
	ptr += len;
	totallen += len;
	
	len = sprintf((void *)ptr, "\n");
	ptr += len;
	totallen += len;
	
	len = sprintf((void *)ptr, "recv_count:\n");
	ptr += len;
	totallen += len;
	
	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;

	uint64_t x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->rc.recv_packet_count;
	}

	len = sprintf((void *)ptr, "recv_packet_count: %ld\n", x);
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->rc.recv_packet_bytes;
	}

	len = sprintf((void *)ptr, "recv_packet_bytes: %ld\n", x);
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->rc.recv_packet_count_sum;
	}

	len = sprintf((void *)ptr, "recv_packet_count_sum: %ld\n", x);
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->rc.recv_packet_bytes_sum;
	}

	len = sprintf((void *)ptr, "recv_packet_bytes_sum: %ld\n", x);
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;

	
	len = sprintf((void *)ptr, "\n");
	ptr += len;
	totallen += len;
	
	len = sprintf((void *)ptr, "rx_stat:\n");
	ptr += len;
	totallen += len;
	
	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->rxstat.rx_err;
	}

	len = sprintf((void *)ptr, "rx_err: %ld\n", x);
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->rxstat.addr_err;
	}

	len = sprintf((void *)ptr, "addr_err: %ld\n", x);
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->rxstat.rx_ok;
	}

	len = sprintf((void *)ptr, "rx_ok: %ld\n", x);
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;

	
	len = sprintf((void *)ptr, "\n");
	ptr += len;
	totallen += len;
	
	len = sprintf((void *)ptr, "ether_stat:\n");
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->l2stat.headerlen_err;
	}

	len = sprintf((void *)ptr, "headerlen_err: %ld\n", x);
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->l2stat.unsupport;
	}

	len = sprintf((void *)ptr, "unsupport: %ld\n", x);
	ptr += len;
	totallen += len;

	x = 0;
	for(i = 0; i < CPU_HW_RUNNING_MAX; i++)
	{
		x += pktstat[i]->l2stat.rx_ok;
	}

	len = sprintf((void *)ptr, "rx_ok: %ld\n", x);
	ptr += len;
	totallen += len;
	
	printf("total len is %d\n",totallen);
	
	oct_send_response(wq, ((rpc_msg_t *)data)->opcode, out, totallen);
}


void dp_show_mem_pool(cvmx_wqe_t *wq, void *data)
{
	char out[1024];
	uint32_t len = 0;
	int i;
	uint32_t totallen = 0;
	uint8_t *ptr;
	memset((void *)out, 0, sizeof(out));

	ptr = (uint8_t *)&out;

	len = sprintf((void *)ptr, "mem pool stat:\n");
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;
	
	len = sprintf((void *)ptr, "small pool(%d bytes):\n", MEM_POOL_SMALL_BUFFER_SIZE);
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "total slice num %d.\n", MEM_POOL_SMALL_BUFFER_NUM);
	ptr += len;
	totallen += len;

	for(i = 0; i < MEM_POOL_INTERNAL_NUM; i++)
	{
		len = sprintf((void *)ptr, "pool %d:  free num %d(%d)\n", i, mem_pool[MEM_POOL_ID_SMALL_BUFFER]->mpc.msc[i].freenum, MEM_POOL_SMALL_BUFFER_NUM/MEM_POOL_INTERNAL_NUM);
		ptr += len;
		totallen += len;
	}

	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "large pool(%d bytes):\n", MEM_POOL_LARGE_BUFFER_SIZE);
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "total slice num %d.\n", MEM_POOL_LARGE_BUFFER_NUM);
	ptr += len;
	totallen += len;
	
	for(i = 0; i < MEM_POOL_INTERNAL_NUM; i++)
	{
		len = sprintf((void *)ptr, "pool %d:  free num %d(%d)\n", i, mem_pool[MEM_POOL_ID_LARGE_BUFFER]->mpc.msc[i].freenum, MEM_POOL_SMALL_BUFFER_NUM/MEM_POOL_INTERNAL_NUM);
		ptr += len;
		totallen += len;
	}

	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;


	len = sprintf((void *)ptr, "sos mem pool stat:\n");
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "----------------\n");
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "Global pool info:\n");
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "start: %p    totalsize: %d\n", sos_mem_pool->start, sos_mem_pool->total_size);
	ptr += len;
	totallen += len;

	len = sprintf((void *)ptr, "start: %p    totalsize: %d\n", sos_mem_pool->current_start, sos_mem_pool->current_size);
	ptr += len;
	totallen += len;





	

	printf("total len is %d\n",totallen);
	
	oct_send_response(wq, ((rpc_msg_t *)data)->opcode, out, totallen);
	
}




void oct_rx_process_command(cvmx_wqe_t *wq)
{
	uint16_t opcode = oct_rx_command_get(wq);
	void *data;
	if(opcode == COMMAND_INVALID)
	{
		oct_packet_free(wq, wqe_pool);
		return;
	}

	data = cvmx_phys_to_ptr(wq->packet_ptr.s.addr);
	
	switch(opcode)
	{
		case COMMAND_SHOW_BUILD_TIME:
		{
			dp_show_build_time(wq, data);
			break;
		}
		case COMMAND_SHOW_PKT_STAT:
		{
			dp_show_pkt_stat(wq, data);
			break;
		}
		case COMMAND_SHOW_MEM_POOL:
		{
			dp_show_mem_pool(wq, data);
			break;
		}
		default:
		{
			printf("unsupport command\n");
			break;
		}
	}
}
