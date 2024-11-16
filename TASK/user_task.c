/**
 ****************************************************************************************************
 * @file        user_task.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       用户任务函数入口
 ****************************************************************************************************
 */

#include <string.h>
#include "stdio.h"
#include <stdlib.h>
#include "user_task.h"
#include "los_queue.h"
#include "NB_IoT.h"
#include "tim.h"

/*定义任务句柄*/
static UINT32 Receive_Task_Handle=0;
static UINT32 User_Task_Handle = 0;
static UINT32 ETH_Task_Handle = 0;
static UINT32 LoRA_Task_Handle = 0;
static UINT32 LoRA_command_Task_Handle = 0;
static UINT32 Screen_Task_Handle = 0;
static UINT32 G_Task_Handle = 0;
static uint8_t len;
int number = 2;

/*定义消息队列句柄*/
UINT32 RS485_Queue_Handle;
UINT32 LoRA_Queue_Handle;

/*定义事件标志组*/
EVENT_CB_S EventGroup_CB;

#define RS485_EVENT (0x01 << 0)

/*定义消息队列长度*/
#define  RS485_QUEUE_LEN  	  10
#define  RS485_QUEUE_SIZE     5

#define  LoRA_QUEUE_LEN  	  5
#define  LoRA_QUEUE_SIZE      10

int Level=0;		//定义报警等级
unsigned char Co_flag=0;

/*函数声明*/
static UINT32 Creat_RS485_Task(void);
static UINT32 Creat_ETH_TCP_Task(void);
static void RS485_Task(void);
static void ETH_TCP_Task(void);
static void LoRA_Task(void);
static UINT32 Creat_LoRA_Task(void);
static void LoRA_command_Task(void);
static UINT32 Creat_LoRA_Command(void);
static void Screen_Task(void);
static UINT32 Creat_Screen_Task(void);
static void G_Task(void);
static UINT32 Creat_G_Task(void);
static UINT32 Creat_NB_Task(void);
static void NB_Task(void);
static void SD_Task(void);
static UINT32 Creat_SD_Task(void);

