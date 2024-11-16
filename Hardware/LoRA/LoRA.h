#ifndef __LORA_H
#define __LORA_H
#include "usart.h"
#include "stdio.h"
#include <string.h>
#include <stdint.h>
#include "circle_buffer.h"

extern UART_HandleTypeDef huart5_LoRa;
static uint8_t LoRa__tempdata[10];
static uint8_t LoRa_RX_CNT=0; 

void LoRA_Receive_Data(uint8_t *buf,uint8_t *len);
void LoRa_Run(int command);
void LoRa_ChangeBaud(uint32_t Baud);
#endif
