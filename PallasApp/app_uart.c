
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"
#include <string.h>

#include "app_log.h"
#include "app_tool.h"
#include "app_uart.h"
#include "app_opts.h"
#include "app_led.h"
#include "iap.h"
#include "mb.h"
#include "mb_crc.h"
#include "app_etcr2900.h"
#include "app_parameter.h"
#include "app_collect.h"

#include "kalyke_4G_task.h"
#include "kalyke_internet_task.h"
#include "plc_variable.h"
#include "dlt645_2007.h"
/* Private define ------------------------------------------------------------*/
#define UART1_RX_TIMER_ID   1
#define UART2_RX_TIMER_ID   3
#define UART3_RX_TIMER_ID   5


//#define DEBUG_4G

//#ifdef DEBUG_4G
//#define LOGE_4G    LOGE
//#define LOGW_4G    LOGW
//#define LOGI_4G    LOGI
//#define LOGD_4G    LOGD
//#define LOGV_4G    LOGV
//#else
//#define LOGE_4G(...)
//#define LOGW_4G(...)
//#define LOGI_4G(...)
//#define LOGD_4G(...)
//#define LOGV_4G(...)
//#endif


QueueHandle_t g4GTCPQueue;
volatile static bool g4GMqttPublishing = false;
//static QueueHandle_t g4GMqttMsgQueue;
TimerHandle_t gUart4RxTimer =  NULL;

/* Private variables ---------------------------------------------------------*/
/* Definitions for uartTask */
osThreadId_t uartTaskHandle;
const osThreadAttr_t uartTask_attributes =
{
    .name = "uartTask",
    .priority = (osPriority_t) uartTaskPriority,
    .stack_size = 1024
};

QueueHandle_t gtv_UartTaskMsgQueueHandle;

/*定时器句柄*/
TimerHandle_t ltv_Uart1RxTime;
TimerHandle_t ltv_Uart2RxTime;
TimerHandle_t ltv_Uart3RxTime;


volatile bsp_uart_status_info_st gtv_UartPortStatus[MAX_UART_PORT];
/* uart recevie buffer */
uint8_t gcv_Uart1RecvBuf[RX_LEN_UART1];
uint8_t gcv_Uart2RecvBuf[RX_LEN_UART2];
uint8_t gcv_Uart3RecvBuf[RX_LEN_UART3];



/* Private function prototypes -----------------------------------------------*/
void UartTask(void *argument);




/* Private user code ---------------------------------------------------------*/




/*
 读寄存器返回  02 03 06 00 0B 00 16 00 21 B1 98
 读线圈返回       02 01 01 03 11 CD
*/
static void modlink_copy(uint16_t port)
{
    LOGV("bsp_uart", "Enter %s(), port = %u, aFlag = %u", __func__, port, gtv_UartPortStatus[port].aFlag);
    if (gtv_UartPortStatus[port].aFlag == 0)
    {
        if (gtv_UartPortStatus[port].aBuf)
        {
            vPortFree(gtv_UartPortStatus[port].aBuf);
            gtv_UartPortStatus[port].aBuf = NULL;
        }
        return;
    }

    //hexdump(gtv_UartPortStatus[port].aBuf, gtv_UartPortStatus[port].msv_RxLength);
    
//    uint16_t recvCrc = *(unsigned short *)(gtv_UartPortStatus[port].aBuf + gtv_UartPortStatus[port].msv_RxLength - 2);
//    uint16_t calculateCrc = calc_crc16(gtv_UartPortStatus[port].aBuf, gtv_UartPortStatus[port].msv_RxLength - 2);
//    LOGI("bsp_uart", "recvCrc = 0x%X, calculateCrc = 0x%X", recvCrc, calculateCrc);
//    if (recvCrc != calculateCrc)
//    {
//        LOGE("bsp_uart", "OMG, CRC ERROR!!!");
//        return;
//    }
    LOGE("bsp_uart", "OMG, 111111 ok!!!");
    int n;

    if (gtv_UartPortStatus[port].aBuf[1] == 3 ||
        gtv_UartPortStatus[port].aBuf[1] == 4) //读寄存器返回
    {
        n = gtv_UartPortStatus[port].aBuf[2] / 2; // 读取的元件个数
    }
    else if (gtv_UartPortStatus[port].aBuf[1] == 1 || //读线圈返回
             gtv_UartPortStatus[port].aBuf[1] == 2)
    {
        n = gtv_UartPortStatus[port].aBuf[2];
    }
    else
    {
        LOGE("bsp_uart", "We do not know 0x%X, so just return!!!", gtv_UartPortStatus[port].aBuf[1]);
        return;
    }
    LOGE("bsp_uart", "OMG, 222222 ok!!!");
    uint8_t *pBuf = &gtv_UartPortStatus[port].aBuf[3];
    for (int i = 0; i < n; i++)
    {
        if (gtv_UartPortStatus[port].aBuf[1] == 3 ||
            gtv_UartPortStatus[port].aBuf[1] == 4)
        {
            gtv_UartPortStatus[port].msp_FreeRxBuff[i] = GET_BIGPU16_DATA(pBuf);
            pBuf++;
            pBuf++;
        }
        else
        {
            gtv_UartPortStatus[port].msp_FreeRxBuff[i] = GET_PU8_DATA(pBuf);
            pBuf++;
        }
    }
    LOGE("bsp_uart", "OMG, 333333 ok!!!");
    
    vPortFree(gtv_UartPortStatus[port].aBuf);
    gtv_UartPortStatus[port].aBuf = NULL;
}


