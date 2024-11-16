/**
 ****************************************************************************************************
 * @file        lwip_tcp_server.c
 * @version     V1.0
 * @date        2024-06-26
 * @brief       构建tcp服务器
 ****************************************************************************************************
 */
#include <stdint.h>
#include <stdio.h>
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip_tcp_server.h"
#include "lwip_comm.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "screen.h"
#include "los_event.h"
#include "los_task.h"
#include "SD.h"


#define LWIP_DEMO_PORT               8080   /*本地端口号 */

 
/* 发送数据内容 */
char *g_lwip_command_sendbuf1 = "command1#";
char *g_lwip_command_sendbuf2 = "command2#";
char *g_lwip_command_sendbuf3 = "command3#";
char *g_lwip_command_sendbuf4 = "command4#";
/* TCP Server 全局状态标记变量
 * bit7:0，无数据发送；1有数据发送
 * bit6:0,没用收到数据；收到数据了
 * bit5:0,没有客户端连接上
 * bit4~0:保留 */
uint8_t g_lwip_send_flag;
static PServerData g_SevData;
/* tcp连接状态̬ */
enum tcp_server_states
{
    ES_TCPSERVER_NONE = 0,                  /*初始化 */
    ES_TCPSERVER_ACCEPTED,                  /* 连接状态 */
    ES_TCPSERVER_CLOSING,                   /* 关闭连接状态̬ */
};

/* LWIP回调函数使用结构体 */
struct tcp_server_struct
{
    uint8_t state;                          /* 当前状态̬ */
    struct tcp_pcb *pcb;                    /*指向TCP控制块 */
    struct pbuf *p;                         /*指向tx/rx/pbuf */
};

static err_t lwip_tcp_server_accept(void *arg,struct tcp_pcb *newpcb,err_t err);
static err_t lwip_tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void lwip_tcp_server_error(void *arg,err_t err);
static err_t lwip_tcp_server_usersent(struct tcp_pcb *tpcb,char *buf);
static err_t lwip_tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
static err_t lwip_tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void lwip_tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct *es);
static void lwip_tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es);
static void lwip_tcp_server_remove_timewait(void);

/*******************************************************************************
* 名    称：lwip_run
* 功    能：构建tcp服务器
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void lwip_run()
{
    err_t err;
    struct tcp_pcb *tcppcbnew;          
		struct tcp_pcb *tcppcbconn;         /* 定义TCP服务器控制块 */
    
    char *tbuf;
    uint8_t res = 0;

    tbuf = malloc(200);       /*申请内存 */

    if (tbuf == NULL)return ;        

    sprintf((char *)tbuf, "Server IP:%d.%d.%d.%d", g_lwipdev.ip[0], g_lwipdev.ip[1], g_lwipdev.ip[2], g_lwipdev.ip[3]); 
	g_SevData->IP = tbuf;
    sprintf((char *)tbuf, "Server Port:%d", LWIP_DEMO_PORT);                                                            
	g_SevData->PORT = tbuf;
    tcppcbnew = tcp_new();              /*创建一个新的pcb */
	g_SevData->recvbuf = g_lwip_demo_recvbuf;
    if (tcppcbnew)                    
    {
        err = tcp_bind(tcppcbnew, IP_ADDR_ANY, LWIP_DEMO_PORT); /*将本地ip与端口号绑定ַ */

        if (err == ERR_OK)              
        {
            tcppcbconn = tcp_listen(tcppcbnew);                 
            tcp_accept(tcppcbconn, lwip_tcp_server_accept);     
        }
        else
				{
					res = 1;
				}
					
    }
    else res = 1;
		
    while (res == 0)
    {
				/*改成监测命令下发*/
				int t=0;
				unsigned int uwEvent;
				uwEvent=LOS_EventRead(&Screen_EventGroup_CB,
													ETH_Command1_EVENT|ETH_Command2_EVENT|ETH_Command3_EVENT|ETH_Command4_EVENT,
													LOS_WAITMODE_OR|LOS_WAITMODE_CLR,
													10);
				if(ETH_Command1_EVENT == uwEvent)
				{
					lwip_tcp_server_usersent(tcppcbnew,g_lwip_command_sendbuf1);
					LOS_EventClear(&Screen_EventGroup_CB, ETH_Command1_EVENT);
					printf("command1#\r\n");
				}
				else if(ETH_Command2_EVENT == uwEvent)
				{
					lwip_tcp_server_usersent(tcppcbnew,g_lwip_command_sendbuf2);
					LOS_EventClear(&Screen_EventGroup_CB, ETH_Command1_EVENT);
				}
				else if(ETH_Command3_EVENT == uwEvent)
				{
					lwip_tcp_server_usersent(tcppcbnew,g_lwip_command_sendbuf3);
					LOS_EventClear(&Screen_EventGroup_CB, ETH_Command1_EVENT);
				}
				else if(ETH_Command4_EVENT == uwEvent)
				{
					lwip_tcp_server_usersent(tcppcbnew,g_lwip_command_sendbuf4);
					LOS_EventClear(&Screen_EventGroup_CB, ETH_Command1_EVENT);
				}
				if (g_lwip_send_flag & 1 << 6)                          /* 是否收到数据 */
				{
					SetTextValue(4,3,g_lwip_demo_recvbuf);
					printf("%s",g_lwip_demo_recvbuf);/* 将数据发给串口屏或者上云 */
											
					 g_lwip_send_flag &= ~(1 << 6);                      /* 标记 */
				}
				lwip_periodic_handle();
				lwip_pkt_handle();
				LOS_TaskDelay(20);
				t++;

    }
}

