
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "event_groups.h"

#include "app_log.h"
#include "app_parameter.h"
#include "internet.h"
#include "mqtt.h"
#include "app_tool.h"
#include "app_uart.h"
#include "app_parameter.h"
#include "app_collect.h"
#include "app_payload.h"
#include "app_led.h"

#include "MQTTClient.h"
#include "kalyke_internet_task.h"
/* Private define ------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/
/* Definitions for mqttTask */
osThreadId_t mqttTaskHandle;
const osThreadAttr_t mqttTask_attributes =
{
    .name = "mqttTask",
    .priority = (osPriority_t) mqttTaskPriority,
    .stack_size = 1024
};

TimerHandle_t MqttPingTimr_Handle = NULL;

static uint8_t mqttReadBuffer[16];
static uint8_t mqttSendBuffer[1024];
static MQTTClient mqttClient;
static Network mqttNetwork;

uint8_t gMqttInitErrorTimes = 0;
uint8_t gPublishErrorTimes = 0;
uint8_t gMqttPingErrorTimes = 0;

__IO uint8_t gMqttPingState = MQTT_NO_PING;


/* Private function prototypes -----------------------------------------------*/
void MqttTask(void *argument);


/* Private user code ---------------------------------------------------------*/
/**
  * @brief  MQTT发送
  * @param  *topic    发送主题
  * @param  *payload  发送主体
  * @param  payloadlen  主体长度
  * @retval MQTT_SEND_OK   MQTT发送成功
            MQTT_SEND_ERR   MQTT发送失败
            MQTT_SEND_ERROVER   MQTT发送失败并且超过失败次数
  */
uint8_t Mqtt_Transmit(char *topic, char *payload, int payloadlen)
{

    MQTTMessage mqttMsg;
    mqttMsg.qos = QOS0;
    mqttMsg.retained = 0;
    mqttMsg.dup = 0;
    mqttMsg.id = 0;
    mqttMsg.payload = payload;
    mqttMsg.payloadlen = payloadlen;

    W5500MutexLock();
    int ret = MQTTPublish(&mqttClient, topic, &mqttMsg);
    W5500MutexUnlock();

    LOGI("mqtt", "MQTTPublish return : %d", ret);
    if (ret != SUCCESSS)
    {
        LOGE("mqtt", "This payload send ERROR!!!");
        gPublishErrorTimes++;
        if (gPublishErrorTimes >= PUBLISH_ERROR_TIMES)
        {
            gPublishErrorTimes = 0;
            PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_MQTT);

            return MQTT_SEND_ERROVER;
        }

        return MQTT_SEND_ERR;
    }
    else
    {
        if(gPublishErrorTimes != 0)
        {
            gPublishErrorTimes = 0;
        }

        return MQTT_SEND_OK;
    }
}

/**
  * @brief  获取一个payload并发送报警
  * @param  *topic    发送主题
  * @param  *payload  发送主体
  * @param  payloadlen  主体长度
  * @retval MQTT_SEND_OK   MQTT发送成功
            MQTT_SEND_ERR   MQTT发送失败
            MQTT_SEND_ERROVER   MQTT发送失败并且超过失败次数
  */
uint8_t GetOnePayload_AlmTrsm(void)
{
    LOGW("mqtt", "Start to mqtt alarm transmit !!!!!!!!!!!!\r\n");
    char *payload = (char *)pvPortMalloc(1024);
    char *topic = (char *)pvPortMalloc(128);
    int payloadlen = 0;
    uint8_t ret;

    memset(topic, 0, 128);
    memcpy(topic, gFlashParam.st.mqttAlarmPub, 100);

    memset(payload, 0, 1024);
    GetOnePayload(payload);
    payloadlen = strlen(payload);
    LOGD("mqtt_TrsmNomal", "Publish Topic  : %s", topic);
    LOGI("mqtt_TrsmNomal", "Publish Payload: %s", payload);

    ret = Mqtt_Transmit(topic, payload, payloadlen);

    vPortFree(topic);
    vPortFree(payload);

    return ret;
}

