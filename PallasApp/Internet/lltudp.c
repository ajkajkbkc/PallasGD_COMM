
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

#include "app_log.h"
#include "app_tool.h"
#include "app_parameter.h"
#include "mb_crc.h"

#include "internet.h"
#include "hzudp.h"
#include "lltudp.h"



/* Private define ------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/
/* Definitions for lltudpTask */
osThreadId_t lltudpTaskHandle;
const osThreadAttr_t lltudpTask_attributes =
{
    .name = "lltudpTask",
    .priority = (osPriority_t) lltudpTaskPriority,
    .stack_size = 1024
};




/* Private function prototypes -----------------------------------------------*/
void lltudpTask(void *argument);

uint8_t is_lltudp_protocol(unsigned char *ptr, unsigned short len);
void lltudp_parse_config(unsigned char *ptr, unsigned short len);

/* Private user code ---------------------------------------------------------*/
/**
  * @brief  UDP接收数据解析
  * @param  数据(*pUDPBuf)
  * @param  长度(len)
  * @retval None
  */
void handle_lltudp_recv_data(uint8_t *pUDPBuf, uint16_t len)
{
    LOGD("Huaziudp", "Enter %s(), pUDPBuf = 0x%08X, len = %d", __func__, *pUDPBuf, len);

    if(is_lltudp_protocol(pUDPBuf, len))
    {
        lltudp_parse_config(pUDPBuf, len);
    }
}

/* LLTUDP protocol -------------------------------------------------------- */
/**
  * @brief  判断是否是路路通UDP协议
  * @param  *ptr 数据帧
  * @param  len 数据长度
  * @retval 0：不是
            1：是
  */
uint8_t is_lltudp_protocol(unsigned char *ptr, unsigned short len)
{
    if(len != GET_PU32_DATA(ptr + 8)) //帧长
    {
        LOGE("lltudp", "ERROR LENGTH!!");
        return 0;
    }

    if(gFlashParam.st.idNum != GET_PU32_DATA(ptr + 12) ) //模块地址
    {
        LOGE("lltudp", "ERROR ADRESS!!");
        return 0;
    }

#if   PROD_TYPE == PROD_FS
    if( (gFlashParam.st.Prod_Param & PARAM_XX) != PARAM_DES4)
    {
        if(LLTUDP_TYPE_JCII != GET_PU32_DATA(ptr + 16))
        {
            LOGE("lltudp", "ERROR DEVICE TYPE!!");
            return 0;
        }
    }
    else
    {
        if(LLTUDP_TYPE_ZJCII != GET_PU32_DATA(ptr + 16))
        {
            LOGE("lltudp", "ERROR DEVICE TYPE!!");
            return 0;
        }
    }
#elif PROD_TYPE == PROD_FR
    if(LLTUDP_TYPE_JDII != GET_PU32_DATA(ptr + 16))
    {
        LOGE("lltudp", "ERROR DEVICE TYPE!!");
        return 0;
    }
#endif

    return 1;
}

/**
  * @brief  解析路路通UDP协议
  * @param  *ptr 数据帧
  * @param  len 数据长度
  * @retval
  */
void lltudp_parse_config(unsigned char *ptr, unsigned short len)
{
    LOGV("lltudp", "Enter %s()", __func__);

    uint8_t sendlen;

    sendlen = 4;
    *((uint32_t *)(ptr + sendlen)) = LLTUDP_VERSION;  //version

    sendlen += 4;
    *((uint32_t *)(ptr + sendlen)) = 46;  //return frame length

#if   PROD_TYPE == PROD_FS
    sendlen += 4;
    memcpy(ptr + sendlen, &gFS_Elem, 30);

    sendlen += 30;
#elif PROD_TYPE == PROD_FR
    sendlen += 4;
    *((uint16_t *)(ptr + sendlen)) = gFlashParam.st.GResGetTimes;
    sendlen += 2;
    *((uint16_t *)(ptr + sendlen)) = gFlashParam.st.GResGetInter;
    sendlen += 2;
    *((uint16_t *)(ptr + sendlen)) = gFlashParam.st.GResVal;
    sendlen += 2;
    *((uint16_t *)(ptr + sendlen)) = 0xFFFF;
    sendlen += 2;
    *((uint16_t *)(ptr + sendlen)) = 0xFFFF;

    sendlen += 10;
#endif
    *((uint32_t *)(ptr + sendlen)) = calc_crc16(ptr, sendlen); //crc

    sendlen += 4;
    udp_send_buffer(ptr, sendlen);
}

/* ------------------------------------------------------------------------- */
/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_lltudpTask(void)
{
    lltudpTaskHandle = osThreadNew(lltudpTask, NULL, &lltudpTask_attributes);
}

/**
  * @brief  Function implementing the HuaziudpTask thread.
  * @param  argument: Not used
  * @retval None
  */
void lltudpTask(void *argument)
{
    LOGD("Huaziudp", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());
    uint8_t sn = APP_SOCKET_HUAZIUDP;
    uint16_t len;

    osDelay(10000);  //10s后
    for(;;)
    {
        osDelay(100);

        if((gParam.st.NetLink_State & NET_STATE_OK) != 0)
        {
            PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_UDP);
            continue ;
        }

        W5500MutexLock();

        //LOGW("Huaziudp", "getSn_SR(sn) = 0x%02x", getSn_SR(sn));
        switch(getSn_SR(sn))
        {
        case SOCK_UDP:
            if(getSn_IR(sn) & Sn_IR_RECV)
            {
                setSn_IR(sn, Sn_IR_RECV);
            }

            if((len = getSn_RX_RSR(sn)) > 0)
            {
                LOGI("Huaziudp", "Recv length: %u", len);
                memset(gUdpRecvbuf, 0, len + 1);
                g_udp_status = recvfrom(sn, gUdpRecvbuf, len, g_udp_remote_ip, &g_udp_remote_port);
                LOGV("Huaziudp", "Recv: %s\r\n", gUdpRecvbuf);
//                hexdump(gUdpRecvbuf, len);
                //UDP_SendStatus = sendto(sn, gUdpRecvbuf, len, udp_remote_ip, udp_remote_port);
                handle_lltudp_recv_data(gUdpRecvbuf, len);
            }
            break;

        case SOCK_CLOSED:
            if (socket(sn, Sn_MR_UDP, gFlashParam.st.s0LocalPort, 0) == 1)
            {
            }
            else
            {
                PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_UDP);
            }
            break;
        }



        check_udp_status();

        W5500MutexUnlock();

    }
}