void InitServerData(PServerData PtSev)
{
	PtSev = g_SevData;
}

/*******************************************************************************
* 名    称：lwip_tcp_server_accept
* 功    能：LWIP TCP_accept的回调函数
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
err_t lwip_tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    err_t ret_err;
    struct tcp_server_struct *es;
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);
    tcp_setprio(newpcb, TCP_PRIO_MIN);                                  
    es = (struct tcp_server_struct *)mem_malloc(sizeof(struct tcp_server_struct)); 

    if (es != NULL)                                                 
    {
				printf("Connect TCP Client Success.\r\n");
				SetTextValue(4,10,"Connected");
				SetButtonValue(4,11,0);
        es->state = ES_TCPSERVER_ACCEPTED;                             
        es->pcb = newpcb;
        es->p = NULL;
        
        tcp_arg(newpcb, es);
        tcp_recv(newpcb, lwip_tcp_server_recv);                         
        tcp_err(newpcb, lwip_tcp_server_error);                        
        tcp_poll(newpcb, lwip_tcp_server_poll, 1);                      
        tcp_sent(newpcb, lwip_tcp_server_sent);                         

        g_lwip_send_flag |= 1 << 5;                                     
        g_lwipdev.remoteip[0] = newpcb->remote_ip.addr & 0xff;          /* IADDR4 */
        g_lwipdev.remoteip[1] = (newpcb->remote_ip.addr >> 8) & 0xff;   /* IADDR3 */
        g_lwipdev.remoteip[2] = (newpcb->remote_ip.addr >> 16) & 0xff;  /* IADDR2 */
        g_lwipdev.remoteip[3] = (newpcb->remote_ip.addr >> 24) & 0xff;  /* IADDR1 */
        ret_err = ERR_OK;
    }
    else
    {
				printf("Disconnect TCP Client.\r\n");
				SetTextValue(4,10,"Disconnected");
				SetButtonValue(4,11,0);
        ret_err = ERR_MEM;
    }

    return ret_err;
}

