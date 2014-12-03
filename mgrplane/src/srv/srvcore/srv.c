#include "trans.h"
#include "common.h"




int main(int argc, char *argv[])
{
	int ch;
	
	while ((ch = getopt(argc, argv, "d:h")) != -1) {
		switch (ch) {
		case 'k':
			debugprint = 1;
			break;
		}

	}

	if (!debugprint) {
		daemon(0, 1);
	}


	server_init();

	printf("server init done.\n");

	printf("now server running....\n");
	server_run();
	
	return 0;
}



