#include <mbuf.h>
void l7_deliver(mbuf_t *m)
{
	printf("l7 enter\n");
	packet_destroy(m);
	return; 
}