/*******************************************************************************
* 名    称：lwip_tcp_server_recv
* 功    能：LWIP TCP_recv的回调函数
* 参数说明：arg：传入参数
						tpcb：tcp控制块
						P：数据包首地址
* 返 回 值：err：错误码
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
err_t lwip_tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    err_t ret_err;
    uint32_t data_len = 0;
    struct pbuf *q;
    struct tcp_server_struct *es;
    LWIP_ASSERT("arg != NULL", arg != NULL);
    es = (struct tcp_server_struct *)arg;

	if (p == NULL)                                                          /*数据为空则关闭TCP连接 */
    {
        es->state = ES_TCPSERVER_CLOSING;         
        es->p = p;
        ret_err = ERR_OK;
    }
    else if (err != ERR_OK)                                                 /*数据非空但是出现意外错误 */
    {
        if (p)pbuf_free(p);                                                 /*释放pbuf */

        ret_err = err;
    }
    else if (es->state == ES_TCPSERVER_ACCEPTED)                            /* 处于连接状态*/
    {
        if (p != NULL)                                                      /* 当处于连接状态并接受到数据不为空时打印 */
        {
					memset(g_lwip_demo_recvbuf, 0, LWIP_DEMO_RX_BUFSIZE);           /* 清空接收区 */

					for (q = p; q != NULL; q = q->next)                             /* 遍历pbuf链表 */
            {
                /* 判断pbuf数据是否大于LWIP_DEMO_RX_BUFSIZE存储极限，大于则只拷贝可容纳的数据量 */
                
                if (q->len > (LWIP_DEMO_RX_BUFSIZE - data_len)) memcpy(g_lwip_demo_recvbuf + data_len, q->payload, (LWIP_DEMO_RX_BUFSIZE - data_len)); /* �������� */
                else memcpy(g_lwip_demo_recvbuf + data_len, q->payload, q->len);

                data_len += q->len;

                if (data_len > LWIP_DEMO_RX_BUFSIZE) break;              
            }

            g_lwip_send_flag |= 1 << 6;                                     /*关闭服务器清空释放内存*/
            g_lwipdev.remoteip[0] = tpcb->remote_ip.addr & 0xff;            /* IADDR4 */
            g_lwipdev.remoteip[1] = (tpcb->remote_ip.addr >> 8) & 0xff;     /* IADDR3 */
            g_lwipdev.remoteip[2] = (tpcb->remote_ip.addr >> 16) & 0xff;    /* IADDR2 */
            g_lwipdev.remoteip[3] = (tpcb->remote_ip.addr >> 24) & 0xff;    /* IADDR1 */
            tcp_recved(tpcb, p->tot_len);
            pbuf_free(p);                                                   
            ret_err = ERR_OK;
        }
    }
    else                                                                   
    {
        tcp_recved(tpcb, p->tot_len);                                       
        es->p = NULL;
        pbuf_free(p);                                                      
        ret_err = ERR_OK;
    }

    return ret_err;
}