/**
  * @brief  MQTT报警发送任务
  * @param  None
  * @retval None
  */
void Mqtt_TrsmAlmTask(void)
{
    static uint32_t TrsmItvSecCnt = 0;
    uint8_t ret;

    //没到时间监测报警
    if( gParam.st.SecCnt - TrsmItvSecCnt < MQTT_ALL_ALM_INTERVAL )
    {
        return;
    }

    //一次都没采集过，就没必要监测报警
//    if(gGetAtLeastOne == 0)
//    {
//        return;
//    }

    TrsmItvSecCnt = gParam.st.SecCnt;

    gPyldRtu_Idx = 0;
    gPyldP_Idx = 0;
    gPyldOneP_Idx = 0;

    CtrlPMutexLock();

    for(;;)
    {
        if(gPyldP_Idx == gCtrlP_Num)  //payload结束
        {
            break ;
        }

        /* 数据发生报警 --------------------------------------------------------------------------------------------------- */
        //报警
        if( (gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_1] == STATE_ALARM_START) ||
                (gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_2] == STATE_ALARM_START) )
        {
            ret = GetOnePayload_AlmTrsm();
            if(ret == MQTT_SEND_ERROVER)
            {
                break;
            }
            else if(ret == MQTT_SEND_ERR)
            {
                continue;
            }
            else if(ret == MQTT_SEND_OK)
            {
                if(gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_1] == STATE_ALARM_START)
                {
                    gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_1] = STATE_ALARM_RUNING;
                }
                if(gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_2] == STATE_ALARM_START)
                {
                    gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_2] = STATE_ALARM_RUNING;
                }
            }
        }
        //解除报警
        else if( (gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_1] == STATE_ALARM_RELEASE) ||
                 (gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_2] == STATE_ALARM_RELEASE) )
        {
            ret = GetOnePayload_AlmTrsm();
            if(ret == MQTT_SEND_ERROVER)
            {
                break;
            }
            else if(ret == MQTT_SEND_ERR)
            {
                continue;
            }
            else if(ret == MQTT_SEND_OK)
            {
                if(gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_1] == STATE_ALARM_RELEASE)
                {
                    gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_1] = STATE_NORMAL;
                }
                if(gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_2] == STATE_ALARM_RELEASE)
                {
                    gCtrlP_Info[gPyldP_Idx].AlmState[ALARM_LEVEL_2] = STATE_NORMAL;
                }
            }
        }
        /* 特殊数据发生变化 ------------------------------------------------------------------------------------------------ */
        //雷击计数变化了后就要上传
        if( (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_DES3) ||
                (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_DES4) ||
                (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_DES9) ||
                (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_DESA) ||
                (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_DEP4))
        {
            if(gPyldOneP_Idx == 3) //次数变化
            {
                if(gCtrlP_Info[gPyldP_Idx].NewValue != gCtrlP_Info[gPyldP_Idx].OldValue)
                {
                    ret = GetOnePayload_AlmTrsm();
                    if(ret == MQTT_SEND_ERROVER)
                    {
                        break;
                    }
                    else if(ret == MQTT_SEND_ERR)
                    {
                        continue;
                    }
                    else if(ret == MQTT_SEND_OK)
                    {
                        gCtrlP_Info[gPyldP_Idx].OldValue = gCtrlP_Info[gPyldP_Idx].NewValue;
                    }
                }
            }
        }
        //模块第0个值发生变化，该一个模块所有数据发送上去。
        else if( (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_M1AI) ||
                 (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_L2AI) ||
                 (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_G1A3))
        {
            if(gPyldOneP_Idx == 0) //次数变化
            {
                if(gCtrlP_Info[gPyldP_Idx].NewValue == 0)
                {
                    gCtrlP_Info[gPyldP_Idx].OldValue = gCtrlP_Info[gPyldP_Idx].NewValue;
                }
                else
                {
                    if(gCtrlP_Info[gPyldP_Idx].NewValue != gCtrlP_Info[gPyldP_Idx].OldValue)
                    {
                        for(uint8_t i = 0; i < CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType]; i++)
                        {
                            ret = GetOnePayload_AlmTrsm();
                            if(ret == MQTT_SEND_ERROVER)
                            {
                                break;
                            }
                            else if(ret == MQTT_SEND_ERR)
                            {
                                i--;  //try agin
                                continue;
                            }
                            else if(ret == MQTT_SEND_OK)
                            {
                                //if(i == 0)
                                //{
                                //gCtrlP_Info[gPyldP_Idx].OldValue = gCtrlP_Info[gPyldP_Idx].NewValue;
                                //}
                                if(i < (CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType] - 1) ) //最后一次不加
                                {
                                    gPyldP_Idx++;
                                    gPyldOneP_Idx++;
                                }
                            }
                        }
                        if(ret == MQTT_SEND_OK) //全部发送成功，才赋值
                        {
                            gCtrlP_Info[gPyldP_Idx - gPyldOneP_Idx].OldValue = gCtrlP_Info[gPyldP_Idx - gPyldOneP_Idx].NewValue; //赋值雷击次数/采集次数
                        }
                    }
                }
            }
        }
        //智能型电涌保护器计数变化
        else if( (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_DEM2) ||
                 (gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType == TYPE_DEM4) )
        {
            if(gPyldOneP_Idx == 4) //次数变化
            {
                if(gCtrlP_Info[gPyldP_Idx].NewValue != gCtrlP_Info[gPyldP_Idx].OldValue)
                {
                    ret = GetOnePayload_AlmTrsm();
                    if(ret == MQTT_SEND_ERROVER)
                    {
                        break;
                    }
                    else if(ret == MQTT_SEND_ERR)
                    {
                        continue;
                    }
                    else if(ret == MQTT_SEND_OK)
                    {
                        gCtrlP_Info[gPyldP_Idx].OldValue = gCtrlP_Info[gPyldP_Idx].NewValue;
                    }
                }
            }
        }

        /* ---------------------------------------------------------------------------------------------------------------------- */

        gPyldP_Idx++;
        gPyldOneP_Idx++;
        if(gPyldOneP_Idx == CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType])
        {
            gPyldRtu_Idx++;
            gPyldOneP_Idx = 0;
        }
    }

    CtrlPMutexUnlock();
}