/**
  * @brief  处理从串口1收到的数据
  * @param  pBuf 数据起始地址
  * @param  cyLen 数据长度
  * @retval None
  */
static void HandleUART1RecvData(unsigned char *lcp_Buff, unsigned short lsv_Length)
{
    LOGV("uart", "Enter %s()", __func__);

//    LED_UART1_RX_FLASH();
    
    if(gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_DEFAULT)
    {
#if (APP_IAP == 1)
        if(is_iap_update(lcp_Buff, lsv_Length))
        {
            HAL_NVIC_SystemReset();
        }
#endif

#if PROD_TYPE == PROD_FG
        if( (is_RtuReturn_protocol(lcp_Buff, lsv_Length)) && ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1) )
        {
            /* Handle in is_RtuReturn_protocol() */
        }
#elif PROD_TYPE == PROD_FA
        if(is_AFD_protocol(&gtv_AfdDevice, lcp_Buff, lsv_Length, 0))
        {
            gtv_AfdDevice.mcv_HandleFlag = 0;
            uart2_send_buffer(pBuf, cyLen);
        }
#else
        if(is_dlt645_protocol(lcp_Buff, lsv_Length))
        {
            md_slave_msg_pack smsg = {0,};

            smsg.mcp_ReceiveBuff = lcp_Buff;
            smsg.msv_ReceiveLen = lsv_Length;
            smsg.mcv_Sender = MB_SENDER_UART1;
            smsg.uart_resp_func = bsp_uart1_send_buffer;
            smsg.mcp_RespBuff = (unsigned char *)pvPortMalloc(1024);
            LOGI("uart", "smsg.mcp_RespBuff = 0x%08X, Free Heap Size = %d", (uint32_t)smsg.mcp_RespBuff, xPortGetFreeHeapSize());
            DLT645_2007_DataAnalysis(&smsg);
            vPortFree(smsg.mcp_RespBuff);
        }
#endif
        else if(is_mb_protocol(lcp_Buff, lsv_Length))
        {
            md_slave_msg_pack smsg = {0,};

            smsg.mcv_IsBroadcastInfo = lcp_Buff[0] == 0 ? 1 : 0;
            smsg.mcp_ReceiveBuff = lcp_Buff;
            smsg.msv_ReceiveLen = lsv_Length;
            smsg.mcv_Sender = MB_SENDER_UART1;
            smsg.uart_resp_func = bsp_uart1_send_buffer;
            smsg.mcp_RespBuff = (unsigned char *)pvPortMalloc(1024);
            LOGI("uart", "smsg.mcp_RespBuff = 0x%08X, Free Heap Size = %d", (uint32_t)smsg.mcp_RespBuff, xPortGetFreeHeapSize());
            mb_slave_msg_handler(&smsg);
            vPortFree(smsg.mcp_RespBuff);
        }
    }
    else if(gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_MASTER)
    {                   
        for(int a=0; a < lsv_Length; a++)
        {
            if (gtv_UartPortStatus[1].aFlag == 0)
            {
                gtv_UartPortStatus[1].msp_FreeRxBuff[a] = lcp_Buff[a];
//                LOGE("bsp_uart", "msp_FreeRxBuff[%d] = 0x%X", a, lcp_Buff[a]);
//                LOGE("bsp_uart", "lcp_Buff[%d] = 0x%X", a, gtv_UartPortStatus[1].msp_FreeRxBuff[a]);
            }   
            else
            {
                gtv_UartPortStatus[1].aBuf[a] = lcp_Buff[a];
//                LOGV("bsp_uart", "lcp_Buff[a] = 0x%X", lcp_Buff[a]);
//                LOGE("bsp_uart", "aBuf[a] = 0x%X", gtv_UartPortStatus[1].aBuf[gtv_UartPortStatus[1].msv_RxLength++]);
            }
            
        }    
        modlink_copy(1);
        //gtv_UartPortStatus[UART_PORT1].mcv_Status = UART_STATE_IDLE;
    }
    else if(gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_FINDFLKRTU)
    {
//        Handle_FindFlkRtu_RecvData(lcp_Buff, lsv_Length);
    }
    else if(gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_FINDHLKRTU)
    {
//        Handle_FindHlkRtu_RecvData(lcp_Buff, lsv_Length);
    }
}

/**
  * @brief  处理从串口2收到的数据
  * @param  pBuf 数据起始地址
  * @param  cyLen 数据长度
  * @retval None
  */
//static void HandleUART2RecvData(unsigned char *lcp_Buff, unsigned short lsv_Length)
//{
//    LOGV("uart", "Enter %s()", __func__);


//    LED_UART2_RX_FLASH();

//    if(gtv_UartPortStatus[UART_PORT2].mcv_Mode == UART_MODE_DEFAULT)
//    {
//#if PROD_TYPE == PROD_FG
//        if( (is_RtuReturn_protocol(lcp_Buff, lsv_Length)) && ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2) )
//        {
//            /* Handle in is_RtuReturn_protocol() */
//        }
//#elif PROD_TYPE == PROD_FA
//        if(is_AFD_protocol(&gtv_AfdDevice, pBuf, cyLen, 0))
//        {
//            gtv_AfdDevice.mcv_HandleFlag = 0;
//            uart2_send_buffer(pBuf, cyLen);
//        }
//#else
//        if(0)
//        {
//            /* do nothing */
//        }
//#endif
//        else if(is_mb_protocol(lcp_Buff, lsv_Length))
//        {
//            md_slave_msg_pack smsg = {0,};

