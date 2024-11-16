/**
 ****************************************************************************************************
 * @file        ethernet_chip.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       lan8720配置驱动
 ****************************************************************************************************
 */
#include "ethernet_chip.h"


#define ETH_CHIP_SW_RESET_TO    ((uint32_t)500U) /*软件复位等待时间*/   
#define ETH_CHIP_INIT_TO        ((uint32_t)2000U)   /*初始化等待时间*/
#define ETH_CHIP_MAX_DEV_ADDR   ((uint32_t)31U)    /*PHY地址最大值*/ 


#define LAN8720A_PHYREGISTER2                   0x0007

/*******************************************************************************
* 名    称：eth_chip_regster_bus_io
* 功    能：将IO函数注册到组件对象
* 参数说明： pobj：设备对象；ioctx：设备IO功能
* 返 回 值：TH_CHIP_STATUS_OK：OK；ETH_CHIP_STATUS_ERROR：缺少功能
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t  eth_chip_regster_bus_io(eth_chip_object_t *pobj, eth_chip_ioc_tx_t *ioctx)
{
    if (!pobj || !ioctx->readreg || !ioctx->writereg || !ioctx->gettick)
    {
        return ETH_CHIP_STATUS_ERROR;
    }

    pobj->io.init = ioctx->init;
    pobj->io.deinit = ioctx->deinit;
    pobj->io.readreg = ioctx->readreg;
    pobj->io.writereg = ioctx->writereg;
    pobj->io.gettick = ioctx->gettick;

    return ETH_CHIP_STATUS_OK;
}

/*******************************************************************************
* 名    称：eth_chip_init
* 功    能：初始化ETH_CHIP及硬件资源
* 参数说明： pobj：设备对象；
* 返 回 值：ETH_CHIP_STATUS_OK：成功
            ETH_CHIP_STATUS_ADDRESS_ERROR：找不到设备地址
			ETH_CHIP_STATUS_READ_ERROR：不能读取寄存器
			ETH_CHIP_STATUS_WRITE_ERROR：不能写入寄存器
			ETH_CHIP_STATUS_RESET_TIMEOUT：无法软复位
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_init(eth_chip_object_t *pobj)
{
		uint32_t tickstart = 0, regvalue = 0, addr = 0;
		int32_t status = ETH_CHIP_STATUS_OK;



		pobj->io.readreg(addr, PHY_REGISTER2, &regvalue);
               
    if (pobj->is_initialized == 0)
    {
        if (pobj->io.init != 0)
        {
            /* MDC时钟 */
            pobj->io.init();
        }

        /*设置PHY地址为32*/
        pobj->devaddr = ETH_CHIP_MAX_DEV_ADDR + 1;

        /* 查找PHY地址 */  
        for (addr = 0; addr <= ETH_CHIP_MAX_DEV_ADDR; addr ++)
        {
            if (pobj->io.readreg(addr, ETH_CHIP_PHYSCSR, &regvalue) < 0)
            { 
                status = ETH_CHIP_STATUS_READ_ERROR;
                /*无法读取这个设备地址继续寻找 */
                continue;
            }

            if (addr >= ETH_CHIP_MAX_DEV_ADDR)
            {
                addr = 0;
                pobj->devaddr = addr;
                status = ETH_CHIP_STATUS_OK;
                break;
            }
            
            
            if ((regvalue & ETH_CHIP_PHY_COUNT) == addr)
            {
                pobj->devaddr = addr;
                status = ETH_CHIP_STATUS_OK;
                break;
            }
        }           
        if (pobj->devaddr > ETH_CHIP_MAX_DEV_ADDR)
        {
            status = ETH_CHIP_STATUS_ADDRESS_ERROR;
        }

       
        if (status == ETH_CHIP_STATUS_OK)
        {
           
            if (pobj->io.writereg(pobj->devaddr, ETH_CHIP_BCR, ETH_CHIP_BCR_SOFT_RESET) >= 0)
            {                
                if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_BCR, &regvalue) >= 0)
                { 
                    tickstart = pobj->io.gettick();              
                    while (regvalue & ETH_CHIP_BCR_SOFT_RESET)
                    {
                        if ((pobj->io.gettick() - tickstart) <= ETH_CHIP_SW_RESET_TO)
                        {
                            if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_BCR, &regvalue) < 0)
                            { 
                                status = ETH_CHIP_STATUS_READ_ERROR;
                                break;
                            }
                        }
                        else
                        {
                            status = ETH_CHIP_STATUS_RESET_TIMEOUT;
                            break;
                        }
                    } 
                }
                else
                {
                    status = ETH_CHIP_STATUS_READ_ERROR;
                }
            }
            else
            {
                status = ETH_CHIP_STATUS_WRITE_ERROR;
            }
        }
    } 
    if (status == ETH_CHIP_STATUS_OK)
    {
        tickstart =  pobj->io.gettick();
        while ((pobj->io.gettick() - tickstart) <= ETH_CHIP_INIT_TO)
        {
        }
        pobj->is_initialized = 1;
    }
		

    return status;
}

