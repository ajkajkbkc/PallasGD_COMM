
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

#include "app_log.h"
#include "app_tool.h"
#include "app_opts.h"
#include "hzudp.h"
#include "app_parameter.h"
#include "internet.h"



/* Private define ------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/
/* Definitions for hzudpTask */
osThreadId_t hzudpTaskHandle;
const osThreadAttr_t HzudpTask_attributes =
{
    .name = "hzudpTask",
    .priority = (osPriority_t) hzudpTaskPriority,
    .stack_size = 1024
};

#if   PROD_TYPE == PROD_FS || PROD_TYPE == PROD_FR
gw_hzudpcfg_st g_gw_hzudpcfg;
#elif PROD_TYPE == PROD_FL
gw_hzfludpcfg_st g_gw_hzfludpcfg;
#endif


uint32_t gHZLoopTimeTick = 0;
uint32_t gHZUpTimeTick = 0;
uint32_t gHZHeartTimeTick = 0;
uint32_t gHZResendTimeTick = 0;

//lltudp task use too
uint8_t gUDPErrorTimes = 0;
uint8_t gUdpRecvbuf[2048];
uint8_t g_udp_remote_ip[4];
uint16_t g_udp_remote_port;
int32_t g_udp_status;


/* Private function prototypes -----------------------------------------------*/
void HzudpTask(void *argument);

#if   PROD_TYPE == PROD_FS || PROD_TYPE == PROD_FR
/* GW HZUDP protocol -------------------------------------------------------- */
void hzudp_send_cmd(unsigned char cmd);
uint8_t is_hzudp_protocol(unsigned char *ptr, unsigned short len);
void hzudp_parse_config(unsigned char *ptr, unsigned short len);
void gwhzudp_parse_rtudata(void);
void gw_hzudp_init(void);
#elif PROD_FL
/* GW HZFLUDP protocol -------------------------------------------------------- */
void hzfludp_send_cmd(unsigned short data_tpye);
uint8_t is_hzfludp_protocol(unsigned char *ptr, unsigned short len);
void hzfludp_parse_config(unsigned char *ptr, unsigned short len);
void gwhzfludp_parse_rtudata(void);
void gw_hzfludp_init(void);
#endif

/* Private user code ---------------------------------------------------------*/
/* ------------------------------------ udp option -------------------------------------- */
void udp_send_buffer(unsigned char *pBuff, unsigned short len)
{
    g_udp_status = sendto(APP_SOCKET_HUAZIUDP, pBuff, len, g_udp_remote_ip, g_udp_remote_port);
}

void check_udp_status(void)
{
    static uint32_t s_tick = 0;

    if(gParam.st.SecCnt - s_tick >= UDP_CHECK_TIME)
    {
        s_tick = gParam.st.SecCnt;
        LOGE("Huaziudp", "UDP_Status = %d", g_udp_status);
        if(g_udp_status < 0)  //异常
        {
            gUDPErrorTimes++;
            disconnect(APP_SOCKET_HUAZIUDP);
            PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_UDP);
            LOGE("Huaziudp", "UDP error, now error times is %d", gUDPErrorTimes);
        }
        else
        {
            if(gUDPErrorTimes != 0)
            {
                gUDPErrorTimes = 0;
            }
            PAR_CLEAR_BIT(gParam.st.NetLink_State, NET_STATE_UDP);
        }
    }
}

/**
  * @brief  UDP接收数据解析
  * @param  数据(*pUDPBuf)
  * @param  长度(len)
  * @retval None
  */
void handle_hzudp_recv_data(uint8_t *pUDPBuf, uint16_t len)
{
    LOGD("Huaziudp", "Enter %s(), pUDPBuf = 0x%08X, len = %d", __func__, *pUDPBuf, len);

#if   PROD_TYPE == PROD_FS || PROD_TYPE == PROD_FR
    if(is_hzudp_protocol(pUDPBuf, len))
    {
        hzudp_parse_config(pUDPBuf, len);
    }
#elif PROD_TYPE == PROD_FL
    if(is_hzfludp_protocol(pUDPBuf, len))
    {
        hzfludp_parse_config(pUDPBuf, len);
    }
#endif
}


