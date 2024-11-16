/**
 ****************************************************************************************************
 * @file        4G.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       4G模块功能驱动文件
 ****************************************************************************************************
 */

#include "4G.h"
#include "user_task.h"
uint8_t G_data[10];
uint8_t cnt = 0;
uint16_t status = 200;
uint16_t statuss = 1000;

/*******************************************************************************
* 名    称：UART8_IRQHandler
* 功    能：中断接收处理函数
* 参数说明：无
* 返 回 值：无
* 说    明：处理云端下发的命令
* 调用方法：内部调用
*******************************************************************************/
void UART7_IRQHandler(void)
{
  uint8_t res;
	static uint8_t sign = 0;		//舵机的状态标志位
	if((__HAL_UART_GET_FLAG(&huart7_4G,UART_FLAG_RXNE)!=RESET))  
	{	 	
		HAL_UART_Receive(&huart7_4G,&res,1,1000);
		G_data[cnt++]=res;
		if(G_data[cnt-1] == '#') 
		{			
			G_data[cnt-1] = '\0';			
			if(strcmp((char*)G_data, "ON") == 0)
			{
				LOS_EventWrite(&Screen_EventGroup_CB,LoRA_Command1_EVENT);
				statuss = 500;
				cnt = 0;
			} 
			else if(strcmp((char*)G_data, "Command3") == 0)
			{	
				LOS_EventWrite(&Screen_EventGroup_CB,LoRA_Command2_EVENT);
				statuss = 1000;
				cnt = 0;				
			}
			else if(strcmp((char*)G_data, "Command4") == 0)
			{	
				LOS_EventWrite(&Screen_EventGroup_CB,LoRA_Command3_EVENT);
				if(sign == 0){status = 0;sign++;}
				else{status = 1000;sign=0;}
				cnt = 0;			
			}
			else if(strcmp((char*)G_data, "OFF") == 0)
			{
				LOS_EventWrite(&Screen_EventGroup_CB,LoRA_Command4_EVENT);
				statuss = 0;
				cnt = 0;	
			}		
		}

	}
	if(__HAL_UART_GET_FLAG(&huart7_4G,UART_FLAG_IDLE) != RESET)
	{
		//一帧数据接收完成
		__HAL_UART_CLEAR_IDLEFLAG(&huart7_4G);
	
	}
}



/*******************************************************************************
* 名    称：Mqtt_UpLoading
* 功    能：4G传输数据上云
* 参数说明：RS485data：485传感器数据buf首地址；Smokedata：co传感器数据buf首地址；
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
void Mqtt_UpLoading(char* temperature1, char* humidity1,  char*  temperature2,  char*  humidity2, char* gas)
{
	UINT32 uwEvent;
	int alarmTimes = 0; // 初始化告警次数为0
	char jsonMessages[1024];
	char content[54];
	uint8_t year = pMytime->years;
	uint8_t month = pMytime->months;
	uint8_t day = pMytime->days;
	uint8_t hours = pMytime->hours;
	uint8_t minutes = pMytime->minutes;
	uint8_t seconds = pMytime->seconds;

	uwEvent=LOS_EventRead(&EventGroup_CB, 
							LoRA_EVENT|Temp1_EVENT|Temp2_EVENT, 
							LOS_WAITMODE_OR,	
							0);
	if (LoRA_EVENT == uwEvent)
	{
		snprintf(content,sizeof(content),"设备1浓度异常：超过阈值20");
		 LOS_EventClear(&EventGroup_CB, ~LoRA_EVENT);
		alarmTimes++;
	}
	else if(Temp1_EVENT == uwEvent)
	{
		snprintf(content,sizeof(content),"设备1温度异常：超过阈值40℃");
		 LOS_EventClear(&EventGroup_CB, ~Temp1_EVENT);
		alarmTimes++;
	}
	else if(Temp2_EVENT == uwEvent)
	{
		snprintf(content,sizeof(content),"设备2温度异常：超过阈值40℃");
		 LOS_EventClear(&EventGroup_CB, ~Temp2_EVENT);
		alarmTimes++;
	}
	else
	{
		snprintf(content,sizeof(content)," ");
	}
    // 构建 JSON message
	 
	sprintf(jsonMessages,
		"{\"services\":[{\"service_id\":\"Smoke\",\"properties\":{"
		"\"time\":\"%02d/%02d %02d:%02d:%02d\","
		"\"times\":\"%d年%02d月%02d日 %02d:%02d:%02d\","
		"\"content\":\"%s\","
//        "\"steering\":{\"total\":%d, \"status\":%d},"
//        "\"motor\":{\"totals\":%d, \"statuss\":%d},"
		"\"total\":%d,"
		"\"status\":%d," 
		"\"totals\":%d,"
		"\"statuss\":%d,"
		"\"s\":1," 
		"\"alarmtimes\":%d,"
		"\"temperature1\":%s,"
		"\"humidity1\":%s,"
		"\"temperature2\":%s,"
		"\"humidity2\":%s,"
		"\"gas\":%s"
		"}}]}",
		month, day, hours, minutes, seconds, // time
		year, month, day, hours, minutes, seconds, // times
		content,
		1000, status, // steering
		1000, statuss, // motor
		alarmTimes, // alarm times
		temperature1, humidity1, // temperature1 and humidity1
		temperature2, humidity2, // temperature2 and humidity2
		gas); // gas
			
	HAL_UART_Transmit(&huart7_4G, (uint8_t*)jsonMessages, strlen(jsonMessages), 1000);
}

