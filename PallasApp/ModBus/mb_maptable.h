
#ifndef __MB_MAPTABLE_H
#define __MB_MAPTABLE_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/*Modbus元件寻址方式*/
enum __MODBUS_ELEMENT_ADDR_E
{
    //MB_BIT_ELEMENT  = 0x01,
    MB_WORD_ELEMENT,
    MB_ELEMENT_MAX
};

/*Modbus 元件类型定义*/
enum __MODBUS_ELEMENT_TYPE_E
{
    /* 自定义类型 compatible old protocol ------------------------------------- */
    MB_WORD_IDINFO,           //模块ID
    MB_WORD_MQTTUSERNAME,     //MQTT用户名
    MB_WORD_MQTTPASSWORD,     //MQTT密码
    MB_WORD_MQTTPUB,          //MQTT推送主题
    MB_WORD_MQTTSUB,          //MQTT订阅主题
    MB_WORD_MQTTALARMPUB,     //MQTT报警推送主题
    MB_WORD_GATEWAYIP,        //网关IP
    MB_WORD_LOCALIP,          //本机IP
    MB_WORD_S0TARGETIP,       //S0目标IP
    MB_WORD_S0LOCALPORT,      //S0本机端口
    MB_WORD_S0TARGETPORT,     //S0目标端口
    MB_WORD_S1TARGETIP,       //S1目标IP
    MB_WORD_S1LOCALPORT,      //S1本机端口
    MB_WORD_S1TARGETPORT,     //S1目标端口
    MB_WORD_NODECHECKINTER,   //网关定时查询终端的时间间隔
    MB_WORD_MQTTPUBINTER,     //MQTT上传时间间隔（单位：分钟）
    MB_WORD_MACADDR,          //MAC地址
    MB_WORD_MASKIP,           //子网掩码
    MB_WORD_RTCTIME,          //读取模块时间
    MB_WORD_DESXLJCLEAR,      //清除雷击计数
    MB_WORD_IDNUMBER,         //读取模块站号
    MB_WORD_L1AICLEAR,        //雷电流内存清除(只写)
    MB_WORD_G1A3INTERVAL,     //接地电阻采集频率
    MB_WORD_G1A3GETNUM,       //接地电阻采集次数
    MB_WORD_VALUE,            //读取模块当前值
    MB_WORD_SETDES4LIM,       //SPD前四个要素的门限（改变灯的状态）
    MB_WORD_G1A3OFFSET,       //接地电阻修正系数
    MB_WORD_RSTZIGBEE,        //zigbee恢复出厂设置

    MB_WORD_POWER_QUALITY,    //电能质量数据
    MB_WORD_HARMONIC,         //谐波数据
    MB_WORD_SYNC_SAMPLED,     //同步采样数据 7路每路146个数据
    
    /* new -------------------------------------------------------------------- */
    MB_WORD_SD,               //flash存放word(32-bit)
    MB_HALF_WORD_SD,          //flash存放half-word(16-bit)
    MB_BYTE_SD,               //flash存放byte(8-bit)

    MB_WORD_PARAM,            //不存入flash的变量word(32-bit)
    MB_HALF_WORD_PARAM,       //不存入flash的变量half-word(16-bit)
    MB_BYTE_PARAM,            //不存入flash的变量byte(8-bit)

    MB_HALF_WORD_LTLIST,      //雷电流数据表

    MB_WORD_METER,            //ATT7022等计量寄存器
    MB_WORD_CALICMD,          //校表指令

    MB_HALF_WORD_TOU,         //分时计费(TOU)配置与电能数据 0x3000~
};


/*是否能广播*/
enum __MODBUS_BROADCAST_E
{
    MB_BROADCAST = 1,
    MB_NOBROADCAST,

};

typedef struct __MODBUS_ELEMENT_MAP_TABLE_INFO_T
{
    /*寻址类型：位、字*/
    unsigned char mcv_AddrType;
    /*元件类型*/
    unsigned char mcv_ElementType;
    /*是否可广播*/
    unsigned char mcv_Broadcast;
    /*开始协议地址*/
    unsigned short msv_StartAddr;
    /*结束协议地址*/
    unsigned short msv_EndAddr;
    /*开始元件编号*/
    unsigned short msv_StartElement;
    /*最大元件数量*/
    unsigned short msv_MaxNum;
} mb_element_map_table_info;

/* Exported constants --------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
//#define  LTLIST_START_MBADDR   0x0468


/* Private functions ---------------------------------------------------------*/
bool mb_slave_convert_element_info(unsigned char lcv_AddrType, unsigned short lsv_MbAddr, unsigned short lsv_ElementCnt, \
                                   unsigned char *lcp_ElementType, unsigned short *lsp_ElementAddr, unsigned char *lcp_CanBroadcast);


#endif /* __MB_MAPTABLE_H */