#if   PROD_TYPE == PROD_FS || PROD_TYPE == PROD_FR
/* GW HZUDP protocol -------------------------------------------------------- */
/**
 * @brief  华咨UDP协议发送
 * @param  cmd HZUDP_H_ASKCFG 请求配置
               HZUDP_H_HAERT  心跳包
               HZUDP_H_DATA   数据包
 * @retval
 */
void hzudp_send_cmd(unsigned char cmd)
{
    LOGV("HuaziUDP", "Enter %s()", __func__);

    uint8_t sendlen, sendbuf[50];

    sendlen = 0;
    sendbuf[sendlen++] = cmd;

    if(cmd == HZUDP_H_DATA)
    {
#if  PROD_TYPE == PROD_FS
        sendbuf[sendlen++] = (uint8_t)(gFlashParam.st.idNum >> 8);
        sendbuf[sendlen++] = (uint8_t)gFlashParam.st.idNum;

        sendbuf[sendlen++] = 30; //RtuPNum_Table[TYPE_DESA] * 2
        for(uint8_t i = 0; i < 15; i++)  //RtuPNum_Table[TYPE_DESA]
        {
            sendbuf[sendlen++] = (uint8_t)(gFS_Elem.lsv_buff[i] >> 8);
            sendbuf[sendlen++] = (uint8_t)gFS_Elem.lsv_buff[i];
        }
#elif PROD_TYPE == PROD_FR
        sendbuf[sendlen++] = (uint8_t)(gFlashParam.st.idNum >> 8);
        sendbuf[sendlen++] = (uint8_t)gFlashParam.st.idNum;

        sendbuf[sendlen++] = 10; //RtuPNum_Table[TYPE_G1A3] * 2

        sendbuf[sendlen++] = (uint8_t)(gFlashParam.st.GResGetTimes >> 8);
        sendbuf[sendlen++] = (uint8_t)gFlashParam.st.GResGetTimes;
        sendbuf[sendlen++] = (uint8_t)(gFlashParam.st.GResGetInter >> 8);
        sendbuf[sendlen++] = (uint8_t)gFlashParam.st.GResGetInter;
        sendbuf[sendlen++] = (uint8_t)(gFlashParam.st.GResVal >> 8);
        sendbuf[sendlen++] = (uint8_t)gFlashParam.st.GResVal;
        sendbuf[sendlen++] = 0xFF;
        sendbuf[sendlen++] = 0xFF;
        sendbuf[sendlen++] = 0xFF;
        sendbuf[sendlen++] = 0xFF;
#endif
    }
    else
    {
        sendbuf[sendlen++] = (uint8_t)(gFlashParam.st.idNum >> 8);
        sendbuf[sendlen++] = (uint8_t)gFlashParam.st.idNum;
    }

    sendbuf[sendlen++] = HZUDP_T_ALL;

    g_gw_hzudpcfg.udp_send_func(sendbuf, sendlen);
}

/**
  * @brief  判断是否是华咨UDP协议
  * @param  *ptr 数据帧
  * @param  len 数据长度
  * @retval 0：不是
            1：是无配置信息
            2：是有配置信息
  */
uint8_t is_hzudp_protocol(unsigned char *ptr, unsigned short len)
{
    if( (ptr[0] != HZUDP_H_GETCFG) || (ptr[len - 1] != HZUDP_T_ALL) )
    {
        LOGD("HuaiziUDP", "Its not HuaziUDP protocol");
        return 0;
    }

    if( ptr[len - 2] == 0xEE )
    {
        if(GET_BIGPU16_DATA(ptr + 1) != gFlashParam.st.idNum)
        {
            LOGD("HuaiziUDP", "Its noconfig protocol, but adr err");

            return 0;
        }

        LOGD("HuaiziUDP", "Its noconfig protocol");
        return 1;
    }

    uint16_t data_len = GET_BIGPU16_DATA(ptr + 10);
    uint16_t cks1_len = (ptr[12] * 3) + 1;
    uint16_t cks2_len = len - 13;
    LOGD("HuaiziUDP", "data_len = %d, cks1_len = %d, cks2_len = %d", data_len, cks1_len, cks2_len);

    if( (data_len != cks1_len) || (data_len != cks2_len) )
    {
        return 0;
    }

    return 2;
}

