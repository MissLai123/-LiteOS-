/**
 ****************************************************************************************************
 * @file        NB_IoT.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       NB_IoT模块功能驱动文件
 ****************************************************************************************************
 */
#include "NB_IoT.h"
#include "usart.h"

char NB_Data[64]; 						

//NB-IoT上云函数
void NB_IoT_SendNMGS(const char* destination)  
{  
  
    // 构造AT指令 
    snprintf(NB_Data, sizeof(NB_Data), "AT+NMGS=5,\"%s\"\r\n", destination);  
  
    // 发送数据
    HAL_UART_Transmit(&huart3_NB, (uint8_t*)NB_Data, strlen(NB_Data), 1000);
}