//            smsg.mcv_IsBroadcastInfo = lcp_Buff[0] == 0 ? 1 : 0;
//            smsg.mcp_ReceiveBuff = lcp_Buff;
//            smsg.msv_ReceiveLen = lsv_Length;
//            smsg.mcv_Sender = MB_SENDER_UART2;
//            smsg.uart_resp_func = bsp_uart2_send_buffer;
//            smsg.mcp_RespBuff = (unsigned char *)pvPortMalloc(2048);
//            LOGI("uart", "smsg.mcp_RespBuff = 0x%08X, Free Heap Size = %d", (uint32_t)smsg.mcp_RespBuff, xPortGetFreeHeapSize());
//            mb_slave_msg_handler(&smsg);
//            vPortFree(smsg.mcp_RespBuff);
//        }
//    }
//    else if(gtv_UartPortStatus[UART_PORT2].mcv_Mode == UART_MODE_FINDFLKRTU)
//    {
//        Handle_FindFlkRtu_RecvData(lcp_Buff, lsv_Length);
//    }
//    else if(gtv_UartPortStatus[UART_PORT2].mcv_Mode == UART_MODE_FINDHLKRTU)
//    {
//        Handle_FindHlkRtu_RecvData(lcp_Buff, lsv_Length);
//    }

//}

/**
  * @brief  处理从串口3收到的数据
  * @param  pBuf 数据起始地址
  * @param  cyLen 数据长度
  * @retval None
  */
static void HandleUART3RecvData(unsigned char *lcp_Buff, unsigned short lsv_Length)
{
    LOGV("uart", "Enter %s()", __func__);

    //LED_UART3_RX_FLASH();

    if(gtv_UartPortStatus[UART_PORT3].mcv_Mode == UART_MODE_DEFAULT)
    {
#if (APP_IAP == 1)
        if(is_iap_update(lcp_Buff, lsv_Length))
        {
            HAL_NVIC_SystemReset();
        }
#endif

#if PROD_TYPE == PROD_FG
        if( (is_RtuReturn_protocol(lcp_Buff, lsv_Length)) && ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1) )
        {
            /* Handle in is_RtuReturn_protocol() */
        }
#elif PROD_TYPE == PROD_FA
        if(is_AFD_protocol(&gtv_AfdDevice, lcp_Buff, lsv_Length, 0))
        {
            gtv_AfdDevice.mcv_HandleFlag = 0;
            uart2_send_buffer(pBuf, cyLen);
        }
#else
        if(0)
        {
            /* do nothing */
        }
#endif
        else if(is_mb_protocol(lcp_Buff, lsv_Length))
        {
            md_slave_msg_pack smsg = {0,};

            smsg.mcv_IsBroadcastInfo = lcp_Buff[0] == 0 ? 1 : 0;
            smsg.mcp_ReceiveBuff = lcp_Buff;
            smsg.msv_ReceiveLen = lsv_Length;
            smsg.mcv_Sender = MB_SENDER_UART3;
            smsg.uart_resp_func = bsp_uart3_send_buffer;
            smsg.mcp_RespBuff = (unsigned char *)pvPortMalloc(1024);
            LOGI("uart", "smsg.mcp_RespBuff = 0x%08X, Free Heap Size = %d", (uint32_t)smsg.mcp_RespBuff, xPortGetFreeHeapSize());
            mb_slave_msg_handler(&smsg);
            vPortFree(smsg.mcp_RespBuff);
        }
    }
    else if(gtv_UartPortStatus[UART_PORT3].mcv_Mode == UART_MODE_MASTER)
    {                   
        for(int a=0; a < lsv_Length; a++)
        {
            if (gtv_UartPortStatus[UART_PORT3].aFlag == 0)
            {
                gtv_UartPortStatus[UART_PORT3].msp_FreeRxBuff[a] = lcp_Buff[a];
//                LOGE("bsp_uart", "msp_FreeRxBuff[%d] = 0x%X", a, lcp_Buff[a]);
//                LOGE("bsp_uart", "lcp_Buff[%d] = 0x%X", a, gtv_UartPortStatus[1].msp_FreeRxBuff[a]);
            }   
            else
            {
                gtv_UartPortStatus[UART_PORT3].aBuf[a] = lcp_Buff[a];
//                LOGV("bsp_uart", "lcp_Buff[a] = 0x%X", lcp_Buff[a]);
//                LOGE("bsp_uart", "aBuf[a] = 0x%X", gtv_UartPortStatus[1].aBuf[gtv_UartPortStatus[1].msv_RxLength++]);
            }
            
        }    
        modlink_copy(UART_PORT3);
        //gtv_UartPortStatus[UART_PORT1].mcv_Status = UART_STATE_IDLE;
    }
    else if(gtv_UartPortStatus[UART_PORT3].mcv_Mode == UART_MODE_FINDFLKRTU)
    {
//        Handle_FindFlkRtu_RecvData(lcp_Buff, lsv_Length);
    }
    else if(gtv_UartPortStatus[UART_PORT3].mcv_Mode == UART_MODE_FINDHLKRTU)
    {
//        Handle_FindHlkRtu_RecvData(lcp_Buff, lsv_Length);
    }
}