/*******************************************************************************
* 名    称：eth_chip_deinit
* 功    能：反初始化
* 参数说明： pobj：设备对象；
* 返 回 值：ETH_CHIP_STATUS_OK：反初始化失败成功
            ETH_CHIP_STATUS_ERROR：反初始化失败
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_deinit(eth_chip_object_t *pobj)
{
    if (pobj->is_initialized)
    {
        if (pobj->io.deinit != 0)
        {
            if (pobj->io.deinit() < 0)
            {
                return ETH_CHIP_STATUS_ERROR;
            }
        }

        pobj->is_initialized = 0;  
    }

    return ETH_CHIP_STATUS_OK;
}

/*******************************************************************************
* 名    称：eth_chip_disable_power_down_mode
* 功    能：关闭ETH_CHIP的下电模式
* 参数说明： pobj：设备对象；
* 返 回 值：ETH_CHIP_STATUS_OK：关闭成功
						ETH_CHIP_STATUS_READ_ERROR：不能读取寄存器
						ETH_CHIP_STATUS_WRITE_ERROR：不能写寄存器
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_disable_power_down_mode(eth_chip_object_t *pobj)
{
    uint32_t readval = 0;
    int32_t status = ETH_CHIP_STATUS_OK;

    if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
    {
        readval &= ~ETH_CHIP_BCR_POWER_DOWN;

        if (pobj->io.writereg(pobj->devaddr, ETH_CHIP_BCR, readval) < 0)
        {
            status =  ETH_CHIP_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = ETH_CHIP_STATUS_READ_ERROR;
    }

    return status;
}

/*******************************************************************************
* 名    称：eth_chip_enable_power_down_mode
* 功    能：使能ETH_CHIP的下电模式
* 参数说明： pobj：设备对象；
* 返 回 值：ETH_CHIP_STATUS_OK：关闭成功
						ETH_CHIP_STATUS_READ_ERROR：不能读取寄存器
						ETH_CHIP_STATUS_WRITE_ERROR：不能写寄存器
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_enable_power_down_mode(eth_chip_object_t *pobj)
{
    uint32_t readval = 0;
    int32_t status = ETH_CHIP_STATUS_OK;

    if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
    {
        readval |= ETH_CHIP_BCR_POWER_DOWN;

        if (pobj->io.writereg(pobj->devaddr, ETH_CHIP_BCR, readval) < 0)
        {
            status =  ETH_CHIP_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = ETH_CHIP_STATUS_READ_ERROR;
    }

    return status;
}

/*******************************************************************************
* 名    称：eth_chip_start_auto_nego
* 功    能：启动自动协商功能
* 参数说明： pobj：设备对象；
* 返 回 值：ETH_CHIP_STATUS_OK：关闭成功
						ETH_CHIP_STATUS_READ_ERROR：不能读取寄存器
						ETH_CHIP_STATUS_WRITE_ERROR：不能写寄存器
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_start_auto_nego(eth_chip_object_t *pobj)
{
    uint32_t readval = 0;
    int32_t status = ETH_CHIP_STATUS_OK;

    if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
    {
        readval |= ETH_CHIP_BCR_AUTONEGO_EN;

        if (pobj->io.writereg(pobj->devaddr, ETH_CHIP_BCR, readval) < 0)
        {
            status =  ETH_CHIP_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = ETH_CHIP_STATUS_READ_ERROR;
    }

    return status;
}

/*******************************************************************************
* 名    称：eth_chip_get_link_state
* 功    能：获取ETH_CHIP设备的链路状态
* 参数说明： pobj：设备对象；
* 返 回 值：ETH_CHIP_STATUS_100MBITS_FULLDUPLEX：100M,全双工
					  ETH_CHIP_STATUS_100MBITS_HALFDUPLEX：100M,半双工
					  ETH_CHIP_STATUS_10MBITS_FULLDUPLEX：10M，全双工
					  ETH_CHIP_STATUS_10MBITS_HALFDUPLEX：10M，半双工
            ETH_CHIP_STATUS_READ_ERROR：不能读取寄存器
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_get_link_state(eth_chip_object_t *pobj)
{
    uint32_t readval = 0;

    if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_PHYSCSR, &readval) < 0)
    {
        return ETH_CHIP_STATUS_READ_ERROR;
    }

    if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_PHYSCSR, &readval) < 0)
    {
        return ETH_CHIP_STATUS_READ_ERROR;
    }

    if (((readval & ETH_CHIP_SPEED_STATUS) != ETH_CHIP_SPEED_STATUS) && ((readval & ETH_CHIP_DUPLEX_STATUS) != 0))
    {
        return ETH_CHIP_STATUS_100MBITS_FULLDUPLEX;
    }
    else if (((readval & ETH_CHIP_SPEED_STATUS) != ETH_CHIP_SPEED_STATUS))
    {
        return ETH_CHIP_STATUS_100MBITS_HALFDUPLEX;
    }
    else if (((readval & ETH_CHIP_BCR_DUPLEX_MODE) != ETH_CHIP_BCR_DUPLEX_MODE))
    {
        return ETH_CHIP_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
        return ETH_CHIP_STATUS_10MBITS_HALFDUPLEX;
    }
}

/*******************************************************************************
* 名    称：eth_chip_set_link_state
* 功    能：设置ETH_CHIP设备的链路状态
* 参数说明：pobj：设备对象；pLinkState：指向链路状态的指针
* 返 回 值：ETH_CHIP_STATUS_OK：设置成功
					  ETH_CHIP_STATUS_ERROR：设置失败
						ETH_CHIP_STATUS_READ_ERROR：不能读取寄存器
						ETH_CHIP_STATUS_WRITE_ERROR：不能写入寄存器
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_set_link_state(eth_chip_object_t *pobj, uint32_t linkstate)
{
    uint32_t bcrvalue = 0;
    int32_t status = ETH_CHIP_STATUS_OK;

    if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_BCR, &bcrvalue) >= 0)
    {
        
        bcrvalue &= ~(ETH_CHIP_BCR_AUTONEGO_EN | ETH_CHIP_BCR_SPEED_SELECT | ETH_CHIP_BCR_DUPLEX_MODE);

        if (linkstate == ETH_CHIP_STATUS_100MBITS_FULLDUPLEX)
        {
            bcrvalue |= (ETH_CHIP_BCR_SPEED_SELECT | ETH_CHIP_BCR_DUPLEX_MODE);
        }
        else if (linkstate == ETH_CHIP_STATUS_100MBITS_HALFDUPLEX)
        {
            bcrvalue |= ETH_CHIP_BCR_SPEED_SELECT;
        }
        else if (linkstate == ETH_CHIP_STATUS_10MBITS_FULLDUPLEX)
        {
            bcrvalue |= ETH_CHIP_BCR_DUPLEX_MODE;
        }
        else
        {
            
            status = ETH_CHIP_STATUS_ERROR;
        }
    }
    else
    {
        status = ETH_CHIP_STATUS_READ_ERROR;
    }

    if(status == ETH_CHIP_STATUS_OK)
    {
        
        if(pobj->io.writereg(pobj->devaddr, ETH_CHIP_BCR, bcrvalue) < 0)
        {
            status = ETH_CHIP_STATUS_WRITE_ERROR;
        }
    }

    return status;
}

/*******************************************************************************
* 名    称：eth_chip_enable_loop_back_mode
* 功    能：使启用回环模式
* 参数说明：pobj：设备对象；
* 返 回 值：ETH_CHIP_STATUS_OK：设置成功
						ETH_CHIP_STATUS_READ_ERROR：不能读取寄存器
						ETH_CHIP_STATUS_WRITE_ERROR：不能写寄存器
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_enable_loop_back_mode(eth_chip_object_t *pobj)
{
    uint32_t readval = 0;
    int32_t status = ETH_CHIP_STATUS_OK;

    if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
    {
        readval |= ETH_CHIP_BCR_LOOPBACK;

        
        if (pobj->io.writereg(pobj->devaddr, ETH_CHIP_BCR, readval) < 0)
        {
            status = ETH_CHIP_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = ETH_CHIP_STATUS_READ_ERROR;
    }

    return status;
}

/*******************************************************************************
* 名    称：eth_chip_disable_loop_back_mode
* 功    能：禁用回环模式
* 参数说明：pobj：设备对象；
* 返 回 值：ETH_CHIP_STATUS_OK：设置成功
						ETH_CHIP_STATUS_READ_ERROR：不能读取寄存器
						ETH_CHIP_STATUS_WRITE_ERROR：不能写寄存器
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
int32_t eth_chip_disable_loop_back_mode(eth_chip_object_t *pobj)
{
    uint32_t readval = 0;
    int32_t status = ETH_CHIP_STATUS_OK;

    if (pobj->io.readreg(pobj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
    {
        readval &= ~ETH_CHIP_BCR_LOOPBACK;
        
        if (pobj->io.writereg(pobj->devaddr, ETH_CHIP_BCR, readval) < 0)
        {
            status =  ETH_CHIP_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = ETH_CHIP_STATUS_READ_ERROR;
    }

    return status;
}
