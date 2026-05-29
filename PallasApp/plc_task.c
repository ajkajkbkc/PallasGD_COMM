/**
  ******************************************************************************
  * @file    plc_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   创建系统运行中各任务
  ******************************************************************************
  */
//#include "fsl_debug_console.h"
#include "plc_task.h"
//#include "plc_sysinit.h"
//#include "plc_executer.h"
#include "plc_variable.h"
#include "plc_element.h"
//#include "bsp_led.h"
//#include "bsp_iwdg.h"

//#include "plc_internalmanage.h"
//#include "timers.h"
//#include "plc_executer.h"
//#include "kalyke_version.h"
//#include "plc_netcfg.h"
#include "kalyke_event.h"
//#include "plc_sysblock.h"
//#include "plc_errormsg.h"
//#include "plc_interrupt.h"
//#include "daisy_task.h"
//#include "kalyke_modbus_com.h"
#include "app_uart.h"
//#include "kalyke_uart_task.h"
//#include "kalyke_collect_task.h"
//#include "kalyke_oled_key_task.h"
#include "plc_netcfg.h"

#include "timers.h"
#include "app_log.h"
#include "app_parameter.h"
#include "mb.h"
#include "mb_crc.h"
#include "kalyke_4G_task.h"
/*------------------------------------------------------------------------------
*   PLC TASK相关全局变量定义
*-----------------------------------------------------------------------------*/
osThreadId_t PLCTaskHandle;
const osThreadAttr_t PLC_attributes =
{
    .name = "kalyke_PLC_task",
    .priority = (osPriority_t) osPriorityAboveNormal4,
    .stack_size = 1024
};

//#define DEBUG_PLC

//#ifdef DEBUG_PLC
//#define LOGE_PLC    LOGE
//#define LOGW_PLC    LOGW
//#define LOGI_PLC    LOGI
//#define LOGD_PLC    LOGD
//#define LOGV_PLC    LOGV
//#else
//#define LOGE(...)
//#define LOGW(...)
//#define LOGI(...)
//#define LOGD(...)
//#define LOGV(...)
//#endif

#if PRINT_LOG_OPEN == 1
static const char *TAG = "PLC";
#endif

static TimerHandle_t gModbusRespTimer[MAX_SUPPORT_UART_PORT];
static TaskHandle_t gModbusHandle[MAX_SUPPORT_UART_PORT];
static bool gRespTimeout[MAX_SUPPORT_UART_PORT];
//static TimerHandle_t gPlcTimerHandle;
//TaskHandle_t gtv_PlcTaskHandler;
void (*pFunc)(void);  /*定义一个函数指针*/

#if 0
static void timerCallback( TimerHandle_t xTimer )
{
    PRINTF("Enter %s()\r\n", __func__);
    plc_run_do();
}
#endif

void plc_run(void)
{
    #if 0
    PRINTF("Enter %s()\r\n", __func__);
    if (gPlcTimerHandle == NULL)
    {
        gPlcTimerHandle = xTimerCreate((const char *)"timer_plc",
                                (TickType_t  )4000 / portTICK_PERIOD_MS,
                                (UBaseType_t )pdFALSE,
                                (void *      )10,
                                (TimerCallbackFunction_t)timerCallback);
    }
    xTimerStart(gPlcTimerHandle, 100);
    #endif
}

void plc_re_run(void)
{
    #if 0
    xTimerReset(gPlcTimerHandle, 100);
    #endif
}

static void kalyke_modbus_stop_resp_time_out_timer(uint16_t lsv_PortNum)
{
    LOGD(TAG, "Enter %s(), lsv_PortNum: %u", __func__, lsv_PortNum);
    if (gModbusRespTimer[lsv_PortNum])
    {
        xTimerStop(gModbusRespTimer[lsv_PortNum], 10);
    }
}

static void kalyke_modbus_next_index(unsigned short lsv_PortNum, modbus_cfg_st *ltp_ModlinkHead)
{
    ltp_ModlinkHead->index[lsv_PortNum]++;
    if (ltp_ModlinkHead->index[lsv_PortNum] >= ltp_ModlinkHead->listNum)
    {
        ltp_ModlinkHead->index[lsv_PortNum] = 0;
    }
}

/**
  * @brief  获取环形缓冲区写指针
  * @param  None
  * @retval None
  */
