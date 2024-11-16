#ifndef __SD_H__
#define __SD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "SD.h"


// ȫ�ֱ�������
	
#define BLOCK_SIZE            512            // SD�����С     
#define NUMBER_OF_BLOCKS      1              // ���Կ�����(С��15)
//#define WRITE_READ_ADDRESS    0x00000000     // ���Զ�д��ַ

extern __align(4) uint8_t Buffer_Block_Tx_Buffer[BLOCK_SIZE*NUMBER_OF_BLOCKS]; // ����һ�������Ļ��������ڴ洢�ַ���
extern __align(4) uint8_t Buffer_Block_Rx_Buffer[BLOCK_SIZE*NUMBER_OF_BLOCKS]; // ��ȡ
	
// ��������
void SD_Write(const char *data);
uint32_t SD_Read(void);
void SD_Clear(uint32_t EndADDRSSS);	

#ifdef __cplusplus
}
#endif
#endif /* __SD_H__ */

