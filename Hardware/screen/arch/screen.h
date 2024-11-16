#ifndef __SCREEN_H
#define __SCREEN_H
#include "los_event.h"
#include "hmi_driver.h"
#include "cmd_process.h"
#include "lwip_tcp_server.h"

#define uchar    unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long
#define int16    short int
#define int32    long
	
extern uint8 size;
extern uint8  cmd_buffer[CMD_MAX_SIZE];                                                     //指令缓存
static uint16 current_screen_id = 0;                                                 //当前画面ID
static int32 progress_value = 0;                                                     //进度条测试值
static int32 test_value = 0;                                                         //测试值
static uint8 update_en = 0;                                                          //更新标记
static int32 meter_flag = 0;                                                         //仪表指针往返标志位
static int32 num = 0;                                                                //曲线采样点计数
static int sec = 1;                                                                  //时间秒
static int32 curves_type = 0;                                                        //曲线标志位  0为正弦波，1为锯齿波                  
static int32 second_flag=0;                                                          //时间标志位
static int32 icon_flag = 0;                                                          //图标标志位
static uint8 Select_H ;                                                              //滑动选择小时
static uint8 Select_M ;                                                              //滑动选择分钟 
static uint8 Last_H ;                                                                //上一个选择小时
static uint8 Last_M;                                                                 //上一个选择分钟 
static int32 Progress_Value = 0;  
static uint8_t back_value;			//定义命令返回值
extern uint8_t *b_value;				//指向命令返回值的指针
static uint8_t Rx6Buffer;
static uint8_t Button_Flag = 0;
static uint32_t str_p = 0;

extern EVENT_CB_S Screen_EventGroup_CB;
#define LoRA_Command1_EVENT (0x01 << 0)
#define LoRA_Command2_EVENT (0x01 << 1)
#define LoRA_Command3_EVENT (0x01 << 2)
#define LoRA_Command4_EVENT (0x01 << 3)
#define ETH_Command1_EVENT (0x01 << 4)
#define ETH_Command2_EVENT (0x01 << 5)
#define ETH_Command3_EVENT (0x01 << 6)
#define ETH_Command4_EVENT (0x01 << 7)
#define ETH_Data_EVENT (0x01 << 8)

typedef struct {
    uint8_t seconds;
    uint8_t years;
    uint8_t months;
    uint8_t weeks;
    uint8_t days;
    uint8_t hours;
    uint8_t minutes;
} TimeInfo;	//串口屏RTC变量结构体

extern TimeInfo Delivertime;		//定义的结构体名
extern TimeInfo *pMytime;			//结构体指针

void ProcessMessage( PCTRL_MSG msg, uint16 size );
void NOTIFYHandShake(void);
void NotifyScreen(uint16 screen_id);
void NotifyTouchXY(uint8 press,uint16 x,uint16 y);
void UpdateUI(void);
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state);
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str);
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value);
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value);
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value);
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8 item, uint8 state);
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item);
void NotifyTimer(uint16 screen_id, uint16 control_id);
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length);
void NotifyWriteFlash(uint8 status);
//void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second);
TimeInfo NotifyReadRTC(uint8_t year, uint8_t month, uint8_t week, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

void LoRa_RS485_BaudInit(void);
#endif