/**
  * @brief  串口接收（在中断里调用）
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void uartReceive_IDLE_FromISR(UART_HandleTypeDef *huart)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if( (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE)) != RESET)
    {
        if(huart->Instance == USART1)
        {
            if(gtv_UartPortStatus[UART_PORT1].mcv_Status == UART_STATE_IDLE)
            {
                gtv_UartPortStatus[UART_PORT1].mcv_Status = UART_STATE_RX;
                gtv_UartPortStatus[UART_PORT1].msv_RxCount = 0;
                gcv_Uart1RecvBuf[gtv_UartPortStatus[UART_PORT1].msv_RxCount++] = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);

                /* ------ start uart1 timer ----- */
                if( xTimerStartFromISR( ltv_Uart1RxTime, &xHigherPriorityTaskWoken ) != pdPASS )
                {
                    // The start command was not executed successfully.  Take appropriate
                    // action here.
                    LOGE("uart", "Start ltv_Uart1RxTime not executed successfully");
                }
                if( xHigherPriorityTaskWoken != pdFALSE )
                {
                    // Call the interrupt safe yield function here (actual function
                    // depends on the FreeRTOS port being used).
                }
                /* ------ ----------------- ----- */
            }
            else if(gtv_UartPortStatus[UART_PORT1].mcv_Status == UART_STATE_RX)
            {
                if(gtv_UartPortStatus[UART_PORT1].msv_RxCount < RX_LEN_UART1)
                {
                    gcv_Uart1RecvBuf[gtv_UartPortStatus[UART_PORT1].msv_RxCount++] = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
                }
                else  //需要进行读取的操作 不然不会继续接收
                {
                    if((uint8_t)(huart->Instance->DR & (uint8_t)0x00FF)){}
                }

                /* ------ reset uart1 timer ----- */
                if( xTimerResetFromISR( ltv_Uart1RxTime, &xHigherPriorityTaskWoken ) != pdPASS )
                {
                    // The start command was not executed successfully.  Take appropriate
                    // action here.
                    LOGE("uart", "Reset ltv_Uart1RxTime not executed successfully");
                }
                if( xHigherPriorityTaskWoken != pdFALSE )
                {
                    // Call the interrupt safe yield function here (actual function
                    // depends on the FreeRTOS port being used).
                }
                /* ------ ----------------- ----- */
            }
        }
        else if(huart->Instance == USART2)
        {
            if(gtv_UartPortStatus[UART_PORT2].mcv_Status == UART_STATE_IDLE)
            {
                gtv_UartPortStatus[UART_PORT2].mcv_Status = UART_STATE_RX;
                gtv_UartPortStatus[UART_PORT2].msv_RxCount = 0;
                gcv_Uart2RecvBuf[gtv_UartPortStatus[UART_PORT2].msv_RxCount++] = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);

                /* ------ start uart2 timer ----- */
                if( xTimerStartFromISR( ltv_Uart2RxTime, &xHigherPriorityTaskWoken ) != pdPASS )
                {
                    // The start command was not executed successfully.  Take appropriate
                    // action here.
                    LOGE("uart", "Start ltv_Uart2RxTime not executed successfully");
                }
                if( xHigherPriorityTaskWoken != pdFALSE )
                {
                    // Call the interrupt safe yield function here (actual function
                    // depends on the FreeRTOS port being used).
                }
                /* ------ ----------------- ----- */
            }
            else if(gtv_UartPortStatus[UART_PORT2].mcv_Status == UART_STATE_RX)
            {
                if(gtv_UartPortStatus[UART_PORT2].msv_RxCount < RX_LEN_UART2)
                {
                    gcv_Uart2RecvBuf[gtv_UartPortStatus[UART_PORT2].msv_RxCount++] = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
                }
                else  //需要进行读取的操作 不然不会继续接收
                {
                    if((uint8_t)(huart->Instance->DR & (uint8_t)0x00FF)){}
                }

                /* ------ reset uart2 timer ----- */
                if( xTimerResetFromISR( ltv_Uart2RxTime, &xHigherPriorityTaskWoken ) != pdPASS )
                {
                    // The start command was not executed successfully.  Take appropriate
                    // action here.
                    LOGE("uart", "Reset ltv_Uart2RxTime not executed successfully");
                }
                if( xHigherPriorityTaskWoken != pdFALSE )
                {
                    // Call the interrupt safe yield function here (actual function
                    // depends on the FreeRTOS port being used).
                }
                /* ------ ----------------- ----- */
            }
        }
        else if(huart->Instance == USART3)
        {
            if(gtv_UartPortStatus[UART_PORT3].mcv_Status == UART_STATE_IDLE)
            {
                gtv_UartPortStatus[UART_PORT3].mcv_Status = UART_STATE_RX;
                gtv_UartPortStatus[UART_PORT3].msv_RxCount = 0;
                gcv_Uart3RecvBuf[gtv_UartPortStatus[UART_PORT3].msv_RxCount++] = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);

                /* ------ start uart3 timer ----- */
                if( xTimerStartFromISR( ltv_Uart3RxTime, &xHigherPriorityTaskWoken ) != pdPASS )
                {
                    // The start command was not executed successfully.  Take appropriate
                    // action here.
                    LOGE("uart", "Start ltv_Uart3RxTime not executed successfully");
                }
                if( xHigherPriorityTaskWoken != pdFALSE )
                {
                    // Call the interrupt safe yield function here (actual function
                    // depends on the FreeRTOS port being used).
                }
                /* ------ ----------------- ----- */
            }
            else if(gtv_UartPortStatus[UART_PORT3].mcv_Status == UART_STATE_RX)
            {
                if(gtv_UartPortStatus[UART_PORT3].msv_RxCount < RX_LEN_UART3)
                {
                    gcv_Uart3RecvBuf[gtv_UartPortStatus[UART_PORT3].msv_RxCount++] = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
                }
                else  //需要进行读取的操作 不然不会继续接收
                {
                    if((uint8_t)(huart->Instance->DR & (uint8_t)0x00FF)){}
                }

                /* ------ reset uart3 timer ----- */
                if( xTimerResetFromISR( ltv_Uart3RxTime, &xHigherPriorityTaskWoken ) != pdPASS )
                {
                    // The start command was not executed successfully.  Take appropriate
                    // action here.
                    LOGE("uart", "Reset ltv_Uart3RxTime not executed successfully");
                }
                if( xHigherPriorityTaskWoken != pdFALSE )
                {
                    // Call the interrupt safe yield function here (actual function
                    // depends on the FreeRTOS port being used).
                }
                /* ------ ----------------- ----- */
            }
        }

        //__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE);
    }
}

