
/* Private includes ----------------------------------------------------------*/
#include "cmsis_os.h"
#include <string.h>

#include "main.h"
#include "socket.h"
#include "app_log.h"
#include "app_parameter.h"
#include "app_opts.h"
#include "app_rtc.h"
#include "app_tool.h"
#include "app_led.h"

#include "internet.h"
#include "mbtcp.h"
#include "mqtt.h"
#include "hzudp.h"
#include "lltudp.h"


/* Private define ------------------------------------------------------------*/
/* 0：使用FreeRTOS原API  1：使用封装后的API */
#define  USE_CMSIS_OS_API   1   /* 使用原API会死机（上电后，建立mbtcp连接去读数据，会死机进入 configASSERT() ），使用封装后的API不会 */
#if (USE_CMSIS_OS_API == 0)
#include "semphr.h"
#endif


/* Private variables ---------------------------------------------------------*/
/* Definitions for mbTcpTask */
osThreadId_t internetTaskHandle;
const osThreadAttr_t internet_attributes =
{
    .name = "internetTask",
    .priority = (osPriority_t) internetTaskPriority,
    .stack_size = 1024
};

unsigned char W5500FifoSize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2,}, {2, 2, 2, 2, 2, 2, 2, 2}};

#if (USE_CMSIS_OS_API == 0)
SemaphoreHandle_t gW5500_MuxSem = NULL;
#else
osMutexDef( gW5500MutexDef );
osMutexId gW5500MutexId;
#endif


/* Private function prototypes -----------------------------------------------*/
void InternetTask(void *argument);


/* Private user code ---------------------------------------------------------*/
/* W5500 port -------------------------------------------------------- */

void W5500WriteByte(unsigned char byte)
{
    HAL_SPI_Transmit(&hspi1, &byte, 1, HAL_MAX_DELAY);
}

unsigned char W5500ReadByte(void)
{
    unsigned char recvbuf;

    HAL_SPI_Receive(&hspi1, &recvbuf, 1, HAL_MAX_DELAY);

    return recvbuf;
}

void W5500Select(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}

void W5500DeSelect(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  W5500硬件接口初始化
  * @param  None
  * @retval None
  */
static void W5500_PortInitialize(void)
{
    /* spi function register */
    reg_wizchip_spi_cbfunc(W5500ReadByte, W5500WriteByte);

    /* CS function register */
    reg_wizchip_cs_cbfunc(W5500Select, W5500DeSelect);

}

/**
  * @brief  W5500硬件复位
  * @param  None
  * @retval None
  */
void W5500_HardwareReset(void)
{
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    osDelay(1);
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    osDelay(1600);

    gFlashParam.st.W5500ResetCnt++;
    //Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
    LOGW("internet", "W5500Hardware Reset!!!!!!!");
}

/**
  * @brief  打印本机地址等信息
  * @param  None
  * @retval None
  */
static void print_network_information(wiz_NetInfo *netInfo)
{
    wizchip_getnetinfo(netInfo);
    LOGD("internet", "Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n\r", netInfo->mac[0], netInfo->mac[1], netInfo->mac[2], netInfo->mac[3], netInfo->mac[4], netInfo->mac[5]);
    LOGD("internet", "IP address : %d.%d.%d.%d\n\r", netInfo->ip[0], netInfo->ip[1], netInfo->ip[2], netInfo->ip[3]);
    LOGD("internet", "SM Mask    : %d.%d.%d.%d\n\r", netInfo->sn[0], netInfo->sn[1], netInfo->sn[2], netInfo->sn[3]);
    LOGD("internet", "Gate way   : %d.%d.%d.%d\n\r", netInfo->gw[0], netInfo->gw[1], netInfo->gw[2], netInfo->gw[3]);
    LOGD("internet", "DNS Server : %d.%d.%d.%d\n\r", netInfo->dns[0], netInfo->dns[1], netInfo->dns[2], netInfo->dns[3]);
}

/**
  * @brief  配置本机地址等信息
  * @param  None
  * @retval true or false
  */
bool W5500_ConfigInfo(void)
{
    wiz_NetInfo wizNETINFO, GetwizNETINFO;

    memcpy(wizNETINFO.mac, gFlashParam.st.macAddr, 6);
    memcpy(wizNETINFO.ip, gFlashParam.st.localIP, 4);
    memcpy(wizNETINFO.sn, gFlashParam.st.maskIP, 4);
    memcpy(wizNETINFO.gw, gFlashParam.st.gatewayIP, 4);
    memcpy(wizNETINFO.dns, gFlashParam.st.DnsServerIP, 4);

    wizNETINFO.dhcp = NETINFO_STATIC;

    ctlnetwork(CN_SET_NETINFO, (void *)&wizNETINFO);

    ctlwizchip(CW_INIT_WIZCHIP, (void *)W5500FifoSize); //Init. TX & RX Memory size of w5500

    wiz_NetTimeout netTimeout = {3, 2000};
    ctlnetwork(CN_SET_TIMEOUT, (void *)&netTimeout);

    print_network_information(&GetwizNETINFO);
    if (memcmp(wizNETINFO.mac, GetwizNETINFO.mac, 6) ||
            memcmp(wizNETINFO.ip,  GetwizNETINFO.ip, 4) ||
            memcmp(wizNETINFO.sn,  GetwizNETINFO.sn, 4) ||
            memcmp(wizNETINFO.gw,  GetwizNETINFO.gw, 4) ||
            memcmp(wizNETINFO.dns, GetwizNETINFO.dns, 4) ) //读取内容不一样
    {
        LOGE("internet", "W5500 Config Information error!!!");

        PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_SPI);
        return false;
    }

    PAR_CLEAR_BIT(gParam.st.NetLink_State, NET_STATE_SPI);  //SPI OK
    return true;
}

