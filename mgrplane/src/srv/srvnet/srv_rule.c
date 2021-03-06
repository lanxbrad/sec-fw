#include "srv_rule.h"
#include <message.h>
#include <trans.h>
#include <pow.h>
#include <rpc-common.h>
#include <shm.h>




rule_list_t *rule_list;





static inline int Rule_compare(RCP_BLOCK_ACL_RULE_TUPLE *rule1, RCP_BLOCK_ACL_RULE_TUPLE *rule2)
{
	return memcmp((void *)rule1, (void *)rule2, sizeof(RCP_BLOCK_ACL_RULE_TUPLE));
}


static inline int find_first_free()
{
	int i;
	for(i = 0; i < RULE_ENTRY_MAX; i++)
	{
		if(rule_list->rule_entry[i].entry_status == RULE_ENTRY_STATUS_FREE)
			return i;
	}

	return -1;
}

int Rule_duplicate_check(RCP_BLOCK_ACL_RULE_TUPLE *rule)
{
	int i = 0;
	for (i = 0; i < RULE_ENTRY_MAX; i++)
	{
		if(rule_list->rule_entry[i].entry_status == RULE_ENTRY_STATUS_FREE)
			continue;

		if(0 == Rule_compare(rule, &rule_list->rule_entry[i].rule_tuple))
		{
			return RULE_EXIST;
		}
	}

	return RULE_OK;
}


int Rule_list_init()
{
	/*TODO:alloc rule_list a share mem*/
	int fd;

	fd = shm_open(SHM_RULE_LIST_NAME, O_RDWR | O_CREAT | O_TRUNC, 0);

	if (fd < 0) {
		printf("Failed to setup CVMX_SHARED(shm_open)");
		return -1;
	}

	//if (shm_unlink(SHM_RULE_LIST_NAME) < 0)
	//		printf("Failed to shm_unlink shm_name");

	ftruncate(fd, sizeof(rule_list_t));
		
	
	void *ptr = mmap(NULL, sizeof(rule_list_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == NULL) 
	{
		printf("Failed to setup rule list (mmap copy)");
		return -1;
	}
	rule_list = (rule_list_t *)ptr;

	memset((void *)rule_list, 0, sizeof(rule_list_t));
	
	rule_list->rule_entry_free = RULE_ENTRY_MAX;

	return 0;
}


int Rule_add(RCP_BLOCK_ACL_RULE_TUPLE *rule)
{
	int index; 

	if(rule_list->rule_entry_free == 0)
	{
		printf("Rule Full\n");
		return RULE_FULL;
	}

	if( RULE_EXIST == Rule_duplicate_check(rule))
	{
		printf("Rule already exist\n");
		return RULE_EXIST;
	}

	index = find_first_free();
	if(-1 == index)
	{
		printf("Rule Full\n");
		return RULE_FULL;
	}

	memcpy( (void *)&rule_list->rule_entry[index].rule_tuple, rule, sizeof(RCP_BLOCK_ACL_RULE_TUPLE));

	rule_list->rule_entry_free--;
	rule_list->rule_entry[index].entry_status = RULE_ENTRY_STATUS_USED;
	rule_list->build_status = RULE_BUILD_UNCOMMIT;

	return RULE_OK;

}

int Rule_del(RCP_BLOCK_ACL_RULE_TUPLE *rule)
{
	int i;
	int ret;
	
	if(rule_list->rule_entry_free == RULE_ENTRY_MAX)
	{
		return RULE_NOT_EXIST;
	}

	for( i = 0; i < RULE_ENTRY_MAX; i++ )
	{
		if(rule_list->rule_entry[i].entry_status == RULE_ENTRY_STATUS_FREE)
		{
			continue;
		}

		ret = Rule_compare(&rule_list->rule_entry[i].rule_tuple, rule);
		if(0 == ret)
		{
			rule_list->rule_entry[i].entry_status = RULE_ENTRY_STATUS_FREE;
			rule_list->rule_entry_free++;
			rule_list->build_status = RULE_BUILD_UNCOMMIT;
			return RULE_OK;
		}
		else
		{
			continue;
		}
	}

	return RULE_NOT_EXIST;
}

int Rule_del_by_id(RCP_BLOCK_ACL_RULE_ID *id)
{
	if(RULE_ENTRY_MAX == rule_list->rule_entry_free)
	{
		return RULE_NOT_EXIST;
	}

	if(RULE_ENTRY_STATUS_FREE == rule_list->rule_entry[id->rule_id].entry_status)
	{
		return RULE_NOT_EXIST;
	}
	else 
	{
		rule_list->rule_entry[id->rule_id].entry_status = RULE_ENTRY_STATUS_FREE;
		rule_list->rule_entry_free++;
		rule_list->build_status = RULE_BUILD_UNCOMMIT;
		return RULE_OK;
	}
	
}




int Rule_del_all()
{
	int i;
	for (i = 0; i < RULE_ENTRY_MAX; i++)
	{
		rule_list->rule_entry[i].entry_status = RULE_ENTRY_STATUS_FREE;
	}
	rule_list->rule_entry_free = RULE_ENTRY_MAX;
	rule_list->build_status = RULE_BUILD_UNCOMMIT;

	return RULE_OK;
}



void Rule_Save_File(FILE *fp)
{
	int i;

	for( i = 0; i < RULE_ENTRY_MAX; i++ )
	{
		if(rule_list->rule_entry[i].entry_status == RULE_ENTRY_STATUS_FREE)
		{
			continue;
		}

		fprintf(fp, 
			"%d:  sip:%x  sip_mask:%d, dip:%x  dip_mask:%d, sport_start:%d, sport_end:%d\n", 
			i,
			rule_list->rule_entry[i].rule_tuple.sip,
			rule_list->rule_entry[i].rule_tuple.sip_mask,
			rule_list->rule_entry[i].rule_tuple.dip,
			rule_list->rule_entry[i].rule_tuple.dip_mask,
			rule_list->rule_entry[i].rule_tuple.sport_start,
			rule_list->rule_entry[i].rule_tuple.sport_end);
	}
}




int Rule_show_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	LOG("Rule_show_acl_rule\n");
	char tmp_rule_file[80];
	FILE *fp = NULL;
	uint32_t result_code;
	uint8_t s_buf[MAX_BUF];
	cmd_type_t cmd_ack = SHOW_ACL_RULE_ACK;

	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;

	sprintf(tmp_rule_file, "/tmp/tmp_rule");

	fp = fopen(tmp_rule_file, "w+");

	if ( NULL != fp )
	{
		fprintf(fp, "ACL Rule:\n");
		Rule_Save_File(fp);
		fclose(fp);
		result_code = RCP_RESULT_OK;
	}
	else
	{
		result_code = RCP_RESULT_FILE_ERR;
	}
	
	rcp_param_p->nparam = 1;
	rcp_param_p->params_list.params[0].CliResultCode.result_code = result_code;
	
	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);

	return 0;
}