/**
  * @brief  定时器回掉函数
  * @param  None
  * @retval None
  */
static void bsp_timer_callback_func(TimerHandle_t ltv_TimeHandle)
{
    unsigned int liv_TimerId;
    uart_msg_st ltv_UartMsg;
    BaseType_t ltv_Err;

    liv_TimerId = (unsigned int)pvTimerGetTimerID(ltv_TimeHandle);
    LOGV("uart", "Enter %s, liv_TimerId = %d\r\n", __func__, liv_TimerId);

    if(liv_TimerId == UART1_RX_TIMER_ID)
    {
        if(gtv_UartPortStatus[UART_PORT1].mcv_Status == UART_STATE_RX)
        {
            gtv_UartPortStatus[UART_PORT1].mcv_Status = UART_STATE_IDLE;

//            hexdump(gcv_Uart1RecvBuf, gtv_UartPortStatus[UART_PORT1].msv_RxCount);
            
            ltv_UartMsg.mcp_DataBuff = gcv_Uart1RecvBuf;
            ltv_UartMsg.msv_MsgLength = gtv_UartPortStatus[UART_PORT1].msv_RxCount;
            ltv_UartMsg.mcv_UartPort = UART_PORT1;
            if(ltv_UartMsg.msv_MsgLength != 0)
            {
                ltv_Err = xQueueSend(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, 10);
                if(ltv_Err != pdPASS)
                {
                    /*错误处理*/
                    LOGE("uart", "xQueueSend to gtv_UartTaskMsgQueueHandle ERROR!\r\n");
                }
                else
                {
                    LOGI("bsp_uart", "xQueueSend : msv_MsgLength = %u", ltv_UartMsg.msv_MsgLength);
                }
            }
        }
    }
    else if(liv_TimerId == UART2_RX_TIMER_ID)
    {
        if(gtv_UartPortStatus[UART_PORT2].mcv_Status == UART_STATE_RX)
        {
            gtv_UartPortStatus[UART_PORT2].mcv_Status = UART_STATE_IDLE;
            
            LOGI("4G_task", "Enter %s, gUart2RecvLength = %u", __func__, gtv_UartPortStatus[UART_PORT2].msv_RxCount);
            LOGI("4G_task", "pATResp = %s", gcv_Uart2RecvBuf);

            //hexdump(gcv_Uart2RecvBuf, gtv_UartPortStatus[UART_PORT2].msv_RxCount);
    
            ltv_UartMsg.mcp_DataBuff = gcv_Uart2RecvBuf;
            ltv_UartMsg.msv_MsgLength = gtv_UartPortStatus[UART_PORT2].msv_RxCount;
            ltv_UartMsg.mcv_UartPort = UART_PORT2;
            if(ltv_UartMsg.msv_MsgLength != 0)
            {
                ltv_Err = xQueueSend(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, 10);
                if(ltv_Err != pdPASS)
                {
                    /*错误处理*/
                    LOGE("uart", "xQueueSend to gtv_UartTaskMsgQueueHandle ERROR!\r\n");
                }
                else
                {
                    //LOGI("bsp_uart", "xQueueSend : msv_MsgLength = %u", ltv_UartMsg.msv_MsgLength);
                }
            }
        }
    }
    else if(liv_TimerId == UART3_RX_TIMER_ID)
    {
        if(gtv_UartPortStatus[UART_PORT3].mcv_Status == UART_STATE_RX)
        {
            gtv_UartPortStatus[UART_PORT3].mcv_Status = UART_STATE_IDLE;

            ltv_UartMsg.mcp_DataBuff = gcv_Uart3RecvBuf;
            ltv_UartMsg.msv_MsgLength = gtv_UartPortStatus[UART_PORT3].msv_RxCount;
            ltv_UartMsg.mcv_UartPort = UART_PORT3;
            if(ltv_UartMsg.msv_MsgLength != 0)
            {
                ltv_Err = xQueueSend(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, 10);
                if(ltv_Err != pdPASS)
                {
                    /*错误处理*/
                    LOGE("uart", "xQueueSend to gtv_UartTaskMsgQueueHandle ERROR!\r\n");
                }
                else
                {
                    LOGI("bsp_uart", "xQueueSend : msv_MsgLength = %u", ltv_UartMsg.msv_MsgLength);
                }
            }
        }
    }
}

