#ifndef ETHERENT_CHIP_H
#define ETHERENT_CHIP_H
#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif   

/* PHY芯片寄存器映射表*/ 
#define ETH_CHIP_BCR                            ((uint16_t)0x0000U)
#define ETH_CHIP_BSR                            ((uint16_t)0x0001U)
#define PHY_REGISTER2                           ((uint16_t)0x0002U)
#define PHY_REGISTER3                           ((uint16_t)0x0003U)

/* 操作BCR寄存器值 */
#define ETH_CHIP_BCR_SOFT_RESET                 ((uint16_t)0x8000U)
#define ETH_CHIP_BCR_LOOPBACK                   ((uint16_t)0x4000U)
#define ETH_CHIP_BCR_SPEED_SELECT               ((uint16_t)0x2000U)
#define ETH_CHIP_BCR_AUTONEGO_EN                ((uint16_t)0x1000U)
#define ETH_CHIP_BCR_POWER_DOWN                 ((uint16_t)0x0800U)
#define ETH_CHIP_BCR_ISOLATE                    ((uint16_t)0x0400U)
#define ETH_CHIP_BCR_RESTART_AUTONEGO           ((uint16_t)0x0200U)
#define ETH_CHIP_BCR_DUPLEX_MODE                ((uint16_t)0x0100U) 

/* 操作BSR寄存器值 */   
#define ETH_CHIP_BSR_100BASE_T4                 ((uint16_t)0x8000U)
#define ETH_CHIP_BSR_100BASE_TX_FD              ((uint16_t)0x4000U)
#define ETH_CHIP_BSR_100BASE_TX_HD              ((uint16_t)0x2000U)
#define ETH_CHIP_BSR_10BASE_T_FD                ((uint16_t)0x1000U)
#define ETH_CHIP_BSR_10BASE_T_HD                ((uint16_t)0x0800U)
#define ETH_CHIP_BSR_100BASE_T2_FD              ((uint16_t)0x0400U)
#define ETH_CHIP_BSR_100BASE_T2_HD              ((uint16_t)0x0200U)
#define ETH_CHIP_BSR_EXTENDED_STATUS            ((uint16_t)0x0100U)
#define ETH_CHIP_BSR_AUTONEGO_CPLT              ((uint16_t)0x0020U)
#define ETH_CHIP_BSR_REMOTE_FAULT               ((uint16_t)0x0010U)
#define ETH_CHIP_BSR_AUTONEGO_ABILITY           ((uint16_t)0x0008U)
#define ETH_CHIP_BSR_LINK_STATUS                ((uint16_t)0x0004U)
#define ETH_CHIP_BSR_JABBER_DETECT              ((uint16_t)0x0002U)
#define ETH_CHIP_BSR_EXTENDED_CAP               ((uint16_t)0x0001U)

/* 进程状态*/
#define  ETH_CHIP_STATUS_READ_ERROR             ((int32_t)-5)
#define  ETH_CHIP_STATUS_WRITE_ERROR            ((int32_t)-4)
#define  ETH_CHIP_STATUS_ADDRESS_ERROR          ((int32_t)-3)
#define  ETH_CHIP_STATUS_RESET_TIMEOUT          ((int32_t)-2)
#define  ETH_CHIP_STATUS_ERROR                  ((int32_t)-1)
#define  ETH_CHIP_STATUS_OK                     ((int32_t) 0)
#define  ETH_CHIP_STATUS_LINK_DOWN              ((int32_t) 1)
#define  ETH_CHIP_STATUS_100MBITS_FULLDUPLEX    ((int32_t) 2)
#define  ETH_CHIP_STATUS_100MBITS_HALFDUPLEX    ((int32_t) 3)
#define  ETH_CHIP_STATUS_10MBITS_FULLDUPLEX     ((int32_t) 4)
#define  ETH_CHIP_STATUS_10MBITS_HALFDUPLEX     ((int32_t) 5)
#define  ETH_CHIP_STATUS_AUTONEGO_NOTDONE       ((int32_t) 6)

/* lan8720地址 */
#define ETH_CHIP_ADDR                           ((uint16_t)0x0000U)
/* PHY寄存器数量*/
#define ETH_CHIP_PHY_COUNT                      ((uint16_t)0x001FU)



#define PHY_TYPE                                LAN8720
#define LAN8720                                 0
#define ETH_CHIP_PHYSCSR                        ((uint16_t)0x1F)                       /*!< tranceiver status register */
#define ETH_CHIP_SPEED_STATUS                   ((uint16_t)0x0004)                     /*!< configured information of speed: 100Mbit/s */
#define ETH_CHIP_DUPLEX_STATUS                  ((uint16_t)0x0010)                     /*!< configured information of duplex: full-duplex */



typedef int32_t  (*eth_chip_init_func)          (void); 
typedef int32_t  (*eth_chip_deinit_func)        (void);
typedef int32_t  (*eth_chip_readreg_func)       (uint32_t, uint32_t, uint32_t *);
typedef int32_t  (*eth_chip_writereg_func)      (uint32_t, uint32_t, uint32_t);
typedef int32_t  (*eth_chip_gettick_func)       (void);

/* PHY结构体 */ 
typedef struct 
{
    eth_chip_init_func          init;                   
    eth_chip_deinit_func        deinit;                 
    eth_chip_writereg_func      writereg;               
    eth_chip_readreg_func       readreg;                
    eth_chip_gettick_func       gettick;              
} eth_chip_ioc_tx_t;  

/* 注册到组件对象结构体 */
typedef struct 
{
    uint32_t            devaddr;                        /* PHY地址ַ */
    uint32_t            is_initialized;                 /* 设备初始化状态 */
    eth_chip_ioc_tx_t   io;                             /* API入口 */
    void                *pdata;                         /* 传入的形参 */
}eth_chip_object_t;


int32_t eth_chip_regster_bus_io(eth_chip_object_t *pobj, eth_chip_ioc_tx_t *ioctx);             /* 将IO函数注册到组件对象*/
int32_t eth_chip_init(eth_chip_object_t *pobj);                                                 /* 初始化ETH_CHIP并配置所需的硬件资源*/
int32_t eth_chip_deinit(eth_chip_object_t *pobj);                                               /* 反初始化ETH_CHIP及硬件资源 */
int32_t eth_chip_disable_power_down_mode(eth_chip_object_t *pobj);                              /* 关闭ETH_CHIP下电模式*/
int32_t eth_chip_enable_power_down_mode(eth_chip_object_t *pobj);                               /* 是能ETH_CHIP的下电模式 */
int32_t eth_chip_start_auto_nego(eth_chip_object_t *pobj);                                      /*启动自协商功能 */
int32_t eth_chip_get_link_state(eth_chip_object_t *pobj);                                       /* 获取ETH_CHIP链路状态 */
int32_t eth_chip_set_link_state(eth_chip_object_t *pobj, uint32_t linkstate);                   /* 设置ETH_CHIP链路状态 */
int32_t eth_chip_enable_loop_back_mode(eth_chip_object_t *pobj);                                /*启动回环模式 */
int32_t eth_chip_disable_loop_back_mode(eth_chip_object_t *pobj);                               /* 禁用回环模式 */

#ifdef __cplusplus
}
#endif

#endif