unsigned char * ring_buffer_get_write_mem(ring_buffer_st *ltp_RingBuff)
{
    return ltp_RingBuff->mcp_Buff[ltp_RingBuff->mcv_Index];
}

/**
  * @brief  移动访问指针
  * @param  None
  * @retval None
  */
void ring_buffer_switch_write_mem(ring_buffer_st *ltp_RingBuff)
{
    LOGI(TAG, "Enter %s()", __func__);
    if(ltp_RingBuff->mcv_Index+1 < RING_BUFFER_NODE_NUM) {
        ltp_RingBuff->mcv_Index += 1;
    } else {
        ltp_RingBuff->mcv_Index = 0;
    }
}


static void resp_time_out(TimerHandle_t ltv_TimeHandle)
{
    LOGE(TAG, "Enter %s()", __func__);
    uint16_t lsv_PortNum = (uint32_t)pvTimerGetTimerID(ltv_TimeHandle);
    LOGI(TAG, "Enter %s(), lsv_PortNum(%u)", __func__, lsv_PortNum);
    
    modbus_cfg_st *ltp_ModlinkHead = &g_modbus_cfg;
    modbus_item_st *ltp_ModlinkItem = &ltp_ModlinkHead->listPtr[ltp_ModlinkHead->index[lsv_PortNum]];
    
    gRespTimeout[lsv_PortNum] = true;
    *(ltp_ModlinkItem->execResult) = MODLINK_RUN_TIMEOUT;
    RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
    RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
    ltp_ModlinkItem->isExec = 0;     
    kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);

    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_UART_COM0_HAD_RESP << lsv_PortNum);
}

static void kalyke_modbus_start_resp_time_out_timer(uint16_t lsv_PortNum, uint32_t timeout)
{
    //bsp_feed_watch_dog();
    LOGV(TAG, "Enter %s(), lsv_PortNum: %u", __func__, lsv_PortNum);
    gRespTimeout[lsv_PortNum] = false;
    if (gModbusRespTimer[lsv_PortNum] == NULL)
    {
        if (timeout < 2000)
        {
            timeout = 2000;
        }

        char timerName[configMAX_TASK_NAME_LEN] = {0};
        sprintf(timerName, "modbus_timeout%u", lsv_PortNum);
        gModbusRespTimer[lsv_PortNum] = xTimerCreate((const char *)timerName,
                                     (TickType_t  )timeout / portTICK_PERIOD_MS,
                                     (UBaseType_t )pdFALSE,
                                     (void *      )(uint32_t)lsv_PortNum,
                                     (TimerCallbackFunction_t)resp_time_out);
        LOGD(TAG, "Create %s ok.", timerName);
    }
    xTimerStart(gModbusRespTimer[lsv_PortNum], 10);
}

static bool ifTriggered(modbus_item_st *pModbusItem)
{
    LOGV(TAG, "Enter %s(), triggerType = %u, triggerAddr = %u, commType = %u", __func__, pModbusItem->triggerType, pModbusItem->triggerAddr, pModbusItem->commType);
    if (pModbusItem->commType == 0)
    {
        return true;
    }
    if (pModbusItem->triggerType == 1) // M element
    {
        if (plc_get_bit_element_value(M_ELEMENT, pModbusItem->triggerAddr))
        {
            return true;
        }
    }
    else // S element
    {
        if (plc_get_bit_element_value(S_ELEMENT, pModbusItem->triggerAddr))
        {
            return true;
        }
    }
    return false;
}

/**
  * @brief  设置自由口、modbus master模式接收参数
  * @param  flag  0,以modbus模式接收数据
  * @retval None
  */