//static void uart4_callback_func(TimerHandle_t ltv_TimeHandle)
//{
//    LOGV("4G_task", "Enter %s, gUart4RecvLength = %u", __func__, gtv_UartPortStatus[UART_PORT2].msv_RxCount);
//    LOGI("4G_task", "pATResp = %s", gcv_Uart2RecvBuf);
//#ifdef DEBUG_4G
//    hexdump(gcv_Uart2RecvBuf, gtv_UartPortStatus[UART_PORT2].msv_RxCount);
//#endif
//    char *pQMTRECV = NULL;
//    if ((pQMTRECV = strstr((char *)gcv_Uart2RecvBuf, "+QIURC:")) != NULL)//收到TCP数据
//    {
//        if (g4GMqttPublishing == true)
//        {
//            LOGW("4G_task", "g4GMqttPublishing is true, so do not handle +QIURC");
//            gtv_UartPortStatus[UART_PORT2].msv_RxCount = 0;
//            goto EXITME;
//        }
//        if (gtv_UartPortStatus[UART_PORT2].msv_RxCount < 22)
//        {
//            LOGE("4G_task", "data length less than 22, so do not handle +QIURC");
//            gtv_UartPortStatus[UART_PORT2].msv_RxCount = 0;
//            goto EXITME;
//        }
//        //tcp_4G_start_busy_timer();
//        
//        mqtt_msg_st Msg;
//        Msg.type = 3;
//        Msg.msgLength = gUart4RecvLength;
//        Msg.dataBuff = pvPortMalloc(1600);
//        memset(Msg.dataBuff, 0, 1600);
//        memcpy(Msg.dataBuff, pQMTRECV, Msg.msgLength);
//        if (g4GTCPQueue)
//        {
//            BaseType_t ret = xQueueSendToBack(g4GTCPQueue, &Msg, 0);
//            if (ret == pdFALSE)
//            {
//                vPortFree(Msg.dataBuff);
//            }
//        }
//        gtv_UartPortStatus[UART_PORT2].msv_RxCount = 0;
//    }
//    else if ((pQMTRECV = strstr((char *)gcv_Uart2RecvBuf, "SEND ")) != NULL)//TCP每发送一次数据便会返回SEND OK，把这个过滤掉
//    {
//        gtv_UartPortStatus[UART_PORT2].msv_RxCount = 0;
//        //reset_4G_heart_timer();
//    }
//    else if ((pQMTRECV = strstr((char *)gcv_Uart2RecvBuf, "+QMTRECV:")) != NULL)
//    {
//        mqtt_msg_st mqttMsg;
//        mqttMsg.type = 0;
//        mqttMsg.msgLength = strlen(pQMTRECV);
//        mqttMsg.dataBuff = pvPortMalloc(1024);
//        memset(mqttMsg.dataBuff, 0, 1024);
//        memcpy(mqttMsg.dataBuff, pQMTRECV, mqttMsg.msgLength);
//        BaseType_t ret = xQueueSendToBack(g4GMqttMsgQueue, &mqttMsg, 100);
//        if (ret == pdFALSE)
//        {
//            vPortFree(mqttMsg.dataBuff);
//        }
//        gtv_UartPortStatus[UART_PORT2].msv_RxCount = 0;
//    }
//    else if ((pQMTRECV = strstr((char *)gcv_Uart2RecvBuf, "+QMTSTAT:")) != NULL)
//    {
////        notify_QMTSTAT(pQMTRECV);
//        gtv_UartPortStatus[UART_PORT2].msv_RxCount = 0;
//    }
//EXITME:
//    gtv_UartPortStatus[UART_PORT2].mcv_Status = UART_STATE_IDLE;
//    LOGD("4G_task", "Leave %s", __func__);
//}

/* ---------------------------------------------------------- UART_1 ------------------------------------------------------------- */
static void bsp_uart1_time_init(unsigned short lsv_RxPeriod)
{
    LOGV("uart", "Enter %s, lsv_RxPeriod = %d\r\n", __func__, lsv_RxPeriod);
    if (lsv_RxPeriod == 0)
    {
        lsv_RxPeriod = 100;
    }
    if((ltv_Uart1RxTime == NULL) && (lsv_RxPeriod > 0))
    {
        ltv_Uart1RxTime = xTimerCreate((const char *)"Uart1 RxTime",
                                       (TickType_t  )lsv_RxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART1_RX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }
}

void bsp_uart1_init(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits)
{
    LOGV("uart", "Enter %s, llv_BaudRate=%ld, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d.\r\n", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = llv_BaudRate;
    huart1.Init.WordLength = lcv_WordLength;
    huart1.Init.StopBits = lcv_StopBits;
    huart1.Init.Parity = lcv_Parity;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }

    gtv_UartPortStatus[UART_PORT1].mcv_Status = UART_STATE_IDLE;
    gtv_UartPortStatus[UART_PORT1].mcv_Mode = UART_MODE_DEFAULT;
    bsp_uart1_time_init(WORD_TIMEOUT);

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART1_IRQn);  //使能串口中断
}

