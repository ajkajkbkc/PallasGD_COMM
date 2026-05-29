
/* Private includes ----------------------------------------------------------*/
#include "mb.h"
#include "mb_maptable.h"
#include "mb_crc.h"
#include "app_parameter.h"
#include "app_log.h"
#include "app_tool.h"
#include "app_rtc.h"
#include "app_uart.h"
#include "app_etcr2900.h"
#include "app_att7022eu.h"
#include "app_rn8209d.h"

#include "bsp_dct.h"
//#include "plc_element.h"
#include "plc_netcfg.h"
#include "app_version.h"
//#include "plc_variable.h"

#include "app_oled.h"
#include "app_att7022eu.h"
#include "internet.h"
#include "plc_task.h"
#include "kalyke_monitor_task.h"
#include "app_ntc.h"
#include "kalyke_4G_task.h"
#include "app_led.h"
#include "module_ESE.h"
/* Private define ------------------------------------------------------------*/


mb_slave_diagnositic_info_st gtp_ModbusSlaveDiagInfo[MB_SENDER_MAX];
mb_file_trans_st *gtv_ModbusFileTrans[MB_DL_MAX] = { NULL, };

/* Private variables ---------------------------------------------------------*/
#if PRINT_LOG_OPEN == 1
static const char *TAG = "mb_ctl";
#endif


/* Private function prototypes -----------------------------------------------*/



/* Private user code ---------------------------------------------------------*/
/**
  * @brief  字 -> 字节指针
  * @param  word
  * @param  pbyte
  * @retval None
  */
void word_to_bytePoint(uint16_t word, uint8_t *pbyte)
{
    if(gFlashParam.st.mb_word_bytetype == WORD_ByteL_ByteH)
    {
        pbyte[0] = (unsigned char)(word);
        pbyte[1] = (unsigned char)(word >> 8);
    }
    else
    {
        pbyte[0] = (unsigned char)(word >> 8);
        pbyte[1] = (unsigned char)(word);
    }
}

/**
  * @brief  双字 -> 字节指针
  * @param  dword
  * @param  pbyte
  * @retval None
  */
void dword_to_bytePoint(uint32_t dword, uint8_t *pbyte)
{
    uint16_t wordH, wordL;

    if(gFlashParam.st.mb_dword_wordtype == DWORD_WordH_WordL)
    {
        wordH = (unsigned short)(dword >> 16);
        wordL = (unsigned short)(dword);
    }
    else
    {
        wordH = (unsigned short)(dword);
        wordL = (unsigned short)(dword >> 16);
    }

    word_to_bytePoint(wordH, &pbyte[0]);
    word_to_bytePoint(wordL, &pbyte[2]);
}

/**
  * @brief  字节指针 -> 字
  * @param  pbyte
  * @retval 值
  */
uint16_t bytePoint_to_word(uint8_t *pbyte)
{
    uint16_t word;

    if(gFlashParam.st.mb_word_bytetype == WORD_ByteL_ByteH)
    {
        word = pbyte[0] | (pbyte[1] << 8);
    }
    else
    {
        word = (pbyte[0] << 8) | pbyte[1];
    }

    return word;
}

/**
  * @brief  字节指针 -> 双字
  * @param  pbyte
  * @retval 值
  */
uint32_t bytePoint_to_dword(uint8_t *pbyte)
{
    uint32_t dword;
    uint16_t wordH, wordL;

    if(gFlashParam.st.mb_dword_wordtype == DWORD_WordH_WordL)
    {
        wordH = bytePoint_to_word(&pbyte[0]);
        wordL = bytePoint_to_word(&pbyte[2]);
    }
    else
    {
        wordH = bytePoint_to_word(&pbyte[2]);
        wordL = bytePoint_to_word(&pbyte[0]);
    }
    dword = (wordH << 16) | wordL;

    return dword;
}

/**
  * @brief  modbus返回信息
  * @param  *pMsg 数据
  * @retval None
  */
void mb_slave_verify_resp_msg(md_slave_msg_pack *pMsg)
{
    //sender is UART
    if((pMsg->mcv_Sender == MB_SENDER_UART1) || (pMsg->mcv_Sender == MB_SENDER_UART2) || (pMsg->mcv_Sender == MB_SENDER_UART3) || (pMsg->mcv_Sender == MB_SENDER_UART4) )
    {
        uint16_t lsv_Crc = calc_crc16(pMsg->mcp_RespBuff, pMsg->msv_RespLen);
        pMsg->mcp_RespBuff[pMsg->msv_RespLen] = (unsigned char)(lsv_Crc & 0xFF);
        pMsg->mcp_RespBuff[pMsg->msv_RespLen + 1] = (unsigned char)(lsv_Crc >> 0x08);
        pMsg->msv_RespLen += 2;
        //hexdump(pMsg->mcp_RespBuff, pMsg->msv_RespLen); //after add crc, print log
        if(pMsg->uart_resp_func != NULL)
        {
            pMsg->uart_resp_func(pMsg->mcp_RespBuff, pMsg->msv_RespLen);
        }
    }
    //sender is MQTT
    else if (pMsg->mcv_Sender == MB_SENDER_MQTT)
    {
        if (pMsg->mqtt_resp_func != NULL)
        {
            pMsg->mqtt_resp_func(pMsg->mcp_RespBuff);
        }
    }
    //sender is TCP
    else if (pMsg->mcv_Sender == MB_SENDER_TCP)
    {
        if (pMsg->tcp_resp_func != NULL)
        {
            pMsg->tcp_resp_func(pMsg->mcp_RespBuff, pMsg->msv_RespLen, pMsg->clientID);
        }
    }
    //no sender
    else if (pMsg->mcv_Sender == ADD_CRC_NO_SENDER)
    {
        uint16_t lsv_Crc = calc_crc16(pMsg->mcp_RespBuff, pMsg->msv_RespLen);
        pMsg->mcp_RespBuff[pMsg->msv_RespLen] = (unsigned char)(lsv_Crc & 0xFF);
        pMsg->mcp_RespBuff[pMsg->msv_RespLen + 1] = (unsigned char)(lsv_Crc >> 0x08);
        pMsg->msv_RespLen += 2;
    }
}

/**
  * @brief  modbus返回错误信息
  * @param  *pMsg 数据
  * @retval None
  */
void mb_slave_error_resp(md_slave_msg_pack *pMsg)
{
    unsigned short lsv_Crc;

    if(pMsg->mcv_IsBroadcastInfo)
    {
        LOGE("mb", "Broadcast , just return");
        return;
    }

    pMsg->mcp_RespBuff[0] = pMsg->mcp_ReceiveBuff[0];
    pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1] | 0x80;
    pMsg->mcp_RespBuff[2] = pMsg->mcv_ErrorCode;
    pMsg->msv_RespLen = 3;

    if ((pMsg->mcv_Sender == MB_SENDER_UART1) || (pMsg->mcv_Sender == MB_SENDER_UART2) ||
            (pMsg->mcv_Sender == MB_SENDER_UART3) || (pMsg->mcv_Sender == MB_SENDER_UART4) )
    {
        lsv_Crc = calc_crc16(pMsg->mcp_RespBuff, 3);

        /*CRC是小端序,低字节在前*/
        pMsg->mcp_RespBuff[3] = (unsigned char)lsv_Crc;
        pMsg->mcp_RespBuff[4] = (unsigned char)(lsv_Crc >> 8);
        pMsg->msv_RespLen = 5;

        if(pMsg->uart_resp_func != NULL)
        {
            //pMsg->uart_resp_func(pMsg->mcp_RespBuff, pMsg->msv_RespLen); //错误不需要返回
        }
    }
    else if (pMsg->mcv_Sender == MB_SENDER_MQTT)
    {
        if (pMsg->mqtt_resp_func != NULL)
        {
            //pMsg->mqtt_resp_func(pMsg->mcp_RespBuff); //错误不需要返回
        }
    }
    else if (pMsg->mcv_Sender == MB_SENDER_TCP)
    {
        if (pMsg->tcp_resp_func != NULL)
        {
            //pMsg->tcp_resp_func(pMsg->mcp_RespBuff, pMsg->msv_RespLen, pMsg->clientID); //错误不需要返回
        }
    }
}

/**
  * @brief  读寄存器
  * @param  None
  * @retval None
  */
