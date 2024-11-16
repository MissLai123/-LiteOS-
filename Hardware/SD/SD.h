#ifndef __SD_H__
#define __SD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "SD.h"


// 全局变量声明
	
#define BLOCK_SIZE            512            // SD卡块大小     
#define NUMBER_OF_BLOCKS      1              // 测试块数量(小于15)
//#define WRITE_READ_ADDRESS    0x00000000     // 测试读写地址

extern __align(4) uint8_t Buffer_Block_Tx_Buffer[BLOCK_SIZE*NUMBER_OF_BLOCKS]; // 创建一个连续的缓冲区用于存储字符串
extern __align(4) uint8_t Buffer_Block_Rx_Buffer[BLOCK_SIZE*NUMBER_OF_BLOCKS]; // 读取
	
// 函数声明
void SD_Write(const char *data);
uint32_t SD_Read(void);
void SD_Clear(uint32_t EndADDRSSS);	

#ifdef __cplusplus
}
#endif
#endif /* __SD_H__ */