void bsp_uart1_deinit(void)
{
    //LOGV("bsp_uart", "Enter %s()\r\n", __func__);
    if(ltv_Uart1RxTime != NULL)
    {
        xTimerDelete(ltv_Uart1RxTime, 100);
    }

    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_UART_DeInit(&huart1);
}

void bsp_uart1_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length)
{
    LOGV("uart", "Enter %s()", __func__);

//    hexdump(lcp_SendBuff, lsv_Length);
    HAL_UART_Transmit(&huart1, lcp_SendBuff, lsv_Length, HAL_MAX_DELAY);
    //LED_UART1_TX_FLASH();  //发送灯闪烁
}

/* ---------------------------------------------------------- UART_2 ------------------------------------------------------------- */
static void bsp_uart2_time_init(unsigned short lsv_RxPeriod)
{
    LOGV("uart", "Enter %s, lsv_RxPeriod = %d\r\n", __func__, lsv_RxPeriod);
    if (lsv_RxPeriod == 0)
    {
        lsv_RxPeriod = 100;
    }
    if((ltv_Uart2RxTime == NULL) && (lsv_RxPeriod > 0))
    {
        ltv_Uart2RxTime = xTimerCreate((const char *)"Uart2 RxTime",
                                       (TickType_t  )lsv_RxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART2_RX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }
}

void bsp_uart2_init(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits)
{
    LOGV("uart", "Enter %s, llv_BaudRate=%ld, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d.\r\n", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = llv_BaudRate;
    huart2.Init.WordLength = lcv_WordLength;
    huart2.Init.StopBits = lcv_StopBits;
    huart2.Init.Parity = lcv_Parity;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }

    gtv_UartPortStatus[UART_PORT2].mcv_Status = UART_STATE_IDLE;
    bsp_uart2_time_init(WORD_TIMEOUT);

    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART2_IRQn);  //使能串口中断
}

void bsp_uart2_deinit(void)
{
    LOGV("bsp_uart", "Enter %s()\r\n", __func__);
    if(ltv_Uart2RxTime != NULL)
    {
        xTimerDelete(ltv_Uart2RxTime, 100);
    }

    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_UART_DeInit(&huart2);
}

void bsp_uart2_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length)
{
    LOGV("uart", "Enter %s()", __func__);

    //hexdump(lcp_SendBuff, lsv_Length);
    HAL_UART_Transmit(&huart2, lcp_SendBuff, lsv_Length, HAL_MAX_DELAY);
    //LED_UART2_TX_FLASH();  //发送灯闪烁
}

/* ---------------------------------------------------------- UART_3 ------------------------------------------------------------- */
static void bsp_uart3_time_init(unsigned short lsv_RxPeriod)
{
    LOGV("uart", "Enter %s, lsv_RxPeriod = %d\r\n", __func__, lsv_RxPeriod);
    if (lsv_RxPeriod == 0)
    {
        lsv_RxPeriod = 100;
    }
    if((ltv_Uart3RxTime == NULL) && (lsv_RxPeriod > 0))
    {
        ltv_Uart3RxTime = xTimerCreate((const char *)"Uart3 RxTime",
                                       (TickType_t  )lsv_RxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART3_RX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }
}

void bsp_uart3_init(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits)
{
    LOGV("uart", "Enter %s, llv_BaudRate=%ld, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d.\r\n", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = llv_BaudRate;
    huart3.Init.WordLength = lcv_WordLength;
    huart3.Init.StopBits = lcv_StopBits;
    huart3.Init.Parity = lcv_Parity;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        Error_Handler();
    }

    gtv_UartPortStatus[UART_PORT3].mcv_Status = UART_STATE_IDLE;
    //gtv_UartPortStatus[UART_PORT3].mcv_Mode = UART_MODE_DEFAULT;
    bsp_uart3_time_init(WORD_TIMEOUT);

    __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART3_IRQn);  //使能串口中断
  
}

void bsp_uart3_deinit(void)
{
    //LOGV("bsp_uart", "Enter %s()\r\n", __func__);
    if(ltv_Uart3RxTime != NULL)
    {
        xTimerDelete(ltv_Uart3RxTime, 100);
    }

    HAL_NVIC_DisableIRQ(USART3_IRQn);
    HAL_UART_DeInit(&huart3);
}

void bsp_uart3_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length)
{
    LOGV("uart", "Enter %s()", __func__);

//    hexdump(lcp_SendBuff, lsv_Length);
    HAL_UART_Transmit(&huart3, lcp_SendBuff, lsv_Length, HAL_MAX_DELAY);
}


/* ---------------------------------------------------------- ------ ------------------------------------------------------------- */
/**
  * @brief  Uart Initialize.
  * @param  None
  * @retval None
  */