/**
  * @brief  MQTT定时发送任务
  * @param  None
  * @retval None
  */
void Mqtt_TrsmNormalTask(void)
{
    static uint32_t TrsmItvSecCnt = 0;
    uint8_t ret;

    //不是第一次并且没到时间发送
    if( (TrsmItvSecCnt != 0) && (gParam.st.SecCnt - TrsmItvSecCnt < gFlashParam.st.mqttPublishInterval * 60) )
    {
        return;
    }

    //一次都没采集过，就没必要发送
//    if(gGetAtLeastOne == 0)
//    {
//        return;
//    }

    CtrlPMutexLock();

    TrsmItvSecCnt = gParam.st.SecCnt;

    LOGW("mqtt", "Start to mqtt normal transmit !!!!!!!!!!!!\r\n");
    char *payload = (char *)pvPortMalloc(1024);
    char *topic = (char *)pvPortMalloc(128);
    int payloadlen = 0;

    gPyldRtu_Idx = 0;
    gPyldP_Idx = 0;
    gPyldOneP_Idx = 0;

    memset(topic, 0, 128);
    memcpy(topic, gFlashParam.st.mqttPub, 100);

    for(;;)
    {
        memset(payload, 0, 1024);

//        if(gPyldP_Idx == gCtrlP_Num)  //payload结束
//        {
//            break ;
//        }

        kalyke_cycle_post_TEST(payload);
        payloadlen = strlen(payload);
        
        LOGD("mqtt_TrsmNomal", "Publish Topic  : %s", topic);
        LOGI("mqtt_TrsmNomal", "Publish Payload: %s", payload);

        ret = Mqtt_Transmit(topic, payload, payloadlen);
        if(ret == MQTT_SEND_OK)
        {
            break ;
//            gPyldP_Idx++;
//            gPyldOneP_Idx++;
//            if(gPyldOneP_Idx == CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType])
//            {
//                gPyldRtu_Idx++;
//                gPyldOneP_Idx = 0;
//            }
        }
        else
        {
            if(ret == MQTT_SEND_ERROVER)
            {
                TrsmItvSecCnt = 0;
                break ;
            }
            else if(ret == MQTT_SEND_ERR)
            {
                continue;
            }
        }
    }

    vPortFree(topic);
    vPortFree(payload);

    CtrlPMutexUnlock();
}

