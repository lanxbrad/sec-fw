/********************************************************************************
 *
 *        Copyright (C) 2014-2015  Beijing winicssec Technology 
 *        All rights reserved
 *
 *        filename :       oct-init.h
 *        description :    
 *
 *        created by  luoye  at  2014-11-18
 *
 ********************************************************************************/

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

/* This group used to rx timer work */
#define TIMER_GROUP          8



#define TIMER_FLAG_OF_WORK    0x11
#define TIMER_THREAD_MAGIC    0xabab

typedef struct Oct_Timer_Thread_t Oct_Timer_Threat;

typedef void (*timer_thread_fn)(Oct_Timer_Threat *, void *);


struct Oct_Timer_Thread_t
{
	uint32_t magic;
	uint16_t free;
	uint16_t tick;
	timer_thread_fn fn;
	void *param;
};







extern CVMX_SHARED int intercept_port;

/* wqe pool */
extern CVMX_SHARED int wqe_pool;

extern uint64_t oct_cpu_rate;

extern int local_cpu_id;


extern int OCT_CPU_Init();
extern int OCT_UserApp_Init();
extern void OCT_RX_Group_Init();
extern int OCT_Intercept_Port_Init();
extern int OCT_Timer_Init();
extern int OCT_Timer_Create(uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t qos, uint64_t grp, timer_thread_fn fn,
								void *param, uint32_t param_len, uint16_t tick);
extern void OCT_Timer_Thread_Process(cvmx_wqe_t *wq);

#endif