void UART_Init(void)
{
//    uint32_t Parity1, StopBits1, WordLength1;
//    const unsigned long cllv_BaudRate[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

//    bsp_uart_config_st *p_uartconfig1;
//    p_uartconfig1 = (bsp_uart_config_st *)&gFlashParam.st.uartparam[0];

//    /*波特率*/
//    if(p_uartconfig1->BaudRate > 8)
//    {
//        p_uartconfig1->BaudRate = 3;
//    }

//    /*奇偶校验位*/
//    if(p_uartconfig1->ParityEnable)
//    {
//        if(p_uartconfig1->Parity)
//            Parity1 = UART_PARITY_EVEN;
//        else
//            Parity1 = UART_PARITY_ODD;
//    }
//    else
//    {
//        Parity1 = UART_PARITY_NONE;
//    }

//    /*停止位*/
//    StopBits1 = p_uartconfig1->StopBits ? UART_STOPBITS_2 : UART_STOPBITS_1;

//    /*数据长度*/
//    WordLength1= p_uartconfig1->WordLength ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B;

    //bsp_uart1_init(cllv_BaudRate[p_uartconfig1->BaudRate], Parity1, WordLength1, StopBits1);
    bsp_uart1_init(9600, UART_PARITY_NONE, UART_WORDLENGTH_8B, UART_STOPBITS_1);
    bsp_uart2_init(115200, UART_PARITY_NONE, UART_WORDLENGTH_8B, UART_STOPBITS_1);
    bsp_uart3_init(9600, UART_PARITY_NONE, UART_WORDLENGTH_8B, UART_STOPBITS_1);
//#if   PROD_TYPE == PROD_FR
//    bsp_uart3_init(9600, UART_PARITY_NONE, UART_WORDLENGTH_8B, UART_STOPBITS_1);
//#endif

    uartlog_init();
}

/**
  * @brief  新建串口线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_uartTask(void)
{
    uartTaskHandle = osThreadNew(UartTask, NULL, &uartTask_attributes);
}

/**
  * @brief  Function implementing the uartTask thread.
  * @param  argument: Not used
  * @retval None
  */
void UartTask(void *argument)
{
    LOGD("uart", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());
    uart_msg_st ltv_UartMsg;

    /*创建消息队列*/
    gtv_UartTaskMsgQueueHandle = xQueueCreate(10, sizeof(uart_msg_st));
//    configASSERT(gtv_UartTaskMsgQueueHandle != NULL);
    
    for(;;)
    {
        LOGD("uart", "Let us wait UART something happen...");
        xQueueReceive(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, portMAX_DELAY);
        hexdump(ltv_UartMsg.mcp_DataBuff, ltv_UartMsg.msv_MsgLength);

        switch(ltv_UartMsg.mcv_UartPort)
        {
        case UART_PORT1:
            HandleUART1RecvData(ltv_UartMsg.mcp_DataBuff, ltv_UartMsg.msv_MsgLength);
            memset(gcv_Uart1RecvBuf, 0, RX_LEN_UART1);
            break;

        case UART_PORT2:
//            HandleUART2RecvData(ltv_UartMsg.mcp_DataBuff, ltv_UartMsg.msv_MsgLength);
//            memset(gcv_Uart2RecvBuf, 0, RX_LEN_UART2);   
            break;

        case UART_PORT3:
            HandleUART3RecvData(ltv_UartMsg.mcp_DataBuff, ltv_UartMsg.msv_MsgLength);
            memset(gcv_Uart3RecvBuf, 0, RX_LEN_UART3);
            break;

        default:
            break;
        }
    }
}

/* ---------------------------------- zigbee opts ------------------------------------------- */
/**
  * @brief  zigbee复位
  * @param  None
  * @retval None
  */
void zigbee_reset(void)
{
    HAL_GPIO_WritePin(ZIGBEE_RST_GPIO_Port, ZIGBEE_RST_Pin, GPIO_PIN_RESET);
    osDelay(10);
    HAL_GPIO_WritePin(ZIGBEE_RST_GPIO_Port, ZIGBEE_RST_Pin, GPIO_PIN_SET);
}

/**
  * @brief  zigbee恢复出厂设置
  * @param  None
  * @retval None
  */
void zigbee_reset_default(void)
{
    HAL_GPIO_WritePin(ZIGBEE_DEF_GPIO_Port, ZIGBEE_DEF_Pin, GPIO_PIN_RESET);
    //osDelay(10);
    zigbee_reset();
    osDelay(10);
    HAL_GPIO_WritePin(ZIGBEE_DEF_GPIO_Port, ZIGBEE_DEF_Pin, GPIO_PIN_SET);
}


void uart_addCRC_send_buffer(UART_HandleTypeDef *huart, unsigned char *pSendBuf, unsigned short len)
{
    LOGD("uart", "Enter %s()", __func__);

    unsigned short lsv_Crc;

    lsv_Crc = calc_crc16(pSendBuf, len);

    /*CRC是小端序,低字节在前*/
    pSendBuf[len] = (unsigned char)lsv_Crc;
    pSendBuf[len + 1] = (unsigned char)(lsv_Crc >> 8);
    len += 2;

    HAL_UART_Transmit(huart, pSendBuf, len, HAL_MAX_DELAY);

    if(huart->Instance == USART1)
    {
        //LED_UART1_TX_FLASH();  //发送灯闪烁
    }
    else if(huart->Instance == USART2)
    {
        //LED_UART2_TX_FLASH();  //发送灯闪烁
    }
    else if(huart->Instance == USART3)
    {
        //LED_UART2_TX_FLASH();  //发送灯闪烁
    }
}

