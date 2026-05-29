
#ifndef __HZUDP_H
#define __HZUDP_H

/* Includes ------------------------------------------------------------------*/




/* Private defines -----------------------------------------------------------*/
#define GW_RTU_MAX       1      //可连接终端数量
#define GW_RTU_RES_MAX   20     //一个终端的寄存器数量

//数据帧报头报尾定义
#define HZUDP_H_ASKCFG   0xF1   //请求配置报头
#define HZUDP_H_GETCFG   0xF2   //接收配置报头
#define HZUDP_H_HAERT    0xF3   //心跳包报头
#define HZUDP_H_OKCFG    0xFA   //成功配置报头
#define HZUDP_H_ERRCFG   0xFB   //失败配置报头
#define HZUDP_H_DATA     0xFF   //数据包报头
#define HZUDP_T_ALL      0xDD   //报尾

//华咨采集器定义
#define HZUDP_TYPE_JCII  0x11   //四要素SPD监测
#define HZUDP_TYPE_ZJCII 0x12   //多要素SPD监测
#define HZUDP_TYPE_JDII  0x13   //接地电阻

//雷电流
#define HZFLUDP_H_DATA   0xFE   //雷电流数据报头
#define HZFLUDP_T_DATA   0xFA   //帧尾
#define HZFLUDP_TYPE     0x02   //设备类型
#define HZFLUDP_MODEL    0x01   //设备型号
#define HZFLUDP_VERSION  0x04   //版本号

//雷电流数据类型
#define HZFLUDP_DTYPE_1      0x0001  //雷击计数+极性+峰值+电荷量+单位能量比+波形数据+时间
#define HZFLUDP_DTYPE_3      0x0003  //雷击计数+峰值+时间
#define HZFLUDP_DTYPE_HEART  0x0FF1  //心跳

#define  UDP_CHECK_TIME            10     //检测UDP状态的时间间隔(s)
#define  UDP_ERROR_TIMES           30     //UDP失败多少次即复位W5500


//typedef struct _GW_RtuInfo_ST
//{
//unsigned short Adr;   //终端地址
//unsigned char  Type;  //终端类型

//unsigned short Res[GW_RTU_RES_MAX];  //终端存储值
//} gw_rtuinfo_st;

/* 配置信息 */
typedef struct _GW_HzUDPConfig_ST
{
    //unsigned short GetRtuInterval;  //获取RTU直接的时间间隔

    unsigned short LoopTime;    //轮询周期
    unsigned short UpTime;      //定时上传周期
    unsigned char  TmpThre;     //温度阈值
    unsigned char  CurThre;     //漏流阈值
    unsigned char  ResThre;     //地阻阈值
    unsigned char  HeartTime;   //心跳周期
    unsigned char  ResendTime;  //数据重发周期
    //unsigned char  RtuNum;      //终端个数
    //gw_rtuinfo_st  RtuInfo[GW_RTU_MAX];  //终端信息

    unsigned char  OkCfg;   //0:nocfg    1:okcfg
    unsigned char  Almflg;  //0:normal   1:alarm

    void (*udp_send_func)(unsigned char *, unsigned short);
    //void (*uart_send_func)(char *, unsigned short);
} gw_hzudpcfg_st;

/* 配置信息 */
typedef struct _GW_HzFlUDPConfig_ST
{
    //unsigned short LtTimes;         //网关自身计数雷击次数
    //unsigned short GetRtuInterval;  //获取RTU直接的时间间隔
    //unsigned char  RtuData[11];  //终端数据 雷击计数(2B)+峰值(2B)+时间(7B)
    unsigned char  Almflg;  //0:normal   1:alarm

    void (*udp_send_func)(unsigned char *, unsigned short);
    //void (*uart_send_func)(char *, unsigned short);
} gw_hzfludpcfg_st;




/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/
extern uint8_t gUDPErrorTimes;
extern uint8_t gUdpRecvbuf[2048];
extern uint8_t g_udp_remote_ip[4];
extern uint16_t g_udp_remote_port;
extern int32_t g_udp_status;


/* Private functions ---------------------------------------------------------*/
/* ------------------------------------ udp option -------------------------------------- */
void udp_send_buffer(unsigned char *pBuff, unsigned short len);
void check_udp_status(void);

void osThreadNew_HuaziudpTask(void);

void udpparam_init(void);

#endif /* __HZUDP_H */