/*******************************************************************************
* 名    称：User_Task
* 功    能：初始化用户任务，创建其他任务，创建全局队列变量
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void User_Task(void)
{
	UINT32 uwRet = LOS_OK;
	
	uwRet = Creat_SD_Task();																		//SD任务
	if (uwRet != LOS_OK) {
		printError("Creat_SD_Task creat failed 0x%X\n",uwRet);
	}
	printf("Creat_SD_Task create success. \r\n");								
	
	uwRet = Creat_ETH_TCP_Task();																//以太网任务
	if (uwRet != LOS_OK) {
		printError("Creat_ETH_TCP_Task creat failed 0x%X\n",uwRet);
		printf("Creat_ETH_TCP_Task creat failed 0x%X\n",uwRet);		

	}
	printf("Creat_ETH_TCP_Task create success. \r\n");
	
		uwRet = Creat_RS485_Task();																//RS485任务
	if (uwRet != LOS_OK) {
		printError("RS485_Task creat failed 0x%X\n",uwRet);				

	}
	printf("RS485_Task create success \r\n");
	
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==GPIO_PIN_SET)
	{
		uwRet = Creat_G_Task();																		//4G任务
		if (uwRet != LOS_OK) {
			printError("Creat_G_Task creat failed 0x%X\n",uwRet);
			
		}
		printf("Creat_G_Task create success. \r\n");
	}
	else{
		uwRet = Creat_NB_Task();																	//NB-IoT任务
		if (uwRet != LOS_OK) {
			printError("Creat_NB_Task creat failed 0x%X\n",uwRet);
		}
		printf("Creat_NB_Task create success. \r\n");
	}
	
	uwRet = Creat_LoRA_Task();																	//LoRa接受任务
	if (uwRet != LOS_OK) {
		printError("Creat_LoRA_Task creat failed 0x%X\n",uwRet);
	}
	printf("Creat_LoRA_Task create success. \r\n");
	
	uwRet = Creat_LoRA_Command();																//LoRa命令下发任务
	if (uwRet != LOS_OK) {
		printError("Creat_LoRA_Command creat failed 0x%X\n",uwRet);
	}
	printf("Creat_LoRA_Command create success. \r\n");
	
	uwRet = Creat_Screen_Task();																//串口屏任务
	if (uwRet != LOS_OK) {
		printError("Creat_Screen_Task creat failed 0x%X\n",uwRet);
	}
	printf("Creat_Screen_Task create success. \r\n");

	while(1)
	{		
		uwRet = LOS_TaskDelay(10);
		if(uwRet != LOS_OK)
		{
				return;
		}
	}
	
}

/*******************************************************************************
* 名    称：Creat_Receive_Task
* 功    能：创建485通信任务
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static UINT32 Creat_RS485_Task(void)
{
	
	UINT32 uwRet = LOS_OK;
	uwRet = LOS_QueueCreate("RS485_Queue",				
							RS485_QUEUE_LEN,		
							&RS485_Queue_Handle,	
							0,						
							RS485_QUEUE_SIZE);
	if (uwRet != LOS_OK)
    {
		printError("RS485_Queue create defeat 0x%X\n",uwRet);
		return uwRet;
    }
	printf("RS485_Queue create success.\r\n");
	TSK_INIT_PARAM_S task_init_param;

	task_init_param.usTaskPrio = 1; 
	task_init_param.pcName = "RS485_Task";
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)RS485_Task;
	task_init_param.uwStackSize = 0x800; 

	uwRet = LOS_TaskCreate(&Receive_Task_Handle, &task_init_param);
	return uwRet;
}

/*******************************************************************************
* 名    称：RS485_Task
* 功    能：485通信线程
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void RS485_Task(void)
{
	UINT32 uwRet = LOS_OK;
	float pVal[4];
	float *flag;
	g_usLosTaskLock=0;
	char Temperature1[8];
	char Humidity1[8];
	char Temperature2[8];
	char Humidity2[8];
	uint8_t T_Flag=0;
	while(1)                                
	{
			flag = RS485_Run(0);
			
			pVal[0]=flag[0];
			pVal[1]=flag[1];
			LOS_TaskDelay(500);
			flag = RS485_Run(1);
			if(pVal[3]>40.0 && T_Flag==0)
			{
				HAL_UART_Transmit(&huart8_LoRA, (uint8_t *)"Command4#", 9, 1000);
				LOS_EventWrite(&EventGroup_CB,Temp1_EVENT);
				T_Flag=1;
			}
			else if(pVal[3]<35.0 && T_Flag==1){
				HAL_UART_Transmit(&huart8_LoRA, (uint8_t *)"Command4#", 9, 1000);
				T_Flag=0;
			}
			if(pVal[1]>40.0)
			{
				LOS_EventWrite(&EventGroup_CB,Temp2_EVENT);
			}
			pVal[2]=flag[0];
			pVal[3]=flag[1];
		
		snprintf(Humidity1,sizeof(Humidity1), "%.2f", pVal[0]);
		snprintf(Temperature1,sizeof(Temperature1), "%.2f",pVal[1]);
		snprintf(Humidity2,sizeof(Humidity2), "%.2f", pVal[2]);
		snprintf(Temperature2,sizeof(Temperature2), "%.2f",pVal[3]);

		SetTextFloat(11,3, pVal[1],2,1);		//温度1
		SetTextFloat(11,4,pVal[3],2,1);			//温度2
		SetTextFloat(11,13, pVal[0],2,1);		//湿度1
		SetTextFloat(11,16,pVal[2],2,1);		//湿度2
		HistoryGraph_SetValueFloat(11,11,pVal,4);		//历史曲线控件4个通道使能
	    /*485传感器上传消息队列*/
		LOS_QueueWrite(RS485_Queue_Handle,Humidity1,sizeof(Humidity1),LOS_WAIT_FOREVER);
		LOS_QueueWrite(RS485_Queue_Handle,Temperature1,sizeof(Temperature1),LOS_WAIT_FOREVER);
		LOS_QueueWrite(RS485_Queue_Handle,Humidity2,sizeof(Humidity2),LOS_WAIT_FOREVER);
		LOS_QueueWrite(RS485_Queue_Handle,Temperature2,sizeof(Temperature2),LOS_WAIT_FOREVER);
		if(uwRet != LOS_OK)
		{
				return;
		}
		
	}	
}