void mb_slave_read_holding_register(md_slave_msg_pack *pMsg)
{
    bool lcv_Ret;
    uint8_t r_reg;
    unsigned short lsv_ModbusAddr, lsv_ElementAddr, lsv_ElementCnt, lsv_ElementValue;
    unsigned char lcv_ElementType, lcv_Broadcast;
    unsigned short i;
    unsigned short lsv_DataLen = 0;
    uint16_t lsv_temp;
    uint16_t lsv_buf[200] = {0};
    rtc_datetime_t datetime;
    uint32_t llv_C32Value;

    if(pMsg->msv_ReceiveLen != 8)
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        return;
    }

    lsv_ModbusAddr = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[2]);
    lsv_ElementCnt = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);

    if(lsv_ElementCnt > MB_MAX_R_WORD_NUM) //超过读字元件最大数量
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
        mb_slave_error_resp(pMsg);
        return;
    }

    lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_ModbusAddr, lsv_ElementCnt, &lcv_ElementType, &lsv_ElementAddr, &lcv_Broadcast);
    if(lcv_Ret != true)
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
        mb_slave_error_resp(pMsg);
        return;
    }

    if(pMsg->mcv_Sender != MB_SENDER_TCP) //mdtcp不需要判断站号
    {
        /*支持广播消息*/
        if( (pMsg->mcv_IsBroadcastInfo) && (lcv_Broadcast == MB_NOBROADCAST) )
        {
            LOGE("mb3", "In Broadcast, modbus address 0x%04x illegal!", lsv_ModbusAddr);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
            mb_slave_error_resp(pMsg);
            return ;
        }
    }

    switch(lcv_ElementType)
    {
    /* 自定义类型 compatible old protocol ------------------------------------- */
    case MB_WORD_IDINFO:
        lsv_DataLen = lsv_ElementCnt * 2;
        memcpy(&pMsg->mcp_RespBuff[3], gFlashParam.st.idInfo, lsv_DataLen);
        break;

    case MB_WORD_MQTTUSERNAME:
        lsv_DataLen = lsv_ElementCnt * 2;
        memcpy(&pMsg->mcp_RespBuff[3], gFlashParam.st.mqttUserName, lsv_DataLen);
        break;

    case MB_WORD_MQTTPASSWORD:
        lsv_DataLen = lsv_ElementCnt * 2;
        memcpy(&pMsg->mcp_RespBuff[3], gFlashParam.st.mqttPassword, lsv_DataLen);
        break;

    case MB_WORD_MQTTPUB:
        lsv_DataLen = lsv_ElementCnt * 2;
        memcpy(&pMsg->mcp_RespBuff[3], gFlashParam.st.mqttPub, lsv_DataLen);
        break;

    case MB_WORD_MQTTSUB:
        lsv_DataLen = lsv_ElementCnt * 2;
        memcpy(&pMsg->mcp_RespBuff[3], gFlashParam.st.mqttSub, lsv_DataLen);
        break;

    case MB_WORD_MQTTALARMPUB:
        lsv_DataLen = lsv_ElementCnt * 2;
        memcpy(&pMsg->mcp_RespBuff[3], gFlashParam.st.mqttAlarmPub, lsv_DataLen);
        break;

    case MB_WORD_GATEWAYIP:
        lsv_DataLen = lsv_ElementCnt * 2;
        for (i = 0; i < 4; i++)
        {
            pMsg->mcp_RespBuff[3 + i] = gFlashParam.st.gatewayIP[3 - i];
        }
        break;

    case MB_WORD_LOCALIP:
        lsv_DataLen = lsv_ElementCnt * 2;
        for (i = 0; i < 4; i++)
        {
            pMsg->mcp_RespBuff[3 + i] = gFlashParam.st.localIP[3 - i];
        }
        break;

    case MB_WORD_S0TARGETIP:
        lsv_DataLen = lsv_ElementCnt * 2;
        for (i = 0; i < 4; i++)
        {
            pMsg->mcp_RespBuff[3 + i] = gFlashParam.st.s0TargetIP[3 - i];
        }
        break;

    case MB_WORD_S0LOCALPORT:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (gFlashParam.st.s0LocalPort & 0xFF00) >> 8;
        pMsg->mcp_RespBuff[4] = (gFlashParam.st.s0LocalPort & 0x00FF);
        break;

    case MB_WORD_S0TARGETPORT:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (gFlashParam.st.s0TargetPort & 0xFF00) >> 8;
        pMsg->mcp_RespBuff[4] = (gFlashParam.st.s0TargetPort & 0x00FF);
        break;

    case MB_WORD_S1TARGETIP:
        lsv_DataLen = lsv_ElementCnt * 2;
        for (i = 0; i < 4; i++)
        {
            pMsg->mcp_RespBuff[3 + i] = gFlashParam.st.s1TargetIP[3 - i];
        }
        break;

    case MB_WORD_S1LOCALPORT:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (gFlashParam.st.s1LocalPort & 0xFF00) >> 8;
        pMsg->mcp_RespBuff[4] = (gFlashParam.st.s1LocalPort & 0x00FF);
        break;

    case MB_WORD_S1TARGETPORT:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (gFlashParam.st.s1TargetPort & 0xFF00) >> 8;
        pMsg->mcp_RespBuff[4] = (gFlashParam.st.s1TargetPort & 0x00FF);
        break;

    case MB_WORD_NODECHECKINTER:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (gFlashParam.st.getRtuCycleTime & 0xFF00) >> 8;
        pMsg->mcp_RespBuff[4] = (gFlashParam.st.getRtuCycleTime & 0x00FF);
        break;

    case MB_WORD_MQTTPUBINTER:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (gFlashParam.st.mqttPublishInterval & 0xFF00) >> 8;
        pMsg->mcp_RespBuff[4] = (gFlashParam.st.mqttPublishInterval & 0x00FF);
        break;

    case MB_WORD_MACADDR:
        lsv_DataLen = lsv_ElementCnt * 2;
        memcpy(&pMsg->mcp_RespBuff[3], gFlashParam.st.macAddr, lsv_DataLen);
        break;

    case MB_WORD_MASKIP:
        lsv_DataLen = lsv_ElementCnt * 2;
        for (i = 0; i < 4; i++)
        {
            pMsg->mcp_RespBuff[3 + i] = gFlashParam.st.maskIP[3 - i];
        }
        break;

    case MB_WORD_RTCTIME:
        lsv_DataLen = lsv_ElementCnt * 2;
        RTC_GetDatetime(&datetime);
        pMsg->mcp_RespBuff[3] = datetime.year % 100;
        pMsg->mcp_RespBuff[4] = datetime.month;
        pMsg->mcp_RespBuff[5] = datetime.day;
        pMsg->mcp_RespBuff[6] = datetime.hour;
        pMsg->mcp_RespBuff[7] = datetime.minute;
        pMsg->mcp_RespBuff[8] = datetime.second;
        break;

    case MB_WORD_DESXLJCLEAR:
        /* only write */
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = 0 >> 8;
        pMsg->mcp_RespBuff[4] = 0;
        break;

    case MB_WORD_IDNUMBER:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = gFlashParam.st.idNum >> 8;
        pMsg->mcp_RespBuff[4] = gFlashParam.st.idNum;
        break;

    case MB_WORD_L1AICLEAR:
        lsv_DataLen = lsv_ElementCnt * 2;
//        pMsg->mcp_RespBuff[3] = (uint16_t)gLtList.st.length >> 8;
//        pMsg->mcp_RespBuff[4] = (uint8_t)gLtList.st.length;
        break;

    case MB_WORD_G1A3INTERVAL:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (uint16_t)gFlashParam.st.GResGetInter >> 8;
        pMsg->mcp_RespBuff[4] = (uint8_t)gFlashParam.st.GResGetInter;
        break;

    case MB_WORD_G1A3GETNUM:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (uint16_t)gFlashParam.st.GResGetTimes >> 8;
        pMsg->mcp_RespBuff[4] = (uint8_t)gFlashParam.st.GResGetTimes;
        break;

    case MB_WORD_VALUE:
#if PROD_TYPE == PROD_FL
        lsv_temp = gLtList.st.length < 1 ? 1 : gLtList.st.length;
        lsv_temp = (lsv_temp - 1) * (sizeof(LtElem_st) / 2) + 2; //显示最后一次雷击信息 +2指gLtList.st.length所占空间2个16bit寄存器
        lsv_buf[0] = (uint16_t)gLtList.st.length;
        memcpy(&lsv_buf[1], &gLtList.lsv_buff[lsv_temp], sizeof(LtElem_st)); //sizeof(LtElem_st) = 20

        for(i = 0 ; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
    
#elif (PROD_TYPE == PROD_FD) || (PROD_TYPE == PROD_FS)
        lsv_temp = 0; /* do nothing */
        memset(lsv_buf, 0, sizeof(lsv_buf));
        lsv_buf[0] = lsv_temp;
        memcpy(lsv_buf, &gFS_Elem.st, sizeof(gFS_Elem));
        for(i = 0 ; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
#elif PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA

        memset(lsv_buf, 0, sizeof(lsv_buf));
        gESE_Elem.st.DO[0] = gParam.st.AlmOutput0;
        gESE_Elem.st.DO[1] = gParam.st.AlmOutput1;
        memcpy(lsv_buf, &gESE_Elem, sizeof(gESE_Elem));  //前面移gESE_Elem数据
        lsv_temp = sizeof(gESE_Elem) / 2;
        memcpy(lsv_buf + lsv_temp, &gMeterParam, sizeof(gMeterParam));  //后面移gMeterParam数据
        lsv_temp += sizeof(gMeterParam) / 2;
        memcpy(lsv_buf + lsv_temp, &gMeterEnergy, sizeof(gMeterEnergy));  //后面移gMeterEnergy数据
        lsv_temp = sizeof(gESE_Elem) / 2;

        for(i = 0; i < lsv_ElementCnt; i++)
        {
            if(lsv_ElementAddr < lsv_temp) //前面的数据都是u16型
            {
                lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            }
            else //后面的数据都是u32大端存储
            {
                if(gFlashParam.st.mb_dword_wordtype == DWORD_WordH_WordL)
                {
                    lsv_ElementValue = (lsv_ElementAddr + i) % 2 ? lsv_buf[lsv_ElementAddr + i - 1] : lsv_buf[lsv_ElementAddr + i + 1]; //因为float型存储方式不同（高位在前）
                }
                else
                {
                    lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];  //低位在前
                }
            }
            word_to_bytePoint(lsv_ElementValue, &pMsg->mcp_RespBuff[3 + i * 2]);
            lsv_DataLen += 2;
        }
    
#elif PROD_TYPE == PROD_FR
        lsv_temp = 0; /* do nothing */
        memset(lsv_buf, 0, sizeof(lsv_buf));
        lsv_buf[0] = lsv_temp;
        lsv_buf[0] = gFlashParam.st.GResGetTimes;
        lsv_buf[1] = gFlashParam.st.GResGetInter;
        lsv_buf[2] = gFlashParam.st.GResVal;
        lsv_buf[3] = 0xFFFF;
        lsv_buf[4] = 0xFFFF;

        for(i = 0 ; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
#elif PROD_TYPE == PROD_FA
        lsv_temp = 0; /* do nothing */
        memset(lsv_buf, 0, sizeof(lsv_buf));
        lsv_buf[0] = lsv_temp;
        lsv_buf[1] = lsv_temp;
        lsv_buf[2] = gtv_AfdDevice.mcv_AlmStatus;
        lsv_buf[3] = gtv_AfdDevice.msv_ArcTimes;

        for(i = 0 ; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
#elif PROD_TYPE == PROD_FSS
        lsv_temp = 0; /* do nothing */
        memset(lsv_buf, 0, sizeof(lsv_buf));
        lsv_buf[0] = lsv_temp;
        memcpy(lsv_buf, &gFSS_Elem, sizeof(gFSS_Elem));
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
#else /* do nothing, cancer program warning */
        lsv_temp = 0;
        memset(lsv_buf, 0, sizeof(lsv_buf));
        lsv_buf[0] = lsv_temp;
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
#endif
        break;

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB
    case MB_WORD_POWER_QUALITY:
        memset(lsv_buf, 0, sizeof(lsv_buf));
        memcpy(lsv_buf, &gPowerQualityParam, sizeof(gPowerQualityParam));
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            if(gFlashParam.st.mb_dword_wordtype == DWORD_WordH_WordL)
            {
                lsv_ElementValue = (lsv_ElementAddr + i) % 2 ? lsv_buf[lsv_ElementAddr + i - 1] : lsv_buf[lsv_ElementAddr + i + 1]; //因为float型存储方式不同（高位在前）
            }
            else
            {
                lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];  //低位在前
            }
            word_to_bytePoint(lsv_ElementValue, &pMsg->mcp_RespBuff[3 + i * 2]);
            lsv_DataLen += 2;
        }
        break;

#if PROD_TYPE == PROD_SFE
    case MB_WORD_HARMONIC:
        memset(lsv_buf, 0, sizeof(lsv_buf));
        memcpy(lsv_buf, &gHarmonicParam, sizeof(gHarmonicParam));
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            word_to_bytePoint(lsv_ElementValue, &pMsg->mcp_RespBuff[3 + i * 2]);
            lsv_DataLen += 2;
        }
        break;
#endif

#endif

    case MB_WORD_SETDES4LIM:
        memset(lsv_buf, 0, sizeof(lsv_buf));
#if PROD_TYPE == PROD_FSS
        lsv_buf[0] = gFlashParam.st.SPD_L1_Up;
        lsv_buf[1] = gFlashParam.st.SPD_L2_Up;
        lsv_buf[2] = gFlashParam.st.SPD_L3_Up;
#else
        lsv_buf[0] = gFlashParam.st.SPD_YL_Up;
        lsv_buf[1] = gFlashParam.st.SPD_KL_Up;
        lsv_buf[2] = gFlashParam.st.SPD_PE_Up;
#endif
        for(i = 0 ; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
        break;

    case MB_WORD_G1A3OFFSET:
        memset(lsv_buf, 0, sizeof(lsv_buf));
        lsv_buf[0] = gFlashParam.st.GRes_K;
        lsv_buf[1] = gFlashParam.st.GRes_B;
        for(i = 0 ; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = lsv_buf[lsv_ElementAddr + i];
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
        break;

    case MB_WORD_RSTZIGBEE:
        /* only write */
        break;

    /* new -------------------------------------------------------------------- */
    case MB_WORD_SD:
        if( (lsv_ElementCnt % 2 != 0) || (lsv_ModbusAddr % 2 == 0) )//当读取长度为奇数或读取地址为偶数时，为非法数据
        {
            LOGE("mb", "read double word register addr 0x%04x, len %d illegal", lsv_ModbusAddr, lsv_ElementCnt);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
            mb_slave_error_resp(pMsg);
            return ;
        }
        
        for(i = 0; i < lsv_ElementCnt / 2; i++)
        {
            llv_C32Value = GET_FLASH_WORD_PARAM_VALUE(lsv_ElementAddr + i * 2);
            pMsg->mcp_RespBuff[3 + i * 4] = (unsigned char)(llv_C32Value >> 24);
            pMsg->mcp_RespBuff[4 + i * 4] = (unsigned char)(llv_C32Value >> 16);
            pMsg->mcp_RespBuff[5 + i * 4] = (unsigned char)(llv_C32Value >> 8);
            pMsg->mcp_RespBuff[6 + i * 4] = (unsigned char)(llv_C32Value);
            lsv_DataLen += 4;
        }
        break;
        
//        for(i = 0; i < lsv_ElementCnt / 2; i++)
//        {
//            llv_C32Value = GET_FLASH_WORD_PARAM_VALUE(lsv_ElementAddr + i * 2);
//            dword_to_bytePoint(llv_C32Value, &pMsg->mcp_RespBuff[3 + i * 4]);
//            lsv_DataLen += 4;
//        }
//        break;

    case MB_HALF_WORD_SD:
        for(i = 0; i < lsv_ElementCnt; i++)
        {
//            lsv_ElementValue = GET_FLASH_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i);
//            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
//            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
//            lsv_DataLen += 2;
            
            lsv_ElementValue = GET_FLASH_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i);
            word_to_bytePoint(lsv_ElementValue, &pMsg->mcp_RespBuff[3 + i * 2]);
            lsv_DataLen += 2;
        }
        break;

    case MB_BYTE_SD:
        if(lsv_ModbusAddr == 0x07D1) //0x07D1是海雷克终端的寄存器地址，不能返回信息
        {
            return ;
        }
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = GET_FLASH_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i);
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            lsv_DataLen += 2;
        }
        break;

    case MB_WORD_PARAM:
        if( (lsv_ElementCnt % 2 != 0) || (lsv_ModbusAddr % 2 == 0) )//当读取长度为奇数或读取地址为偶数时，为非法数据
        {
            LOGE("mb", "read double word register addr 0x%04x, len %d illegal", lsv_ModbusAddr, lsv_ElementCnt);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
            mb_slave_error_resp(pMsg);
            return ;
        }
        for(i = 0; i < lsv_ElementCnt / 2; i++)
        {
            llv_C32Value = GET_WORD_PARAM_VALUE(lsv_ElementAddr + i * 2);
            pMsg->mcp_RespBuff[3 + i * 4] = (unsigned char)(llv_C32Value >> 24);
            pMsg->mcp_RespBuff[4 + i * 4] = (unsigned char)(llv_C32Value >> 16);
            pMsg->mcp_RespBuff[5 + i * 4] = (unsigned char)(llv_C32Value >> 8);
            pMsg->mcp_RespBuff[6 + i * 4] = (unsigned char)(llv_C32Value);
            lsv_DataLen += 4;
        }
        break;

    case MB_HALF_WORD_PARAM:
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = GET_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i);
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
        break;

    case MB_BYTE_PARAM:
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = GET_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i);
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            lsv_DataLen += 2;
        }
        break;

    case MB_HALF_WORD_LTLIST:
        //LOGW("mb", "enter MB_HALF_WORD_LTLIST......");
        for(i = 0; i < lsv_ElementCnt; i++)
        {
//            lsv_ElementValue = GET_HALFWORD_LTLIST_VALUE(lsv_ElementAddr + i);
            pMsg->mcp_RespBuff[3 + i * 2] = (unsigned char)(lsv_ElementValue >> 8);
            pMsg->mcp_RespBuff[4 + i * 2] = (unsigned char)(lsv_ElementValue);
            lsv_DataLen += 2;
        }
        break;

#if  PROD_TYPE == PROD_FSS
    case MB_WORD_METER:
        //LOGW("mb", "enter MB_WORD_METER......");
        if( lsv_ElementCnt % 2 != 0)//当读取长度为奇数时，为非法数据
        {
            LOGE("mb", "read double word register addr 0x%04x, len %d illegal", lsv_ModbusAddr, lsv_ElementCnt);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
            mb_slave_error_resp(pMsg);
            return ;
        }
        r_reg = (uint8_t)lsv_ModbusAddr;
        for(i = 0; i < lsv_ElementCnt / 2; i++)
        {
            if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEA4)  //三相全电流
            {
                llv_C32Value = ATT7022_Get_MeterPara(r_reg);
            }
            else
            {
                llv_C32Value = RN8209D_Get_MeterPara(r_reg);
            }

            pMsg->mcp_RespBuff[3 + i * 4] = (unsigned char)(llv_C32Value >> 8);
            pMsg->mcp_RespBuff[4 + i * 4] = (unsigned char)(llv_C32Value);
            pMsg->mcp_RespBuff[5 + i * 4] = (unsigned char)(llv_C32Value >> 24);
            pMsg->mcp_RespBuff[6 + i * 4] = (unsigned char)(llv_C32Value >> 16);
            r_reg += 1;
            lsv_DataLen += 4;
        }
        break;

    case MB_WORD_CALICMD:
        lsv_DataLen = lsv_ElementCnt * 2;
        if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEA4)  //三相全电流
        {
            pMsg->mcp_RespBuff[3] = (gMeterRunInfo.EmuWork & 0xFF00) >> 8;
            pMsg->mcp_RespBuff[4] = (gMeterRunInfo.EmuWork & 0x00FF);
        }
        else
        {
            pMsg->mcp_RespBuff[3] = (rn_gMeterRunInfo.EmuWork & 0xFF00) >> 8;
            pMsg->mcp_RespBuff[4] = (rn_gMeterRunInfo.EmuWork & 0x00FF);
        }
        break;
#elif  PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
    case MB_WORD_METER:
        //LOGW("mb", "enter MB_WORD_METER......");
        if( lsv_ElementCnt % 2 != 0)//当读取长度为奇数时，为非法数据
        {
            LOGE("mb", "read double word register addr 0x%04x, len %d illegal", lsv_ModbusAddr, lsv_ElementCnt);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
            mb_slave_error_resp(pMsg);
            return ;
        }
        r_reg = (uint8_t)lsv_ModbusAddr;
        for(i = 0; i < lsv_ElementCnt / 2; i++)
        {
            llv_C32Value = ATT7022_Get_MeterPara(r_reg);
            dword_to_bytePoint(llv_C32Value, &pMsg->mcp_RespBuff[3 + i * 4]);
            r_reg += 1;
            lsv_DataLen += 4;
        }
        break;

    case MB_WORD_CALICMD:
        lsv_DataLen = lsv_ElementCnt * 2;
        pMsg->mcp_RespBuff[3] = (gMeterRunInfo.EmuWork & 0xFF00) >> 8;
        pMsg->mcp_RespBuff[4] = (gMeterRunInfo.EmuWork & 0x00FF);
        break;
#endif

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB
    case MB_WORD_SYNC_SAMPLED:
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            lsv_ElementValue = *(uint16_t *)&Sync_SampleD.mbD[lsv_ElementAddr + i];
            word_to_bytePoint(lsv_ElementValue, &pMsg->mcp_RespBuff[3 + i * 2]);
            lsv_DataLen += 2;
        }
        break;
#endif    
    
    default:
        LOGE("mb", "In %s(), modbus address 0x%04x ,function code 0x03 illegal!", __func__, lsv_ModbusAddr);
        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
        mb_slave_error_resp(pMsg);
        return ;
    }

    /*组响应帧*/
    //PCsend:00 03 00 16 00 02 24 1E resp:00 03 04 F2 45 52 34 F5 29
    pMsg->mcp_RespBuff[0] = pMsg->mcp_ReceiveBuff[0];
    pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
    pMsg->mcp_RespBuff[2] = lsv_DataLen;
    pMsg->msv_RespLen = lsv_DataLen + 3;

    mb_slave_verify_resp_msg(pMsg);
}

