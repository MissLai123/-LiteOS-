#ifndef __USER_TASK_H
#define __USER_TASK_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes LiteOS------------------------------------------------------------------*/

#include "los_base.h"
#include "los_config.h"
#include "los_sys.h"
#include "los_typedef.h"
#include "los_task.ph"
#include "los_hwi.h"
#include "los_sem.h"
#include "los_event.h"
#include "los_memory.h"
#include "los_queue.ph"
#include "los_swtmr.h"
#include "rs485.h"
#include "gpio.h"
#include "usart.h"
#include "lwip_tcp_server.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "LoRa.h"
#include "screen.h"
#include "cmd_queue.h"
#include "4G.h"
#include "SD.h"

extern int error_len;
UINT32 Creat_User_Task1(VOID);
static void User_Task(void);
extern EVENT_CB_S EventGroup_CB;
#define LoRA_EVENT (0x01 << 1)
#define Temp1_EVENT (0x01 << 2)
#define Temp2_EVENT (0x01 << 3)
#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */
