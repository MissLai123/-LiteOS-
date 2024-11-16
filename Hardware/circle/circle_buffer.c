/**
 ****************************************************************************************************
 * @file        4G.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       环形buf功能模块
 ****************************************************************************************************
 */

#include <stdint.h>
#include "circle_buffer.h"
/*******************************************************************************
* 名    称：circle_buf_init
* 功    能：创建一个环形buf
* 参数说明：pCircleBuf：环形buf结构体；len：大小；buf：结构体buf首地址
* 返 回 值：无
* 说    明：本结构体只对buf地址进行操作
* 调用方法：外部调用
*******************************************************************************/
void circle_buf_init(p_circle_buf pCircleBuf, uint32_t len, uint8_t *buf)
{
	pCircleBuf->r = pCircleBuf->w = 0;
	pCircleBuf->len = len;
	pCircleBuf->buf = buf;
}

/*******************************************************************************
* 名    称：circle_buf_read
* 功    能：读取上一次写入buf的数据
* 参数说明：pCircleBuf：环形buf结构体；pVal：存储数据首地址
* 返 回 值：0 表示已经读取完毕
* 说    明：
* 调用方法：外部调用
*******************************************************************************/
int circle_buf_read(p_circle_buf pCircleBuf, uint8_t *pVal)
{
	while(pCircleBuf->r != pCircleBuf->w)
	{
		*pVal = pCircleBuf->buf[pCircleBuf->r];
		pVal++;
		pCircleBuf->r++;
		
		if (pCircleBuf->r == pCircleBuf->len)
			pCircleBuf->r = 0;
		
	}
  return 0;
}

/*******************************************************************************
* 名    称：circle_buf_write
* 功    能：写入一个数据val
* 参数说明：pCircleBuf：环形buf结构体；pVal：写入的数据
* 返 回 值：0 写入成功 -1环形buf已满写入失败
* 说    明：本结构体不存在越界，可根据返回值判断是否写满
* 调用方法：外部调用
*******************************************************************************/
int circle_buf_write(p_circle_buf pCircleBuf, uint8_t val)
{
	uint32_t next_w;
	
	next_w = pCircleBuf->w + 1;
	if (next_w == pCircleBuf->len)
		next_w = 0;
	
	if (next_w != pCircleBuf->r)
	{
		pCircleBuf->buf[pCircleBuf->w] = val;
		pCircleBuf->w = next_w;
		return 0;
	}
	else
	{
		return -1;
	}
}