///**
//  * @brief  读寄存器，支持D SD Z T C R
//  * @param  None
//  * @retval None
//  */
//void mb_slave_read_holding_register(md_slave_msg_pack *pMsg)
//{
//    LOGE("mb", "Enter %s\r\n", __func__);
//    unsigned char lcv_Ret;
//    unsigned short lsv_ModbusAddr;
//    unsigned short lsv_ElementAddr;
//    unsigned char lcv_ElementType;
//    unsigned short lsv_ElementCnt;
//    unsigned char i;
//    unsigned short lsv_ElementValue;
//    unsigned long llv_C32Value;
//    unsigned short lsv_DataLen;

//    /*不支持广播消息*/
//    if(pMsg->mcv_IsBroadcastInfo) {
//        return;
//    }

////    if(pMsg->msv_ReceiveLen != 8) {
////        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
////        mb_slave_error_resp(pMsg);
////        LOGE("mb", "Enter1111111111111");
////        return;
////    }
//        
//    lsv_ModbusAddr = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[2]);
//    lsv_ElementCnt = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);
//    
//    if(lsv_ElementCnt > MB_MAX_R_WORD_NUM) {
//        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//        mb_slave_error_resp(pMsg);
//        LOGE("mb", "Enter2222222222222");
//        return;
//    }

//    lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_ModbusAddr, &lcv_ElementType, &lsv_ElementAddr);
//    LOGE("MB_WORD_ELEMENT", "MB_WORD_ELEMENT = %d", MB_WORD_ELEMENT);
//    LOGE("lsv_ModbusAddr", "lsv_ModbusAddr = %d", lsv_ModbusAddr);
//    LOGE("lcv_ElementType", "lcv_ElementType = %d", lcv_ElementType);

//    if(lcv_Ret != pdPASS) {
//        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//        mb_slave_error_resp(pMsg);
//        LOGE("mb", "Enter3333333333333");
//        return;
//    }

//    lsv_DataLen = 0;

//    LOGE("lcv_ElementType", "lcv_ElementType = %d", lcv_ElementType);
//    
//    switch(lcv_ElementType) {
//        case MB_WORD_D:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > D_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                lsv_ElementValue = GET_D_ELEMENT_VALUE(lsv_ElementAddr+i);
//                pMsg->mcp_RespBuff[3+i*2] = (unsigned char)(lsv_ElementValue >> 8);
//                pMsg->mcp_RespBuff[3+i*2+1] = (unsigned char)(lsv_ElementValue);
//                lsv_DataLen += 2;
//            }
//            break;

//        case MB_WORD_SD:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > SD_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                lsv_ElementValue = GET_SD_ELEMENT_VALUE(lsv_ElementAddr+i);
//                pMsg->mcp_RespBuff[3+i*2] = (unsigned char)(lsv_ElementValue >> 8);
//                pMsg->mcp_RespBuff[3+i*2+1] = (unsigned char)(lsv_ElementValue);
//                lsv_DataLen += 2;
//            }
//            break;

//        case MB_WORD_Z:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > Z_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                lsv_ElementValue = GET_Z_ELEMENT_VALUE(lsv_ElementAddr+i);
//                pMsg->mcp_RespBuff[3+i*2] = (unsigned char)(lsv_ElementValue >> 8);
//                pMsg->mcp_RespBuff[3+i*2+1] = (unsigned char)(lsv_ElementValue);
//                lsv_DataLen += 2;
//            }
//            break;

//        case MB_WORD_C:
//            if(lsv_ElementAddr < C16_RANG) {
//                /*16bit 计数器*/
//                if(lsv_ElementAddr + lsv_ElementCnt -1 > C16_RANG) {
//                    pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                    mb_slave_error_resp(pMsg);
//                    return;
//                }

//                for(i=0; i<lsv_ElementCnt; i++) {
//                    lsv_ElementValue = GET_C16_CURRENT_VALUE(lsv_ElementAddr+i);
//                    pMsg->mcp_RespBuff[3+i*2] = (unsigned char)(lsv_ElementValue >> 8);
//                    pMsg->mcp_RespBuff[3+i*2+1] = (unsigned char)(lsv_ElementValue);
//                    lsv_DataLen += 2;
//                }
//            } else {
//                /*32Bit 计数器*/
//                if(lsv_ElementAddr + lsv_ElementCnt -1 > gtp_PlcElementInfo->msv_CElement.msv_ElementCnt) {
//                    pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                    mb_slave_error_resp(pMsg);
//                    return;
//                }

//                for(i=0; i<lsv_ElementCnt; i++) {
//                    llv_C32Value = GET_C32_CURRENT_VALUE(lsv_ElementAddr+i);
//                    pMsg->mcp_RespBuff[3+i*4] = (unsigned char)(llv_C32Value >> 24);
//                    pMsg->mcp_RespBuff[3+i*4+1] = (unsigned char)(llv_C32Value >> 16);
//                    pMsg->mcp_RespBuff[3+i*4+2] = (unsigned char)(llv_C32Value >> 8);
//                    pMsg->mcp_RespBuff[3+i*4+3] = (unsigned char)(llv_C32Value);
//                    lsv_DataLen += 4;
//                }

//            }
//            break;

//        case MB_WORD_T:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > T_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                lsv_ElementValue = GET_T_CURRENT_VALUE(lsv_ElementAddr+i);
//                pMsg->mcp_RespBuff[3+i*2] = (unsigned char)(lsv_ElementValue >> 8);
//                pMsg->mcp_RespBuff[3+i*2+1] = (unsigned char)(lsv_ElementValue);
//                lsv_DataLen += 2;
//            }
//            break;

//        case MB_WORD_R:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > R_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                lsv_ElementValue = GET_R_ELEMENT_VALUE(lsv_ElementAddr+i);
//                pMsg->mcp_RespBuff[3+i*2] = (unsigned char)(lsv_ElementValue >> 8);
//                pMsg->mcp_RespBuff[3+i*2+1] = (unsigned char)(lsv_ElementValue);
//                lsv_DataLen += 2;
//            }
//            break;

//    }

//    /*组响应帧*/
//    for(i=0; i<2; i++)
//        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

//    pMsg->mcp_RespBuff[2] = lsv_DataLen;

//    pMsg->msv_RespLen = lsv_DataLen + 3;
//    mb_slave_verify_resp_msg(pMsg);

//}

///**
//  * @brief  写单寄存器 D SD Z T C R
//  * @param  None
//  * @retval None
//  */
//void mb_slave_write_register(md_slave_msg_pack *pMsg)
//{
//    unsigned char lcv_Ret;
//    unsigned short lsv_ModbusAddr, lsv_ElementAddr, lsv_ElementValue;
//    unsigned char lcv_ElementType;
//    unsigned char i;

//    lsv_ModbusAddr = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[2]);
//    lsv_ElementValue = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);

//    lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_ModbusAddr, &lcv_ElementType, &lsv_ElementAddr);
//    if(lcv_Ret != pdPASS) {
//        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//        mb_slave_error_resp(pMsg);
//        return;
//    }

//    switch(lcv_ElementType) {
//        case MB_WORD_D:
//            SET_D_ELEMENT_VALUE(lsv_ElementAddr, lsv_ElementValue);
//            break;
//        case MB_WORD_SD:
//            /*20170811: 需要增加写入权限校验...*/
//            SET_SD_ELEMENT_VALUE(lsv_ElementAddr, lsv_ElementValue);
//            break;
//        case MB_WORD_Z:
//            SET_Z_ELEMENT_VALUE(lsv_ElementAddr, lsv_ElementValue);
//            break;
//        case MB_WORD_T:
//            SET_T_CURRENT_VALUE(lsv_ElementAddr, lsv_ElementValue);
//            break;
//        case MB_WORD_R:
//            SET_R_ELEMENT_VALUE(lsv_ElementAddr, lsv_ElementValue);
//            break;
//        case MB_WORD_C:
//            if(lsv_ElementAddr < C16_RANG)
//                SET_C16_CURRENT_VALUE(lsv_ElementAddr, lsv_ElementValue);
//            else
//                SET_C32_CURRENT_VALUE(lsv_ElementAddr, lsv_ElementValue);
//            break;
//    }

//    /*组响应帧,返回帧为请求帧的复制*/
//    for(i=0; i<6; i++)
//        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

//    pMsg->msv_RespLen = 6;
//    mb_slave_verify_resp_msg(pMsg);
//}

/**
  * @brief  写单寄存器
  * @param  None
  * @retval None
  */
void mb_slave_write_register(md_slave_msg_pack *pMsg)
{
    unsigned char lcv_Ret;
    unsigned short lsv_ModbusAddr, lsv_ElementAddr, lsv_ElementValue;
    unsigned char lcv_ElementType, lcv_Broadcast;
    unsigned char i;
    //LtElem_st LtElemTemp;

    lsv_ModbusAddr = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[2]);
    lsv_ElementValue = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);

    lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_ModbusAddr, 1, &lcv_ElementType, &lsv_ElementAddr, &lcv_Broadcast);
    if(lcv_Ret != true)
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
        mb_slave_error_resp(pMsg);
        return;
    }

    if(pMsg->mcv_Sender != MB_SENDER_TCP) //mdtcp不需要判断站号
    {
        /*支持广播消息*/
        if( (pMsg->mcv_IsBroadcastInfo) && (lcv_Broadcast == MB_NOBROADCAST) )
        {
            LOGE("mb6", "In Broadcast, modbus address 0x%04x illegal!", lsv_ModbusAddr);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
            mb_slave_error_resp(pMsg);
            return ;
        }
    }

    switch(lcv_ElementType)
    {
    /* 自定义类型 compatible old protocol ------------------------------------- */
    case MB_WORD_MQTTUSERNAME:
        //can not use 0x06 function code
        break;

    case MB_WORD_MQTTPASSWORD:
        //can not use 0x06 function code
        break;

    case MB_WORD_MQTTPUB:
        //can not use 0x06 function code
        break;

    case MB_WORD_MQTTSUB:
        //can not use 0x06 function code
        break;

    case MB_WORD_MQTTALARMPUB:
        //can not use 0x06 function code
        break;

    case MB_WORD_GATEWAYIP:
        //can not use 0x06 function code
        break;

    case MB_WORD_LOCALIP:
        //can not use 0x06 function code
        break;

    case MB_WORD_S0TARGETIP:
        //can not use 0x06 function code
        break;

    case MB_WORD_S0LOCALPORT:
        //can not use 0x06 function code
        break;

    case MB_WORD_S0TARGETPORT:
        //can not use 0x06 function code
        break;

    case MB_WORD_S1TARGETIP:
        //can not use 0x06 function code
        break;

    case MB_WORD_S1LOCALPORT:
        //can not use 0x06 function code
        break;

    case MB_WORD_S1TARGETPORT:
        //can not use 0x06 function code
        break;

    case MB_WORD_NODECHECKINTER:
        //can not use 0x06 function code
        break;

    case MB_WORD_MQTTPUBINTER:
        //can not use 0x06 function code
        break;

    case MB_WORD_MACADDR:
        //can not use 0x06 function code
        break;

    case MB_WORD_MASKIP:
        //can not use 0x06 function code
        break;

    case MB_WORD_RTCTIME:
        //can not use 0x06 function code
        break;

    case MB_WORD_DESXLJCLEAR:
#if PROD_TYPE == PROD_FSS
        gFSS_Elem.st.LtNum = 0;
#else
        meter_clear_energy(lsv_ElementValue);
#endif
        break;

    case MB_WORD_IDNUMBER:
        gFlashParam.st.idNum = lsv_ElementValue;
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_L1AICLEAR:
        if(lsv_ElementValue == 0) //clear all
        {
//            LtListInit();
        }
        else //clear one 1 ~ 100
        {
//            if(LtListFlashDelete(lsv_ElementValue, &LtElemTemp))
//            {
//                LOGW("mb", "the following %d LtElem delete sucess: ", lsv_ElementValue);
//                PrintLtElem(LtElemTemp);
//            }
//            else
//            {
//                LOGE("mb", "the following %d LtElem delete fail: ", lsv_ElementValue);
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
//                mb_slave_error_resp(pMsg);
//                return;
//            }
        }
        break;

    case MB_WORD_G1A3INTERVAL:
        gFlashParam.st.GResGetInter = lsv_ElementValue == 0 ? 1 : lsv_ElementValue;
//        gR2900ItvTime = gFlashParam.st.GResGetInter * 60;  //获取秒数
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_G1A3GETNUM:
        gFlashParam.st.GResGetTimes = lsv_ElementValue;
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_VALUE:
#if PROD_TYPE == PROD_FL
        if(lsv_ModbusAddr == 0x0106)
        {
            if(lsv_ElementValue == 0)
            {
                LtListInit();
            }
        }
#elif PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_ESA
    if(lsv_ModbusAddr == 0x0106)
    {
        gParam.st.AlmOutput0 = lsv_ElementValue == 0 ? BOOL_Normal : BOOL_Alarm;
        SET_BIT(gFlashParam.st.AlmOutput_SourceLogic0, Output_OnlyCmd_Msk);  //一旦用指令控制，则不进行报警控制        
    }
    else if(lsv_ModbusAddr == 0x0107)
    {        
        gParam.st.AlmOutput1 = lsv_ElementValue == 0 ? BOOL_Normal : BOOL_Alarm;
        SET_BIT(gFlashParam.st.AlmOutput_SourceLogic1, Output_OnlyCmd_Msk);  //一旦用指令控制，则不进行报警控制              
    }  
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));    
#elif (PROD_TYPE == PROD_FD) || (PROD_TYPE == PROD_FS)
        if(lsv_ModbusAddr == 0x0109)
        {
            gFS_Elem.st.LtNum = lsv_ElementValue;
        }
#elif PROD_TYPE == PROD_FSS
        if(lsv_ModbusAddr == 0x010E)
        {
            gFS_Elem.st.LtNum = lsv_ElementValue;
        }