/**
  * @brief  获取连接状态
  * @param  None
  * @retval true or false
  */
bool Get_PHYLINK(void)
{
    static uint8_t errCnt = 0, okCnt = 0;
    int8_t phylink_state = PHY_LINK_OFF;

    W5500MutexLock();
    ctlwizchip(CW_GET_PHYLINK, (void *)&phylink_state);//get phy status
    W5500MutexUnlock();

    if(phylink_state != PHY_LINK_ON)
    {
        LOGE("internet", "PHY link ERROR, PHY link status = %u", phylink_state);
        okCnt = 0;
        errCnt++;
        if(errCnt > 5) //超过5次，标志位才置一
        {
            errCnt = 0;
            PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_PHYLINK);

            LOGE("internet", "Set flag NET_STATE_PHYLINK!");
        }
        return false ;
    }

    errCnt = 0;
    okCnt++;
    if(okCnt > 5)
    {
        okCnt = 0;
        PAR_CLEAR_BIT(gParam.st.NetLink_State, NET_STATE_PHYLINK);  //PHYLINK OK
    }

    return true;
}

/**
  * @brief  W5500初始化
  * @param  None
  * @retval None
  */
void W5500_Init(void)
{
    LOGV("internet", "Enter %s()", __func__);
    int8_t phylink_state = PHY_LINK_OFF;

    //W5500硬件复位
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1600);

    W5500_PortInitialize();
    if(!W5500_ConfigInfo()) //config fail
    {
        return ;
    }

    ctlwizchip(CW_GET_PHYLINK, (void *)&phylink_state);//get phy status
    if(phylink_state != PHY_LINK_ON)
    {
        PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_PHYLINK);
        LOGE("internet", "W5500 Init, PHY link ERROR, Set flag NET_STATE_PHYLINK!");
    }
}

/**
  * @brief  W5500复位重新配置
  * @param  None
  * @retval true or false
  */
bool W5500_ResetConfig(void)
{
    int8_t phylink_state = PHY_LINK_OFF;

    gParam.st.NetLink_State = 0xFFFF;

    W5500MutexLock();
    W5500_HardwareReset();
    W5500_PortInitialize();
    if(!W5500_ConfigInfo())
    {
        return false;
    }

    ctlwizchip(CW_GET_PHYLINK, (void *)&phylink_state);//get phy status
    W5500MutexUnlock();

    if(phylink_state != PHY_LINK_ON)
    {
        PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_PHYLINK);
        LOGE("internet", "W5500 Reset Config, PHY link ERROR, Set flag NET_STATE_PHYLINK!");

        return false;
    }

    return true;
}

