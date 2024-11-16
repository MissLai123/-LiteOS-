/**
 ****************************************************************************************************
 * @file        screen.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       Screen模块GUI驱动文件
 ****************************************************************************************************
 */
#include "LoRa.h"
#include "RS485.h"
#include "screen.h"
#include "cmd_process.h"
#include "hmi_driver.h"
#include "gpio.h"
#include "cmd_queue.h"
#include "SD.h"
#include "4G.h"

/*! 
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/

EVENT_CB_S Screen_EventGroup_CB;

uint8  cmd_buffer[CMD_MAX_SIZE];
uint8 size = 0;

TimeInfo Delivertime;
TimeInfo *pMytime = &Delivertime;		//读取串口屏RTC时间

/*******************************************************************************
* 名    称：HAL_UART_RxCpltCallback
* 功    能：串口屏回调函数
* 参数说明：串口句柄
* 返 回 值：无
* 说    明：接收串口屏指令
* 调用方法：内部调用
*******************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART6)																							//将指令放入指令队列中，并且开启下一次串口6的中断
	{ 
		queue_push(Rx6Buffer);
		HAL_UART_Receive_IT(&huart6_Screen,&Rx6Buffer,1);
	}	
	
}

/*******************************************************************************
* 名    称：ProcessMessage
* 功    能：串口屏消息处理函数
* 参数说明：指令结构体；大小；
* 返 回 值：无
* 说    明：处理串口屏指令
* 调用方法：外部调用
*******************************************************************************/
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
    uint8 cmd_type = msg->cmd_type;                                                  //指令类型
    uint8 ctrl_msg = msg->ctrl_msg;                                                  //消息的类型
    uint8 control_type = msg->control_type;                                          //控件类型
    uint16 screen_id = PTR2U16(&msg->screen_id);                                     //画面ID
    uint16 control_id = PTR2U16(&msg->control_id);                                   //控件ID
    uint32 value = PTR2U32(msg->param);                                              //数值

    switch(cmd_type)
    {  
    case NOTIFY_TOUCH_PRESS:                                                        //触摸屏按下
    case NOTIFY_TOUCH_RELEASE:                                                      //触摸屏松开
        NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4)); 
        break;                                                                    
    case NOTIFY_WRITE_FLASH_OK:                                                     //写FLASH成功
        NotifyWriteFlash(1);                                                      
        break;                                                                    
    case NOTIFY_WRITE_FLASH_FAILD:                                                  //写FLASH失败
        NotifyWriteFlash(0);                                                      
        break;                                                                    
    case NOTIFY_READ_FLASH_OK:                                                      //读取FLASH成功
        NotifyReadFlash(1,cmd_buffer+2,size-6);                                     //去除帧头帧尾
        break;                                                                    
    case NOTIFY_READ_FLASH_FAILD:                                                   //读取FLASH失败
        NotifyReadFlash(0,0,0);                                                   
        break;                                                                    
    case NOTIFY_READ_RTC: 	
				Delivertime = NotifyReadRTC(cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7],cmd_buffer[8]);
        break;
    case NOTIFY_CONTROL:
        {
            if(ctrl_msg==MSG_GET_CURRENT_SCREEN)                                    //画面ID变化通知
            {
                NotifyScreen(screen_id);                                            //画面切换调动的函数
            }
            else
            {
                switch(control_type)
                {
                case kCtrlButton:                                                   //按钮控件
										//printf("BuTTon");
                    NotifyButton(screen_id,control_id,msg->param[1]);                  
                    break;                                                             
                case kCtrlText:                                                     //文本控件
                    NotifyText(screen_id,control_id,msg->param);                       
                    break;                                                             
                case kCtrlProgress:                                                 //进度条控件
                    NotifyProgress(screen_id,control_id,value);                        
                    break;                                                             
                case kCtrlSlider:                                                   //滑动条控件
                    NotifySlider(screen_id,control_id,value);                          
                    break;                                                             
                case kCtrlMeter:                                                    //仪表控件
                    NotifyMeter(screen_id,control_id,value);                           
                    break;                                                             
                case kCtrlMenu:                                                     //菜单控件
                    NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);      
                    break;                                                              
                case kCtrlSelector:                                                 //选择控件
                    NotifySelector(screen_id,control_id,msg->param[0]);                
                    break;                                                              
                case kCtrlRTC:                                                      //倒计时控件
                    NotifyTimer(screen_id,control_id);
                    break;
                default:
                    break;
                }
            } 
            break;  
        } 
    case NOTIFY_HandShake:                                                          //握手通知                                                     
        NOTIFYHandShake();
        break;
    default:
        break;
    }
}