#elif PROD_TYPE == PROD_FR
        if(lsv_ModbusAddr == 0x0106)
        {
            gFlashParam.st.GResGetTimes = lsv_ElementValue;
        }
        else if(lsv_ModbusAddr == 0x0107)
        {
            gFlashParam.st.GResGetInter = lsv_ElementValue == 0 ? 1 : lsv_ElementValue;
            gR2900ItvTime = gFlashParam.st.GResGetInter * 60;  //获取秒数
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
#elif PROD_TYPE == PROD_FA
        if(lsv_ModbusAddr == 0x0108)
        {
            if(lsv_ElementValue != 0)
            {
                gtv_AfdDevice.mcv_AlmStatus = 1;
            }
            else
            {
                gtv_AfdDevice.mcv_AlmStatus = 0;
            }
        }
        else if(lsv_ModbusAddr == 0x0109)
        {
            gtv_AfdDevice.msv_ArcTimes = lsv_ElementValue;
        }
#else /* do nothing, cancer program warning */
        //can not use 0x06 function code
#endif
        break;

    case MB_WORD_SETDES4LIM:
#if PROD_TYPE == PROD_FSS
        if( (lsv_ModbusAddr == 0x0134) || (lsv_ModbusAddr == 0x0145) )
        {
            gFlashParam.st.SPD_YL_Up = lsv_ElementValue;
        }
        else if( (lsv_ModbusAddr == 0x0135) || (lsv_ModbusAddr == 0x0146) )
        {
            gFlashParam.st.SPD_KL_Up = lsv_ElementValue;
        }
        else if( (lsv_ModbusAddr == 0x0136) || (lsv_ModbusAddr == 0x0147) )
        {
            gFlashParam.st.SPD_PE_Up = lsv_ElementValue;
        }
#else
        if( (lsv_ModbusAddr == 0x0134) || (lsv_ModbusAddr == 0x0145) )
        {
            gFlashParam.st.SPD_L1_Up = lsv_ElementValue;
        }
        else if( (lsv_ModbusAddr == 0x0135) || (lsv_ModbusAddr == 0x0146) )
        {
            gFlashParam.st.SPD_L2_Up = lsv_ElementValue;
        }
        else if( (lsv_ModbusAddr == 0x0136) || (lsv_ModbusAddr == 0x0147) )
        {
            gFlashParam.st.SPD_L3_Up = lsv_ElementValue;
        }
#endif
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_G1A3OFFSET:
        if(lsv_ModbusAddr == 0x0180) //阻值矫正系数K
        {
            gFlashParam.st.GRes_K = lsv_ElementValue;
        }
        else if(lsv_ModbusAddr == 0x0181) //阻值矫正常数B
        {
            gFlashParam.st.GRes_B = lsv_ElementValue;
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_RSTZIGBEE:
        if(lsv_ElementValue)
        {
            zigbee_reset_default();
        }
        else
        {
            zigbee_reset();
        }
        break;

    /* new -------------------------------------------------------------------- */
    case MB_WORD_SD:
        //WORD_SD can not use 0x06 function code
        break;

    case MB_HALF_WORD_SD:
        SET_FLASH_HALFWORD_PARAM_VALUE(lsv_ElementAddr, lsv_ElementValue);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_BYTE_SD:
        lsv_ElementValue = GET_SMLPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);
        SET_FLASH_HALFWORD_PARAM_VALUE(lsv_ElementAddr, lsv_ElementValue);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_PARAM:
        //WORD_SD can not use 0x06 function code
        break;

    case MB_HALF_WORD_PARAM:
        SET_HALFWORD_PARAM_VALUE(lsv_ElementAddr, lsv_ElementValue);
        break;

    case MB_BYTE_PARAM:
        lsv_ElementValue = GET_SMLPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);
        SET_HALFWORD_PARAM_VALUE(lsv_ElementAddr, lsv_ElementValue);
        break;

    case MB_HALF_WORD_LTLIST:
        //MB_HALF_WORD_LTLIST can not use 0x06 function code. Only read
        break;

#if  PROD_TYPE == PROD_FSS
    case MB_WORD_CALICMD:
        if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEA4)  //三相全电流
        {
            gMeterRunInfo.EmuWork = lsv_ElementValue;
        }
        else
        {
            rn_gMeterRunInfo.EmuWork = lsv_ElementValue;
        }
        break;
#endif

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
    case MB_WORD_CALICMD:
        gMeterRunInfo.EmuWork = lsv_ElementValue;
        break;
#endif
    
    default:
        LOGE("mb", "In %s(), modbus address 0x%04x ,function code 0x06 illegal!", __func__, lsv_ModbusAddr);
        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
        mb_slave_error_resp(pMsg);
        return ;
    }

    /* PC发送
     * 站号 功能码 寄存器地址  数据   CRC
     *  01    06     02 01     00 01  18 72
     * 组响应帧,返回帧为请求帧的复制
     */
    for(i = 0; i < 6; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    pMsg->msv_RespLen = 6;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  写多寄存器
  * @param  None
  * @retval None
  */
void mb_slave_write_multiple_registers(md_slave_msg_pack *pMsg)
{
    unsigned char lcv_Ret;
    unsigned short lsv_ModbusAddr, lsv_ElementCnt, lsv_ElementAddr;
    unsigned char lcv_ElementType, lcv_ValueByteNum, lcv_Broadcast;
    unsigned short i;
    unsigned short *lsp_Value;
    rtc_datetime_t datetime;
    uint32_t llv_C32Value;

    lsv_ModbusAddr = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[2]);
    lsv_ElementCnt = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);
    lcv_ValueByteNum = pMsg->mcp_ReceiveBuff[6];
    LOGD("mb", "Enter %s(), lsv_ModbusAddr = 0x%04x, lsv_ElementCnt = %u, lcv_ValueByteNum = %u", __func__, lsv_ModbusAddr, lsv_ElementCnt, lcv_ValueByteNum);
    if((lsv_ElementCnt > MB_MAX_W_WORD_NUM) || (lsv_ElementCnt < 1))
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        return;
    }

    if(lsv_ElementCnt != lcv_ValueByteNum >> 1)
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        return;
    }

    if(lcv_ValueByteNum + 9 != pMsg->msv_ReceiveLen)  //写入长度与帧内长度不一致
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        return;
    }

    lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_ModbusAddr, lsv_ElementCnt, &lcv_ElementType, &lsv_ElementAddr, &lcv_Broadcast);
    if(lcv_Ret != true)
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
        mb_slave_error_resp(pMsg);
        return;
    }

    lsp_Value = (unsigned short *)&pMsg->mcp_ReceiveBuff[7];
    LOGD("mb", "lcv_ElementType = %u, lsv_ElementAddr = %u", lcv_ElementType, lsv_ElementAddr);

    if(pMsg->mcv_Sender != MB_SENDER_TCP) //mdtcp不需要判断站号
    {
        /*支持广播消息*/
        if( (pMsg->mcv_IsBroadcastInfo) && (lcv_Broadcast == MB_NOBROADCAST) )
        {
            LOGE("mb10", "In Broadcast, modbus address 0x%04x illegal!", lsv_ModbusAddr);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
            mb_slave_error_resp(pMsg);
            return ;
        }
    }

    switch(lcv_ElementType)
    {
    /* 自定义类型 compatible old protocol ------------------------------------- */
    case MB_WORD_IDINFO:
        memset(gFlashParam.st.idInfo, 0, sizeof(gFlashParam.st.idInfo));
        memcpy(gFlashParam.st.idInfo, (uint8_t *)lsp_Value, lcv_ValueByteNum);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_MQTTUSERNAME:
        memset(gFlashParam.st.mqttUserName, 0, sizeof(gFlashParam.st.mqttUserName));
        memcpy(gFlashParam.st.mqttUserName, (uint8_t *)lsp_Value, lcv_ValueByteNum);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_MQTTPASSWORD:
        memset(gFlashParam.st.mqttPassword, 0, sizeof(gFlashParam.st.mqttPassword));
        memcpy(gFlashParam.st.mqttPassword, (uint8_t *)lsp_Value, lcv_ValueByteNum);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_MQTTPUB:
        memset(gFlashParam.st.mqttPub, 0, sizeof(gFlashParam.st.mqttPub));
        memcpy(gFlashParam.st.mqttPub, (uint8_t *)lsp_Value, lcv_ValueByteNum);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_MQTTSUB:
        memset(gFlashParam.st.mqttSub, 0, sizeof(gFlashParam.st.mqttSub));
        memcpy(gFlashParam.st.mqttSub, (uint8_t *)lsp_Value, lcv_ValueByteNum);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_MQTTALARMPUB:
        memset(gFlashParam.st.mqttAlarmPub, 0, sizeof(gFlashParam.st.mqttAlarmPub));
        memcpy(gFlashParam.st.mqttAlarmPub, (uint8_t *)lsp_Value, lcv_ValueByteNum);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_GATEWAYIP:
        for (i = 0; i < 4; i++)
        {
            gFlashParam.st.gatewayIP[3 - i] = *((uint8_t *)lsp_Value + i);
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_LOCALIP:
        for (i = 0; i < 4; i++)
        {
            gFlashParam.st.localIP[3 - i] = *((uint8_t *)lsp_Value + i);
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_S0TARGETIP:
        for (i = 0; i < 4; i++)
        {
            gFlashParam.st.s0TargetIP[3 - i] = *((uint8_t *)lsp_Value + i);
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_S0LOCALPORT:
        gFlashParam.st.s0LocalPort = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_S0TARGETPORT:
        gFlashParam.st.s0TargetPort = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_S1TARGETIP:
        for (i = 0; i < 4; i++)
        {
            gFlashParam.st.s1TargetIP[3 - i] = *((uint8_t *)lsp_Value + i);
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_S1LOCALPORT:
        gFlashParam.st.s1LocalPort = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_S1TARGETPORT:
        gFlashParam.st.s1TargetPort = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_NODECHECKINTER:
        gFlashParam.st.getRtuCycleTime = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_MQTTPUBINTER:
        gFlashParam.st.mqttPublishInterval = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_MACADDR:
        memcpy(gFlashParam.st.macAddr, (uint8_t *)lsp_Value, lcv_ValueByteNum);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_MASKIP:
        for (i = 0; i < 4; i++)
        {
            gFlashParam.st.maskIP[3 - i] = *((uint8_t *)lsp_Value + i);
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_RTCTIME:
        if( (*((uint8_t *)lsp_Value + 1) > 12) || (*((uint8_t *)lsp_Value + 2) > 31) || (*((uint8_t *)lsp_Value + 3) > 23) || \
                (*((uint8_t *)lsp_Value + 4) > 59) || (*((uint8_t *)lsp_Value + 5) > 59) )
        {
            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
            mb_slave_error_resp(pMsg);
            return ;
        }
        datetime.year   = 2000 + *((uint8_t *)lsp_Value);
        datetime.month  = *((uint8_t *)lsp_Value + 1);
        datetime.day    = *((uint8_t *)lsp_Value + 2);
        datetime.hour   = *((uint8_t *)lsp_Value + 3);
        datetime.minute = *((uint8_t *)lsp_Value + 4);
        datetime.second = *((uint8_t *)lsp_Value + 5);
        RTC_SetDatetime(&datetime);
        break;

    case MB_WORD_DESXLJCLEAR:
#if PROD_TYPE == PROD_FSS
        gFSS_Elem.st.LtNum = 0;
#else
        meter_clear_energy(lsp_Value[0]);
#endif
        break;

    case MB_WORD_IDNUMBER:
        gFlashParam.st.idNum = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[0]);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_L1AICLEAR:
        //can not use 0x10 function code
        break;

    case MB_WORD_G1A3INTERVAL:
        gFlashParam.st.GResGetInter = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[0]) == 0 ? 1 : GET_BIGPU16_DATA((uint8_t *)&lsp_Value[0]);
//        gR2900ItvTime = gFlashParam.st.GResGetInter * 60;  //获取秒数
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_G1A3GETNUM:
        gFlashParam.st.GResGetTimes = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[0]);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_VALUE:
        //can not use 0x10 function code
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
    if(lsv_ModbusAddr == 0x0106)
    {
        gParam.st.AlmOutput0 = bytePoint_to_word((uint8_t *)&lsp_Value[0]) == 0 ? BOOL_Normal : BOOL_Alarm;
        SET_BIT(gFlashParam.st.AlmOutput_SourceLogic0, Output_OnlyCmd_Msk);  //一旦用指令控制，则不进行报警控制
        gParam.st.AlmOutput1 = bytePoint_to_word((uint8_t *)&lsp_Value[1]) == 0 ? BOOL_Normal : BOOL_Alarm;
        SET_BIT(gFlashParam.st.AlmOutput_SourceLogic1, Output_OnlyCmd_Msk);  //一旦用指令控制，则不进行报警控制   
    }
//    else if(lsv_ModbusAddr == 0x0107)
//    {
//        gParam.st.AlmOutput1 = bytePoint_to_word((uint8_t *)&lsp_Value[1]) == 0 ? BOOL_Normal : BOOL_Alarm;
//        SET_BIT(gFlashParam.st.AlmOutput_SourceLogic1, Output_OnlyCmd_Msk);  //一旦用指令控制，则不进行报警控制            
//    }
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
#endif
        break;

    case MB_WORD_SETDES4LIM:
#if PROD_TYPE == PROD_FSS
        gFlashParam.st.SPD_YL_Up = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[0]);
        gFlashParam.st.SPD_KL_Up = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[1]);
        gFlashParam.st.SPD_PE_Up = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[2]);
#else
        gFlashParam.st.SPD_L1_Up = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[0]);
        gFlashParam.st.SPD_L2_Up = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[1]);
        gFlashParam.st.SPD_L3_Up = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[2]);
#endif
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_G1A3OFFSET:
        gFlashParam.st.GRes_K = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[0]);
        gFlashParam.st.GRes_B = GET_BIGPU16_DATA((uint8_t *)&lsp_Value[1]);
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_RSTZIGBEE:
        if(lsp_Value[0])
        {
            zigbee_reset_default();
        }
        else
        {
            zigbee_reset();
        }
        break;

    /* new -------------------------------------------------------------------- */
    case MB_WORD_SD:
        if( (lsv_ElementCnt % 2 != 0) || (lsv_ModbusAddr % 2 == 0) )//当读取长度为奇数或读取地址为偶数时，为非法数据
        {
            LOGE("mb", "read double word register addr 0x%04x, len %d illegal", lsv_ModbusAddr, lsv_ElementCnt);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
            mb_slave_error_resp(pMsg);
            return ;
        }
        for(i = 0; i < lsv_ElementCnt; i += 2)
        {
            //llv_C32Value = (unsigned long)(lsp_Value[i]<<16) + lsp_Value[i+1];
            llv_C32Value = GET_BIGPU32_DATA((uint8_t *)&lsp_Value[i]);
						if(lsv_ModbusAddr == 0x0415 && gFlashParam.flash_buff[lsv_ElementAddr + i] != llv_C32Value) //配置产品型号参数
            {
                gFlashParam.st.magicNum = 0; //恢复出厂设置
            }
            SET_FLASH_WORD_PARAM_VALUE(lsv_ElementAddr + i, llv_C32Value);
        }
//        if( lsv_ModbusAddr == 0x042B && llv_C32Value == (PROD_FL | PARAM_M1AI) )  //配置产品参数为雷电流
//        {
//            gFlashParam.st.Adc_K01_K02 = 300;

//            gFlashParam.st.PAdc01_K = 280;     //280  => 0.0280
//            gFlashParam.st.PAdc01_B = -278;    //-278 => -2.78
//            gFlashParam.st.NAdc01_K = 280;
//            gFlashParam.st.NAdc01_B = -278;
//            gFlashParam.st.PAdc02_K = 311;     //311  => 0.0311
//            gFlashParam.st.PAdc02_B = -370;    //-370 => -3.70
//            gFlashParam.st.NAdc02_K = 311;
//            gFlashParam.st.NAdc02_B = -370;
//        }
//        else if( lsv_ModbusAddr == 0x042B && llv_C32Value == (PROD_FL | PARAM_L2AI))  //配置产品参数为瞬态电流
//        {
//            gFlashParam.st.Adc_K01_K02 = 2000;