/**
  * @brief  MQTT接收任务
  * @param  None
  * @retval None
  */
void Mqtt_RecvTask(void)
{
    W5500MutexLock();
    MQTTYield(&mqttClient, 1000);
    W5500MutexUnlock();
}

/**
  * @brief  MQTT接收到达
  * @param  MessageData *md 数据包
  * @retval None
  */
static void messageArrived(MessageData *md)
{
    LOGV("mqtt", "Enter %s()", __func__);
    MQTTMessage *message = md->message;
    char *topic = (char *)pvPortMalloc(md->topicName->lenstring.len + 128);
    memset(topic, 0, md->topicName->lenstring.len + 128);
    memcpy(topic, md->topicName->lenstring.data, md->topicName->lenstring.len);
    LOGD("mqtt", "Topic is : %s", topic);

    char *content = (char *)pvPortMalloc(message->payloadlen + 128);
    memset(content, 0, message->payloadlen + 128);
    memcpy(content, message->payload, message->payloadlen);
    LOGI("mqtt", "Payload is : %s", content);

    UnpackPayLoad(content);  //解析MQTT下发的数据
    vPortFree(topic);
    vPortFree(content);
}

/**
  * @brief  MQTT初始化
  * @param  None
  * @retval true / false
  */
bool Mqtt_Init(void)
{
    LOGV("mqtt", "Enter %s()", __func__ );

    //portENTER_CRITICAL();
    W5500MutexLock();

    uint16_t targetPort = 1883;
    uint8_t targetIP[4] = {8, 129, 232, 136};

    memset(&mqttNetwork, 0, sizeof(mqttNetwork));
    memset(&mqttClient, 0, sizeof(mqttClient));
    mqttNetwork.my_socket = APP_SOCKET_MQTT;

    memcpy(targetIP, gFlashParam.st.s1TargetIP, 4);
    targetPort = gFlashParam.st.s1TargetPort;
    LOGW("mqtt", "MQTT target IP address : %d.%d.%d.%d, port : %d", targetIP[0], targetIP[1], targetIP[2], targetIP[3], targetPort);
    NewNetwork(&mqttNetwork, APP_SOCKET_MQTT);

    ConnectNetwork(&mqttNetwork, targetIP, targetPort);
    MQTTClientInit(&mqttClient, &mqttNetwork, 1000, mqttSendBuffer, sizeof(mqttSendBuffer), mqttReadBuffer, sizeof(mqttReadBuffer));

    char client[24] = {0};
    memcpy(client, gFlashParam.st.idInfo, 16);
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = g_plc_netcfg.mqtt.client_id;  
    data.username.cstring = g_plc_netcfg.mqtt.username;
    data.password.cstring = g_plc_netcfg.mqtt.password;
    data.keepAliveInterval = 120;
    data.cleansession = 1;

    int rcCon = MQTTConnect(&mqttClient, &data);
    LOGI("mqtt", "Connected %d, tips:0 sucess, -1 failure, -2 buffer pverflow", rcCon);

    //LOGD("mqtt", "Subscribing to %s", gFlashParam.st.mqttSub);
    //int rcSub = MQTTSubscribe(&mqttClient, (char *)gFlashParam.st.mqttSub, QOS2, messageArrived);
    LOGE("mqtt", "Subscribing to %s", g_plc_netcfg.mqtt.publish_topic);    
    int rcSub = MQTTSubscribe(&mqttClient, (char *)g_plc_netcfg.mqtt.publish_topic, QOS2, messageArrived);
    LOGI("mqtt", "Subscribed %d,  tips:0 sucess, -1 failure, -2 buffer pverflow", rcSub);

    W5500MutexUnlock();

    if (rcCon == SUCCESSS && rcSub == SUCCESSS)
    {
        PAR_CLEAR_BIT(gParam.st.NetLink_State, NET_STATE_MQTT);

        //portEXIT_CRITICAL();  //退出临界段
        return true;
    }
    else
    {
        PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_MQTT);

        //portEXIT_CRITICAL();  //退出临界段
        return false;
    }
}