void W5500MutexInit(void)
{
#if (USE_CMSIS_OS_API == 0)
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

    /* 创建MuxSem */
    gW5500_MuxSem = xSemaphoreCreateMutex();
    if(NULL != gW5500_MuxSem)
    {
        LOGD("internet", "gW5500_MuxSem mutex create success !\r\n");
    }
    xSemaphoreTake(gW5500_MuxSem, portMAX_DELAY);

    xReturn = xSemaphoreGive( gW5500_MuxSem );//给出互斥量
    if( xReturn == pdTRUE )
    {
        LOGD("internet", "gW5500_MuxSem give success !\r\n");
    }
#else
    gW5500MutexId = osMutexNew( osMutex(gW5500MutexDef) );
#endif
}

int W5500MutexLock(void)
{
#if (USE_CMSIS_OS_API == 0)
    return xSemaphoreTake(gW5500_MuxSem, portMAX_DELAY);
#else
    return osMutexWait( gW5500MutexId, osWaitForever );
#endif
}

int W5500MutexUnlock(void)
{
#if (USE_CMSIS_OS_API == 0)
    return xSemaphoreGive( gW5500_MuxSem );
#else
    return osMutexRelease( gW5500MutexId );
#endif
}

/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_internetTask(void)
{
    internetTaskHandle = osThreadNew(InternetTask, NULL, &internet_attributes);
}

/**
  * @brief  Function implementing the uartTask thread.
  * @param  argument: Not used
  * @retval None
  */
void InternetTask(void *argument)
{
    LOGD("internet", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());
    uint8_t phylink_errCnt = 0;

    W5500MutexInit();

    //start thread
    if(gFlashParam.st.Prod_Protocol & PROTOCOL_MBTCP)
    {
        //osThreadNew_mbTcpTask();
    }
    if(gFlashParam.st.Prod_Protocol & PROTOCOL_FLKMQTT)
    {
        osThreadNew_mqttTask();
    }
    if(gFlashParam.st.Prod_Protocol & PROTOCOL_HZUDP)
    {
        //osThreadNew_HuaziudpTask();
    }
    else if(gFlashParam.st.Prod_Protocol & PROTOCOL_LLTUDP) //if use hzudp, can not use lltudp
    {
        //osThreadNew_lltudpTask();
    }

    for(;;)
    {
        osDelay(100);

        /* --------------------------- check connect ----------------------------- */
        if(PAR_READ_BIT(gParam.st.NetLink_State, NET_STATE_SPI))
        {
            osDelay(30000);  //~30s
            W5500_ResetConfig();
            continue ;
        }

//    if(phylink_state == PHY_LINK_ON)
//    {
//        WANCOMPLETE = 1;
//    }
//    else if(phylink_state != PHY_LINK_ON)
//    {
//        WANCOMPLETE = 0;
//    }
        if(Get_PHYLINK() != true)
        {
            //osDelay(100);  //~1s
            phylink_errCnt++;
            if(phylink_errCnt > 100)
            {
                phylink_errCnt = 0;
                W5500_ResetConfig();
            }
            continue ;
        }
        else if(phylink_errCnt != 0)
        {
            phylink_errCnt = 0;
        }

        if(gFlashParam.st.Prod_Protocol & PROTOCOL_MBTCP)
        {
            if(gMbTcpErrorTimes > MBTCP_ERROR_TIMES)
            {
                gMbTcpErrorTimes = 0;
                W5500_ResetConfig();
            }
        }

        if(gFlashParam.st.Prod_Protocol & PROTOCOL_FLKMQTT)
        {
            if(gMqttInitErrorTimes > MQTTINIT_ERROR_TIMES)
            {
                gMqttInitErrorTimes = 0;
                W5500_ResetConfig();
            }
        }

        if( (gFlashParam.st.Prod_Protocol & PROTOCOL_HZUDP) || (gFlashParam.st.Prod_Protocol & PROTOCOL_LLTUDP) )
        {
            if(gUDPErrorTimes > UDP_ERROR_TIMES)
            {
                gUDPErrorTimes = 0;
                W5500_ResetConfig();
            }
        }
    }
}
