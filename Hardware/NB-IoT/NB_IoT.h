#ifndef __NB_IOT_H
#define __NB_IOT_H
#include "usart.h"
#include "stdio.h"
#include <string.h>
#include <stdint.h>
#include "circle_buffer.h"

extern UART_HandleTypeDef huart3_NB;
extern char NB_Data[64]; 						 

void NB_IoT_SendNMGS(const char* destination);  
 
 
#endif