//            gFlashParam.st.PAdc01_K = 4900;    //4900  => 0.4900
//            gFlashParam.st.PAdc01_B = -5455;   //-5455 => -54.55
//            gFlashParam.st.NAdc01_K = 4900;
//            gFlashParam.st.NAdc01_B = -5455;
//            gFlashParam.st.PAdc02_K = 5580;    //5580  => 0.5580
//            gFlashParam.st.PAdc02_B = -18773;  //-18773 => -187.73
//            gFlashParam.st.NAdc02_K = 5580;
//            gFlashParam.st.NAdc02_B = -18773;
//        }
//        else if( lsv_ModbusAddr == 0x042B && ((llv_C32Value & PROD_XX) == PROD_FSS))  //配置产品参数为智能SPD
//        {
//            gFlashParam.st.magicNum = 0;  //恢复出厂设置
//        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

//        if( (lsv_ElementCnt % 2 != 0) || (lsv_ModbusAddr % 2 == 0) )//当读取长度为奇数或读取地址为偶数时，为非法数据
//        {
//            LOGE("mb", "read double word register addr 0x%04x, len %d illegal", lsv_ModbusAddr, lsv_ElementCnt);
//            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
//            mb_slave_error_resp(pMsg);
//            return ;
//        }
//        for(i = 0; i < lsv_ElementCnt; i += 2)
//        {
//            llv_C32Value = bytePoint_to_dword((uint8_t *)&lsp_Value[i]);
//            if(lsv_ModbusAddr == 0x042B && gFlashParam.flash_buff[lsv_ElementAddr + i] != llv_C32Value) //配置产品型号参数
//            {
//                gFlashParam.st.magicNum = 0; //恢复出厂设置
//            }
//            SET_FLASH_WORD_PARAM_VALUE(lsv_ElementAddr + i, llv_C32Value);
//        }
//        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//        break;

    case MB_HALF_WORD_SD:
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            SET_FLASH_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i, GET_BIGPU16_DATA((uint8_t *)&lsp_Value[i]));
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_BYTE_SD:
        if( (lsv_ModbusAddr >= 1622) && (lsv_ModbusAddr < 1647) )  //域名
        {
            memset(gFlashParam.st.domainName, 0, sizeof(gFlashParam.st.domainName));
        }
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            SET_FLASH_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i, GET_SMLPU16_DATA((uint8_t *)&lsp_Value[i]));
        }
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        break;

    case MB_WORD_PARAM:
        if( (lsv_ElementCnt % 2 != 0) || (lsv_ModbusAddr % 2 == 0) )//当读取长度为奇数或读取地址为偶数时，为非法数据
        {
            LOGE("mb", "read double word register addr 0x%04x, len %d illegal", lsv_ModbusAddr, lsv_ElementCnt);
            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
            mb_slave_error_resp(pMsg);
            return ;
        }
        for(i = 0; i < lsv_ElementCnt; i += 2)
        {
            //llv_C32Value = (unsigned long)(lsp_Value[i]<<16) + lsp_Value[i+1];
            llv_C32Value = GET_BIGPU32_DATA((uint8_t *)&lsp_Value[i]);
            SET_WORD_PARAM_VALUE(lsv_ElementAddr + i, llv_C32Value);
        }
        break;

    case MB_HALF_WORD_PARAM:
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            SET_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i, GET_BIGPU16_DATA((uint8_t *)&lsp_Value[i]));
        }
        break;

    case MB_BYTE_PARAM:
        for(i = 0; i < lsv_ElementCnt; i++)
        {
            SET_HALFWORD_PARAM_VALUE(lsv_ElementAddr + i, GET_SMLPU16_DATA((uint8_t *)&lsp_Value[i]));
        }
        break;

    case MB_HALF_WORD_LTLIST:
        //MB_HALF_WORD_LTLIST can not use 0x10 function code
        break;

#if  PROD_TYPE == PROD_FSS
    case MB_WORD_CALICMD:
        if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEA4)  //三相全电流
        {
            gMeterRunInfo.EmuWork = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        }
        else
        {
            rn_gMeterRunInfo.EmuWork = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        }
        break;
#endif

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
    case MB_WORD_CALICMD:
        gMeterRunInfo.EmuWork = GET_BIGPU16_DATA((uint8_t *)lsp_Value);
        break;
#endif
    
    default:
        LOGE("mb", "In %s(), modbus address 0x%04x ,function code 0x10 illegal!", __func__, lsv_ModbusAddr);
        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
        mb_slave_error_resp(pMsg);
        return ;
    }

    /* PC发送
     * 站号 功能码 寄存器地址 寄存器长度 写入字节数 数据1 数据2 数据3  CRC
     * 01    10     02 01      00 03        06     01 02 03 04 05 06 06 3B
     * 组响应帧
     */
    for(i = 0; i < 6; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    pMsg->msv_RespLen = 6;
    mb_slave_verify_resp_msg(pMsg);
}

///**
//  * @brief  写多寄存器
//  * @param  None
//  * @retval None
//  */
//void mb_slave_write_multiple_registers(md_slave_msg_pack *pMsg)
//{
//    unsigned char lcv_Ret;
//    unsigned short lsv_ModbusAddr, lsv_ElementCnt, lsv_ElementAddr;
//    unsigned char lcv_ElementType, lcv_ValueByteNum;
//    unsigned short i;
//    unsigned short *lsp_Value;
//    unsigned long llv_C32Value;

//    lsv_ModbusAddr = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[2]);
//    lsv_ElementCnt = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);
//    lcv_ValueByteNum = pMsg->mcp_ReceiveBuff[6];

//    LOGD("mb", "Enter %s(), lsv_ModbusAddr = %u, lsv_ElementCnt = %u, lcv_ValueByteNum = %u", __func__, lsv_ModbusAddr, lsv_ElementCnt, lcv_ValueByteNum);
//    if((lsv_ElementCnt > MB_MAX_W_WORD_NUM) || (lsv_ElementCnt < 1)) {
//        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
//        mb_slave_error_resp(pMsg);
//        return;
//    }

//    if(lsv_ElementCnt != lcv_ValueByteNum>>1) {
//        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
//        mb_slave_error_resp(pMsg);
//        return;
//    }

//    lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_ModbusAddr, &lcv_ElementType, &lsv_ElementAddr);
//    if(lcv_Ret != pdPASS) {
//        pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//        mb_slave_error_resp(pMsg);
//        return;
//    }

//    lsp_Value = (unsigned short *)&pMsg->mcp_ReceiveBuff[7];
//    LOGD("mb", "lcv_ElementType = %u, lsv_ElementAddr = %u", lcv_ElementType, lsv_ElementAddr);
//    switch(lcv_ElementType) {
//        case MB_WORD_D:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > D_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                SET_D_ELEMENT_VALUE(lsv_ElementAddr+i, GET_BIGPU16_DATA((uint8_t*)&lsp_Value[i]));
//            }
//            break;

//        case MB_WORD_SD:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > SD_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                /*20170811:需要增加SD元件写入权限校验...*/
//                SET_SD_ELEMENT_VALUE(lsv_ElementAddr+i, GET_BIGPU16_DATA((uint8_t*)&lsp_Value[i]));
//            }
//            break;

//        case MB_WORD_Z:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > Z_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                SET_Z_ELEMENT_VALUE(lsv_ElementAddr+i, GET_BIGPU16_DATA((uint8_t*)&lsp_Value[i]));
//            }
//            break;

//        case MB_WORD_T:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > T_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                SET_T_CURRENT_VALUE(lsv_ElementAddr+i, GET_BIGPU16_DATA((uint8_t*)&lsp_Value[i]));
//            }
//            break;

//        case MB_WORD_C:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > C_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            if((lsv_ElementAddr) < C16_RANG) {

//                for(i=0; i<lsv_ElementCnt; i++) {
//                    SET_C16_CURRENT_VALUE(lsv_ElementAddr+i, GET_BIGPU16_DATA((uint8_t*)&lsp_Value[i]));
//                }
//            } else {
//                lsv_ElementCnt <<= 1;

//                for(i=0; i<lsv_ElementCnt; i+=2) {
//                    //llv_C32Value = (unsigned long)(lsp_Value[i]<<16) + lsp_Value[i+1];
//                    llv_C32Value = GET_BIGPU32_DATA((uint8_t*)&lsp_Value[i]);
//                    SET_C32_CURRENT_VALUE(lsv_ElementAddr+i/2, llv_C32Value);
//                }
//            }
//            break;

//        case MB_WORD_R:
//            if(lsv_ElementAddr + lsv_ElementCnt -1 > R_RANG) {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//                mb_slave_error_resp(pMsg);
//                return;
//            }

//            for(i=0; i<lsv_ElementCnt; i++) {
//                SET_R_ELEMENT_VALUE(lsv_ElementAddr+i, GET_BIGPU16_DATA((uint8_t*)&lsp_Value[i]));
//            }
//            break;

//        default:
//            pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
//            mb_slave_error_resp(pMsg);
//            return;
//    }

//    /*组响应帧*/
//    for(i=0; i<6; i++)
//        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

//    pMsg->msv_RespLen = 6;
//    mb_slave_verify_resp_msg(pMsg);
//}

/**
  * @brief  检测CRC校验是否正确
  * @param  *pStr 数据
  * @param  sLen 数据长度
  * @retval true/false
  */
bool checkRev_crc16(uint8_t *pStr, uint16_t sLen)
{
    unsigned short lsv_Crc;

    if(sLen < 2)
    {
        return false;
    }

    lsv_Crc = *(pStr + sLen - 1);
    lsv_Crc <<= 8;
    lsv_Crc |= *(pStr + sLen - 2);
    if( lsv_Crc != calc_crc16(pStr, sLen - 2) )
    {
        return false;
    }
    return true;
}

/**
  * @brief  判断是否是modbus协议
  * @param  *pData 数据
  * @param  len 数据长度
  * @retval true/false
  */
bool is_mb_protocol(uint8_t *pData, uint16_t len)
{
    if (pData[0] != (uint8_t)gFlashParam.st.idNum && pData[0] != 0x00) //判断站号
    {
        LOGE("mb", "recvive idNum is 0x%02x, but device idNum is 0x%02x", pData[0], (uint8_t)gFlashParam.st.idNum);
        return false;
    }

    if(len < 5)
    {
        return false;
    }

    if(checkRev_crc16(pData, len) != true)
    {
        return false;
    }

    switch (pData[1])
    {
    case MB_READ_HOLDING_REGISTER:
        if(len != 8)  //length error
        {
            return false;
        }
        return true;

    case MB_WRITE_REGISTER:
        return true;

    case MB_WRITE_MULTIPLE_REGISTERS:
        if(pData[6] + 9 != len)  //length error
        {
            return false;
        }
        return true;
        
    case MB_DOWNLOWD_FUNC:
        return true;
            
    case MB_RESET_REGISTERS:
//        if(len != 5)  //length error
//        {
//            return false;
//        }
        return true;

    default:
        return false;
    }
}


/**
  * @brief  读取PLC信息
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_read_plc_info(md_slave_msg_pack *pMsg)
{
    //printf("Enter %s\r\n", __func__);
    unsigned char i;

    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    /*设备类型*/
    pMsg->mcp_RespBuff[3] = (unsigned char)(gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId >> 8);
    pMsg->mcp_RespBuff[4] = (unsigned char)(gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId & 0xFF);

    /*设备ID*/
    for(i=0; i<12; i++) {
        pMsg->mcp_RespBuff[5+i] = gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[i];
    }

//    LOGV("mb_ctrl", "Version:%u, Rongliang:%u\r\n", GET_SD_ELEMENT_VALUE(1), GET_SD_ELEMENT_VALUE(2));
//    /*版本号*/
//    uint16_t ver = FIRMWARE_IMAGE_ID;
//    pMsg->mcp_RespBuff[17] = (unsigned char)(ver >> 0x08);
//    pMsg->mcp_RespBuff[18] = (unsigned char)(ver & 0xFF);

//    /*程序容量*/
//    ver = PROGRAM_CAPACITY;
//    pMsg->mcp_RespBuff[19] = (unsigned char)(ver >> 0x08);
//    pMsg->mcp_RespBuff[20] = (unsigned char)(ver & 0xFF);
//    /*系统错误*/
//    pMsg->mcp_RespBuff[21] = (unsigned char)(GET_SD_ELEMENT_VALUE(3)>>0x08);
//    pMsg->mcp_RespBuff[22] = (unsigned char)(GET_SD_ELEMENT_VALUE(3)&0xFF);
//    /*运行错误*/
//    pMsg->mcp_RespBuff[23] = (unsigned char)(GET_SD_ELEMENT_VALUE(20)>>0x08);
//    pMsg->mcp_RespBuff[24] = (unsigned char)(GET_SD_ELEMENT_VALUE(20)&0xFF);
//    /*电池电压值*/
//    pMsg->mcp_RespBuff[25] = 0;
//    pMsg->mcp_RespBuff[26] = 33;
//    /*PLC运行状态*/
//    pMsg->mcp_RespBuff[27] = 0;
//    pMsg->mcp_RespBuff[28] = plc_get_bit_element_value(SM_ELEMENT, 0);
//    /*当前扫描速率*/
//    pMsg->mcp_RespBuff[29] = (unsigned char)(GET_SD_ELEMENT_VALUE(30)>>0x08);
//    pMsg->mcp_RespBuff[30] = (unsigned char)(GET_SD_ELEMENT_VALUE(30)&0xFF);
//    /*最小扫描速率*/
//    pMsg->mcp_RespBuff[31] = (unsigned char)(GET_SD_ELEMENT_VALUE(31)>>0x08);
//    pMsg->mcp_RespBuff[32] = (unsigned char)(GET_SD_ELEMENT_VALUE(31)&0xFF);
//    /*最大扫描速率*/
//    pMsg->mcp_RespBuff[33] = (unsigned char)(GET_SD_ELEMENT_VALUE(32)>>0x08);
//    pMsg->mcp_RespBuff[34] = (unsigned char)(GET_SD_ELEMENT_VALUE(32)&0xFF);

//    /*IP地址*/
//    memcpy(&(pMsg->mcp_RespBuff[35]),(unsigned char *)&(g_plc_netcfg.wan.ip.addr),4);
//    /*子网掩码*/
//    memcpy (&(pMsg->mcp_RespBuff[39]),(unsigned char *)&(g_plc_netcfg.wan.mask.addr),4);
//    /*网关*/
//    memcpy (&(pMsg->mcp_RespBuff[43]),(unsigned char *)&(g_plc_netcfg.wan.gate.addr),4);
//    /*DNS*/
//    memcpy (&(pMsg->mcp_RespBuff[47]) , (unsigned char *)&(g_plc_netcfg.wan.dns.addr),4);

//    /*当前连接*/
//    pMsg->mcp_RespBuff[51] =  (unsigned char)(GET_SD_ELEMENT_VALUE(SD223)>>0x08);
//    pMsg->mcp_RespBuff[52] =  (unsigned char)(GET_SD_ELEMENT_VALUE(SD223)&0xFF);
//    /*4G信号强度*/
//    pMsg->mcp_RespBuff[53] = (unsigned char)(GET_SD_ELEMENT_VALUE(SD226)>>0x08);
//    pMsg->mcp_RespBuff[54] = (unsigned char)(GET_SD_ELEMENT_VALUE(SD226)&0xFF);

