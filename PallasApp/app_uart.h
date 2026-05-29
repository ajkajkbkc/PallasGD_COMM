
#ifndef __APP_UART_H
#define __APP_UART_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private defines -----------------------------------------------------------*/
/* 串口一次接收最大长度 */
//#define AT_SEND_MAX_BYTE  1440
//#define AT_RECV_MAX_BYTE  (AT_SEND_MAX_BYTE + 128)



#define RX_LEN_UART1      1024
#define RX_LEN_UART2      512 //AT_RECV_MAX_BYTE
#define RX_LEN_UART3      800

#define WORD_TIMEOUT      10
#define FRAME_TIMEOUT     300

/*UART状态定义*/
enum __UART_STATUS_E
{
    /*端口空闲*/
    UART_STATE_IDLE = 0x01,
    /*接收*/
    UART_STATE_RX = 0x02,
    /*发送*/
    UART_STATE_TX = 0x04,
};

/*UART端口定义*/
enum __UART_PORT_E
{
    /* 串口1 */
    UART_PORT1 = 0,
    /* 串口2 */
    UART_PORT2 = 1,
    /* 串口3 */
    UART_PORT3 = 2,
    /*  */
    MAX_UART_PORT,
};

///*UART工作模式定义*/
//enum __UART_MODE_E
//{
//    /*默认*/
//    UART_MODE_DEFAULT = 0x00,
//    /*寻找flk终端状态*/
//    UART_MODE_FINDFLKRTU,
//    /*寻找hlk终端状态*/
//    UART_MODE_MB_MASTER,
//};

/*UART工作模式定义*/
enum __UART_MODE_E
{
    /*默认*/
    UART_MODE_DEFAULT,
    /*从模式*/
    UART_MODE_SALVE,
    /*主模式*/
    UART_MODE_MASTER,
    /*寻找flk终端状态*/
    UART_MODE_FINDFLKRTU,
    /*寻找hlk终端状态*/
    UART_MODE_FINDHLKRTU,
};

/*uart 缓冲区定义*/
typedef struct __BSP_UART_STATUS_PLC_INFO_ST {
    /*当前端口状态*/
    unsigned char mcv_Status;
    /*接收数据长度*/
    unsigned short msv_RxLength;
    /*待发送数据长度*/
    unsigned short msv_TxTotalLength;
    /*已发送数据长度*/
    unsigned short msv_TxLength;
    /*接收缓存区*/
    unsigned char *mcp_RxBuff;
    /*发送缓存区*/
    unsigned char *mcp_TxBuff;
    /*自由口、Modbus master接收缓冲区地址*/
    unsigned short *msp_FreeRxBuff;
    /*自由口、Modbus master接收最大长度*/
    unsigned short msv_FreeRxMaxCnt;
    /*Modbus master重发次数*/
    unsigned char mcv_RetryCnt;

    unsigned char aFlag; // 0, modbus; 1, modlink
    unsigned char *aBuf;
} bsp_uart_status_plc_info_st;

/* Exported types ------------------------------------------------------------*/
/*uart 缓冲区定义*/
typedef struct __BSP_UART_STATUS_INFO_ST
{
    /*串口模式：*/
    unsigned char mcv_Mode;
    /*当前端口状态*/
    unsigned char mcv_Status;
    /*接收数据长度计数*/
    unsigned short msv_RxCount;
    
//    /*当前端口状态*/
//    unsigned char mcv_Status;
    /*接收数据长度*/
    unsigned short msv_RxLength;
    /*待发送数据长度*/
    unsigned short msv_TxTotalLength;
    /*已发送数据长度*/
    unsigned short msv_TxLength;
    /*接收缓存区*/
    unsigned char *mcp_RxBuff;
    /*发送缓存区*/
    unsigned char *mcp_TxBuff;
    /*自由口、Modbus master接收缓冲区地址*/
    unsigned short *msp_FreeRxBuff;
    /*自由口、Modbus master接收最大长度*/
    unsigned short msv_FreeRxMaxCnt;
    /*Modbus master重发次数*/
    unsigned char mcv_RetryCnt;

    unsigned char aFlag; // 0, modbus; 1, modlink
    unsigned char *aBuf;
} bsp_uart_status_info_st;


/* uart任务消息队列结构体 */
typedef struct __UART_MSG_ST
{
    /*设备UART端口号*/
    unsigned char mcv_UartPort;
    /*数据长度*/
    unsigned short msv_MsgLength;
    /*数据缓存区指针*/
    unsigned char *mcp_DataBuff;
} uart_msg_st;

/*串口系统块配置字解析结构体*/
typedef struct __BSP_UART_CONFIG_ST
{
    /*端口波特率配置
    000 = 1200, 001 = 2400, 010 = 4800, 011 = 9600, 100 = 19200, 101 = 38400, 110 = 57600; 111 = 115200
    */
    unsigned short BaudRate     : 3;
    /*停止位 0：1位, 1: 2位*/
    unsigned short StopBits     : 1;
    /*校验位 0: 偶校验, 1: 奇校验*/
    unsigned short Parity       : 1;
    /*校验使能     0：不校验, 1: 校验*/
    unsigned short ParityEnable : 1;
    /*数据长度      0: 8, 1: 7*/
    unsigned short WordLength   : 1;

    /*自由口起始字节允许*/
    unsigned short StartWordEn  : 1;
    /*自由口结束字节允许*/
    unsigned short EndWordEn    : 1;
    /*自由口字符间超时时间使能*/
    unsigned short CharTimeOutEn    : 1;
    /*自由口帧间超时时间使能*/
    unsigned short FrameTimeOutEn   : 1;

    /*Modbus传输模式 0：RTU, 1: ASCII*/
    unsigned short ModbusTransMode  : 1;
    /*Modbus主从模式 0：从站, 1：主站*/
    unsigned short ModebusMode      : 1;

    /*协议类型
    001: 自由口 010：Modbus 011：KCBus
    */
    unsigned short ProtocolType     : 3;
} bsp_uart_config_st;


/* Exported constants --------------------------------------------------------*/
extern volatile bsp_uart_status_info_st gtv_UartPortStatus[MAX_UART_PORT];

extern uint8_t gcv_Uart2RecvBuf[RX_LEN_UART2];
static uint16_t gUart4RecvLength = 0;
/* Private functions ---------------------------------------------------------*/
void osThreadNew_uartTask(void);

void UART_Init(void);
void uartReceive_IDLE_FromISR(UART_HandleTypeDef *huart);

void bsp_uart1_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length);
extern void bsp_uart2_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length);
void bsp_uart3_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length);
void zigbee_reset(void);
void zigbee_reset_default(void);

void uart_addCRC_send_buffer(UART_HandleTypeDef *huart, unsigned char *pSendBuf, unsigned short len);
void bsp_uart1_init(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits);

#endif /* __APP_UART_H */
