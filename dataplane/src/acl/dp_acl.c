#include <sec-common.h>
#include <shm.h>
#include <acl_rule.h>






rule_list_t *rule_list;



uint32_t DP_Acl_Rule_Init()
{
	int fd;
	
	fd = shm_open(SHM_RULE_LIST_NAME, O_RDWR, 0);

	if (fd < 0) {
		printf("Failed to setup CVMX_SHARED(shm_open)");
		return SEC_NO;
	}

	//if (shm_unlink(SHM_RULE_LIST_NAME) < 0)
	//	printf("Failed to shm_unlink shm_name");

	ftruncate(fd, sizeof(rule_list_t));
		
	
	void *ptr = mmap(NULL, sizeof(rule_list_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == NULL) 
	{
		printf("Failed to setup rule list (mmap copy)");
		return SEC_NO;
	}
	rule_list = (rule_list_t *)ptr;

	return SEC_OK;
	
}