/**
  * @brief  解析华咨UDP协议
  * @param  *ptr 数据帧
  * @param  len 数据长度
  * @retval
  */
void hzudp_parse_config(unsigned char *ptr, unsigned short len)
{
    LOGV("HuaziUDP", "Enter %s()", __func__);
    uint8_t *ptemp;

    ptemp = ptr;
//    hexdump(ptemp, len);

    if( ptemp[len - 2] == 0xEE )
    {
        LOGD("HuaiziUDP", "Its noconfig protocol");

        /* 响应 */
        *ptr = HZUDP_H_ERRCFG;
        g_gw_hzudpcfg.udp_send_func(ptr, len);
        g_gw_hzudpcfg.OkCfg = 0;

        return;
    }

    ptemp++;
    g_gw_hzudpcfg.LoopTime = GET_BIGPU16_DATA(ptemp);
    LOGD("HuaziUDP", "g_gw_hzudpcfg.LoopTime = %d", g_gw_hzudpcfg.LoopTime);

    ptemp += 2;
    g_gw_hzudpcfg.UpTime = GET_BIGPU16_DATA(ptemp);
    LOGD("HuaziUDP", "g_gw_hzudpcfg.UpTime = %d", g_gw_hzudpcfg.UpTime);

    ptemp += 2;
    g_gw_hzudpcfg.TmpThre = *ptemp;
    LOGD("HuaziUDP", "g_gw_hzudpcfg.TmpThre = %d", g_gw_hzudpcfg.TmpThre);

    ptemp++;
    g_gw_hzudpcfg.CurThre = *ptemp;
    LOGD("HuaziUDP", "g_gw_hzudpcfg.CurThre = %d", g_gw_hzudpcfg.CurThre);

    ptemp++;
    g_gw_hzudpcfg.ResThre = *ptemp;
    LOGD("HuaziUDP", "g_gw_hzudpcfg.ResThre = %d", g_gw_hzudpcfg.ResThre);

    ptemp++;
    g_gw_hzudpcfg.HeartTime = *ptemp;
    LOGD("HuaziUDP", "g_gw_usercfg.HeartTime = %d", g_gw_hzudpcfg.HeartTime);

    ptemp++;
    g_gw_hzudpcfg.ResendTime = *ptemp;
    LOGD("HuaziUDP", "g_gw_hzudpcfg.ResendTime = %d", g_gw_hzudpcfg.ResendTime);

    /* 避免周期误设为0，导致死机 */
    if(g_gw_hzudpcfg.LoopTime < 1)
    {
        g_gw_hzudpcfg.LoopTime = 1;
    }
    if(g_gw_hzudpcfg.UpTime < 1)
    {
        g_gw_hzudpcfg.UpTime = 1;
    }
    if(g_gw_hzudpcfg.HeartTime < 1)
    {
        g_gw_hzudpcfg.HeartTime = 1;
    }
    if(g_gw_hzudpcfg.ResendTime < 1)
    {
        g_gw_hzudpcfg.ResendTime = 1;
    }

    /* 响应配置成功 */
    *ptr = HZUDP_H_OKCFG;
    g_gw_hzudpcfg.udp_send_func(ptr, len);
    g_gw_hzudpcfg.OkCfg = 1;
}

