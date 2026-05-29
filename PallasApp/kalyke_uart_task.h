/**
  ******************************************************************************
  * @file    kalyke_uart_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   串口任务
  ******************************************************************************
  */

#ifndef __KALYKE_UART_TASK_H
#define __KALYKE_UART_TASK_H
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*uart任务消息队列结构体*/
typedef struct __UART_MSG_ST{
    /*设备UART端口号*/
    unsigned char mcv_UartPort;
    /*数据长度*/
    unsigned short msv_MsgLength;
    /*数据缓存区指针*/
    unsigned char *mcp_DataBuff;
}uart_msg_st;

/*Modbus 数据传输结构体定义*/
typedef struct __MD_SLAVE_PACK_ST {
    /*信息发送源*/
    unsigned char mcv_Sender;
    /*接收数据长度*/
    unsigned short msv_ReceiveLen;
    /*发送数据长度*/
    unsigned short msv_RespLen;
    /*是否为广播数据包*/
    unsigned char mcv_IsBroadcastInfo;
    /*错误码*/
    unsigned char mcv_ErrorCode;

    /*接收数据缓存*/
    unsigned char *mcp_ReceiveBuff;
    /*响应数据缓存指针*/
    unsigned char *mcp_RespBuff;

    unsigned char isTcpClient; // 1: This is TCP client.
    unsigned char clientID;
    /*MODBUS从站站号*/
    unsigned char slaveID;
    void (*tcp_resp_func)(unsigned char *, unsigned short, unsigned char);
    
    /*响应回掉函数指针*/
    void (*resp_func)(unsigned char *, unsigned short);
} md_slave_pack;

extern QueueHandle_t gtv_UartTaskMsgQueueHandle;
/*------------------------------------------------------------------------------
*   UART task相关宏, 变量, 函数定义
*-----------------------------------------------------------------------------*/

extern TaskHandle_t gtv_UartTaskHandler;

extern void uart_task(void *p_arg);

void osThread_uartTask(void);
#endif /* __KALYKE_UART_TASK_H */
