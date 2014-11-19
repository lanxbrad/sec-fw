#ifndef __OCT_INIT_H__
#define __OCT_INIT_H__

#include <oct-common.h>


/* This define is the POW group packet destine to the Linux kernel should
    use. This must match the ethernet driver's pow_receive_group parameter */
#define TO_LINUX_GROUP          14

/* This define is the POW group packet from the Linux kernel use. This must
    match the ethernet driver's pow_send_group parameter */
#define FROM_LINUX_GROUP        13

/* This define is the POW group this program uses for packet interception.
    Packets from intercept_port are routed to this POW group instead of the
    normal ethernet pow_receive_group */
#define FROM_INPUT_PORT_GROUP   0




#define TIME_PER_TICK_US                 1000//2000
#define TIME_TICK_MAX                    400000











extern CVMX_SHARED int intercept_port;

/* wqe pool */
extern CVMX_SHARED int wqe_pool;


extern int OCT_CPU_Init();
extern int OCT_UserApp_Init();
extern void OCT_RX_Group_Init();
extern int OCT_Intercept_Port_Init(int argc, char *argv[]);
extern int OCT_Timer_Init();
extern int OCT_Create_Timer();

#endif