/*******************************************************************************
* 名    称：lwip_tcp_server_error
* 功    能：LWIP TCP_err的回调函数
* 参数说明：arg：传入参数
			err：错误码
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void lwip_tcp_server_error(void *arg, err_t err)
{
		SetTextValue(4,10,"Disconnected");
		SetButtonValue(4,11,1);
    LWIP_UNUSED_ARG(err);
    printf("tcp error:%x\r\n", (uint32_t)arg);
    if (arg != NULL)mem_free(arg); 
}

/*******************************************************************************
* 名    称：lwip_tcp_server_usersent
* 功    能：数据发送
* 参数说明：tpcb：tcp控制块
						buf：命令下发buf
* 返 回 值：err：错误码
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
err_t lwip_tcp_server_usersent(struct tcp_pcb *tpcb,char *buf)
{
    err_t ret_err;
    struct tcp_server_struct *es;
    es = tpcb->callback_arg;

    if (es != NULL)                                                       
    {
        es->p = pbuf_alloc(PBUF_TRANSPORT, strlen((char *)buf), PBUF_POOL); 
        pbuf_take(es->p, (char *)buf, strlen((char *)buf)); 
        lwip_tcp_server_senddata(tpcb, es);                                                 
        g_lwip_send_flag &= ~(1 << 7);                                                      

        if (es->p != NULL)pbuf_free(es->p);                                                

        ret_err = ERR_OK;
    }
    else
    {
//        tcp_abort(tpcb);                                                                    
        ret_err = ERR_ABRT;
    }

    return ret_err;
}

/*******************************************************************************
* 名    称：lwip_tcp_server_poll
* 功    能：tcp_poll的回调函数
* 参数说明：tpcb：tcp控制块
						arg：传入参数
* 返 回 值：err：错误码
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
err_t lwip_tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct tcp_server_struct *es;
    es = (struct tcp_server_struct *)arg;

    if (es->state == ES_TCPSERVER_CLOSING)			
    {
				SetTextValue(4,10,"Disconnected");
				SetButtonValue(4,11,1);
        lwip_tcp_server_connection_close(tpcb, es);    
    }

    ret_err = ERR_OK;
    return ret_err;
}

/*******************************************************************************
* 名    称：lwip_tcp_server_sent
* 功    能：tcp_sent的回调函数
* 参数说明：tpcb：tcp控制块
						arg：传入参数
* 返 回 值：err：错误码
* 说    明：(客户端接收到ACK信号后发送数据)
* 调用方法：外部调用
*******************************************************************************/
err_t lwip_tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct tcp_server_struct *es;
    LWIP_UNUSED_ARG(len);
    es = (struct tcp_server_struct *) arg;

    if (es->p)lwip_tcp_server_senddata(tpcb, es);   

    return ERR_OK;
}

/*******************************************************************************
* 名    称：lwip_tcp_server_senddata
* 功    能：发送数据
* 参数说明：tpcb：tcp控制块
						es：LWIP回调函数使用的结构体
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void lwip_tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
    struct pbuf *ptr;
    uint16_t plen;
    err_t wr_err = ERR_OK;

    while ((wr_err == ERR_OK) && es->p && (es->p->len <= tcp_sndbuf(tpcb)))
    {
        ptr = es->p;
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);    /*将要发送的数据加入发送缓冲队列中*/

        if (wr_err == ERR_OK)
        {
            plen = ptr->len;
            es->p = ptr->next;                                  /*指向下一个pbuf */

            if (es->p)pbuf_ref(es->p);                          

            pbuf_free(ptr);
            tcp_recved(tpcb, plen);
        }
        else if (wr_err == ERR_MEM) es->p = ptr;
        tcp_output(tpcb);                                       /* 发送缓冲队列中的数据 */
    }
}

/*******************************************************************************
* 名    称：lwip_tcp_server_connection_close
* 功    能：关闭tcp连接
* 参数说明：tpcb：tcp控制块
						es：LWIP回调函数使用的结构体
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void lwip_tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
    tcp_close(tpcb);
    tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);

    if (es)mem_free(es);
    g_lwip_send_flag &= ~(1 << 5);                  /* 标记连接断开*/
}

extern void tcp_pcb_purge(struct tcp_pcb *pcb);     
extern struct tcp_pcb *tcp_active_pcbs;            
extern struct tcp_pcb *tcp_tw_pcbs;                 

/*******************************************************************************
* 名    称：lwip_tcp_server_remove_timewait
* 功    能：强制删除TCP Server主动断开时的time wait
* 参数说明：无
* 返 回 值：无
* 说    明：无
* 调用方法：外部调用
*******************************************************************************/
void lwip_tcp_server_remove_timewait(void)
{
    struct tcp_pcb *pcb, *pcb2;
    uint8_t t = 0;

    while (tcp_active_pcbs != NULL && t < 200)
    {
        lwip_periodic_handle();                     
        t++;
        LOS_TaskDelay(10);                               
		}

    pcb = tcp_tw_pcbs;

    while (pcb != NULL)                             
    {
        tcp_pcb_purge(pcb);
        tcp_tw_pcbs = pcb->next;
        pcb2 = pcb;
        pcb = pcb->next;
        memp_free(MEMP_TCP_PCB, pcb2);
    }
}