//    /*串口0*/
////    unsigned short us_Pro = GET_UART_SD_VALUE(0, UART_SD_MODE_CONFIG);
////    pMsg->mcp_RespBuff[55] = 0xFF & ( us_Pro >> 8 );
////    pMsg->mcp_RespBuff[56] = 0xFF & us_Pro;
//    /*串口1*/
////    us_Pro = GET_UART_SD_VALUE(1, UART_SD_MODE_CONFIG);
////    pMsg->mcp_RespBuff[57] = 0xFF & ( us_Pro >> 8 );
////    pMsg->mcp_RespBuff[58] = 0xFF & us_Pro;
//    /*COM2*/
////    us_Pro = GET_UART_SD_VALUE(2, UART_SD_MODE_CONFIG);
////    pMsg->mcp_RespBuff[59] = 0xFF & ( us_Pro >> 8 );
////    pMsg->mcp_RespBuff[60] = 0xFF & us_Pro;
//    /*预留COM3 ~ COM5*/
//    pMsg->mcp_RespBuff[61] = 0;
//    pMsg->mcp_RespBuff[62] = 0;

//    pMsg->mcp_RespBuff[63] = 0;
//    pMsg->mcp_RespBuff[64] = 0;

//    pMsg->mcp_RespBuff[65] = 0;
//    pMsg->mcp_RespBuff[66] = 0;

//    /*系统运行时间*/
////    uint32_t mlv_Systime = gKalykeSecondTickCurrent;
////    printf("\r\n mlv_Systime = %u second\r\n", mlv_Systime);
////    pMsg->mcp_RespBuff[67] = (unsigned char)(mlv_Systime >> 24);
////    pMsg->mcp_RespBuff[68] = (unsigned char)(mlv_Systime >> 16);
////    pMsg->mcp_RespBuff[69] = (unsigned char)(mlv_Systime >> 8);
////    pMsg->mcp_RespBuff[70] = (unsigned char)(mlv_Systime);

//    //版本号
//    pMsg->mcp_RespBuff[71] = (unsigned char)(GET_SD_ELEMENT_VALUE(209)>>0x08);
//    pMsg->mcp_RespBuff[72] = (unsigned char)(GET_SD_ELEMENT_VALUE(209)&0xFF);

//    pMsg->mcp_RespBuff[73] = (unsigned char)(GET_SD_ELEMENT_VALUE(210)>>0x08);
//    pMsg->mcp_RespBuff[74] = (unsigned char)(GET_SD_ELEMENT_VALUE(210)&0xFF);

//    pMsg->mcp_RespBuff[75] = (unsigned char)(GET_SD_ELEMENT_VALUE(211)>>0x08);
//    pMsg->mcp_RespBuff[76] = (unsigned char)(GET_SD_ELEMENT_VALUE(211)&0xFF);

//    pMsg->mcp_RespBuff[77] = (unsigned char)(GET_SD_ELEMENT_VALUE(212)>>0x08);
//    pMsg->mcp_RespBuff[78] = (unsigned char)(GET_SD_ELEMENT_VALUE(212)&0xFF);

//    // LAN口FEXLINK扩展协议
//    pMsg->mcp_RespBuff[79] = g_plc_netcfg.lan.ioExp;
//    /*IP地址*/
//    memcpy (&(pMsg->mcp_RespBuff[80]),(unsigned char *)&(g_plc_netcfg.lan.ip.addr),4);
//    /*子网掩码*/
//    memcpy (&(pMsg->mcp_RespBuff[84]),(unsigned char *)&(g_plc_netcfg.lan.mask.addr),4);
//    /*网关*/
//    memcpy (&(pMsg->mcp_RespBuff[88]),(unsigned char *)&(g_plc_netcfg.lan.gate.addr),4);

//    // WAN口DHCP
//    pMsg->mcp_RespBuff[92] = g_plc_netcfg.wan.notUseDHCP;


//    /*保留空间填充0xBB*/
//    for (i = 93; i < 100; i++)
//    {
//        pMsg->mcp_RespBuff[i] = 0xBB;
//    }


    pMsg->msv_RespLen = 100;
    mb_slave_verify_resp_msg(pMsg);
    
    //ota_reset();
    //showMem();
}

/**
  * @brief  读取PLC状态
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_read_plc_status(md_slave_msg_pack *pMsg)
{
    unsigned char i;

    for(i=0; i<3; i++)
    pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

//    pMsg->mcp_RespBuff[3] = plc_get_bit_element_value(SM_ELEMENT, 0);
//    pMsg->mcp_RespBuff[4] = (unsigned char)(GET_SD_ELEMENT_VALUE(30)>>0x08);
//    pMsg->mcp_RespBuff[5] = (unsigned char)(GET_SD_ELEMENT_VALUE(30)&0xFF);	

    pMsg->mcp_RespBuff[3] = 0x01;
    pMsg->mcp_RespBuff[4] = 0x00;
    pMsg->mcp_RespBuff[5] = 0x01;	
    pMsg->msv_RespLen = 6;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  获取文件长度信息
  * @param  pData   文件指针
  *         flag    文件长度字节数
  * @retval None
  */
unsigned long plc_get_file_length(unsigned char *pData, unsigned short flag)
{
    unsigned long mlv_FileLength;

    switch(flag) {
        case 1:
            mlv_FileLength = (unsigned long)GET_PU8_DATA(pData);
            break;
        case 2:
            /*地址未对齐*/
            if(GET_POINT_ADDR(pData)&0x01) {
                mlv_FileLength = (unsigned long)(GET_PU8_DATA(pData) + (GET_PU8_DATA(pData+1) << 8));
            } else {
                mlv_FileLength = (unsigned long)(GET_PU16_DATA(pData));
            }
            break;
        case 4:
            if(GET_POINT_ADDR(pData)&0x03) {
                mlv_FileLength = (unsigned long)(GET_PU8_DATA(pData) + (GET_PU8_DATA(pData+1) << 8) + (GET_PU8_DATA(pData+2) << 16) + (GET_PU8_DATA(pData+3) << 24));
            } else {
                mlv_FileLength = (GET_PU32_DATA(pData));
            }
            break;
        default:
            mlv_FileLength = 0;
    }

    return mlv_FileLength;
}

/**
  * @brief  写缓存区程序至Flash特定区域
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_programme_cmd(md_slave_msg_pack *pMsg)
{
    unsigned char i;
    unsigned long llv_FileLen;
//    unsigned char *lcp_Buffer;
//    unsigned long lVersion;
//    unsigned short nProductType;
    LOGV(TAG, "Enter %s()", __func__);

    mem_part_info_t ltv_FlashPartInfo[] = {
        /*UCODE分区 0 */
        {/*  17  128K ucode   */
            0x803A000,
            1024,
        },
        /*系统块分区 1*/
        { /*  13  64K sys      */
            0x803A400,
            1024,
        },
        /*数据块分区 2*/
        {  /*  16  64K db       */
            0x803A800,
            1024,
        },
        /*POU INFO 3*/
        {  /*  14  16K pou      */
            0x803AC00,
            512,
        },
        /*GVT 4*/
        {  /*  15  16K gvt      */
            0x803AE00,
            512,
        },
        /*MB_DL_NETCFG 5*/
        { /* 10  */
            PLC_INFO_SAVE_ADDR,
            10240,
        },
        /*系统升级bin文件 6*/
        {  /* 两个bin，一个放到0x60080000，一个放到0x60000000，对于RT1061只能如此 */
            0x803D800,
            512,
        },
        /* cbin  7 */
        {  /*  19 CBIN_UPGRADE_START_PAGE */
            0x803DA00,
            512,
        },
        /* PID  8 */
        {  /*  19 CBIN_UPGRADE_START_PAGE */
            0x803DA00,
            512,
        },
        /* PID  9 */
        {  /*  19 CBIN_UPGRADE_START_PAGE */
            0x803DE00,
            512,
        },
    };
    
    for(i=0; i<MB_DL_MAX; i++) {
        LOGD(TAG, "i = %d, flag = %d", i, gtv_ModbusFileTrans[i]->mcv_Flag);
        switch(i) {
            case MB_DL_UCODE:
            case MB_DL_SYS_BLOCK:
            case MB_DL_DATA_BLOCK:
            case MB_DL_POU_INFO:
            case MB_DL_GVT:
            case MB_DL_NETCFG:
            case MB_DL_PID1:
            case MB_DL_PID2:
                /*文件传输完成,写入Flash*/
                if((NULL != gtv_ModbusFileTrans[i]) && (gtv_ModbusFileTrans[i]->mcv_Flag & 0x02))
                {
                    llv_FileLen = plc_get_file_length(gtv_ModbusFileTrans[i]->mcp_FileHandler + 6, 4);
										(void)llv_FileLen;
                    LOGD(TAG, "file length = %d", llv_FileLen);

                    /*写文件到目标分区*/
                    
                Parameter_FlashWrite(ltv_FlashPartInfo[i].startAddr, 
                                        gtv_ModbusFileTrans[i]->mcp_FileHandler, 
                                        gtv_ModbusFileTrans[i]->mlv_FileLen);

//                    char *mqttBuf = pvPortMalloc(10240);
//                    memset(mqttBuf, 0, 10240);
//                    memcpy(mqttBuf, gtv_ModbusFileTrans[i]->mcp_FileHandler, gtv_ModbusFileTrans[i]->mlv_FileLen);
//                    Parameter_FlashWrite(PLC_INFO_SAVE_ADDR, &mqttBuf, sizeof(mqttBuf));      
//                    vPortFree(mqttBuf);
                    
//                    memcpy(gplcList.lsv_buff, gtv_ModbusFileTrans[i]->mcp_FileHandler, gtv_ModbusFileTrans[i]->mlv_FileLen);
//                    Parameter_FlashWrite(PLC_INFO_SAVE_ADDR, &gplcList, sizeof(gplcList));
                    
                    LOGD(TAG, "mcp_FileHandler = 0x%08X, xPortGetFreeHeapSize=%u\r\n", gtv_ModbusFileTrans[i]->mcp_FileHandler, xPortGetFreeHeapSize());
                    /*释放动态内存*/
                    vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                    gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                    vPortFree(gtv_ModbusFileTrans[i]);
                    gtv_ModbusFileTrans[i] = NULL;
                }
                break;

            case MB_DL_SYS_UPGRADE:
                break;

            case MB_DL_PLC_CBIN:
                break;
        }
    }

    /*组响应帧，特殊的响应帧在对应函数完成*/
    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->msv_RespLen = 3;
    mb_slave_verify_resp_msg(pMsg);
}

bool gSuspendFlag = false;

void suspend_task_when_download_ucode(void)
{
    taskENTER_CRITICAL();
    LOGD("tool", "Enter %s(), gSuspendFlag=%u", __func__, gSuspendFlag);
    if (gSuspendFlag == false)
    {
        gSuspendFlag = true;
        
        if (keyOledTaskHandle)
        {
            vTaskSuspend(keyOledTaskHandle);
            keyOledTaskHandle = NULL;
            //LOGV("tool", "keyOledTaskHandle = 0x%08X", keyOledTaskHandle);
        }

        if (ntcTaskHandle)
        {
            vTaskSuspend(ntcTaskHandle);
            ntcTaskHandle = NULL;
            //LOGV("tool", "ntcTaskHandle = 0x%08X", ntcTaskHandle);
        }
        
        if (att7022TaskHandle)
        {
            vTaskSuspend(att7022TaskHandle);
            att7022TaskHandle = NULL;
            //LOGV("tool", "att7022TaskHandle = 0x%08X", att7022TaskHandle);
        }
        
        if (internetTaskHandle)
        {
            vTaskSuspend(internetTaskHandle);
            internetTaskHandle = NULL;
            //LOGV("tool", "internetTaskHandle = 0x%08X", internetTaskHandle);
        }

        if (MonitorTaskHandle)
        {
            vTaskSuspend(MonitorTaskHandle);
            MonitorTaskHandle = NULL;
            //LOGV("tool", "MonitorTaskHandle = 0x%08X", MonitorTaskHandle);
        }
            
        if (PLCTaskHandle)
        {
            vTaskSuspend(PLCTaskHandle);
            PLCTaskHandle = NULL;
            //LOGV("tool", "PLCTaskHandle = 0x%08X", PLCTaskHandle);
        }
        
        if (internet4GTaskHandle)
        {
            vTaskSuspend(internet4GTaskHandle);
            internet4GTaskHandle = NULL;
            //LOGV("tool", "internet4GTaskHandle = 0x%08X", internet4GTaskHandle);
        }

//        if (led1_TaskHandle)
//        {
//            vTaskDelete(led1_TaskHandle);
//            led1_TaskHandle = NULL;
//            //LOGV("tool", "led1_TaskHandle = 0x%08X", led1_TaskHandle);
//        }
//        
//        if (led2_TaskHandle)
//        {
//            vTaskDelete(led2_TaskHandle);
//            led2_TaskHandle = NULL;
//            //LOGV("tool", "led2_TaskHandle = 0x%08X", led2_TaskHandle);
//        }
    }
    taskEXIT_CRITICAL();
}


/**
  * @brief  CTRL 功能码处理函数
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_manage(md_slave_msg_pack *pMsg)
{
    unsigned char i;

    switch(pMsg->mcp_ReceiveBuff[2]) {
        /*读取时钟信息*/
        case MB_CTRL_READ_RTC:
//            mb_slave_ctrl_read_realtime(pMsg);
            return;
        /*写入时钟信息*/
        case MB_CTRL_WRITE_RTC:
//            mb_slave_ctrl_set_realtime(pMsg);
            return;
        /*运行PLC程序*/
        case MB_CTRL_RUN_CMD:
//            mb_slave_ctrl_run_cmd(pMsg);
            break;
        /*停止运行PLC*/
        case MB_CTRL_STOP_CMD:
//            mb_slave_ctrl_stop_cmd(pMsg);
            break;
        /*批量强制位元件*/
        case MB_CTRL_FORCE_BITS:
//            mb_slave_ctrl_force_bits(pMsg);
            break;
        /*批量字元件强制*/
        case MB_CTRL_FORCE_WORDS:
//            mb_slave_ctrl_force_words(pMsg);
            break;
        /*取消位元件强制*/
        case MB_CTRL_UNFORCE_BITS:
//            mb_slave_ctrl_unforce_bits(pMsg);
            break;
        /*取消字元件强制*/
        case MB_CTRL_UNFORCE_WORDS:
//            mb_slave_ctrl_unforce_words(pMsg);
            break;
        /*取消全部强制*/
        case MB_CTRL_UNFORCE_ALL:
//            mb_slave_ctrl_unforce_all(pMsg);
            break;
        /*读取PLC信息*/
        case MB_CTRL_READ_PLC_INFO:
            mb_slave_ctrl_read_plc_info(pMsg);
            return;
        /*读PLC状态*/
        case MB_CTRL_READ_PLC_STATUS:
            mb_slave_ctrl_read_plc_status(pMsg);
            return;
        /*固化指令*/
        case MB_CTRL_PROGRAMME_CMD:
            /*在线模式开启时，固化需要暂停PLC运行*/
            LOGD(TAG, "Begin GuHua!!!\r\n");
//            unsigned char IsOnlineProgram = gtv_PlcRunStatus.mcv_IsOnlineProgram;
//            if(IsOnlineProgram)
//            {
//                LOGE(TAG, "In online mode  we need SSSSStop plc before GuHua!!!\r\n");
//                mb_slave_ctrl_stop_cmd(pMsg);
//            }
//            suspend_task_when_download_ucode();
//            gGUHUAing = 1;
            mb_slave_ctrl_programme_cmd(pMsg);