void bsp_set_free_port_receive_para(unsigned short lsv_PortNum, unsigned short *lsp_RxBuff, unsigned short lsv_RxCnt, uint8_t flag)
{
    LOGD("bsp_uart", "Enter %s(), lsv_PortNum = %u, lsp_RxBuff = 0x%08X, lsv_RxCnt = %u, flag = %u", __func__, lsv_PortNum, lsp_RxBuff, lsv_RxCnt, flag);
    gtv_UartPortStatus[lsv_PortNum].msp_FreeRxBuff = lsp_RxBuff;
    gtv_UartPortStatus[lsv_PortNum].msv_FreeRxMaxCnt = lsv_RxCnt;
    gtv_UartPortStatus[lsv_PortNum].msv_RxLength = 0;

    if (gtv_UartPortStatus[lsv_PortNum].aBuf)
    {
        vPortFree(gtv_UartPortStatus[lsv_PortNum].aBuf);
    }
    gtv_UartPortStatus[lsv_PortNum].aBuf = pvPortMalloc(1024);
    gtv_UartPortStatus[lsv_PortNum].aFlag = flag;
    
    LOGE("bsp_uart", "LEAVE %s(), lsv_PortNum = %u, lsp_RxBuff = 0x%08X, lsv_RxCnt = %u, flag = %u", __func__, lsv_PortNum, lsp_RxBuff, lsv_RxCnt, gtv_UartPortStatus[lsv_PortNum].aFlag);
}

// Just for COM0 use!
static uint8_t gRing1[128];
static uint8_t gRing2[128];
// Just for COM1 use!
static uint8_t gRing3[128];
static uint8_t gRing4[128];
// Just for COM2 use!
static uint8_t gRing5[128];
static uint8_t gRing6[128];

void ring_buffer_init(ring_buffer_st *ltp_RingBuff, unsigned char uartPort)
{
    if (uartPort == 0)
    {
        ltp_RingBuff->mcp_Buff[0] = gRing1;
        ltp_RingBuff->mcp_Buff[1] = gRing2;
    }
    else if (uartPort == 1)
    {
        ltp_RingBuff->mcp_Buff[0] = gRing3;
        ltp_RingBuff->mcp_Buff[1] = gRing4;
    }
    else if (uartPort == 2)
    {
        ltp_RingBuff->mcp_Buff[0] = gRing5;
        ltp_RingBuff->mcp_Buff[1] = gRing6;
    }
    ltp_RingBuff->mcv_Index = 0;
}