/*******************************************************************************
* 名    称：UpdateUI
* 功    能：更新串口屏
* 参数说明：无
* 返 回 值：无
* 说    明：更新串口屏控件
* 调用方法：外部调用
*******************************************************************************/
void UpdateUI()
{
	ReadRTC(); 
	if(current_screen_id == 4)
	 {
			 SetTextValue(4,3,g_lwip_demo_recvbuf);			//Ethernet_Text为以太网上传的数据，类型为字符串，可以使用数组、指针上传			 
	 }	 
}

/*! 
*  \brief  握手通知
*/
void NOTIFYHandShake()
{
   SetButtonValue(3,2,1);
}

/*! 
*  \brief  画面切换通知
*  \details  当前画面改变时(或调用GetScreen)，执行此函数
*  \param screen_id 当前画面ID
*/
void NotifyScreen(uint16 screen_id)
{

}

/*! 
*  \brief  触摸坐标事件响应
*  \param press 1按下触摸屏，3松开触摸屏
*  \param x x坐标
*  \param y y坐标
*/
void NotifyTouchXY(uint8 press,uint16 x,uint16 y)
{ 
	
}

/*! 
*  \brief  按钮控件通知
*  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param state 按钮状态：0弹起，1按下
*/
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{ 
	//printf("button");
	static uint8_t sign = 0;
	if(state == 1)
	{
		if(screen_id == 3)		//画面ID为3，Lora命令对应返回值为1,2,3,4，以太网命令对应返回值为5,6,7,8
		{
			switch(control_id)
			{
				case 1:				 
					LOS_EventWrite(&Screen_EventGroup_CB,LoRA_Command1_EVENT);
					statuss = 200;
					break;
				case 2: 
					LOS_EventWrite(&Screen_EventGroup_CB,LoRA_Command2_EVENT);
					statuss = 1000;
					break;
				case 3:
					LOS_EventWrite(&Screen_EventGroup_CB,LoRA_Command3_EVENT);
					if(sign == 0){status = 0;sign++;}
					else{status = 1000;sign=0;}
					break;
				case 4:	 
					LOS_EventWrite(&Screen_EventGroup_CB,LoRA_Command4_EVENT);
					statuss = 0;
					break;
				case 5:
					LOS_EventWrite(&Screen_EventGroup_CB,ETH_Command1_EVENT);
					break;
				case 6:
					LOS_EventWrite(&Screen_EventGroup_CB,ETH_Command2_EVENT);
					break;
				case 7:
					LOS_EventWrite(&Screen_EventGroup_CB,ETH_Command3_EVENT);
					break;
				case 8:
					LOS_EventWrite(&Screen_EventGroup_CB,ETH_Command4_EVENT);
					break;
				default:
					break;
			}	
		}
		if(screen_id == 6)
		{
			if(control_id == 6)
			{
				printf("123");
				uint32_t len = SD_Read();	//点击按钮后导出日志并清空数据
				SD_Clear(len);
			}
		}
	}
}

/*! 
*  \brief  文本控件通知
*  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
*  \details  文本控件的内容以字符串形式下发到MCU，如果文本控件内容是浮点值，
*  \details  则需要在此函数中将下发字符串重新转回浮点值。
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param str 文本控件内容
*/
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
	if(screen_id == 4)			//当用户在串口屏设置波特率时，将通过传送str给mcu，获取该值进行使用，注释变量尚未定义
	{												//当传入是整形或浮点型，可以使用sscanf函数将字符串格式化转化，如sscanf(str,"%ld",&value); 
		if(control_id == 1)
		{
			//LoRa_Baud = *str;
		}
		if(control_id == 2)
		{
			//RS485_Baud = *str;
		}
	}
}                                                                                

/*!                                                                              
*  \brief  进度条控件通知                                                       
*  \details  调用GetControlValue时，执行此函数                                  
*  \param screen_id 画面ID                                                      
*  \param control_id 控件ID                                                     
*  \param value 值                                                              
*/                                                                              
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)           
{  
    if(screen_id == 5)
    {
        Progress_Value = value;                                  
        SetTextInt32(5,2,Progress_Value,0,1);                                        //设置文本框的值     
    }    
}                                                                                