/*******************************************************************************
* 名    称：Creat_ETH_TCP_Task
* 功    能：创建ETH_tcp任务
* 参数说明：无
* 返 回 值：无
* 说    明：LOS_OK:创建成功
* 调用方法：内部调用
*******************************************************************************/
static UINT32 Creat_ETH_TCP_Task(void)
{
	
	UINT32 uwRet = LOS_OK;
	printf("LwIP Init wait.\r\n");
	while (lwip_comm_init() != 0)
	{
		printError("PHY Init failed!!\r\n");
		return LOS_NOK;
	}
	TSK_INIT_PARAM_S task_init_param;

	task_init_param.usTaskPrio = 4; 
	task_init_param.pcName = "ETH_TCP_Task";
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)ETH_TCP_Task;
	task_init_param.uwStackSize = 0x400; 

	uwRet = LOS_TaskCreate(&ETH_Task_Handle, &task_init_param);
	return uwRet;
}

/*******************************************************************************
* 名    称：ETH_TCP_Task
* 功    能：以太网tcp服务器端线程
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void ETH_TCP_Task(void)
{
	UINT32 uwRet = LOS_OK;
#if LWIP_DHCP
    printf("DHCP IP configing... \r\n");       
    
    while ((g_lwipdev.dhcpstatus != 2) && (g_lwipdev.dhcpstatus != 0XFF))         
    {
        lwip_pkt_handle();
        lwip_periodic_handle();
    }
	lwip_run();
#endif	
	while (1)
   {
		 lwip_periodic_handle();
		 lwip_pkt_handle();
		 if (g_lwip_send_flag & 1 << 5)
		 {
				lwip_run();
		 }
		 uwRet = LOS_TaskDelay(20);
		 if(uwRet != LOS_OK)
		 {
			return;
		 }
	 }

}

/*******************************************************************************
* 名    称：Creat_LoRA_Task
* 功    能：创建LoRA通信任务
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static UINT32 Creat_LoRA_Task(void)
{
	UINT32 uwRet = LOS_OK;
	uwRet = LOS_QueueCreate("LoRA_Queue",				
							LoRA_QUEUE_LEN,		
							&LoRA_Queue_Handle,	
							0,						
							LoRA_QUEUE_SIZE);
	if (uwRet != LOS_OK)
    {
		printError("LoRA_Queue create defeat 0x%X\n",uwRet);
		return uwRet;
    }
	printf("LoRA_Queue create success \r\n");
	TSK_INIT_PARAM_S task_init_param;

	task_init_param.usTaskPrio = 4; 
	task_init_param.pcName = "LoRA_Task";
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)LoRA_Task;
	task_init_param.uwStackSize = 0x600; 

	uwRet = LOS_TaskCreate(&LoRA_Task_Handle, &task_init_param);
	return uwRet;
}

/*******************************************************************************
* 名    称：LoRA_Task
* 功    能：LoRA任务线程
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void LoRA_Task(void)
{
	uint8_t rxbuf[10]={0};
	
	while(1)
	{		
		LoRA_Receive_Data(rxbuf,&len);
		
		/*判断是否有数据*/
		if(len)
		{
					if(rxbuf[1]!= 0x2E)
		{
			if(rxbuf[0] >= 2)
			{
				LOS_EventWrite(&EventGroup_CB,LoRA_EVENT);
			}
		}
		else
		{
			Co_flag=0;
		}
			LOS_QueueWrite(LoRA_Queue_Handle,rxbuf,sizeof(rxbuf),LOS_WAIT_FOREVER);printf("%s\r\n",rxbuf);
		}		
	}	
}
/*******************************************************************************
* 名    称：Creat_LoRA_Command
* 功    能：创建用户交互任务
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static UINT32 Creat_LoRA_Command(void)
{
	
	UINT32 uwRet = LOS_OK;	
	TSK_INIT_PARAM_S task_init_param;
	task_init_param.usTaskPrio = 2; 
	task_init_param.pcName = "LoRA_command_Task";
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)LoRA_command_Task;
	task_init_param.uwStackSize = 0x300; 

	uwRet = LOS_TaskCreate(&LoRA_command_Task_Handle, &task_init_param);
	return uwRet;
}

/*******************************************************************************
* 名    称：LoRA_command_Task
* 功    能：LoRA命令下发
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void LoRA_command_Task(void)
{
	UINT32 uwRet;
	UINT32 uwEvent;
	while(1)
	{	
		/*监测整个系统是否需要LoRa下发命令*/
		uwEvent=LOS_EventRead(&Screen_EventGroup_CB, \
							LoRA_Command1_EVENT|LoRA_Command2_EVENT|LoRA_Command3_EVENT|LoRA_Command4_EVENT, \
							LOS_WAITMODE_OR,	\
							LOS_WAIT_FOREVER); 
		if (LoRA_Command1_EVENT == uwEvent)
		{	
			LoRa_Run(0);
		  LOS_EventClear(&Screen_EventGroup_CB, ~LoRA_Command1_EVENT);
		}
		else if(LoRA_Command2_EVENT == uwEvent)
		{
			LoRa_Run(1);
			LOS_EventClear(&Screen_EventGroup_CB, ~LoRA_Command2_EVENT);
		}
		else if(LoRA_Command3_EVENT == uwEvent)
		{
			LoRa_Run(2);
			LOS_EventClear(&Screen_EventGroup_CB, ~LoRA_Command3_EVENT);
		}
		else if(LoRA_Command4_EVENT == uwEvent)
		{
			LoRa_Run(3);
			LOS_EventClear(&Screen_EventGroup_CB, ~LoRA_Command4_EVENT);
		}
		uwRet = LOS_TaskDelay(10);
		if(uwRet != LOS_OK)
		{
			return;
		}
	}	
}
/*******************************************************************************
* 名    称：Creat_Screen_Task
* 功    能：创建screen用户交互任务
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static UINT32 Creat_Screen_Task(void)
{
	
	UINT32 uwRet = LOS_OK;
	uwRet = LOS_EventInit(&Screen_EventGroup_CB);
	if (uwRet != LOS_OK)
	{
	  printError("Screen_EventGroup_CB create defeat 0x%X\n",uwRet);

	}
	printf("Screen_EventGroup_CB create success.\r\n");	
	TSK_INIT_PARAM_S task_init_param;
	task_init_param.usTaskPrio = 1; 
	task_init_param.pcName = "Screen_Task";
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)Screen_Task;
	task_init_param.uwStackSize = 0x400; 

	uwRet = LOS_TaskCreate(&Screen_Task_Handle, &task_init_param);
	return uwRet;
}

/*******************************************************************************
* 名    称：Screen_Task
* 功    能：Screen交互任务线程
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void Screen_Task(void)
{
	UINT32 uwRet = LOS_OK;
	HAL_UART_Receive_IT(&huart6_Screen,&Rx6Buffer,1);			//开启接收中断
	HAL_TIM_Base_Start_IT(&htim6);						//启用定时器6
	SetTextValue(4,10,"Disconnected");				//以太网连接状态初始化
	SetButtonValue(4,11,1);
	while(1)
	{
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);  	 //指令接收    
		if(size>0&&cmd_buffer[1]!=0x07)                                             
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);  	 //指令处理                          
		}                                                                           
		else if(size>0&&cmd_buffer[1]==0x07)       		//接收到串口开机指令MCU进行软重启                                
		{    																																						
			printError("erorr and restart\r\n");
			__ASM volatile ("cpsid i");
			HAL_NVIC_SystemReset();
		} 								
		uwRet = LOS_TaskDelay(10);
		if(uwRet != LOS_OK)
		{
			return;
		}
		
	}	
}
/*******************************************************************************
* 名    称：Creat_G_Task
* 功    能：创建screen用户交互任务
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static UINT32 Creat_G_Task(void)
{
	
	UINT32 uwRet = LOS_OK;
	
	TSK_INIT_PARAM_S task_init_param;
	task_init_param.usTaskPrio = 2; 
	task_init_param.pcName = "G_Task";
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)G_Task;
	task_init_param.uwStackSize = 0x1500; 

	uwRet = LOS_TaskCreate(&G_Task_Handle, &task_init_param);
	return uwRet;
}

/*******************************************************************************
* 名    称：G_Task
* 功    能：4G上云任务线程
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void G_Task(void)
{
  uint8_t LoRa_State = 0;
	UINT32 uwRet;
	char *hVal;
	char *hVal2;
	char *gVal;
	char *TVal;
	char *TVal2;
	UINT32 l_size = 10;
	g_usLosTaskLock=0;
	while(1)
	{	
		uwRet=LOS_QueueRead(LoRA_Queue_Handle,&gVal,l_size,1200);
		if(LOS_ERRNO_QUEUE_TIMEOUT == uwRet)
		{
			gVal="0.00";
			if(!LoRa_State)
			{
				printError("LoRa has disconnected");
				LoRa_State = 1;
			}
		}
		else
		{
			LoRa_State = 0;
		}
		/*等待1200毫秒消息队列无消息则认为传感器1异常数据为0*/
		uwRet=LOS_QueueRead(RS485_Queue_Handle,&hVal,l_size,1200);
		uwRet=LOS_QueueRead(RS485_Queue_Handle,&TVal,l_size,1200);
		
		/*等待1200毫秒消息队列无消息则认为传感器2异常数据为0*/
		uwRet=LOS_QueueRead(RS485_Queue_Handle,&hVal2,l_size,1200);
		uwRet=LOS_QueueRead(RS485_Queue_Handle,&TVal2,l_size,1200);

		/*数据上云*/	
		Mqtt_UpLoading(TVal,hVal,TVal2,hVal2,gVal);
		SetTextValue(11,5,gVal);		//串口屏打印CO浓度
	}
	
}