static int8_t kalyke_modbus_com_task_do(unsigned short lsv_PortNum)
{
    LOGV(TAG, "Enter %s(), lsv_PortNum(%u)", __func__, lsv_PortNum);

//    uint8_t lcv_Ret;
//    unsigned short lsv_SheetNum;
//    unsigned short lsv_EdgeId;

    modbus_cfg_st *ltp_ModlinkHead = &g_modbus_cfg;
    modbus_item_st *ltp_ModlinkItem;

    uart_port_info_st *ltp_UartPort;
//    uint8_t lcv_SuspendFlag;
//    unsigned short i, j;
    unsigned short lsv_TxLength, lsv_RxLength;
    uint8_t *lcp_UartTxBuff = NULL;
    unsigned short lsv_TxCrc;
    
    /*取通信串口号*/
//  lsv_PortNum = g_modbus_cfg.listPtr[g_modbus_cfg.index].port;
    ltp_ModlinkItem = &ltp_ModlinkHead->listPtr[ltp_ModlinkHead->index[lsv_PortNum]];

    /*串口不匹配*/
    if(ltp_ModlinkItem->port != lsv_PortNum)
    {
        kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
        return pdPASS;
    }

    LOGW(TAG, "lsv_PortNum = %u, slaveAddr = %u", lsv_PortNum, ltp_ModlinkItem->slaveAddr);
    
    
    if(lsv_PortNum >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum)
    {
        *(ltp_ModlinkItem->execResult) = MODLINK_COM_ERR; 
        LOGE(TAG, "Port number so big, leave %s()...001", __func__);
        kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
        return ERR_MODBUS_TBL;
    }

    ltp_UartPort = &gtp_UartPort[lsv_PortNum];
    LOGV(TAG, "ltp_UartPort->mcv_Mode = %u", ltp_UartPort->mcv_Mode);
    if(ltp_UartPort->mcv_Mode != UART_TYPE_MB_MASTER)
    {
        *(ltp_ModlinkItem->execResult) = MODLINK_COMCFG_ERR;
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        /*设置主从模式错误标志*/
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_SERVER_SLAVE_MODE_ERR);
        LOGW(TAG, "Uart %u mode is not master, leave %s()...003", lsv_PortNum, __func__);
        kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
        /*返回系统块配置错误*/
        return ERR_MODBUS_TBL;
    }

    LOGI(TAG, "ltp_ModlinkHead = 0x%08X", ltp_ModlinkHead);
    LOGV(TAG, "ltp_ModlinkHead->index = %u, ltp_ModlinkHead->listNum = %u", ltp_ModlinkHead->index[lsv_PortNum], ltp_ModlinkHead->listNum);

    if (ifTriggered(ltp_ModlinkItem) == false)
    {
        *(ltp_ModlinkItem->execResult) = MODLINK_NOT_RUN;
        ltp_ModlinkItem->isExec = 0;
        kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
        return pdPASS;
    }

    LOGI(TAG, "isExec = %u", ltp_ModlinkItem->isExec);
    /*指令未执行*/
    if (!ltp_ModlinkItem->isExec)
    {
        ring_buffer_init(&gtp_UartPort[lsv_PortNum].mtv_SendBuff, lsv_PortNum);
        
        /*获取串口发送缓冲区*/
        lcp_UartTxBuff = ring_buffer_get_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
        configASSERT(lcp_UartTxBuff != NULL); 

        /*切换发送缓冲区，为下次消息准备*/
        ring_buffer_switch_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
        
        LOGI(TAG, "ltp_ModlinkItem->funcCode = 0x%X", ltp_ModlinkItem->funcCode);
        switch(ltp_ModlinkItem->funcCode)
        {
        case MB_READ_HOLDING_REGISTER:
        case MB_READ_MULTIPLE_INPUT_REGISTER: 
            lcp_UartTxBuff[0] = ltp_ModlinkItem->slaveAddr;
            lcp_UartTxBuff[1] = ltp_ModlinkItem->funcCode;
            lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->slaveRegAddr >> 8);
            lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->slaveRegAddr;
            lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->elementCnt >> 8);
            lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->elementCnt;
            lsv_TxLength = 6;

            lsv_RxLength = (ltp_ModlinkItem->elementCnt << 1) + 5;
            /*接收缓冲区越界检查*/
            if(ltp_ModlinkItem->masterBuf + lsv_RxLength > ltp_ModlinkItem->masterBufBoundary)
            {
                LOGE(TAG, "ltp_ModlinkItem->masterBuf = %p, lsv_RxLength=%d", ltp_ModlinkItem->masterBuf, lsv_RxLength);
                LOGE(TAG, "ltp_ModlinkItem->masterBufBoundary = %p", ltp_ModlinkItem->masterBufBoundary);
                
                *(ltp_ModlinkItem->execResult) = MODLINK_PARA_ERR;
                LOGW(TAG, "Leave %s()...011", __func__);
                // return ERR_OVER_ELEMENT_RANG;
                return ERR_MODBUS_TBL;
            }
            /*设置接收参数*/
            if(lsv_PortNum == 1)
            {
                //bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkItem->masterBuf, lsv_RxLength, 1);
            }
            else
            {
                bsp_set_free_port_receive_para(2, ltp_ModlinkItem->masterBuf, lsv_RxLength, 1);
            }
            LOGE(TAG, "lsv_PortNum =  %d", lsv_PortNum);
            break;

        default:
            *(ltp_ModlinkItem->execResult) = MODLINK_PARA_ERR;
            LOGW(TAG, "Leave %s()...014", __func__);
            //return pdFAIL;
            return ERR_MODBUS_TBL;
        }

        lsv_TxCrc = calc_crc16(lcp_UartTxBuff, lsv_TxLength);
        lcp_UartTxBuff[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
        lcp_UartTxBuff[lsv_TxLength] = (unsigned char)lsv_TxCrc;
        
        /*清除相关标志位*/
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_TX_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_RX_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, 0);
        
        ltp_ModlinkItem->timeOut = GET_1MS_TICKS_COUNT() + gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut + 1;
        LOGW(TAG, "msv_TxTimeOut = %u, ltp_ModlinkItem->timeOut = %u", gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut, ltp_ModlinkItem->timeOut);
        ltp_ModlinkItem->isExec = 1;

        *(ltp_ModlinkItem->execResult) = MODLINK_RUNNING;
        
        if(lsv_PortNum == 1)
        {
//            gtv_UartPortStatus[UART_PORT1].mcv_Mode = UART_MODE_MASTER;
//       
//            gtp_UartPort[lsv_PortNum].pSendFunc = bsp_uart1_send_buffer;
        }
        else
        {
            gtv_UartPortStatus[UART_PORT3].mcv_Mode = UART_MODE_MASTER;
        
            gtp_UartPort[lsv_PortNum].pSendFunc = bsp_uart3_send_buffer;
        }
        
        /*发送数据*/
        if(gtp_UartPort[lsv_PortNum].pSendFunc)
        {
            gtp_UartPort[lsv_PortNum].pSendFunc(lcp_UartTxBuff, lsv_TxLength + 2);                             
        }
        kalyke_modbus_start_resp_time_out_timer(lsv_PortNum, gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut + 1);
        return -1;
    }
    else // isExec = 1
    {
        uint32_t curTick = GET_1MS_TICKS_COUNT();
        int32_t interVal = curTick - ltp_ModlinkItem->timeOut;
        LOGI(TAG, "ltp_ModlinkItem->timeOut = %u, curTick = %u, interVal = %d", ltp_ModlinkItem->timeOut, curTick, interVal);
        if(GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH) &&
                GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR))
        {
            *(ltp_ModlinkItem->execResult) = MODLINK_RUN_ERR;
            ltp_ModlinkItem->isExec = 0;
            kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
            LOGW(TAG, "Leave %s()...015", __func__);
            return pdPASS;
        }
        else if(GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH))
        {
            *(ltp_ModlinkItem->execResult) = MODLINK_PASS;
            ltp_ModlinkItem->isExec = 0;
            kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
            LOGW(TAG, "Leave %s()...016", __func__);
            return pdPASS;
        }
        else if(interVal < 0)
        {
            /*帧传输未超时，等待...*/
            LOGW(TAG, "Leave %s()...017, Not time out, waitting...", __func__);
            return pdPASS;
        }
        else
        {
            // 超时了
            *(ltp_ModlinkItem->execResult) = MODLINK_RUN_TIMEOUT;
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
            ltp_ModlinkItem->isExec = 0;
            kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
//            LOGW(TAG, "Leave %s()...018, Time out", __func__);
//            return pdPASS;
        }
    }
    LOGD(TAG, "Leave %s()...019", __func__);
    return pdPASS;
}