//            gGUHUAing = 0;
//            resume_task_after_download_ucode();
//            if(IsOnlineProgram)
//            { 
//                vTaskDelay(20);
//                LOGE(TAG, "In online mode  we need RRRRRRun plc after GuHua!!!\r\n");
//                mb_slave_ctrl_run_cmd(pMsg);
//            }
            LOGD(TAG, "GuHua Over\r\n");
            return;
        /*清除用户程序*/
        case MB_CTRL_CLEAN_USER_CODE:
//            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
//            {
//                mb_slave_ctrl_clean_user_code(pMsg);
//            }
//            else
//            {
//                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
//                mb_slave_error_resp(pMsg);
//                return;
//            }
            break;
        /*清除系统块*/
        case MB_CTRL_CLEAN_SYS_BLOCK:
//            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
//            {
//                mb_slave_ctrl_clean_system_block(pMsg);
//            }
//            else
//            {
//                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
//                mb_slave_error_resp(pMsg);
//                return;
//            }
            break;
        /*清除数据块*/
        case MB_CTRL_CLEAN_DATA_BLOCK:
//            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
//            {
//                mb_slave_ctrl_clean_data_block(pMsg);
//            }
//            else
//            {
//                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
//                mb_slave_error_resp(pMsg);
//                return;
//            }
            break;
        /*清除网络配置*/
        case MB_CTRL_CLEAN_NETCFG:
//            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
//            {
//                mb_slave_ctrl_clean_net_cfg(pMsg);
//            }
//            else
//            {
//                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
//                mb_slave_error_resp(pMsg);
//                return;
//            }
            break;

        /*格式化*/
        case MB_CTRL_CLEAN_ALL:
//            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
//            {
//                mb_slave_ctrl_clean_all_user_data(pMsg);
//            }
//            else
//            {
//                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
//                mb_slave_error_resp(pMsg);
//                return;
//            }
            break;
        /*UCODE校验*/
        case MB_CTRL_VERIFY_UCODE:
//            mb_slave_ctrl_verify_ucode(pMsg);
            return;
        /*PID参数1校验*/
        case MB_CTRL_VERIFY_PID1:
//            mb_slave_ctrl_verify_pid1(pMsg);
            return;
        /*复位*/
        case MB_CTRL_REBOOT:
//            mb_slave_ctrl_system_reboot(pMsg);
            return;
        /*清除错误信息*/
        case MB_CTRL_CLEAN_ERROR_INFO:
//            mb_slave_ctrl_error_info(pMsg);
            break;
        /*清除元件值*/
        case MB_CTRL_CLEAN_ELEMENT:
//            mb_slave_ctrl_clean_element(pMsg);
            break;
        /*禁止上载*/
        case MB_CTRL_SET_UPLOAD_FORBID:
//            mb_slave_ctrl_set_upload_forbid(pMsg);
            break;
        /*解除禁止上载*/
        case MB_CTRL_CLEAN_UPLOAD_FORBID:
//            mb_slave_clean_upload_forbid(pMsg);
            break;
        /*读取错误信息*/
        case MB_CTRL_READ_ERROR_INFO:
//            mb_slave_read_error_info(pMsg);
            return;
        /*设置上载密码*/
        case MB_CTRL_SET_UPLOAD_PWD:
//            mb_slave_ctrl_set_user_password(pMsg, UPLOAD_PASSWORD);
            break;
        /*验证上载密码*/
        case MB_CTRL_CHECK_UPLOAD_PWD:
//            mb_slave_ctrl_check_user_password(pMsg, UPLOAD_PASSWORD);
            break;
        /*设置下载密码*/
        case MB_CTRL_SET_DOWNLOAD_PWD:
//           mb_slave_ctrl_set_user_password(pMsg, DOWNLOAD_PASSWORD);
            break;
        /*验证下载密码*/
        case MB_CTRL_CHECK_DOWNLOAD_PWD:
//            mb_slave_ctrl_check_user_password(pMsg, DOWNLOAD_PASSWORD);
            break;
        /*设置监控密码*/
        case MB_CTRL_SET_MONITOR_PWD:
//            mb_slave_ctrl_set_user_password(pMsg, MONITOR_PASSWORD);
            break;
        /*验证监控密码*/
        case MB_CTRL_CHECK_MONITOR_PWD:
//            mb_slave_ctrl_check_user_password(pMsg, MONITOR_PASSWORD);
            break;
        /*设置时钟密码*/
        case MB_CTRL_SET_TIMER_PWD:
//            mb_slave_ctrl_set_user_password(pMsg, TIMER_PASSWORD);
            break;
        /*验证时钟密码*/
        case MB_CTRL_CHECK_TIMER_PWD:
//            mb_slave_ctrl_check_user_password(pMsg, TIMER_PASSWORD);
            break;		
        /*进入在线编程模式*/
        case MB_CTRL_ENTERY_ONLINE_PROGRAM:
//            mb_slave_ctrl_entry_online_program(pMsg);
            return;
        /*退出在线编程模式*/
        case MB_CTRL_EXIT_ONLINE_PROGRAM:
//            mb_slave_ctrl_exit_online_program(pMsg);
            return;
        /*写设备ID MAC地址等信息*/
        case MB_CTRL_WRITE_DEVICE_INFO:
//            mb_slave_ctrl_write_device_info(pMsg);
            break;
        case MB_CTRL_BLINK_NET_LED:
//            mb_slave_blink_net_led(pMsg);
            break;

        case MB_CTRL_TOU_CHUAN_COM0:
//            gTouChuan = 0;
//            SET_SD_ELEMENT_VALUE(SD231, 0);
//            LOGV("mb_ctrl", "gTouChuan =  %u", gTouChuan);		
            break;
        
        case MB_CTRL_TOU_CHUAN_COM1:
//            gTouChuan = 1;
//            SET_SD_ELEMENT_VALUE(SD231, 1);
//            LOGV("mb_ctrl", "gTouChuan =  %u", gTouChuan);		
            break;
            
        case MB_CTRL_TOU_CHUAN_COM2:
//            gTouChuan = 2;
//            SET_SD_ELEMENT_VALUE(SD231, 2);
//            LOGV("mb_ctrl", "gTouChuan =  %u", gTouChuan);		
            break;

#if (KALYKE_TOUCHUAN_WAN_LAN == 1)
        case MB_CTRL_TOU_CHUAN_WAN:
            mb_slave_set_touchuan(3);
            LOGV("mb_ctrl", "gTouChuan =  %u", gTouChuan);				
            break;
        case MB_CTRL_TOU_CHUAN_LAN:
            mb_slave_set_touchuan(4);
            LOGV("mb_ctrl", "gTouChuan =  %u", gTouChuan);	
            break;
#endif

        case MB_CTRL_TOU_CHUAN_CLOSE:
//            gTouChuan = 0xFF;
//            SET_SD_ELEMENT_VALUE(SD231, 0xFF);
            break;

        case MB_CTRL_SEARCH_SLAVE:
//            daisy_get_info(pMsg);
            return;
    #if (ETHERCAT_SOEM == 1)
        case MB_CTRL_SEARCH_ETH_SLAVE:
            kalyke_slave_scan(pMsg);
            break;
    #endif

        /* 01 6A 90 01(SlaveID) */
        case MB_CTRL_SLAVE_ID:
        #if (DAISY_MASTER_FEATURE == 1)
            gSlaveIDUpgrade = pMsg->mcp_ReceiveBuff[3];
            LOGV("mb_ctrl", "gSlaveIDUpgrade = %u", gSlaveIDUpgrade);
        #endif
            break;

        default:
//            gtp_ModbusSlaveDiagInfo[pMsg->mcv_Sender].msv_SlaveErrCnt ++;
            pMsg->mcv_ErrorCode = MB_ILIEGAL_CODE;
            mb_slave_error_resp(pMsg);
            return;
    }

    /*组响应帧，特殊的响应帧在对应函数完成*/
    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->msv_RespLen = 3;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  下载文件结构体内存分配
  * @param  None
  * @retval None
  */
void mb_slave_init_file_info(mb_file_trans_st *gvt_pFile)
{
    if(gvt_pFile != NULL) {
        gvt_pFile->mcv_FrameCnt = 0;
        gvt_pFile->mcv_PreFrame = 0;
        gvt_pFile->mcv_Flag = 0;
        gvt_pFile->mlv_FileLen = 0;
    }
}

/**
  * @brief  带SDRAM项目文件下载函数
  * @param  None
  * @retval None
  */
unsigned char mb_slave_download_file(md_slave_msg_pack *pMsg)
{
    LOGE("mb", "Enter %s\r\n", __func__);
    mb_file_trans_st * ltv_pFile;
    unsigned char lcv_FrameNum;
    unsigned short lsv_Cnt;
    unsigned char * lcp_SrcBuff;
    unsigned char * lcp_DestBuff;
    unsigned short i;
    unsigned char temp;
//    unsigned char lcv_Ret;
//    unsigned long ulongFileLength;

    switch(pMsg->mcp_ReceiveBuff[2]) {
        /*下载UCODE*/
        case MB_DOWNLOAD_UCODE:
//            hexdump(gtv_ModbusFileTrans, sizeof(gtv_ModbusFileTrans));
//            /*之前有接收完成，未固化文件。在线模式出现*/
//            if((gtv_ModbusFileTrans[MB_DL_UCODE] != NULL) && (gtv_ModbusFileTrans[MB_DL_UCODE]->mcv_Flag & 0x2))
//            {
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_UCODE]->mcv_Flag);
//                LOGD(TAG, "MB_DOWNLOAD_UCODE: ...........000, mcp_FileHandler = 0x%08X", gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler);
//                /*释放动态内存*/
//                vPortFree(gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler);
//                gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler = NULL;
//                vPortFree(gtv_ModbusFileTrans[MB_DL_UCODE]);
//                gtv_ModbusFileTrans[MB_DL_UCODE] = NULL;
//            }
//            LOGD(TAG, "%s: ...........001", __func__);
//            if(NULL == gtv_ModbusFileTrans[MB_DL_UCODE])
//            {
//                LOGI(TAG, "%s: ...........002", __func__);
//                /*在线模式开启时，接收阶段暂停PLC运行*/
//                if(gtv_PlcRunStatus.mcv_IsOnlineProgram)
//                {
//                    LOGD(TAG, "%s: ...........003", __func__);
//                    mb_slave_ctrl_stop_cmd(pMsg);
//                }
//                LOGD(TAG, "%s: ...........004", __func__);
//                gtv_ModbusFileTrans[MB_DL_UCODE] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_UCODE] != NULL);

//                gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler = (unsigned char *)pvPortMalloc(UCODE_FILE_MAX_SIZE);
//                configASSERT(gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_UCODE]);
//            }

//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_UCODE];
            break;

        /*下载系统块*/
        case MB_DOWNLOAD_SYS_BLOCK:
//            if(NULL == gtv_ModbusFileTrans[MB_DL_SYS_BLOCK])
//            {
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcv_Flag);
//                LOGD(TAG, "MB_DOWNLOAD_SYS_BLOCK, mcp_FileHandler = 0x%08X", gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcp_FileHandler);
//                gtv_ModbusFileTrans[MB_DL_SYS_BLOCK] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK] != NULL);

//                gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcp_FileHandler = (unsigned char *)pvPortMalloc(SYS_BLOCK_MAX_SIZE);
//                configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]);
//            }

//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_SYS_BLOCK];
            break;

        /*下载数据块*/
        case MB_DOWNLOAD_DATA_BLOCK:
            
//            if((NULL != gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]) && (gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcv_Flag & 0x2))
//            {
//                LOGI(TAG, "MB_DOWNLOAD_DATA_BLOCK, mcp_FileHandler = 0x%08X", gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler);
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcv_Flag);            
//                /*释放动态内存*/
//                vPortFree(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler);
//                gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler = NULL;
//                vPortFree(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]);
//                gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] = NULL;
//            }
//            if(NULL == gtv_ModbusFileTrans[MB_DL_DATA_BLOCK])
//            {
//                gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] != NULL);

//                gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler = (unsigned char *)pvPortMalloc(DATA_BLOCK_MAX_SIZE);
//                configASSERT(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]);
//            }
//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_DATA_BLOCK];
            break;

        /*下载POU INFO*/
        case MB_DOWNLOAD_POU_INFO:
            /*之前有接收完成，未固化文件。在线模式出现*/
//            if((NULL != gtv_ModbusFileTrans[MB_DL_POU_INFO]) && (gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcv_Flag & 0x2))
//            {
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcv_Flag);
//                LOGI(TAG, "MB_DOWNLOAD_POU_INFO, mcp_FileHandler = 0x%08X", gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler);
//                /*释放动态内存*/
//                vPortFree(gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler);
//                gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler = NULL;
//                vPortFree(gtv_ModbusFileTrans[MB_DL_POU_INFO]);
//                gtv_ModbusFileTrans[MB_DL_POU_INFO] = NULL;
//            }

//            if(NULL == gtv_ModbusFileTrans[MB_DL_POU_INFO])
//            {
//                gtv_ModbusFileTrans[MB_DL_POU_INFO] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_POU_INFO] != NULL);

//                gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler = (unsigned char *)pvPortMalloc(POU_FILE_MAX_SIZE);
//                configASSERT(gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_POU_INFO]);
//            }
//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_POU_INFO];
            break;

        /*下载全局变量表*/
        case MB_DOWNLOAD_GVT:
//            if(NULL == gtv_ModbusFileTrans[MB_DL_GVT])
//            {
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_GVT]->mcv_Flag);
//                gtv_ModbusFileTrans[MB_DL_GVT] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_GVT] != NULL);

//                gtv_ModbusFileTrans[MB_DL_GVT]->mcp_FileHandler = (unsigned char *)pvPortMalloc(GVT_FILE_MAX_SIZE);
//                configASSERT(gtv_ModbusFileTrans[MB_DL_GVT]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_GVT]);
//            }
//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_GVT];
            break;

        /*下载网络参数*/
        case MB_DOWNLOAD_NETCFG:
            if(NULL == gtv_ModbusFileTrans[MB_DL_NETCFG])
            {
//                printf("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_NETCFG]->mcv_Flag);
                gtv_ModbusFileTrans[MB_DL_NETCFG] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                LOGE("MB_DOWNLOAD_NETCFG", "mb_file_trans_st size is %d bytes", sizeof(mb_file_trans_st));
                //configASSERT(gtv_ModbusFileTrans[MB_DL_NETCFG] != NULL);

                gtv_ModbusFileTrans[MB_DL_NETCFG]->mcp_FileHandler = (unsigned char *)pvPortMalloc(1024);
                //configASSERT(gtv_ModbusFileTrans[MB_DL_NETCFG]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_NETCFG]);
                LOGE("MB_DOWNLOAD_NETCFG", "Free heap size is %d bytes", xPortGetFreeHeapSize());
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_NETCFG];
            break;

        /*下载PID参数1*/
        case MB_DOWNLOAD_PID1:
//            if(NULL == gtv_ModbusFileTrans[MB_DL_PID1])
//            {
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_PID1]->mcv_Flag);
//                gtv_ModbusFileTrans[MB_DL_PID1] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_PID1] != NULL);
//                LOGD(TAG, "MB_DOWNLOAD_PID1,Free heap = %u(bytes)", xPortGetFreeHeapSize());
//                gtv_ModbusFileTrans[MB_DL_PID1]->mcp_FileHandler = (unsigned char *)pvPortMalloc(PID1_MAX_SIZE);
//                configASSERT(gtv_ModbusFileTrans[MB_DL_PID1]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_PID1]);
//            }
//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_PID1];
            break;

        /*下载PID参数2*/
        case MB_DOWNLOAD_PID2:
//            if(NULL == gtv_ModbusFileTrans[MB_DL_PID2])
//            {
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_PID2]->mcv_Flag);
//                gtv_ModbusFileTrans[MB_DL_PID2] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_PID2] != NULL);
//                LOGD(TAG, "MB_DOWNLOAD_PID2,Free heap = %u(bytes)", xPortGetFreeHeapSize());
//                gtv_ModbusFileTrans[MB_DL_PID2]->mcp_FileHandler = (unsigned char *)pvPortMalloc(PID2_MAX_SIZE);
//                configASSERT(gtv_ModbusFileTrans[MB_DL_PID2]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_PID2]);
//            }
//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_PID2];
            break;
            
        /*系统升级*/
        case MB_DOWNLOAD_SYS_UPGRADE:
//            if (NULL == gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE])
//            {
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]->mcv_Flag);
//                gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE] != NULL);
//            #if  (0)
//                gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]->mcp_FileHandler = (unsigned char *)pvPortMalloc(SYSTEM_UPGRADE_MAX_SIZE);
//            #else
//                uint32_t nHeapSize = xPortGetFreeHeapSize();
//                gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]->mcp_FileHandler = (unsigned char *)pvPortMalloc(nHeapSize - 40*1024);
//            #endif
//                configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]);
//            }
//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE];

            break;

        /*PLC应用程序CBIN文件升级*/
        case MB_DOWNLOAD_PLC_PROGRAME_CBIN:
            
//            if (NULL == gtv_ModbusFileTrans[MB_DL_PLC_CBIN])
//            {
//                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_PLC_CBIN]->mcv_Flag);
//                hexdump(pMsg->mcp_ReceiveBuff,64); //92 1D 00 00是长度
//                //01 68 6A 01 43 53 52 46 92 1D 00 00 00 00 00 00 
//                //00 30 00 00 00 00 00 00 00 00 00 00 00 00 28 00 
//                //00 00 48 32 12 4F A3 79 37 9E 88 12 4F A3 77 37 
//                //9E 88 12 4F A3 77 37 CE B0 12 4F A3 77 37 9E 9C 
//                
//                gtv_ModbusFileTrans[MB_DL_PLC_CBIN] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
//                configASSERT(gtv_ModbusFileTrans[MB_DL_PLC_CBIN] != NULL);
//                //毛继科:20120416更改了CBIN的文件结构 
//                //最新的文件结构：头标记（四字节CSRF）+ 文件长度（4字节）+ 版本号（4字节）+ 产品型号（2字节）+ 保留12字节
//                ulongFileLength = GET_PU32_DATA(pMsg->mcp_ReceiveBuff+8);
//                PRINTF("cbinFilelength = %u\r\n", ulongFileLength);

//                gtv_ModbusFileTrans[MB_DL_PLC_CBIN]->mcp_FileHandler = (unsigned char *)pvPortMalloc(ulongFileLength);
//                configASSERT(gtv_ModbusFileTrans[MB_DL_PLC_CBIN]->mcp_FileHandler != NULL);

//                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_PLC_CBIN]);
//            }
//            ltv_pFile = gtv_ModbusFileTrans[MB_DL_PLC_CBIN];
            
            break;
    }
//    printf("Enter %s(), [2] = 0x%x, [3] = 0x%x, ", __func__, pMsg->mcp_ReceiveBuff[2], pMsg->mcp_ReceiveBuff[3]);
    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];
//    printf("mcv_FrameCnt = %d, mcv_PreFrame = %d, mcv_Flag = 0x%x\r\n", ltv_pFile->mcv_FrameCnt, ltv_pFile->mcv_PreFrame, ltv_pFile->mcv_Flag);
    lsv_Cnt = pMsg->msv_ReceiveLen - 6;

    if(ltv_pFile->mcv_Flag & 0x01) {
        if(lcv_FrameNum == ltv_pFile->mcv_PreFrame)
            return pdPASS;

        if(lcv_FrameNum == 0xFE || (lcv_FrameNum == (ltv_pFile->mcv_PreFrame + 1))) {
            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt ++;

            lcp_SrcBuff = &pMsg->mcp_ReceiveBuff[4];
            lcp_DestBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + ltv_pFile->mlv_FileLen);

            for(i=0; i<lsv_Cnt; i++) {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }

            ltv_pFile->mlv_FileLen += lsv_Cnt;

            if(lcv_FrameNum == 0xFE)
                ltv_pFile->mcv_Flag |= 0x02;

        } else if(lcv_FrameNum == 0x01 && ltv_pFile->mcv_PreFrame == 0xFD) {
            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt ++;

            /*记录帧号反转次数*/
            temp = ((((ltv_pFile->mcv_Flag & 0x1A) >> 2) +1) << 2);
            ltv_pFile->mcv_Flag &=0xE3;
            ltv_pFile->mcv_Flag |= temp;

            lcp_SrcBuff = &pMsg->mcp_ReceiveBuff[4];
            lcp_DestBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + ltv_pFile->mlv_FileLen);

            for(i=0; i<lsv_Cnt; i++) {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }

            ltv_pFile->mlv_FileLen += lsv_Cnt;

            if(lcv_FrameNum == 0xFE)
                ltv_pFile->mcv_Flag |= 0x02;

        } else {
            return pdFAIL;
        }

    } else {
        /*第一帧数据*/
        if(lcv_FrameNum == 0x01 || lcv_FrameNum == 0xFE) {
            ltv_pFile->mcv_Flag |= 0x01;
            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt = 1;

            lcp_SrcBuff = &pMsg->mcp_ReceiveBuff[4];
            lcp_DestBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + ltv_pFile->mlv_FileLen);

            for(i=0; i<lsv_Cnt; i++)
            {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }
            ltv_pFile->mlv_FileLen += lsv_Cnt;

            if(lcv_FrameNum == 0xFE)
            {
                ltv_pFile->mcv_Flag |= 0x02;
            }
        }
    }

//    /*在线模式处理*/
//    if(gtv_PlcRunStatus.mcv_IsOnlineProgram && (ltv_pFile->mcv_Flag & 0x02))
//    {
//        switch(pMsg->mcp_ReceiveBuff[2])
//        {
//            case MB_DOWNLOAD_UCODE:
//                lcv_Ret = plc_compiler_ucode(ltv_pFile->mcp_FileHandler);
//                if(lcv_Ret != pdPASS)
//                {
//                    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;
//                    /*错误信息处理*/
//                    plc_refresh_error_msg(ERR_COMPILER);
//                    return pdFAIL;
//                }
//                gtv_UserFilePtrSt.UCodePtr = ltv_pFile->mcp_FileHandler;
//                break;

//            case MB_DOWNLOAD_POU_INFO:
//                gtv_UserFilePtrSt.PouInfoPtr = ltv_pFile->mcp_FileHandler;
//                mb_slave_ctrl_run_cmd(pMsg);
//                break;
//        }
//    }

    return pdPASS;
}

/**
  * @brief  download 功能码处理函数
  * @param  None
  * @retval None
  */
void mb_slave_download_manage(md_slave_msg_pack *pMsg)
{
    LOGE("mb", "Enter %s\r\n", __func__);
    unsigned char ret;
    unsigned char i;

    /*判断帧号*/
    if(pMsg->mcp_ReceiveBuff[3] == 0x00 || pMsg->mcp_ReceiveBuff[3] >0xFE) {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
    }

    switch(pMsg->mcp_ReceiveBuff[2]) {
        /*监控位元件*/
        case MB_DOWNLOAD_MONITOR_BITS:
//            if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS) {
//                pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
//                mb_slave_error_resp(pMsg);
//                break;
//            }
//            mb_slave_download_monitor_bits(pMsg);
            break;
        /*监控字元件*/
        case MB_DOWNLOAD_MONITOR_WORDS:
//            if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS) {
//                pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
//                mb_slave_error_resp(pMsg);
//                break;
//            }
//            mb_slave_download_monitor_words(pMsg);
            break;
        /*监控位、字元件*/
        case MB_DOWNLOAD_MONITOR_BITS_WORDS:
//            if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS) {
//                pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
//                mb_slave_error_resp(pMsg);
//                break;
//            }

//            mb_slave_download_monitor_bits_words(pMsg);
//            break;

        /*下载UCODE*/
        case MB_DOWNLOAD_UCODE:
        /*下载系统块*/
        case MB_DOWNLOAD_SYS_BLOCK:
        /*下载数据块*/
        case MB_DOWNLOAD_DATA_BLOCK:
        /*下载POU INFO*/
        case MB_DOWNLOAD_POU_INFO:
        /*下载全局变量表*/
        case MB_DOWNLOAD_GVT:
        /*下载网络参数  */
        case MB_DOWNLOAD_NETCFG:
        /*下载PID参数1  */
        case MB_DOWNLOAD_PID1:
        /*下载PID参数2  */
        case MB_DOWNLOAD_PID2:
//            if(plc_get_password_check_result(DOWNLOAD_PASSWORD) != pdPASS) {
//                printf("check password ERROR!\r\n");
//                pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
//                mb_slave_error_resp(pMsg);
//                break;
//            }

//            suspend_task_when_download_ucode();
            if(gtv_DeviceConfigTable.isSupportSdram) {
                ret = mb_slave_download_file(pMsg);
            } else {
                ret = mb_slave_download_file(pMsg);
            }

            if(ret) {
                for(i=0; i<4; i++)
                    pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

                pMsg->msv_RespLen = 4;
                mb_slave_verify_resp_msg(pMsg);

            } else {
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
            }
            break;
        /*系统升级*/
        case MB_DOWNLOAD_SYS_UPGRADE:
        // 升级从站 01 68 69 
        case MB_DOWNLOAD_SLAVE:
        /*PLC应用程序CBIN文件升级*/
        case MB_DOWNLOAD_PLC_PROGRAME_CBIN:
//            if(gtv_PlcRunStatus.mcv_IsOnlineProgram) {
//                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_BUSY;
//                mb_slave_error_resp(pMsg);
//                break;
//            }

//            suspend_task_when_download_ucode();
//        #if (DAISY_MASTER_FEATURE == 1)
//            if (gSlaveIDUpgrade != 0)
//            {
//                ret = pdPASS;
//                daisy_LAN_send_bin(pMsg->mcp_ReceiveBuff, pMsg->msv_ReceiveLen);
//                EventBits_t uxBits = xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_UPGRADE_SLAVE, pdTRUE, pdFALSE, 2000);
//                if (( uxBits & KALYKE_EVENT_UPGRADE_SLAVE ) != 0)
//                {
//                    
//                    // 01 68 69 03 EE 55 00 00 00 00 00 00 00 00
//                    hexdump(gDaisyLANRecvBuffer, 16);
//                    if (gDaisyLANRecvBuffer[1] == 0x68)
//                    {
//                        ret = pdPASS;
//                    }
//                    else
//                    {
//                        ret = pdFAIL;
//                    }
//                }
//                else
//                {
//                    ret = pdFAIL;
//                    resume_task_after_download_ucode();
//                }
//            }
//            else
//        #endif
//            {
//                if(gtv_DeviceConfigTable.isSupportSdram)
//                {
//                    ret = mb_slave_download_file(pMsg);
//                }
//                else
//                {
//                    //ret = mb_slave_download_file_without_sdram(pMsg);
//                    ret = mb_slave_download_file(pMsg);
//                }
//            }

//            if (ret == pdPASS)
//            {
//                for(i=0; i<4; i++)
//                {
//                    pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
//                }
//                pMsg->msv_RespLen = 4;
//                mb_slave_verify_resp_msg(pMsg);

//            }
//            else
//            {
//                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
//                mb_slave_error_resp(pMsg);
//            }
            break;

        default:
            gtp_ModbusSlaveDiagInfo[pMsg->mcv_Sender].msv_SlaveErrCnt++;
            pMsg->mcv_ErrorCode = MB_ILIEGAL_CODE;
            mb_slave_error_resp(pMsg);
    }
}

/**
  * @brief  解析从站modbus协议
  * @param  *pMsg 数据指针
  * @retval None
  */
void mb_slave_msg_handler(md_slave_msg_pack *pMsg)
{
    LOGV("mb", "Enter %s\r\n", __func__);
    if (pMsg->msv_ReceiveLen < 5)
    {
        LOGE("mb", "msv_ReceiveLen(%u) < 5, so just RETURN!\r\n", pMsg->msv_ReceiveLen);
        if (pMsg->msv_ReceiveLen != 0)
        {
//            hexdump(pMsg->mcp_ReceiveBuff, pMsg->msv_ReceiveLen);
        }
        return;
    }

    if(checkRev_crc16(pMsg->mcp_ReceiveBuff, pMsg->msv_ReceiveLen) != true)
    {
        LOGE("mb", "CRC ERROR!");
        pMsg->mcv_ErrorCode = MB_ERROR_FRAME;
        mb_slave_error_resp(pMsg);
        return;
    }

    switch(pMsg->mcp_ReceiveBuff[1])
    {
    /*读寄存器值*/
    case MB_READ_HOLDING_REGISTER:
        mb_slave_read_holding_register(pMsg);
        break;
    /*写寄存器*/
    case MB_WRITE_REGISTER:
        mb_slave_write_register(pMsg);
        break;
    /*写多个寄存器*/
    case MB_WRITE_MULTIPLE_REGISTERS:
        mb_slave_write_multiple_registers(pMsg);
        break;

    /*下载*/
    case MB_DOWNLOWD_FUNC:
        mb_slave_download_manage(pMsg);
        break;
    /*上载*/
    case MB_UPLOAD_FUNC:
        //mb_slave_upload_manage(pMsg);
        break;
        
    /*系统复位*/
    case MB_RESET_REGISTERS:
        
        if(pMsg->mcp_ReceiveBuff[2] == 0x18)
        {
            memcpy(pMsg->mcp_RespBuff, pMsg->mcp_ReceiveBuff, 5);

            //sender is UART
            if ((pMsg->mcv_Sender == MB_SENDER_UART1) || (pMsg->mcv_Sender == MB_SENDER_UART2) ||
                    (pMsg->mcv_Sender == MB_SENDER_UART3) || (pMsg->mcv_Sender == MB_SENDER_UART4) )
            {
                if(pMsg->uart_resp_func != NULL)
                {
                    pMsg->msv_RespLen = 5;
                    pMsg->uart_resp_func(pMsg->mcp_RespBuff, pMsg->msv_RespLen);
                }
            }
            //sender is TCP
            else if (pMsg->mcv_Sender == MB_SENDER_TCP)
            {
                if (pMsg->tcp_resp_func != NULL)
                {
                    pMsg->msv_RespLen = 3;
                    pMsg->tcp_resp_func(pMsg->mcp_RespBuff, pMsg->msv_RespLen, pMsg->clientID);
                }
            }

            NVIC_SystemReset();
        }
        else
        {
            suspend_task_when_download_ucode();
            mb_slave_ctrl_manage(pMsg);
        }
        break;

    default:
        pMsg->mcv_ErrorCode = MB_ILIEGAL_CODE;
        mb_slave_error_resp(pMsg);
    }
    LOGV("mb", "Leave %s\r\n", __func__);
}



