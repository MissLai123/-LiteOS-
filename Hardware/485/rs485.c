/**
 ****************************************************************************************************
 * @file        RS485.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       485模块功能驱动文件
 ****************************************************************************************************
 */

#include "rs485.h"
#include "los_task.h" 
#include "SD.h"
float RS485_Data[10]={0};

/*******************************************************************************
* 名    称：USART2_IRQHandler
* 功    能：将接收的数据放入缓存buf中
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：硬件自动调用
*******************************************************************************/
void USART2_IRQHandler(void)
{
  uint8_t res;	
  if((__HAL_UART_GET_FLAG(&huart2_RS485,UART_FLAG_RXNE)!=RESET))  
	{			
		HAL_UART_Receive(&huart2_RS485,&res,1,10);
		if(RS485_RX_CNT<64)
		{
			RS485_RX_BUF[RS485_RX_CNT]=res;		
			RS485_RX_CNT++;						
		}
	}
	if(__HAL_UART_GET_FLAG(&huart2_RS485,UART_FLAG_IDLE) != RESET)
	{
		//一帧数据接收完成
		__HAL_UART_CLEAR_IDLEFLAG(&huart2_RS485);
	
	}
}
/*******************************************************************************
* 名    称：RS_Change_Baud
* 功    能：修改波特率
* 参数说明：无
* 返 回 值：Baud：波特率数值
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void RS_Change_Baud(int Baud)
{
	HAL_UART_MspDeInit(&huart2_RS485);
	LOS_TaskDelay(5);
	MX_USART2_UART_Init(Baud);
	HAL_UART_MspInit(&huart2_RS485);
	UART_SetConfig(&huart2_RS485);
}
/*******************************************************************************
* 名    称：RS485_Send_Cmd
* 功    能：发送问询帧
* 参数说明：buf为发送区首地址，len为发送字节长度
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void RS485_Send_Cmd(uint8_t *buf,uint8_t len)
{

	RS485_TX_EN			//发送模式
	HAL_UART_Transmit(&huart2_RS485,buf,len,1000);	
	LOS_TaskDelay(5);
	RS485_RX_CNT=0;	    	
	RS485_RX_EN				//接收模式	
}

/*******************************************************************************
* 名    称：RS485_Receive_Data
* 功    能：接收应答帧
* 参数说明：buf为接收区首地址，len为接收字节长度
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
void RS485_Receive_Data(uint8_t *buf,uint8_t *len)
{
	uint8_t rxlen=RS485_RX_CNT;
	uint8_t i=0;
	*len=0;
//	HAL_UART_Transmit(&huart1_test,RS485_RX_BUF,9,1000);
	if(rxlen==RS485_RX_CNT&&rxlen)
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_RX_BUF[i];	
		}			
		*len=RS485_RX_CNT;				
		RS485_RX_CNT=0;		
	}
}

/*******************************************************************************
* 名    称：Process_data
* 功    能：处理应答帧
* 参数说明：buf为发送区首地址，len为发送字节长度
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
void Process_data(uint8_t *buf,uint8_t len)
{	

		if(CRC16_MODBUS(buf,len))  		
		{
			if(buf[1] == 0x03)									//0x03 读命令
			{
				if(buf[2]==0x02) //读取两个字节
				{										
						RS485_Data[0] =(float)(buf[3] * 256 + buf[4])/10;
				}					
				else if(buf[2]==0x04){
					
						RS485_Data[0] =(float)(buf[3] * 256 + buf[4])/10;
					  RS485_Data[1] = (float)(buf[5] * 256 + buf[6])/10;
					}
				else{}
			}
		}
		else{
			printError("jiao yan cuo wu");//日志报错待开发			
		}	
}
/*******************************************************************************
* 名    称：CRC16_MODBUS
* 功    能：校验收到的数据是否出错
* 参数说明：buf为接收数据首地址，len为接收字节长度
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static int CRC16_MODBUS(uint8_t *buf,uint8_t len)            
{
    unsigned int crc = 0xffff;   
		unsigned char crcl,crch;
    unsigned char i, j;

    for (j = 0; j < len-2; j++)
    {
        crc = crc ^ buf[j];

        for (i = 0; i < len; i++)
        {
            if ((crc & 0x0001) == 1)
            {
                crc >>= 1;
                crc ^= 0xa001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
		crcl=crc;
		crch=crc >> 8;

    if ((buf[7] = crcl) && (buf[8] = crch))
    {
			return 1;
    }
    else {
      return 0;
    }    
}

/*******************************************************************************
* 名    称：RS485_Run
* 功    能：启动485通信
* 参数说明：无
* 返 回 值：无
* 说    明：
* 调用方法：外部调用
*******************************************************************************/
float *RS485_Run(int i)
{
	float *flag;
	flag[0]=0.00;
	flag[1]=0.00;
	RS485_Send_Cmd(READ_BUF[i],sizeof(READ_BUF1));
	LOS_TaskDelay(5);
	Process_data(RS485_RX_BUF,9);
	if(RS485_RX_BUF[0]==0)
	{	
		return flag;
	}
	else{
		memset(RS485_RX_BUF,0,sizeof(RS485_RX_BUF));
		return RS485_Data;
	}
}	