void kalyke_modbus_com_task(void *p_arg)
{
    LOGD(TAG, "kalyke_modbus_com_task RUN. Free heap size is %d bytes.(COM%u)", xPortGetFreeHeapSize(), *(uint16_t*)p_arg);

    vTaskDelay(3000);
    int8_t lcv_RetValue = pdPASS;
    uint16_t curDelay = 1000;
    uint16_t lsv_PortNum = *(uint16_t *)p_arg; //取通信串口号

    while(1)
    {
//        if (gTouChuan != 0xFF)
//        {
//            vTaskDelay(1000);
//            continue;
//        }
//        if (gGUHUAing == 1 || gSuspendFlag == true)
//        {
//            LOGE(TAG, "Enter %s(),GUHUAing..........vTaskDelay(1000).", __func__);
//            vTaskDelay(1000);
//            continue;
//        }

        lcv_RetValue = kalyke_modbus_com_task_do(lsv_PortNum);
        if (-1 == lcv_RetValue) // 数据已成功发送出去了
        {
            xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_UART_COM0_HAD_RESP << lsv_PortNum, pdTRUE, pdFALSE, portMAX_DELAY);
            //curDelay = g_modbus_cfg.listPtr[g_modbus_cfg.index[lsv_PortNum]].delay;
            if (gRespTimeout[lsv_PortNum] == false)
            {
                kalyke_modbus_stop_resp_time_out_timer(lsv_PortNum);
                *g_modbus_cfg.listPtr[g_modbus_cfg.index[lsv_PortNum]].execResult = MODLINK_PASS;
                g_modbus_cfg.listPtr[g_modbus_cfg.index[lsv_PortNum]].isExec = 0;
                kalyke_modbus_next_index(lsv_PortNum, &g_modbus_cfg);
            }
            LOGD(TAG, "kalyke_modbus_com%u_task RUN. Delay %d ms.\r\n", lsv_PortNum, curDelay);
            vTaskDelay(curDelay);
            continue;
        }
        /*执行错误，比如参数错误，主站站号错误等刷新错误标志元件值*/
        if(lcv_RetValue != pdPASS)
        {
//            plc_refresh_error_msg(lcv_RetValue);
        }
        /*lcv_RetValue == pdPASS数据没有发送，比如触发元件为OFF的情况*/
        LOGD(TAG, "kalyke_modbus_com_task RUN. Delay 1 ms.\r\n");
        vTaskDelay(1);
    }
}

