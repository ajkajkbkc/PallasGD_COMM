/**
  ******************************************************************************
  * @file    main.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   
  ******************************************************************************
  */
  
#ifndef _KALYKE_INTERNET_H
#define _KALYKE_INTERNET_H

#include "FreeRTOS.h"
#include "task.h"
//#include "kalyke_opts.h"
#include "plc_netcfg.h"
//#include "fsl_phy.h"

/* ----------------------- MBAP Header --------------------------------------*/
/*
 *
 * <------------------------ MODBUS TCP/IP ADU(1) ------------------------->
 *              <----------- MODBUS PDU (1') ---------------->
 *  +-----------+---------------+------------------------------------------+
 *  | TID | PID | Length | UID  |Code | Data                               |
 *  +-----------+---------------+------------------------------------------+
 *  |     |     |        |      |
 * (2)   (3)   (4)      (5)    (6)
 *
 * (2)  ... MB_TCP_TID          = 0 (Transaction Identifier - 2 Byte)
 * (3)  ... MB_TCP_PID          = 2 (Protocol Identifier - 2 Byte)
 * (4)  ... MB_TCP_LEN          = 4 (Number of bytes - 2 Byte)
 * (5)  ... MB_TCP_UID          = 6 (Unit Identifier - 1 Byte)
 * (6)  ... MB_TCP_FUNC         = 7 (Modbus Function Code)
 *
 * (1)  ... Modbus TCP/IP Application Data Unit
 * (1') ... Modbus Protocol Data Unit
 */
#define MB_TCP_TID1         0
#define MB_TCP_TID2         1
#define MB_TCP_PID          2
#define MB_TCP_LEN          4
#define MB_TCP_UID          6
#define MB_TCP_FUNC         7

enum _MQTT_ERROR_
{
    /* json语法解析错误：没有内存 */
    ERR_MQTT_JSON_NO_MEMORY = 10,
    /* json语法解析错误：Top Level is not an object */
    ERR_MQTT_JSON_NOT_OBJECT = 11,
    /* json内容无效，具体看 Kalyke_extractMqttConfig 函数返回值 */
    ERR_MQTT_JSON_CONTENT = 12,
};

typedef enum _MQTT_SOURCE_TYPE
{
    MQTT_IDLE  = 0,
    MQTT_ENET  = 1,
    MQTT_4G    = 2,
}mqtt_source_type_t;

#if 0
#define MQTT_TOPIC_POST "sys/teiobCPILLff/device/plctest/post"
#define MQTT_TOPIC_RSP  "sys/teiobCPILLff/device/plctest/setback"
#define MQTT_TOPIC_SUB  "sys/teiobCPILLff/device/plctest/set"
#else
#define MQTT_TOPIC_POST g_plc_netcfg.mqtt.publish_topic
#define MQTT_TOPIC_ALARM  g_plc_netcfg.mqtt.publish_topic_alarm
#define MQTT_TOPIC_SUB  g_plc_netcfg.mqtt.subscribe_topic
#endif

typedef struct _MQTT_RECV_MSG_COMMAND_ST
{
    char name[32];
    uint8_t element;
    uint8_t reserve;
    bool valueBool;
    uint8_t dataType;
    float valueFloat;
    int32_t valueInt32;
    int16_t valueInt16;
    uint16_t address;
}mqtt_command_st;

// 从服务器收到的数据存入此结构体中
typedef struct _MQTT_RECV_MSG_ST
{
    char commandType[32];
    uint32_t slave_id;
    char slave_name[16];
    char uuid[64];
    char deviceCode[32];
    uint32_t time; // 从1970年开始的毫秒数
    
    int cmdLength;
    mqtt_command_st *pCmd;

    //For NFYMQTT
    char messageId[32];
    char messageType[32];
    char RPROPERTY[32];
    char WPROPERTY[32];
}mqtt_recv_st;

typedef struct _MQTT_MSG_ST
{
    /* 0:MQTT receive,  1:MQTT publish,  2:init, 
       3:TCP received data, 4: TCP send data, 5: TCP reconnect
    */
    uint32_t type;
    /* payload数据长度 */
    uint32_t msgLength;
    /* payload数据缓存区指针 */
    uint8_t *dataBuff;
    uint8_t *topic;
    uint32_t qos;
} mqtt_msg_st;

typedef struct _TOUCHUAN_MSG_ST
{
    uint16_t msgLength;
    uint8_t *dataBuff;
} touchuan_msg_st;


extern TaskHandle_t gKalykeInternetTaskHandle;
extern void kalyke_internet_task(void *arg);
extern void kalyke_mqtt_publish(char* topic, char *payload, uint8_t qos);
extern void kalyke_ethernet_resume(void);
extern void kalyke_ethernet_suspend(void);
extern uint8_t tcp_client_get_connected_bit(uint8_t clientID);
extern uint8_t tcp_client_get_connectting_bit(uint8_t clientID);
extern void start_tcp_client(uint8_t clientID, uint8_t *pc);
extern void mbc_link_send(uint8_t *pBuff, uint16_t len, uint8_t clientID, uint8_t *ucode_pc, uint8_t flag, uint16_t *registerBuf);
extern void stop_tcp_client(uint8_t clientID);
extern void stop_tcp_client_all(void);
extern void handle_mqtt_recv_data(mqtt_msg_st *pMqttMsg);
extern uint16_t getCurrentInternetDevice(void);
extern void kalyke_set_WAN_default(void);
extern void kalyke_set_LAN_default(void);
//extern void Ali_MQTT_init(void);
extern void MicroLink_touchuan_send_data_2_tcp(uint8_t *pData, uint16_t len);
extern bool is_MicroLinkLogin(uint8_t *data, uint16_t len);
extern void get_Mac(uint8_t *pMac, uint32_t enet);
extern void re_init_ENET2_LAN(void);

extern volatile mqtt_source_type_t gMqttSource;
extern volatile bool gWanOK;
extern volatile bool gLanOK;
extern uint64_t gMcuID;
extern uint8_t gTouChuan;
extern struct netif fsl_netif1; // For LAN
extern uint8_t gTCPSendBuf[1024];
extern uint8_t gMODBUSTCPHead[16];
extern uint8_t gTCPResponseBuf[1024];
extern uint8_t gTCPRecvBuf[1024];
//extern phy_handle_t phyHandle1;
extern uint8_t gGUHUAing;
#endif /* _KALYKE_INTERNET_H */

