#include "trans.h"
#include "common.h"


int main(int argc, char *argv[])
{
	server_init();

	printf("server init done.\n");

	printf("now server running....\n");
	server_run();
	
	return 0;
}