void start_modbus_com(void)
{ 
    LOGV("app_main", "Free heap size is %d bytes", xPortGetFreeHeapSize());
    
//    vTaskDelete(PLCTaskHandle);
//    PLCTaskHandle = NULL;
//    LOGE("tool", "PLCTaskHandle = 0x%08X", PLCTaskHandle);
    
    LOGV(TAG, "Enter %s(), g_modbus_cfg.listNum = %u", __func__, g_modbus_cfg.listNum);
    if (g_modbus_cfg.listNum == 0)
    {
        return;
    }

    /*some parameter init*/
    for(uint16_t i = 0; i < MAX_SUPPORT_UART_PORT; i++)
    {
        gModbusRespTimer[i] = NULL;
        gRespTimeout[i] = false;
        gModbusHandle[i] = NULL;
        g_modbus_cfg.index[i] = 0;
    }
    
    for(uint16_t i = 0; i < g_modbus_cfg.listNum; i++)
    {
        if(gModbusHandle[g_modbus_cfg.listPtr[i].port] == NULL)
        {
            uint16_t *ptaskport = pvPortMalloc(sizeof(uint16_t));
            *ptaskport = g_modbus_cfg.listPtr[i].port;

            char taskName[configMAX_TASK_NAME_LEN] = {0};
            sprintf(taskName, "MODBUS_COM%u_TASK", g_modbus_cfg.listPtr[i].port);
            BaseType_t ret = xTaskCreate((TaskFunction_t)kalyke_modbus_com_task,
                                         (const char *)taskName,
                                         128,
                                         (void *)ptaskport,
                                         3,
                                         &gModbusHandle[g_modbus_cfg.listPtr[i].port]);
            LOGD(TAG, "Create MODBUS_COM%u_TASK!", g_modbus_cfg.listPtr[i].port);
            if (ret != pdPASS)
            {
                LOGE(TAG, "Create MODBUS_COM%u_TASK ERROR!", g_modbus_cfg.listPtr[i].port);
            }

            vTaskDelay(200);
        }
    }

    LOGD(TAG, "Leave %s()", __func__);
}

void osThreadNew_PLCTask(void)
{
    PLCTaskHandle = osThreadNew(plc_task, NULL, &PLC_attributes);
}

/**
  * @brief  plc_task
  * @param  None
  * @retval None
  */