int Rule_add_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	int ret;
	LOG("Rule_add_acl_rule\n");
	uint8_t s_buf[MAX_BUF];
	cmd_type_t cmd_ack = ADD_ACL_RULE_ACK;

	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;

	RCP_BLOCK_ACL_RULE_TUPLE *blocks = (RCP_BLOCK_ACL_RULE_TUPLE *)(from + MESSAGE_HEADER_LENGTH);

	/*ADD RULE INFO INTO LOCAL MANAGER*/
	ret = Rule_add(blocks);
	if(RULE_OK == ret)
	{
		rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_OK;
	}
	else if(RULE_FULL == ret)
	{
		rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_RULE_FULL;
	}
	else if(RULE_EXIST == ret)
	{
		rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_RULE_EXIST;
	}

	rcp_param_p->nparam = 1;
	

	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);
	
	return 0;
}


int Rule_del_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	int ret;
	LOG("Rule_del_acl_rule\n");

	uint8_t s_buf[MAX_BUF];
	cmd_type_t cmd_ack = DEL_ACL_RULE_ACK;

	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;

	RCP_BLOCK_ACL_RULE_TUPLE *blocks = (RCP_BLOCK_ACL_RULE_TUPLE *)(from + MESSAGE_HEADER_LENGTH);

	ret = Rule_del(blocks);
	if( RULE_OK == ret )
	{
		rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_OK;
	}
	else if( RULE_NOT_EXIST == ret )
	{	
		rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_RULE_NOT_EXIST;
	}

	rcp_param_p->nparam = 1;
	
	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);

	return 0;
}


int Rule_del_acl_rule_id(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	int ret;
	LOG("Rule_del_acl_rule_id\n");

	uint8_t s_buf[MAX_BUF];
	cmd_type_t cmd_ack = DEL_ACL_RULE_ID_ACK;

	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;

	RCP_BLOCK_ACL_RULE_ID *blocks = (RCP_BLOCK_ACL_RULE_ID *)(from + MESSAGE_HEADER_LENGTH);

	ret = Rule_del_by_id(blocks);
	if( RULE_OK == ret )
	{
		rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_OK;
	}
	else if( RULE_NOT_EXIST == ret )
	{	
		rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_RULE_NOT_EXIST;
	}

	rcp_param_p->nparam = 1;
	
	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);

	return 0;
}



int Rule_del_acl_rule_all(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	LOG("Rule_del_acl_rule_all\n");
	
	uint8_t s_buf[MAX_BUF];
	cmd_type_t cmd_ack = DEL_ACL_RULE_ALL_ACK;
	
	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;
	
	Rule_del_all();

	rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_OK;

	rcp_param_p->nparam = 1;
	
	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);
	
	return 0;
}


int Rule_commit_acl_rule(uint8_t * from, uint32_t length, uint32_t fd, void *param_p)
{
	LOG("Rule_commit_acl_rule\n");

	uint8_t s_buf[MAX_BUF];
	cmd_type_t cmd_ack = COMMIT_ACL_RULE_ACK;

	
	struct rcp_msg_params_s *rcp_param_p = (struct rcp_msg_params_s *)param_p;

	rcp_param_p->params_list.params[0].CliResultCode.result_code = RCP_RESULT_OK;
	
	rcp_param_p->nparam = 1;
		
	send_rcp_res(cmd_ack, from, s_buf, fd, param_p, 0);
		
	return 0;
}