/**
* @brief  解析RTU的数据
* @param  num No.num RTU的数据
* @retval
*/
void gwhzudp_parse_rtudata(void)
{
#if  PROD_TYPE == PROD_FS
    static uint16_t Last_YL, Last_KL, Last_LtNum, Last_Pe;

    if( (Last_YL != gFS_Elem.st.YL) || (Last_KL != gFS_Elem.st.KL) || (Last_LtNum != gFS_Elem.st.LtNum) || (Last_Pe != gFS_Elem.st.Pe) )    //遥信(SPD)、空开、雷击计数或接地状态发生变化
    {
        Last_YL = gFS_Elem.st.YL;
        Last_KL = gFS_Elem.st.KL;
        Last_LtNum = gFS_Elem.st.LtNum;
        Last_Pe = gFS_Elem.st.Pe;
        g_gw_hzudpcfg.Almflg = 1;
    }
    else if( (g_gw_hzudpcfg.TmpThre * 10 < gFS_Elem.st.Tmp1) || (g_gw_hzudpcfg.TmpThre * 10  < gFS_Elem.st.Tmp2) )//温度大于阈值
    {
        g_gw_hzudpcfg.Almflg = 1;
    }
    else if( (g_gw_hzudpcfg.CurThre * 10000 < gFS_Elem.st.CurA) || (g_gw_hzudpcfg.CurThre * 10000 < gFS_Elem.st.CurB) || \
             (g_gw_hzudpcfg.CurThre * 10000 < gFS_Elem.st.CurC) )//电流大于阈值
    {
        g_gw_hzudpcfg.Almflg = 1;
    }
    else if( (3800 < gFS_Elem.st.VolA) || (3800 < gFS_Elem.st.VolB) || (3800 < gFS_Elem.st.VolC) )   //电压大于380V
    {
        g_gw_hzudpcfg.Almflg = 1;
    }
#elif PROD_TYPE == PROD_FR
    if( g_gw_hzudpcfg.ResThre * 100 < gFlashParam.st.GResVal)//地阻大于阈值
    {
        g_gw_hzudpcfg.Almflg = 1;
    }
#endif
}

void gw_hzudp_init(void)
{
    memcpy(g_udp_remote_ip, gFlashParam.st.s0TargetIP, 4);
    g_udp_remote_port = gFlashParam.st.s0TargetPort;

    g_gw_hzudpcfg.LoopTime   = 0x0001;
    g_gw_hzudpcfg.UpTime     = 0x000A;
    g_gw_hzudpcfg.TmpThre    = 0xFF;
    g_gw_hzudpcfg.CurThre    = 0xFF;
    g_gw_hzudpcfg.ResThre    = 0xFF;
    g_gw_hzudpcfg.HeartTime  = 0x0F;
    g_gw_hzudpcfg.ResendTime = 0x05;

    g_gw_hzudpcfg.OkCfg      = 0;   //0:nocfg    1:okcfg
    g_gw_hzudpcfg.Almflg     = 0;   //0:normal   1:alarm

    g_gw_hzudpcfg.udp_send_func = udp_send_buffer;
}
#elif PROD_TYPE == PROD_FL
/* GW HZFLUDP protocol -------------------------------------------------------- */
/**
 * @brief  华咨UDP雷电流协议发送
 * @param  cmd HZFLUDP_DTYPE_HEART 心跳包
               HZFLUDP_DTYPE_1     数据包1（暂无作用）
               HZFLUDP_DTYPE_3     数据包3
 * @retval
 */
void hzfludp_send_cmd(unsigned short data_tpye)
{
    LOGV("HuaziUDP", "Enter %s()", __func__);

    uint8_t sendlen, sendbuf[50];
    uint8_t i;
    uint8_t lcv_verify = 0;
    uint16_t lsv_temp;

    sendlen = 0;
    sendbuf[sendlen++] = HZFLUDP_H_DATA;

    memcpy(sendbuf + sendlen, gFlashParam.st.idInfo, 20);
    sendlen += 20;

    sendbuf[sendlen++] = HZFLUDP_TYPE;
    sendbuf[sendlen++] = HZFLUDP_MODEL;
    sendbuf[sendlen++] = (uint8_t)(data_tpye >> 8);
    sendbuf[sendlen++] = (uint8_t)data_tpye;
    sendbuf[sendlen++] = HZFLUDP_VERSION;

    if(data_tpye == HZFLUDP_DTYPE_HEART)
    {
        //长度
        sendbuf[sendlen++] = 0;
        sendbuf[sendlen++] = 0;
        //数据
        /* 无数据 */
        //校验:累加和校验(只校验数据位)
        sendbuf[sendlen++] = 0;
    }
    else if(data_tpye == HZFLUDP_DTYPE_3)
    {
        //长度
        sendbuf[sendlen++] = 0;
        sendbuf[sendlen++] = 11;
        //数据
        /*次数*/
        sendbuf[sendlen++] = gLtList.st.length >> 8;
        sendbuf[sendlen++] = gLtList.st.length & 0xFF;
        /*峰值*/
        i = gLtList.st.length < 1 ? 1 : gLtList.st.length;
        lsv_temp = gLtList.st.LtData[i - 1].Peak;
        sendbuf[sendlen++] = lsv_temp / 10;    //高位存峰值整数
        sendbuf[sendlen++] = lsv_temp % 10;    //低位存峰值小数
        /*时间*/
        sendbuf[sendlen++] = 20;
        sendbuf[sendlen++] = gLtList.st.LtData[i - 1].Year;
        sendbuf[sendlen++] = gLtList.st.LtData[i - 1].Month;
        sendbuf[sendlen++] = gLtList.st.LtData[i - 1].Day;
        sendbuf[sendlen++] = gLtList.st.LtData[i - 1].Hour;
        sendbuf[sendlen++] = gLtList.st.LtData[i - 1].Minute;
        sendbuf[sendlen++] = gLtList.st.LtData[i - 1].Second;
        //校验:累加和校验(只校验数据位)
        for(i = 28; i <= 38; i++)           //校验:累加和校验(只校验数据位)
        {
            lcv_verify += sendbuf[i];
        }
        sendbuf[sendlen++] = lcv_verify;
    }

    sendbuf[sendlen++] = HZFLUDP_T_DATA;

    g_gw_hzfludpcfg.udp_send_func(sendbuf, sendlen);
}

