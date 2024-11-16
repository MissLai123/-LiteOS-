/**
 ****************************************************************************************************
 * @file        LoRA.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       4GLoRa模块功能驱动文件
 ****************************************************************************************************
 */

#include "LoRa.h"
#include "usart.h"
#include "los_task.h"


/*******************************************************************************
* 名    称：UART8_IRQHandler
* 功    能：将接收的数据放入缓存buf中
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：硬件自动调用
*******************************************************************************/
void UART8_IRQHandler(void)
{
    uint8_t res;
    if((__HAL_UART_GET_FLAG(&huart8_LoRA,UART_FLAG_RXNE)!=RESET))  
		{	 	
			HAL_UART_Receive(&huart8_LoRA,&res,1,10);				
			if(LoRa_RX_CNT<5)
			{
				LoRa__tempdata[LoRa_RX_CNT]=res;		
				LoRa_RX_CNT++;						
			}
		}
	if(__HAL_UART_GET_FLAG(&huart8_LoRA,UART_FLAG_IDLE) != RESET)
	{
		//一帧数据接收完成
		__HAL_UART_CLEAR_IDLEFLAG(&huart8_LoRA);
	}
}

void LoRA_Receive_Data(uint8_t *buf,uint8_t *len)
{
	uint8_t rxlen=LoRa_RX_CNT;
	uint8_t i=0;
	*len=0;			
	LOS_TaskDelay(5);
	if(rxlen==LoRa_RX_CNT&&rxlen)
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=LoRa__tempdata[i];	
		}		
		*len=LoRa_RX_CNT;				
		LoRa_RX_CNT=0;		
	}
}

/*******************************************************************************
* 名    称：LoRa_ChangeBaud
* 功    能：修改波特率
* 参数说明：无
* 返 回 值：Baud：波特率数值
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void LoRa_ChangeBaud(uint32_t Baud)	//LoRa修改波特率
{
	HAL_UART_MspDeInit(&huart8_LoRA);
	LOS_TaskDelay(5);
	MX_UART8_Init(Baud);
	HAL_UART_MspInit(&huart8_LoRA);
	UART_SetConfig(&huart8_LoRA);
	LOS_TaskDelay(5);
}
/*******************************************************************************
* 名    称：LoRa_SendInstruct1
* 功    能：启动电机
* 参数说明：无
* 返 回 值：无
* 说    明：命令集1
* 调用方法：内部调用
*******************************************************************************/
static void LoRa_SendInstruct1(void)	//
{		
	HAL_UART_Transmit(&huart8_LoRA, (uint8_t *)"ON#", 3, 1000);
}

/*******************************************************************************
* 名    称：LoRa_SendInstruct2
* 功    能：二挡电机
* 参数说明：无
* 返 回 值：无
* 说    明：命令集2
* 调用方法：内部调用
*******************************************************************************/
static void LoRa_SendInstruct2(void)	
{		
	HAL_UART_Transmit(&huart8_LoRA, (uint8_t *)"Command3#", 9, 1000);   
}
/*******************************************************************************
* 名    称：LoRa_SendInstruct3
* 功    能：开关舵机
* 参数说明：无
* 返 回 值：无
* 说    明：命令集3
* 调用方法：内部调用
*******************************************************************************/
static void LoRa_SendInstruct3(void)	
{	
	HAL_UART_Transmit(&huart8_LoRA, (uint8_t *)"Command4#", 9, 1000);
}

/*******************************************************************************
* 名    称：LoRa_SendInstruct4
* 功    能：关闭电机
* 参数说明：无
* 返 回 值：无
* 说    明：命令集4
* 调用方法：内部调用
*******************************************************************************/
static void LoRa_SendInstruct4(void)	
{	
	HAL_UART_Transmit(&huart8_LoRA, (uint8_t *)"OFF#", 4, 1000);   
}

void LoRa_Run(int command)
{
	switch (command){
		case 0:
			LoRa_SendInstruct1();break;
		case 1:
			LoRa_SendInstruct2();break;
		case 2:
			LoRa_SendInstruct3();break;
		case 3:
			LoRa_SendInstruct4();break;
	}
}