/*******************************************************************************
* 名    称：Creat_G_Task
* 功    能：创建screen用户交互任务
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static UINT32 Creat_NB_Task(void)
{
	
	UINT32 uwRet = LOS_OK;
	
	TSK_INIT_PARAM_S task_init_param;
	task_init_param.usTaskPrio = 2; 
	task_init_param.pcName = "NB_Task";
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)NB_Task;
	task_init_param.uwStackSize = 0x600; 

	uwRet = LOS_TaskCreate(&G_Task_Handle, &task_init_param);
	return uwRet;
}
	
/*******************************************************************************
* 名    称：NB_Task（暂未使用）
* 功    能：NB-IoT上云任务线程
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：内部调用
*******************************************************************************/
static void NB_Task(void)
{
	UINT32 uwRet;
	char *pVal;
	char *lVal;
	char *TVal;
	UINT32 l_size = 10;
	g_usLosTaskLock=0;
	
	__HAL_UART_ENABLE_IT(&huart8_LoRA,UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart8_LoRA,UART_IT_IDLE);
	while(1)
	{
			uwRet=LOS_QueueRead(RS485_Queue_Handle,&pVal,l_size,1200);
			uwRet=LOS_QueueRead(RS485_Queue_Handle,&TVal,l_size,1200);
			uwRet=LOS_QueueRead(LoRA_Queue_Handle,&lVal,l_size,1200);
			if(LOS_ERRNO_QUEUE_TIMEOUT == uwRet)
			{
				lVal="0.00";
			}
			snprintf(NB_Data,sizeof(NB_Data),"00%d%s%s%s",Level,TVal,pVal,lVal);
			NB_IoT_SendNMGS(NB_Data);
			printf("%s\r\n",pVal);
			printf("%s\r\n",TVal);
			printf("%s\r\n",lVal);					
	}
	
}