/*!                                                                              
*  \brief  滑动条控件通知                                                       
*  \details  当滑动条改变(或调用GetControlValue)时，执行此函数                  
*  \param screen_id 画面ID                                                      
*  \param control_id 控件ID                                                     
*  \param value 值                                                              
*/                                                                              
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)             
{                                                             
    uchar back[1] = {0};
    if(screen_id==7&&control_id==2)                                                  //滑块控制
    {            
        if(value<100||value>0)                                                       
        {                                                                            
            SetProgressValue(7,1,value);                                             //更新进度条数值
            SetTextInt32(7,3,value,0,1); 
            sprintf(back,"%c",(255-value*2));                                        //设置背光亮度 背光值范围 0~255，0最亮，255最暗
            SetBackLight(back[0]);
        }
    }
    if(screen_id==7&&control_id==5)                                                  //滑块控制
    {                                                                              
        if(value<100||value>0)                                                       
        {                                                                            
            SetProgressValue(7,4,value);                                             //更新进度条数值
            SetTextInt32(7,6,value,0,1);                                             
        }
    }
}

/*! 
*  \brief  仪表控件通知
*  \details  调用GetControlValue时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param value 值
*/
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
    //TODO: 添加用户代码
}

/*! 
*  \brief  菜单控件通知
*  \details  当菜单项按下或松开时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param item 菜单项索引
*  \param state 按钮状态：0松开，1按下
*/
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8 item, uint8 state)
{
    //TODO: 添加用户代码
}

/*! 
*  \brief  选择控件通知
*  \details  当选择控件变化时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param item 当前选项
*/
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item)
{
    if(screen_id == 4&&control_id == 1)                                //获取LoRa波特率的值,0-1200、1-2400、2-4800、3-9600、4-19200、5-38400、6-57600、7-115200
    {
		switch(item)
		{
			case 0:LoRa_ChangeBaud(1200); break;
			case 1:LoRa_ChangeBaud(2400); break;
			case 2:LoRa_ChangeBaud(4800); break;
			case 3:LoRa_ChangeBaud(9600); break;
			case 4:LoRa_ChangeBaud(19200); break;
			case 5:LoRa_ChangeBaud(38400); break;
			case 6:LoRa_ChangeBaud(57600); break;
			case 7:LoRa_ChangeBaud(115200); break;
		}
    } 
    if(screen_id == 4&&control_id == 13)                                //获取RS485波特率的值,0-2400、1-4800、2-9600、3-19200
    {
       	switch(item)
		{
			case 0:RS_Change_Baud(2400);break;
			case 1:RS_Change_Baud(4800);break;
			case 2:RS_Change_Baud(9600);break;
			case 3:RS_Change_Baud(19200);break;
		}
    } 

}

/*! 
*  \brief  定时器超时通知处理
*  \param screen_id 画面ID
*  \param control_id 控件ID
*/
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
    if(screen_id==8&&control_id == 7)
    {
        SetBuzzer(100);
    } 
}

/*! 
*  \brief  读取用户FLASH状态返回
*  \param status 0失败，1成功
*  \param _data 返回数据
*  \param length 数据长度
*/
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length)
{
    //TODO: 添加用户代码
}

/*! 
*  \brief  写用户FLASH状态返回
*  \param status 0失败，1成功
*/
void NotifyWriteFlash(uint8 status)
{
    //TODO: 添加用户代码
}

/*! 
*  \brief  读取RTC时间，注意返回的是BCD码
*  \param year 年（BCD）
*  \param month 月（BCD）
*  \param week 星期（BCD）
*  \param day 日（BCD）
*  \param hour 时（BCD）
*  \param minute 分（BCD）
*  \param second 秒（BCD）
*/
TimeInfo NotifyReadRTC(uint8_t year, uint8_t month, uint8_t week, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    TimeInfo timeInfo; // 创建一个新的TimeInfo实例
    timeInfo.seconds = (0xff & (second >> 4)) * 10 + (0xf & second);
    timeInfo.years = (0xff & (year >> 4)) * 10 + (0xf & year);
    timeInfo.months = (0xff & (month >> 4)) * 10 + (0xf & month);
    timeInfo.weeks = (0xff & (week >> 4)) * 10 + (0xf & week);
    timeInfo.days = (0xff & (day >> 4)) * 10 + (0xf & day);
    timeInfo.hours = (0xff & (hour >> 4)) * 10 + (0xf & hour);
    timeInfo.minutes = (0xff & (minute >> 4)) * 10 + (0xf & minute);
    
    return timeInfo; // 返回timeInfo结构体
}