void plc_task(void *p_arg)
{
    LOGD("plc_task", "plc_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
//    /*系统资源初始化*/
//    plc_element_init();
//    plc_sys_init();
//    plc_user_interrupt_init();
//    plc_system_block_verify(1);
//    guv_StopError.bit.sysblock_err = 0;	
//    char ret = plc_parse_system_block(1);
//    if (ret != pdPASS)
//    {
//        LOGE("plc_task", "plc_parse_system_block FAIL!");
//        guv_StopError.bit.sysblock_err = 1;
//        plc_refresh_error_msg(ERR_SYSTEM_BLOCK);
//    }

//    plc_netcfg_block_verify(1);
   
    uint8_t collectcomplete=0;
    
    gtv_UserFilePtrSt.NetcfgBlockPtr = (unsigned char *)PLC_INFO_SAVE_ADDR;

    char ret = plc_parse_netcfg_block_NoUart();
    if (ret != pdPASS)
    {
        LOGE("plc_task", "plc_parse_netcfg_block_NoUart FAIL!");
//        guv_NonStopError.bit.netconfig_err = 1;
//        plc_refresh_error_msg(ERR_NET_CONFIG);
    }
    LOGE("plc_task", "Wait event: KALYKE_EVENT_ENET_INIT_DONE_PLC");
    
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);

//    LOGD("plc_task", "g_plc_netcfg.lan.ioExp = %X", g_plc_netcfg.lan.ioExp);
//#if (DAISY_MASTER_FEATURE == 1)
//    start_daisy_task();
//#endif
//#if (KALYKE_MODBUS_TCP_SHEET == 1) && (KALYKE_FEATURE_INTERNET_TASK != 0)
//    if (g_plc_netcfg.wan.ioExp != WAN_CONFIG_IO_EXP_FEXLINK) //智能设备网口不执行其他程序
//    {
//        start_modbus_tcp();
//    }
//#endif

//    /*看门狗使能*/
//    bsp_watch_dog_enable();
//    pFunc = bsp_feed_watch_dog;
//    LOGV("plc_task", "pFunc(bsp_feed_watch_dog) = 0x%08X\r\n", pFunc);
    while(1) {
//        /*PLC运行态切换*/
//        plc_sys_status_switch();
//        /*喂狗*/
//        bsp_feed_watch_dog();
//        
//        switch(gtv_PlcRunStatus.mcv_PlcCurrentStatus) {
//            /*停止状态*/
//            case PLC_STOP_STATUS:
//                plc_status_stop();
//                break;

//            /*停止到运行*/
//            case PLC_STOP_TO_RUN_STATUS:
//                plc_status_stop_to_run();
        
        if(g_modbus_cfg.listNum != 0 && collectcomplete == 0)
        {
            //解析通讯配置串口配置
            char uartret = plc_parse_netcfg_block_OnlyUart();
            if (uartret == pdPASS)
            {
                LOGV(TAG, "plc_parse_netcfg_block_OnlyUart SUCCESS!");
            }
            
            start_modbus_com();
            
            collectcomplete = 1;
        }
        
//        if(gWANor4G == 1 && g_modbus_cfg.listNum != 0 && collectcomplete == 0)
//        {
//            //解析通讯配置串口配置
//            char uartret = plc_parse_netcfg_block_OnlyUart();
//            if (uartret == pdPASS)
//            {
//                LOGV(TAG, "plc_parse_netcfg_block_OnlyUart SUCCESS!");
//            }
//            
//            start_modbus_com();
//            collectcomplete = 1;
//        }
//        else if(gWANor4G == 2 && g_modbus_cfg.listNum != 0 && collectcomplete == 0)
//        {
//            if(complete4G == 1)
//            {
//                //解析通讯配置串口配置
//                char uartret = plc_parse_netcfg_block_OnlyUart();
//                if (uartret == pdPASS)
//                {
//                    LOGV(TAG, "plc_parse_netcfg_block_OnlyUart SUCCESS!");
//                }
//                
//                start_modbus_com();
//                collectcomplete = 1;
//            }
//        }
        
//                gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_RUN_STATUS;
//                break;

//            /*运行状态*/
//            case PLC_RUN_STATUS:
//                //taskENTER_CRITICAL();
//                plc_run_deamon();
//                //taskEXIT_CRITICAL();
//                break;

//            /*运行到停止*/
//            case PLC_RUN_TO_STOP_STATUS:
//                plc_status_run_to_stop();
//                break;
//        }        
//        /*喂狗*/
//        bsp_feed_watch_dog();
//#if 0
//        if (gtv_PlcRunStatus.mcv_PlcCurrentStatus == PLC_STOP_STATUS)
//        {
            vTaskDelay(100);
//        }
//        else
//        {
//            taskYIELD();
//        }
//#elif 1
//        vTaskDelay(1);
//#else
//        taskYIELD();
//#endif

//    CONSTANT_SCAN_TAG:
//        /*刷新强制元件*/
//        plc_refresh_force_element_value();

//        /*刷新系统I/O*/
//        plc_refresh_io_port();

//        /*刷新端口指示灯*/
//        //bsp_refresh_io_port_led(gtv_PlcElement.msp_XElement[0], gtv_PlcElement.msp_YElement[0]);

//        /*内务处理*/

//        /* 刷新恒定扫描速率， 非恒定扫描时直接对plc_run_user_program函数计时 */
//    #if (PLC_SCAN_TIME_LOGIC_2 == 1)
//        if (plc_get_bit_element_value(SM_ELEMENT, SM8))
//        {
//            if(plc_refresh_sys_scan_time()) {
//                /*恒定扫描*/
//                goto CONSTANT_SCAN_TAG;
//            }
//        }
//    #else
//        if(plc_refresh_sys_scan_time())
//        {
//            /*恒定扫描*/
//            goto CONSTANT_SCAN_TAG;
//        }
//    #endif
    }
}