static UINT32 Creat_SD_Task(void)
{
	
	UINT32 uwRet = LOS_OK;
	
	TSK_INIT_PARAM_S task_init_param;
	task_init_param.usTaskPrio = 4; 
	task_init_param.pcName = "SD_Task";
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)SD_Task;
	task_init_param.uwStackSize = 0x200; 

	uwRet = LOS_TaskCreate(&G_Task_Handle, &task_init_param);
	return uwRet;
}

static void SD_Task(void)
{	
	UINT32 uwRet = LOS_OK;
	g_usLosTaskLock=0;
	while(1)
	{
		if(error_len>0)
		{
			
			SD_Write(errorMessage);
			error_len=0;
		}
		uwRet = LOS_TaskDelay(1000);
		if(uwRet != LOS_OK)
		{
			return;
		}	
	}
		
}
/*******************************************************************************
* 名    称：Creat_User_Task1
* 功    能：创建用户总任务
* 参数说明：无
* 返 回 值：返回uwRet，LOS_OK正常，LOS_NOK异常
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
UINT32 Creat_User_Task1(VOID)
{
    UINT32 uwRet = LOS_OK;
    uwRet = LOS_EventInit(&EventGroup_CB);
	if (uwRet != LOS_OK)
	{
		printError("EventGroup_CB create defeat 0x%X\n",uwRet);	
	}
		printf("EventGroup_CB create success.\r\n");
	
		TSK_INIT_PARAM_S task_init_param;
	
    task_init_param.usTaskPrio = 0;      		
    task_init_param.pcName = "user_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)User_Task;
    task_init_param.uwStackSize = 0x500;
	
    uwRet = LOS_TaskCreate(&User_Task_Handle, &task_init_param);
	
    printf("*creat user task*\r\n");

    return uwRet;
} 
