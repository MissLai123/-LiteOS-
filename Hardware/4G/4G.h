#ifndef __4G_H
#define __4G_H
#include "usart.h"
#include "stdio.h"
#include <string.h>
#include <stdint.h>
#include "stdio.h"
#include "screen.h"
#include "los_event.h"

extern UART_HandleTypeDef huart7_4G;
//uint8_t Level = 1;    		//0~3		
//int16_t Temperature = 16;  	//-20~1000	
//uint8_t Gas = 5;     			//0~100		
extern unsigned char Co_flag;
extern uint16_t status;
extern uint16_t statuss;
extern uint8_t G_data[10];
void Comm_STMP(void);

static char jsonMessage[256];
void Mqtt_UpLoading(char* temperature1, char* humidity1,  char*  temperature2,  char*  humidity2, char* gas);
#endif
