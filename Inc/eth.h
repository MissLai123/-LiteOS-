/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eth.h
  * @brief   This file contains all the function prototypes for
  *          the eth.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETH_H__
#define __ETH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "stm32h7xx_hal_conf.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ETH_HandleTypeDef    g_eth_handler;                                      /* ��̫����� */
extern __attribute__((at(0x30040000))) ETH_DMADescTypeDef  g_eth_dma_rx_dscr_tab[ETH_RX_DESC_CNT];        /* Ethernet Rx DMA Descriptors */
extern __attribute__((at(0x30040060))) ETH_DMADescTypeDef  g_eth_dma_tx_dscr_tab[ETH_TX_DESC_CNT];

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */


/* USER CODE BEGIN Prototypes */
uint8_t     ethernet_init(void);                                                /* ��̫��оƬ��ʼ�� */
uint32_t    ethernet_read_phy(uint16_t reg);                                    /* ��ȡ��̫��оƬ�Ĵ���ֵ */
void        ethernet_write_phy(uint16_t reg, uint16_t value);                   /* ����̫��оƬָ����ַд��Ĵ���ֵ */
uint8_t     ethernet_chip_get_speed(void);                                      /* �����̫��оƬ���ٶ�ģʽ */
void        NETMPU_Config(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ETH_H__ */

