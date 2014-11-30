#ifndef __CLI_TRANS_H__
#define __CLI_TRANS_H__


extern int g_msg_id;

extern char recv_buf[BUFSIZE];
extern char send_buf[BUFSIZE];



extern int open_cli2server_socket(void);
extern int process_cli_show_cmd(char *rbuf, char *sbuf, int sn);


#endif