uint8_t is_hzfludp_protocol(unsigned char *ptr, unsigned short len)
{
    if(len != 31)
    {
        LOGD("HuaiziUDP", "Len err, Its not HuaziFLUDP protocol");
        return 0;
    }

    if( (ptr[0] != HZFLUDP_H_DATA) || (ptr[len - 1] != HZFLUDP_T_DATA) )
    {
        LOGD("HuaiziUDP", "Head or Tail err, Its not HuaziFLUDP protocol");
        return 0;
    }

    if(ptr[len - 2] != ptr[len - 3])  //求和校验不一致
    {
        LOGD("HuaiziUDP", "SumVerify err, Its not HuaziFLUDP protocol");
        return 0;
    }

    return 1;
}

/**
  * @brief  解析华咨雷电流UDP协议
  * @param  *ptr 数据帧
  * @param  len 数据长度
  * @retval
  */
void hzfludp_parse_config(unsigned char *ptr, unsigned short len)
{
    LOGV("HuaziUDP", "Enter %s()", __func__);

    uint8_t lcv_data = ptr[len - 3];
    if(lcv_data == 00)
    {
        gHZResendTimeTick = 0;
        LOGD("Huaziudp", "L1AI Back Secess!!");
    }
    else if(lcv_data == 01)
    {
        gHZResendTimeTick = gParam.st.SecCnt;  //重新计时
        LOGE("Huaziudp", "The total length does not meet the minimum length requirement.");
    }
    else if(lcv_data == 02)
    {
        gHZResendTimeTick = gParam.st.SecCnt;
        LOGE("Huaziudp", "Data area not found.");
    }
    else if(lcv_data == 03)
    {
        gHZResendTimeTick = gParam.st.SecCnt;
        LOGE("Huaziudp", "No match was found for the corresponding data type.");
    }
    else if(lcv_data == 04)
    {
        gHZResendTimeTick = gParam.st.SecCnt;
        LOGE("Huaziudp", "Data length error.");
    }
    else if(lcv_data == 05)
    {
        gHZResendTimeTick = gParam.st.SecCnt;
        LOGE("Huaziudp", "Checksum error.");
    }
    else
    {
        gHZResendTimeTick = gParam.st.SecCnt;
        LOGE("Huaziudp", "Unkown data.");
    }
}

/**
* @brief  解析RTU的数据
* @param  num No.num RTU的数据
* @retval
*/
void gwhzfludp_parse_rtudata(void)
{
    static uint16_t Last_LtNum = 0xFFFF;

    if(Last_LtNum != gLtList.st.length) //雷击次数发生变化
    {
        LOGW("HuaziUDP", "NEW thunder Info, last_LtTimes = %d, new_LtTimes = %d", Last_LtNum, gLtList.st.length);
        Last_LtNum = gLtList.st.length;
        g_gw_hzfludpcfg.Almflg = 1;
    }
}