/**
 * @brief  MQTT ping 定时器回调函数
 * @param  None
 * @retval None
 */
static void MqttPingTimr_Callback(TimerHandle_t ltv_TimeHandle)
{
    LOGW("mqtt", "Enter %s, gMqttPingState = %d", __func__, gMqttPingState);

    if(gMqttPingState != MQTT_PING_RESP_OK)
    {
        LOGE("mqtt", "MQTT ping ERROR!!!");

        PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_MQTT);
    }
    else
    {
        if(gMqttPingErrorTimes != 0)
        {
            gMqttPingErrorTimes = 0;
        }
    }

    gMqttPingState = MQTT_NO_PING;
}

/**
 * @brief  MQTT ping 定时器回调函数
 * @param  None
 * @retval None
 */
void MqttPingTimr_Start(void)
{
    xTimerStart(MqttPingTimr_Handle, 0);  //开启周期定时器
}

/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_mqttTask(void)
{
    mqttTaskHandle = osThreadNew(MqttTask, NULL, &mqttTask_attributes);
}

/**
  * @brief  Function implementing the MqttTask thread.
  * @param  argument: Not used
  * @retval None
  */
void MqttTask(void *argument)
{
    LOGD("mqtt", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

    MqttPingTimr_Handle = xTimerCreate((const char *          )"OneShotTimer",
                                       (TickType_t            )MQTT_PING_RECV_MAXTIME, /* 定时器周期 */
                                       (UBaseType_t           )pdFALSE, /* 单次模式 */
                                       (void *                )2, /* 为每个计时器分配一个索引的唯一ID */
                                       (TimerCallbackFunction_t)MqttPingTimr_Callback);
    if(MqttPingTimr_Handle != NULL)
    {
        LOGD("mqtt", "MqttPingTimr_Handle timer create success !\r\n");
    }
    for(;;)
    {
        osDelay(100);

        //30s内或正在find Rtu时不进行mqtt
        if( (gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_FINDFLKRTU) || (gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_FINDHLKRTU) || (gParam.st.SecCnt < 5) ||
                (gtv_UartPortStatus[UART_PORT2].mcv_Mode == UART_MODE_FINDFLKRTU) || (gtv_UartPortStatus[UART_PORT2].mcv_Mode == UART_MODE_FINDHLKRTU) )
        {
            continue ;
        }
        if((gParam.st.NetLink_State & NET_STATE_OK) != 0)
        {
            PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_MQTT);
            continue ;
        }

        if((gParam.st.NetLink_State & NET_STATE_MQTT) != 0)
        {
            if(!Mqtt_Init())  //初始化失败
            {
                gMqttInitErrorTimes++;
                disconnect(APP_SOCKET_MQTT);
                LOGE("mqtt", "Mqtt init error, now error times is %d", gMqttInitErrorTimes);
                osDelay(NEXT_TIME_OF_MQTTINIT);

                continue ;
            }
            else
            {
                if(gMqttInitErrorTimes != 0)
                {
                    gMqttInitErrorTimes = 0;
                }
            }
        }

        if(gMqttPingErrorTimes > MQTTPING_ERROR_TIMES)
        {
            gMqttPingErrorTimes = 0;
            PAR_SET_BIT(gParam.st.NetLink_State, NET_STATE_MQTT);
        }

        //Mqtt_TrsmNormalTask();

        //Mqtt_TrsmAlmTask();

        //Mqtt_RecvTask();

    }
}




