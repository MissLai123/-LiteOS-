#ifndef _LWIP_DEMO_H
#define _LWIP_DEMO_H
#include <stdint.h>

#define LWIP_DEMO_RX_BUFSIZE         2000   /* 最大接收数据长度 */
#define LWIP_SEND_DATA              0X80      
extern uint8_t g_lwip_send_flag;              
static uint8_t g_lwip_demo_recvbuf[LWIP_DEMO_RX_BUFSIZE];

typedef struct ServerData {
	char *PORT;
	char *IP;
	uint8_t *recvbuf;
}ServerData, *PServerData;
void lwip_run(void);
void InitServerData(PServerData PtSev);
#endif /* _CLIENT_H */