void gw_hzfludp_init(void)
{
    g_gw_hzfludpcfg.Almflg     = 0;   //0:normal   1:alarm
}
#endif

/* ---------------------------------------- ---------------------------------- ----------------------------------------------- */
/**
  * @brief  udp参数初始化
  * @param  None
  * @retval None
  */
void udpparam_init(void)
{
#if   PROD_TYPE == PROD_FS || PROD_TYPE == PROD_FR
    gw_hzudp_init();
#elif PROD_TYPE == PROD_FL
    gw_hzfludp_init();
#endif
}

/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_HuaziudpTask(void)
{
    hzudpTaskHandle = osThreadNew(HzudpTask, NULL, &HzudpTask_attributes);
}

/**
  * @brief  Function implementing the HuaziudpTask thread.
  * @param  argument: Not used
  * @retval None
  */
void HzudpTask(void *argument)
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
                handle_hzudp_recv_data(gUdpRecvbuf, len);
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

#if   PROD_TYPE == PROD_FS || PROD_TYPE == PROD_FR
        if(g_gw_hzudpcfg.OkCfg == 0)
        {
            if(gParam.st.SecCnt - gHZResendTimeTick > g_gw_hzudpcfg.ResendTime)
            {
                gHZResendTimeTick = gParam.st.SecCnt;
                hzudp_send_cmd(HZUDP_H_ASKCFG);
            }
        }
        else if( (gParam.st.SecCnt - gHZUpTimeTick > g_gw_hzudpcfg.UpTime) || (gHZUpTimeTick == 0) )
        {
            gHZUpTimeTick = gParam.st.SecCnt;
            gHZLoopTimeTick = gParam.st.SecCnt; //定时发送后不需要立即报警发送
            hzudp_send_cmd(HZUDP_H_DATA);
            g_gw_hzudpcfg.Almflg = 0;
        }
        else if(gParam.st.SecCnt - gHZLoopTimeTick > g_gw_hzudpcfg.LoopTime)
        {
            gHZLoopTimeTick = gParam.st.SecCnt;
            gwhzudp_parse_rtudata();
            if(g_gw_hzudpcfg.Almflg == 1)
            {
                hzudp_send_cmd(HZUDP_H_DATA);
                g_gw_hzudpcfg.Almflg = 0;
            }
        }
        else if(gParam.st.SecCnt - gHZHeartTimeTick > g_gw_hzudpcfg.HeartTime)
        {
            gHZHeartTimeTick = gParam.st.SecCnt;
            hzudp_send_cmd(HZUDP_H_HAERT);
        }
#elif  PROD_TYPE == PROD_FL
        if(gHZResendTimeTick != 0)  //未收到服务器反馈响应
        {
            if(gParam.st.SecCnt - gHZResendTimeTick >= 20)
            {
                LOGE("Huaziudp", "No feedback response, send again !");
                gHZResendTimeTick = gParam.st.SecCnt; //重新计时
                hzfludp_send_cmd(HZFLUDP_DTYPE_3);
            }
        }
        else if( (gParam.st.SecCnt - gHZHeartTimeTick >= 7200) || (gHZHeartTimeTick == 0) ) //雷电信息监测设备以2小时为周期发送一帧心跳数据包
        {
            gHZHeartTimeTick = gParam.st.SecCnt;
            hzfludp_send_cmd(HZFLUDP_DTYPE_HEART);
        }
        else
        {
            if(g_gw_hzfludpcfg.Almflg != 0)
            {
                hzfludp_send_cmd(HZFLUDP_DTYPE_3);
                g_gw_hzfludpcfg.Almflg = 0;
                gHZResendTimeTick = gParam.st.SecCnt;
                LOGW("Huaziudp", "LtNum is change! LtNum is %d", gLtList.st.length);
            }
        }

        if(gParam.st.SecCnt - gHZLoopTimeTick > 5) //x秒获取雷电流数据
        {
            gHZLoopTimeTick = gParam.st.SecCnt;
            gwhzfludp_parse_rtudata();
        }
#endif

        check_udp_status();

        W5500MutexUnlock();

    }
}



