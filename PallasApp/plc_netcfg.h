/**
  ******************************************************************************
  * @file    plc_netcfg.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-30
  * @brief   网络配置
  ******************************************************************************
  */

#ifndef __PLC_NETCFG_H
#define __PLC_NETCFG_H
//#include "kalyke_modbus_tcp.h"
//#include "lwip/ip_addr.h"
#include "infra_defs.h"
//#include "bsp_dct.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LAN_CONFIG_IO_EXP_ERR       (0xFF)
extern volatile uint8_t gWANor4G;

/*串口系统块配置字解析结构体*/
typedef struct __UART_SYSBLK_CONFIG_ST
{
    /*端口波特率配置
    000 = 38400, 001 = 19200, 010 = 9600, 011 = 4800, 100 = 2400, 101 = 1200, 110 = 57600; 111 = 115200
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
} uart_sysblk_config_st;

///*UART 校验类型*/
//enum __UART_PARITY_TYPE {
//    UART_PARITY_NO  = 0x00,
//    UART_PARITY_EVEN,
//    UART_PARITY_ODD,
//};

///*停机错误标志*/
//typedef union __PLC_STOP_ERROR_FLAG_U {
//    unsigned short msv_Error;
//    struct {
//        /*SRAM错误*/
//        unsigned short ram_err      :1;
//        /*FALSH错误*/
//        unsigned short flash_err    :1;
//        /*通讯口错误*/
//        unsigned short comm_err     :1;
//        /*实时时钟错误*/
//        unsigned short rtc_err      :1;
//        /*用户程序错误*/
//        unsigned short ucode_err    :1;
//        /*系统块错误*/
//        unsigned short sysblock_err :1;
//        /*数据块错误*/
//        unsigned short datablock_err:1;
//        /*严重执行错误*/
//        unsigned short exec_err     :1;
//        /*本地IO错误*/
//        unsigned short local_io_err :1;
//        /*网络参数配置错误*/
//        unsigned short netcfg_err   :1;
//        /*保留位*/
//        unsigned short reserved     :6;
//    } bit;
//} plc_stop_error_flag;

//extern plc_stop_error_flag guv_StopError;

///*非停机错误标志定义*/
//typedef union __PLC_NON_STOP_ERROR_FLASH_U {
//    unsigned short msv_Error;
//    struct {
//        /*实时时钟错误*/
//        unsigned short rtc_err      :1;
//        /*元件保存错误*/
//        unsigned short element_save_err :1;
//        /*POU信息错误*/
//        unsigned short pou_info_err :1;
//        /*gvt表错误*/
//        unsigned short gvt_err      :1;
//        /*操作数错误*/
//        unsigned short operands_err :1;
//        
//        /*扩展I/O错误*/
//        unsigned short extend_io_num_err :1;
//        /*扩展总线错误*/
//        unsigned short extend_cfg_err    :1;
//        /*扩展总线错误*/
//        unsigned short extend_bus_err    :1;
//        
//        /*EEPROM 写错误*/
//        unsigned short eeprom_wr_err    :1;
//        /*netcfg 错误*/
//        unsigned short netconfig_err    :1;
//        /*用户程序错误*/
//        unsigned short ucode_err        :1;
//        /*数据块错误*/
//        unsigned short datablock_err    :1;
//        /*系统块错误*/
//        unsigned short sysblock_err     :1;
//        /* 系统块配置的从站数与实际不符或未进入op模式 */
//        unsigned short ecat_err     :1;
//        /*保留*/
//        unsigned short reserved         :2;
//    } bit;
//} plc_non_stop_error_flag;

//extern plc_non_stop_error_flag guv_NonStopError;

//typedef struct __PLC_USER_FILE_PTR_ST {
//    /*Ucode 文件指针*/
//    unsigned char * UCodePtr;
//    /*系统块文件指针*/
//    unsigned char * SysBlockPtr;
//    /*网络参数块文件指针*/
//    unsigned char * NetcfgBlockPtr;
//    /*数据块文件指针*/
//    unsigned char * DataBlockPtr;
//    /*POU info文件指针*/
//    unsigned char * PouInfoPtr;
//    /*全局变量表文件指针*/
//    unsigned char * GvtPtr;
//    /*云数据表指针*/
//    unsigned char * CloudDataTablePtr;
//    /*PID1文件指针*/
//    unsigned char * PID1Ptr;
//    /*PID2文件指针*/
//    unsigned char * PID2Ptr;
//} plc_user_file_ptr_st;

//extern plc_user_file_ptr_st gtv_UserFilePtrSt;

struct ip4_addr {
  uint32_t addr;
};

/** ip4_addr_t uses a struct for convenience only, so that the same defines can
 * operate both on ip4_addr_t as well as on ip4_addr_p_t. */
typedef struct ip4_addr ip4_addr_t;

/* 维护中心（隐藏） */
typedef struct _WeiHuCenter_ST
{
    uint8_t domain[40];
    uint16_t port;
} wei_hu_center_st;

/* 云中心配置 */
typedef struct _CloudCenter_ST
{
    uint8_t ipOrDomain; //使用IP还是使用域名, 0: ip; 1: domain
    ip4_addr_t ip;
    uint16_t port;
    ip4_addr_t ip2; //备用IP
    uint16_t port2;
    uint8_t domain[40];
    uint16_t portDomain;
    uint8_t ifConnect; // 0 = 不连接
} cloud_center_st;

/* WAN口配置 */
typedef struct _WANConfig_ST
{
    uint8_t  isParsed; // 1: Have parsed
    ip4_addr_t ip;
    ip4_addr_t mask;
    ip4_addr_t gate;
    ip4_addr_t dns;
    uint8_t ioExp;// 0 = DHCP, 1 = TCP/IP, 5 = Fexlink
    uint8_t mobbustcpServer;//0：启用1个， 1：启用2个，其他：不启用
    uint8_t reserved2;
    uint8_t reserved3;
} wan_config_st;

/* LAN口配置 */
typedef struct _LANConfig_ST
{
    uint8_t  isParsed; // 1: Have parsed
    ip4_addr_t ip;
    ip4_addr_t mask;
    ip4_addr_t gate;
    ip4_addr_t dns;
    uint8_t ioExp; // 0 = Fexlink, 1 = modbusTCP, 2 = EtherCAT
    uint8_t mobbustcpServer;//0：启用1个， 1：启用2个，其他：不启用
    uint8_t reserved2;
    uint8_t reserved3;
} lan_config_st;

/* MQTT配置 */
#define CONFIG_MQTT_MAX_HOST_LEN 64
#define CONFIG_MQTT_MAX_CLIENT_LEN 200
#define CONFIG_MQTT_MAX_USERNAME_LEN 64
#define CONFIG_MQTT_MAX_PASSWORD_LEN 68
#define CONFIG_MQTT_MAX_LWT_TOPIC 64
#define CONFIG_MQTT_MAX_LWT_MSG 64

typedef enum 
{
    ELEM_NULL = 0x00,    //没有软元件
    ELEM_X    = 0x01,    //外部输入继电器
    ELEM_Y    = 0x02,    //外部输出继电器
    ELEM_M    = 0x03,    //辅助继电器
    ELEM_LM   = 0x04,    //局部辅助继电器
    ELEM_SM   = 0x05,    //特殊辅助继电器
    ELEM_S    = 0x06,    //步进状态继电器
    ELEM_T    = 0x07,    //计时器
    ELEM_C    = 0x08,    //计数器
    ELEM_D    = 0x09,    //数据寄存器
    ELEM_V    = 0x0A,    //局部数据寄存器
    ELEM_Z    = 0x0B,    //变址寻址寄存器
    ELEM_SD   = 0x0C,    //特殊数据寄存器
    ELEM_F    = 0x0D,    //
    ELEM_H    = 0x0E,    //
    ELEM_R    = 0x0F,    //扩展数据寄存器

    VALUE_U64   = 0x3F,   //单板U64变量

}element_e;

typedef enum 
{
    DTYPE_BOOL = 0x01,
    DTYPE_U16 = 0x02,
    DTYPE_I16 = 0x04,
    DTYPE_U32 = 0x08,
    DTYPE_I32 = 0x10,
    DTYPE_F32 = 0x20,
    DTYPE_STRING = 0x40,
    DTYPE_SYS = 0x80,

}eleDTYPE_ment_e;

typedef enum 
{
    SYM_IDLE = 0,
    SYM_EQUAL, //等于
    SYM_UNEQUAL,//不等于
    SYM_GT,    //大于
    SYM_LT,    //小于
    SYM_GTE,   //大于等于
    SYM_LTE,   //小于等于
    SYM_CHG,   //改变
    SYM_CFG,   //配置
}symbol_e;

//MiStudio会告诉Kalyke，通过MQTT上报什么
typedef struct _MQTT_MISTUDIO_CONFIG_ST
{
    char name[16];
    uint8_t element;
    uint8_t dataType;
    uint16_t address;
    
    double alarmVal;
    uint8_t alarmType; // false = AlarmEvent, true = AlarmRecover
    uint8_t sym;
    char alarmContent[16];
}mqtt_config_array_st;

typedef struct _MQTT_MISTUDIO_CONFIG_HANYU_ST
{
    uint32_t report_cycle; // second
    uint32_t slave_id;
    char     slave_name[16];
    uint32_t reportContentLen;
    mqtt_config_array_st *pReportContent;
}mqtt_config_array_hanyu_st;

typedef struct _MQTTConfig_ST
{
    uint8_t  isParsed; // 0: we can not use this MQTT config

    bool paused; // true = 暂停MQTT发送
    char vender[32];
    uint32_t reportingCycle; // publish上报周期，最小为3秒
    char host[CONFIG_MQTT_MAX_HOST_LEN];
    uint16_t port;

    /** Client identifier, must be set by caller */
    char client_id[CONFIG_MQTT_MAX_CLIENT_LEN];
    /** User name, set to NULL if not used */
    char username[CONFIG_MQTT_MAX_USERNAME_LEN];
    /** Password, set to NULL if not used */
    char password[CONFIG_MQTT_MAX_PASSWORD_LEN];
    /** keep alive time in seconds, 0 to disable keep alive functionality*/
    uint16_t keepalive;

    /** will topic, set to NULL if will is not to be used,
        will_msg, will_qos and will retain are then ignored */
    char lwt_topic[CONFIG_MQTT_MAX_LWT_TOPIC];
    char lwt_msg[CONFIG_MQTT_MAX_LWT_MSG];
    uint8_t lwt_qos;
    bool lwt_retain;

    char publish_topic[CONFIG_MQTT_MAX_LWT_TOPIC];
    char publish_topic_alarm[CONFIG_MQTT_MAX_LWT_TOPIC];

    char serial_number[64];
    char subscribe_topic[CONFIG_MQTT_MAX_LWT_TOPIC];
    char subscribe_topic_reboot[CONFIG_MQTT_MAX_LWT_TOPIC];
    char subscribe_topic_AlarmSetSD[CONFIG_MQTT_MAX_LWT_TOPIC];
    char subscribe_topic_pub_cycle[CONFIG_MQTT_MAX_LWT_TOPIC];
    char subscribe_topic_pub_now[CONFIG_MQTT_MAX_LWT_TOPIC];
    char subscribe_topic_pause[CONFIG_MQTT_MAX_LWT_TOPIC];
    
    char response_topic[CONFIG_MQTT_MAX_LWT_TOPIC];

    int configLength;
    mqtt_config_array_st *pConfigs;
    mqtt_config_array_hanyu_st *pConfigsHANYU;

    char ProductKey[IOTX_PRODUCT_KEY_LEN + 1];
    char ProductSecret[IOTX_PRODUCT_SECRET_LEN + 1];
    char DeviceName[IOTX_DEVICE_NAME_LEN + 1];
    char DeviceSecret[IOTX_DEVICE_SECRET_LEN + 1];
} mqtt_config_st;

/*Wifi配置*/
typedef struct _WifiConfig_ST
{
    ip4_addr_t ip;
    ip4_addr_t mask;
    ip4_addr_t gate;
    ip4_addr_t dns;

    uint8_t ssid[24];
    uint8_t isCrpy; //是否加密
    uint8_t crpyType;//加密方式
    uint8_t password[24];
    uint8_t notUseDHCP;// 是否通过DHCP自动获取Kalyke的IP地址。1：不使用DHCP
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
} wifi_config_st;

/*透传配置*/
typedef struct _TOUChuanConfig_ST
{
    uint8_t openTT; // 是否启用透传

    uint8_t yiTaiCloudSocketCfg; //以太云socket透传配置
    uint8_t wifiCloudSocketCfg;  // wifi云socket透传配置
    uint8_t threeGCloudSocketCfg;// 3G云socket透传配置

    uint8_t  yiTaiSocket1Cfg; // 以太socket1透传配置
    uint8_t  yiTaiSocket2Cfg; // 以太socket2透传配置
    uint8_t  yiTaiSocket3Cfg; // 以太socket3透传配置

    ip4_addr_t yiTaiSocket1IP;
    uint16_t yiTaiSocket1Port;
    
    ip4_addr_t yiTaiSocket2IP;
    uint16_t yiTaiSocket2Port;

    uint8_t isForbidden_yiTaiCloudSocket;
    uint8_t isForbidden_wifiCloudSocket;
    uint8_t isForbidden_threeGCloudSocket;

    uint8_t isForbidden_yiTaiSocket1;
    uint8_t isForbidden_yiTaiSocket2;
    uint8_t isForbidden_yiTaiSocket3;

    uint8_t reserved1;
    uint8_t reserved2;
    
    uint16_t yiTaiSocket3Port;
} tou_chuan_config_st;

typedef struct _PLCNetConfig_ST
{
    uint8_t surfing;//0 = WAN, 1 = 4G, 2 = wifi
    wei_hu_center_st weihu;
    cloud_center_st cloud;
    wan_config_st wan;
    lan_config_st lan;
    mqtt_config_st mqtt;
    wifi_config_st wifi;
    tou_chuan_config_st touchuan;
} plc_netcfg_st;


#define MODBUS_NAME_LEN  16
typedef struct _MODBUS_ITEM_ST {
    uint8_t sNum; //序列号
    unsigned char name[MODBUS_NAME_LEN];//设备名称
    uint8_t slaveAddr; //从站地址
    uint8_t commType; //执行模式（0=循环；1=触发）
    uint8_t funcCode; //modbus功能码
    
    uint8_t triggerType;//1表示M元件，0表示S元件
    uint16_t triggerAddr; //触发器地址,一般是M元件

    uint16_t slaveRegAddr;//从站寄存器地址
    uint16_t elementCnt;//读写元件个数
    uint16_t *masterBuf; //主站缓冲区地址，一般是D元件地址
    uint16_t *masterBufBoundary;//读写 masterBuf 的边界
    
    /* 0：未执行     1：未执行参数错误 2：正在执行中 3：执行过程出错 4：执行成功*/
    uint16_t *execResult;//一般是D元件地址
    /*运行标志*/
    uint8_t isExec; // 1=已刚刚运行
    /*超时时间*/
    uint32_t timeOut;
    uint16_t delay; // ms
    uint16_t port; //端口号
} modbus_item_st;

typedef struct _MODBUS_CFG_ST {
    /*指令列表项数量*/
    uint8_t listNum;
    /*指令列表数组指针*/
    modbus_item_st *listPtr;
    /*当前执行指令列表下标*/
    uint16_t index[3]; //#define MAX_SUPPORT_UART_PORT   3
    /*缓存区，暂存从站返回数据*/
    uint16_t msp_RecvBuff[32];
} modbus_cfg_st;

typedef struct _MODBUSTCP_ITEM_ST {
    struct netconn *conn;
    uint8_t client_id; // 相同的IP、端口号、源（WAN or LAN）使用一个client_id
    unsigned char name[MODBUS_NAME_LEN];//设备名称
    ip4_addr_t slaveIP; //从站IP地址
    uint16_t slavePort;//从站端口号
    uint8_t commType; //执行模式（0=循环；1=触发）
    uint8_t funcCode; //modbus功能码

    uint8_t triggerType;//1表示M元件，0表示S元件
    uint16_t triggerAddr; //触发器地址,一般是M元件

    uint16_t slaveRegAddr;//从站寄存器地址
    uint8_t elementCnt;//读写元件个数
    uint16_t *masterBuf; //主站缓冲区地址，一般是D元件地址
    uint16_t *masterBufBoundary;//读写 masterBuf 的边界
    /*运行标志*/
    uint8_t execState; // See : _MODBUSTCP_RESULT_E
    /* 同execState */
    uint16_t *execResult;//一般是D元件地址
    /*超时时间*/
    uint32_t timeOut;
    uint16_t wanOrLan;
    uint8_t isExec; // 1=已刚刚运行
} modbus_tcp_item_st;

typedef struct _MODBUS_TCP_CFG_ST {
    /*指令列表项数量*/
    uint16_t listNum;
    /*指令列表数组指针*/
    modbus_tcp_item_st *listPtr;
    /*当前执行指令列表下标*/
    uint16_t index[16]; //#define MAX_MODBUS_TCP_ITEM    16
    uint8_t clientNum;
} modbus_tcp_cfg_st;

extern modbus_cfg_st g_modbus_cfg;
extern modbus_tcp_cfg_st g_modbusTCP_cfg;
extern plc_netcfg_st g_plc_netcfg;
extern char plc_parse_netcfg_block(void);
char plc_parse_netcfg_block_NoUart(void);
char plc_parse_netcfg_block_OnlyUart(void);
#endif /* __PLC_NETCFG_H */

