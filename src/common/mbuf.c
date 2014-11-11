#include <mbuf.h>


void mbuf_init(void)
{
	BUILD_BUG_ON((sizeof(m_buf)) > 96);

	return;
}
