#include "stm32h7xx.h"                  // Device header
#include "usart.h"
#include "stdio.h"
#include "sdmmc.h"
#include <string.h>
#include <inttypes.h>
#include "SD.h"

#define BLOCK_SIZE            512            // SD卡块大小     
#define NUMBER_OF_BLOCKS      1              // 测试块数量(小于15)
#define WRITE_READ_ADDRESS    0x00000000     // 测试读写地址
#define SD_CARD_MAX_SIZE (8ULL * 1024ULL * 1024ULL * 1024ULL)	//SD卡最大容量

__align(4) uint8_t Buffer_Block_Tx_Buffer[BLOCK_SIZE*NUMBER_OF_BLOCKS]; // 创建一个连续的缓冲区用于存储字符串
__align(4) uint8_t Buffer_Block_Rx_Buffer[BLOCK_SIZE*NUMBER_OF_BLOCKS]; // 读取
static uint8_t *str_ptr = Buffer_Block_Tx_Buffer; // 跟踪写入位置

void SD_Write(const char *data)
{
    size_t data_len = strlen(data); // 获取新数据的长度

    // 检查剩余空间是否足够写入新数据
    size_t remaining_space = sizeof(Buffer_Block_Tx_Buffer) - (str_ptr - Buffer_Block_Tx_Buffer);
    if (remaining_space >= data_len + 1) { // 需要+1的空间存放终止符'\0'
        // 复制数据到缓冲区的当前写入位置
        memcpy(str_ptr, data, data_len);
        str_ptr[data_len] = '\0'; // 添加终止符
        str_ptr += data_len + 1; // 更新写入位置

        // 执行SD卡写入操作
        if(HAL_SD_WriteBlocks(&hsd1, Buffer_Block_Tx_Buffer, WRITE_READ_ADDRESS, NUMBER_OF_BLOCKS, 0xFFFF) == HAL_OK)
        {
            const char writeSuccess[] = "Write Successful\r\n";
            HAL_UART_Transmit(&huart1_test, (uint8_t*)writeSuccess, sizeof(writeSuccess) - 1, 1000);
            HAL_Delay(20);        
        } else {
            const char writeError[] = "Write Error\r\n";
            HAL_UART_Transmit(&huart1_test, (uint8_t*)writeError, sizeof(writeError) - 1, 1000);
        }
    } else {
        // 缓冲区空间不足，处理错误
        const char bufferFull[] = "Buffer Full\r\n";
        HAL_UART_Transmit(&huart1_test, (uint8_t*)bufferFull, sizeof(bufferFull) - 1, 1000);
    }
}

uint32_t SD_Read(void)//读取SD卡
{
	uint32_t len=0;	
	if(HAL_SD_ReadBlocks(&hsd1, Buffer_Block_Tx_Buffer, WRITE_READ_ADDRESS, NUMBER_OF_BLOCKS, 0xFFFF) == HAL_OK)
	{
		const char readSuccess[] = "Read Successful\r\n";
		HAL_UART_Transmit(&huart1_test, (uint8_t*)readSuccess, sizeof(readSuccess) - 1, 1000);
		// 重新解析字符串
		str_ptr = Buffer_Block_Tx_Buffer;
		while (*str_ptr)
		{
			while (*str_ptr != '\0')
			{
				// 发送字符直到遇到空字符
				if (HAL_UART_Transmit(&huart1_test, (uint8_t*)str_ptr, 1, 1000) != HAL_OK)
				{						
					Error_Handler();// 发生错误时的处理
				}
				str_ptr++;
				len++;
			}				
			// 每个字符串后添加换行符
			const char newline[] = "\n";
			if (HAL_UART_Transmit(&huart1_test, (uint8_t*)newline, sizeof(newline) - 1, 1000) != HAL_OK)
			{		
				Error_Handler();// 发生错误时的处理
			}			
			str_ptr++;			// 跳过空字符
			len++;
		}
		
		return len;
	}
	
}



void SD_Clear(uint32_t len)//擦除函数
{
	if(HAL_SD_Erase(&hsd1, WRITE_READ_ADDRESS, len) == HAL_OK)
	{
		const char eraseSuccess[] = "Erase Successful\r\n";	//擦除函数
		HAL_UART_Transmit(&huart1_test, (uint8_t*)eraseSuccess, sizeof(eraseSuccess) - 1, 1000);
		HAL_Delay(20);    //如果不延时可能到值写操作失败
		// ...后续的写入和读取操作...
	}	
}
