#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>



#include "message.h"


int g_rcp_fd = -1;
int g_msg_id = 1;
char recv_buf[BUFSIZE];
char send_buf[BUFSIZE];

int open_cli2server_socket(void)
{
	struct sockaddr_in server;

	server.sin_family = PF_INET;
	server.sin_addr.s_addr = htonl(SERV_LOCAL);
	server.sin_port = htons(SERV_CLI_PORT);
	while ((g_rcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) ;
	if (connect(g_rcp_fd, (struct sockaddr *)(&server), sizeof(struct sockaddr)) == -1) {
		close(g_rcp_fd);
		g_rcp_fd = -1;
		return -1;
	} 

	return 0;
}


int process_cli_show_cmd(char *rbuf, char *sbuf, int sn)
{
	unsigned int len, total_len, data_len, more_flag = 0;
	//int rn;
	//int rps_num = 0;
	//int response_end = 0;
	//int request_again = 0;
	MESSAGE_HEAD *msg_header = NULL;

	if (g_rcp_fd <= 0) {
		open_cli2server_socket();
		if (g_rcp_fd <= 0) {
			LOG("Error:can not connect to server,make sure server is running\n");
			return 1;
		}
	}

	/* send message */
	if (send(g_rcp_fd, sbuf, sn, 0) < 0) {
		exit(1);
	}

	do {
		/* first receive the rcp header */
		len = recv(g_rcp_fd, rbuf, MESSAGE_HEADER_LENGTH, 0);
		if (len != MESSAGE_HEADER_LENGTH) {
			LOG("tcpclient socket %d receive rcp header failed\n", g_rcp_fd);
			exit(1);
		}

		msg_header = (MESSAGE_HEAD *) rbuf;
		more_flag = msg_header->more_flag;
		total_len = (msg_header->length) << 2;
		data_len = total_len - MESSAGE_HEADER_LENGTH;

		/* get the data block */
		if (data_len) {
			len = recv(g_rcp_fd, rbuf + MESSAGE_HEADER_LENGTH, data_len, 0);
			if (len != data_len) {
				LOG("tcpclient socket %d receive rcp data block failed,data_len=%d,total_len=%d\n", g_rcp_fd, data_len, total_len);
				exit(1);
			}
			printf("%s", (char *)(rbuf + MESSAGE_HEADER_LENGTH));
		}
		memset(rbuf, 0, BUFSIZE);
	} while (more_flag);


	return 0;
}





