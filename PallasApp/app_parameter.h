
#ifndef __APP_PARAMETER_H
#define __APP_PARAMETER_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
#include "main.h"


/* Private defines -----------------------------------------------------------*/
/* Flash ----- --------------------------------------------------- */
#define FLASH_ONEPAGE_BYTESIZE      2048 //one page size 2048 byte(8-bit)
#define FLASH_ONEPAGE_HALFWORDSIZE  512 //1024 //one page size 1024 half-word(16-bit)
#define FLASH_ONEPAGE_WORDSIZE      512  //one page size 512 word(32-bit)

#define FLASH_MAGIC_NUMBER          0x334455AAU

//STM32F103RC Flash Base Adresses:0x8000000~0x8040000  page:0~127
/* 0x803F800 ~ 0x803FFFF, 2K */
//#define PAR_LTLIST_SAVE_ADDR        0x803F800 //雷电流数据存储

/* 0x803F000 ~ 0x803F7FF, 2K */
#define PAR_SAVE_ADDR               0x803F800      /////0x803F000 //存储flash_param_t

/* 0x803E800 ~ 0x803EFFF, 2K */
#define PAR_RTU_INFO_SAVE_ADDR      0x803F000 //终端信息 (MAX_RTU_NUM * 12 * 2 + 4 bytes = 724 bytes)

/* 0x803D800 ~ 0x803E7FF, 4K */
#define PAR_CTRL_INFO_SAVE_ADDR     0x803E000 //控制点信息 (MAX_CTRLP_NUM * 10 bytes = 3600 bytes)

/* 0x803B000 ~ 0x803D7FF, 10K */
#define PLC_INFO_SAVE_ADDR          0x803B000 

/* paramters ------------------------------------------------------*/
#define PARAM_HALFWORDSIZE          128 //300  //param size 300 half-word(16-bit)

#define KEEP_ADC_SIZE               30  //峰值保持ADC_buff保存的大小
#define ADC_CONVERT_SIZE            2   /* Size of array containing ADC converted values: set to ADC sequencer number of ranks converted, to have a rank in each address */


/* Thunder current monitor ----------------------------------------*/
//#define LT_MAX_NUM                  100  //Save max number of lightning(can not over 100)
//#define LT_LSIT_SIZE                12 //Size by 16bit=(LT_MAX_NUM*(sizeof(LtElem_st))+4)/2

/* Flash paramters operation --------------------------------------*/
#define GET_FLASH_WORD_PARAM_VALUE(element)               (*(uint32_t*)&gFlashParam.flash_buff[element])
#define SET_FLASH_WORD_PARAM_VALUE(element, value)        (*(uint32_t*)&gFlashParam.flash_buff[element] = value)
#define GET_FLASH_HALFWORD_PARAM_VALUE(element)           (*(uint16_t*)&gFlashParam.flash_buff[element])
#define SET_FLASH_HALFWORD_PARAM_VALUE(element, value)    (*(uint16_t*)&gFlashParam.flash_buff[element] = value)

/* Paramters operation --------------------------------------------*/
#define GET_WORD_PARAM_VALUE(element)                     (*(uint32_t*)&gParam.param_buff[element])
#define SET_WORD_PARAM_VALUE(element, value)              (*(uint32_t*)&gParam.param_buff[element] = value)
#define GET_HALFWORD_PARAM_VALUE(element)                 (*(uint16_t*)&gParam.param_buff[element])
#define SET_HALFWORD_PARAM_VALUE(element, value)          (*(uint16_t*)&gParam.param_buff[element] = value)

/* LtList operation -----------------------------------------------*/
#define GET_HALFWORD_LTLIST_VALUE(element)                (*(uint16_t*)&gLtList.lsv_buff[element])
#define SET_HALFWORD_LTLIST_VALUE(element, value)         (*(uint16_t*)&gLtList.lsv_buff[element] = value)


/* Useless parameter default -----------------------------------------------*/
#define FSS_USELESS_PARAM_DEFAULT    (0x7FFF)

#define FSS_Current_Startup          (250)    //启动电流uA

#define BOOL_Open       0  //断开
#define BOOL_Close      1  //闭合
#define BOOL_Normal     0  //正常
#define BOOL_Alarm      1  //异常

#define Output_OnlyCmd_Msk     0x0001  //只能指令控制
#define Output_AndAlmS_Msk     0x0002  //报警输出控制源判断【与】逻辑

#define SysErr0_Oled_Msk          0x0001
#define SysErr0_Meter_Msk         0x0002

/* Exported types ------------------------------------------------------------*/
/* Thunder current monitor */
//typedef struct
//{
//    uint8_t  Month;    //  01 06  月  //小端
//    uint8_t  Year;     //  01 06  年
//    uint8_t  Hour;     //  01 07  时
//    uint8_t  Day;      //  01 07  日
//    uint8_t  Second;   //  01 08  秒
//    uint8_t  Minute;   //  01 08  分
//    uint16_t Peak;     //  01 09  峰值
//    uint16_t Polar;    //  01 0A  极性
//    uint16_t KeepTime; //  01 0B  持续时间
//    uint16_t reserved1;//  01 0C  空 预留
//    uint16_t usQ;      //  01 0D  电荷量
//    uint16_t usWR;     //  01 0E  单位能量
//    uint16_t reserved2;//  01 0F  空 预留
//} LtElem_st;  //占20byte

/* SPD monitoring terminal */
//typedef union
//{
//    uint16_t lsv_buff[16];
//    struct
//    {
//        uint16_t  YL;         //  01 06  遥信
//        uint16_t  KL;         //  01 07  空开
//        uint16_t  Pe;         //  01 08  接地
//        uint16_t  LtNum;      //  01 09  计数
//        uint16_t  Reserved1;  //  01 0A  预留位01
//        uint16_t  VolA;       //  01 0B  A相电压
//        uint16_t  VolB;       //  01 0C  B相电压
//        uint16_t  VolC;       //  01 0D  C相电压
//        uint16_t  CurA;       //  01 0E  A相漏电流
//        uint16_t  CurB;       //  01 0F  B相漏电流
//        uint16_t  CurC;       //  01 10  C相漏电流
//        uint16_t  Tmp1;       //  01 11  温度1
//        uint16_t  Tmp2;       //  01 12  温度2
//        uint16_t  Reserved2;  //  01 13  预留位02
//        uint16_t  Life;       //  01 14  寿命
//    } st;
//} FS_Elem_st;  //占30byte

///* smart SPD element */
//typedef union
//{
//    uint16_t lsv_buff[17];
//    struct
//    {
//        uint16_t  L1;         //  01 06  L1
//        uint16_t  L2;         //  01 07  L2
//        uint16_t  L3;         //  01 08  L3
//        uint16_t  PE;         //  01 09  Pe
//        uint16_t  SW[4];      //  01 0A  SW
//        uint16_t  LtNum;      //  01 0E  计数
//        uint16_t  Cur[3];     //  01 0F  A/B/C相漏电流
//#if 0
//        uint16_t  Tmp1;       //  01 12  温度1
//        uint16_t  Tmp2;       //  01 13  温度2
//        uint16_t  Tmp3;       //  01 14  温度3
//        uint16_t  Tmp4;       //  01 15  温度4
//#else
//        int16_t  Tmp[4];      //  01 12  温度
//#endif
//        uint16_t  Life;       //  01 16  寿命
//    } st;
//} FSS_Elem_st;  //占40byte

///* LT List */
//typedef union
//{
//    uint16_t lsv_buff[LT_LSIT_SIZE];
//    struct
//    {
//        uint32_t length;
//        LtElem_st LtData[LT_MAX_NUM];
//    } st;
//} LtList_st;

/* PLC List */
//typedef union
//{
//    uint16_t lsv_buff[1];
//} PLCList_st;

//extern PLCList_st gplcList;

typedef union
{
    uint16_t flash_buff[FLASH_ONEPAGE_HALFWORDSIZE];  //size: 1024 half-word(16-bit)
    struct
    {
        /* 占用 200 half-word(16-bit)
        * size 100 word(32-bit)
        * mb start addr is 0x0401(D1025), end addr is 0x04C8(D1224)
        * must save 100 word, no use words set in Reserved[]
        * for example: If use two words, must set Reserved[98]
        */                                      //MdAddr
        uint32_t magicNum;                      //(D1025)04 01
        uint32_t idNum;                         //(D1027)04 03  站号
        uint32_t SecCntAll;                     //(D1029)04 05  出厂后运行的总秒数
        uint32_t SysResetCnt;                   //(D1031)04 07  系统重启次数
        uint32_t W5500ResetCnt;                 //(D1033)04 09  W5500重启次数
        uint32_t reserved01[5];                 //(D1035)04 0B
        uint32_t Prod_Param;                    //(D1045)04 15  产品参数
        uint32_t Prod_Protocol;                 //(D1047)04 17  产品通讯协议
        uint32_t Hardware_version;              //(D1049)04 19  硬件版本
        
        uint32_t Common_Reg32_reserve[6];       //(D1051)04 1B  [18]=40-(([D1071]-[D1025])/2) 预留32位通用寄存器

        /* module reg32 (60/100, D1105) -------------------------------- */
//        float N_Ib_ATT7022[3];                  //(D1105)04 51
//        float N_Ib_RN8209D;                     //(D1111)04 57  N为比例系数

//        float Cur_Startup;                      //(D1113)04 59  起动电流
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
        float N_Ib_Vi[3];                       //(D1063)04 27  N为比例系数

        float Vol_Minimun;                      //(D1069)04 2D  最小电压
        float Cur_Minimun;                      //(D1071)04 2F  最小电流
        float Pow_Minimun;                      //(D1073)04 31  最小功率

        float fVol_THUp;                        //(D1075)04 33  过电压报警阈值
        float fVol_THDown;                      //(D1077)04 35  欠电压报警阈值
        float fCur_THUp;                        //(D1079)04 37  过电流报警阈值
        float fCur_THDown;                      //(D1081)04 39  欠电流报警阈值
        float fPow_THUp;                        //(D1083)04 3B  过功率报警阈值
        float fPow_THDown;                      //(D1085)04 3D  欠功率报警阈值

        float EC;                               //(D1087)04 3F  EC电表常数
        float K_Pow;                            //(D1089)04 41  K为功率参数系数，所有功率参数共用
        
        float Vol_Startup;                      //(D1091)04 43  起动电压
        uint32_t Module_Reg32_reserve[16];      //(D1093)04 45  [55]=60-(([D1115]-[D1105])/2) 预留32位模组寄存器
    
#else
        uint32_t Module_Reg32_reserve[55];      //(D1115)04 5B  [55]=60-(([D1115]-[D1105])/2) 预留32位模组寄存器
#endif

        /* 占用 200 half-word(16-bit)
         * size 200 half-word(16-bit)
        * mb start addr is 0x04C9(D1225), end addr is 0x0590(D1424)
        * must save 200 half-word, no use words set in Reserved[]
        * for example: If use two half-words, must set Reserved[198]
        */                                      //MdAddr
        uint16_t localUDPPort;                  //(D1125)04 65  本机UDP端口
        uint16_t s0LocalPort;                   //(D1126)04 66  S0本机端口
        uint16_t s0TargetPort;                  //(D1127)04 67  S0目标端口
        uint16_t s1LocalPort;                   //(D1128)04 68  S1本机端口
        uint16_t s1TargetPort;                  //(D1129)04 69  S1目标端口（服务器 端口）
        uint16_t mqttPublishInterval;           //(D1130)04 6A  MQTT上传时间间隔（单位：分钟）
        uint16_t findRtuInterval;               //(D1131)04 6B  寻找RTU时，时间间隔（单位：ms）
        uint16_t getRtuInterval;                //(D1132)04 6C  获取RTU数据时，时间间隔（单位：ms）
        uint16_t getRtuCycleTime;               //(D1133)04 6D  两轮RTU数据之间的时间间隔（单位：ms）
                
        uint16_t AlarmEN;                       //(D1134)04 6E  DESX的雷击次数
        uint16_t GResGetTimes;                  //(D1135)04 6F  采集接地电阻的次数
        uint16_t GResGetInter;                  //(D1136)04 70  采集接地电阻时间间隔（单位：分钟）
        uint16_t GResVal;                       //(D1137)04 71  接地电阻阻值
                
        uint16_t mb_word_bytetype;              //(D1138)04 72  modbus协议寄存器存储方式（不启用）
        uint16_t mb_dword_wordtype;             //(D1139)04 73  modbus协议双寄存器存储方式
        uint16_t AlmOutput_THUp0;               //(D1140)04 74  报警输出设置 0：断开 1：闭合
        uint16_t AlmOutput_THUp1;               //(D1141)04 75  报警输出设置 0：断开 1：闭合
        uint16_t AlmOutput_SourceLogic0;        //(D1142)04 76  报警输出控制源判断逻辑
        uint16_t AlmOutput_SourceLogic1;        //(D1143)04 77  报警输出控制源判断逻辑
        uint16_t AlmOutput_Source[3];           //(D1144)04 78  报警输出控制源
        
        //DES系列产品补偿系数
        
        int16_t  VolB_B;                        //(D1147)04 7B  电压B相 B
        int16_t  VolC_B;                        //(D1148)04 7C  电压C相 B
        int16_t  CurA_B;                        //(D1149)04 7D  电流A相 B
        int16_t  CurB_B;                        //(D1150)04 7E  电流B相 B
        int16_t  CurC_B;                        //(D1151)04 7F  电流C相 B
        int16_t  Pwr_B;                         //(D1152)04 80  功率 B
        int16_t  PhsAngle_B;                    //(D1153)04 81  相位角 B
        
        //L1AI系列产品补偿系数
        int16_t  PAdc01_B;                      //(D1154)04 82  正Adc B   ADC<Adc_K01_K02时的系数
        int16_t  NAdc01_B;                      //(D1155)04 83  负Adc B
        int16_t  PAdc02_B;                      //(D1156)04 84  正Adc B   ADC>Adc_K01_K02时的系数
        int16_t  NAdc02_B;                      //(D1157)04 85  负Adc B
        //G1A3系列产品补偿系数
        int16_t  GRes_K;                        //(D1158)04 86  阻值矫正系数K
        int16_t  GRes_B;                        //(D1159)04 87  阻值矫正常数B
        //SPD寿命影响计算系数
        uint16_t Life_Temp_Up;                  //(D1160)04 88  温差门限
        uint16_t Life_Cur_Up;                   //(D1161)04 89  漏流门限
        uint16_t Life_Temp_K;                   //(D1162)04 8A  温度影响系数
        uint16_t Life_Cur_K;                    //(D1163)04 8B  漏流影响系数
        uint16_t Life_LT_K;                     //(D1164)04 8C  雷击次数影响系数
        //DES系列产品门限
        uint16_t SPD_YL_Up;                     //(D1165)04 8D  遥信
        uint16_t SPD_KL_Up;                     //(D1166)04 8E  空开
        uint16_t SPD_PE_Up;                     //(D1167)04 8F  接地
        uint16_t FREQ_Up;                       //(D1168)04 90  频率上限
        uint16_t FREQ_Down;                     //(D1169)04 91  频率下限
        uint16_t SPD_Temp_Up;                   //(D1170)04 92  温度上限
        uint16_t SPD_Temp_Down;                 //(D1171)04 93  温度下限
        uint16_t SPD_Cur_Up;                    //(D1172)04 94  电流上限
        uint16_t SPD_Cur_Down;                  //(D1173)04 95  电流下限

        uint16_t usADCCh11Threshold;            //(D1174)04 96  正雷击有效门限（ADC数字量门限）
        uint16_t usADCCh10Threshold;            //(D1175)04 97  负雷击有效门限（ADC数字量门限）

        uint16_t SPD_L1_Up;                     //(D1176)04 98  L1
        uint16_t SPD_L2_Up;                     //(D1177)04 99  L2
        uint16_t SPD_L3_Up;                     //(D1178)04 9A  L3
        uint16_t SPD_SW1_Up;                    //(D1179)04 9B  SW1
        uint16_t SPD_SW2_Up;                    //(D1180)04 9C  SW2
        uint16_t SPD_SW3_Up;                    //(D1181)04 9D  SW3
        uint16_t SPD_SW4_Up;                    //(D1182)04 9E  SW4

        uint16_t NTC_LowPassFilterBW;           //(D1183)04 9F  Use this number to configure the Vbus first order software filter bandwidth.

        uint16_t LtTimesFilter;                 //(D1184)04 A0  雷击计数滤波（响应雷击最快时间，单位ms）

        uint16_t changeGResFlag;                //(D1185)04 A1  =0：关闭阻值改变    >1：启动阻值改变
        uint16_t changeGResMin;                 //(D1186)04 A2  GResMin <= GRes <= GResMax时，改变阻值
        uint16_t changeGResMax;                 //(D1187)04 A3
        uint16_t changeGRes;                    //(D1188)04 A4  阻值改变值
        uint16_t changeGResWidth;               //(D1189)04 A5  阻值改变宽度

        //空闲输出电平
        uint16_t output_idle;                   //(D1190)04 A6  空闲输出电平 0：低电平 1：高电平
        uint16_t output_source;                 //(D1191)04 A7  逻辑输出源 0：指令输出 1：温度异常 2：漏流异常
        uint16_t uartparam[3];                  //(D1192)04 A8  串口配置参数

#if PROD_TYPE == PROD_FSS || PROD_TYPE == PROD_FD
        uint16_t Reserved_HalfWord[30];         //(D1315)05 23  预留16位模组寄存器
        uint16_t meter_cali[80];                //(D1345)05 41  计量校表参数
#elif PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
        uint16_t AlmState_Keep[5];              //(D1195)04 AB  报警保持
        uint16_t Cur_THUp;                      //(D1200)04 B0  电流上限
        uint16_t Cur_THDown;                    //(D1201)04 B1  电流下限
        int16_t Temp_THUp;                      //(D1202)04 B2  温度上限
        int16_t Temp_THDown;                    //(D1203)04 B3  温度下限

        uint16_t DI_THUp[4];                    //(D1204)04 B4  DI1阈值
        uint16_t eUI_THUp[4];                   //(D1208)04 B8  零序电压/负序电压/零序电流/负序电流不平衡度上限
        uint16_t THD_THUp[2];                   //(D1212)04 BC  电压/电流总谐波畸变率上限
        uint16_t HR_THUp[4];                    //(D1214)04 BE  电压奇次/电压偶次/电流奇次/电流偶次谐波含有率上限
        uint16_t Alm_Delay[6];                  //(D1218)04 C2  报警延时
        uint16_t fVol_PNUp;                     //(D1224)04 C8  零地电压报警阈值
        uint16_t meter_cali[80];                //(D1225)04 C9  计量校表参数
#if PROD_TYPE == PROD_SFE
        short DeltaKA0;                         //(D1305)05 19  A相负载电路总谐波
        short DeltaKA2;                         //(D1306)05 1A  A相负载电路二次谐波
        short DeltaKA3;                         //(D1307)05 1B  A相负载电路三次谐波
        short DeltaKB0;                         //(D1308)05 1C  B相负载电路总谐波
        short DeltaKB2;                         //(D1309)05 1D  B相负载电路二次谐波
        short DeltaKB3;                         //(D1310)05 1E  B相负载电路三次谐波
        short DeltaKC0;                         //(D1311)05 1F  C相负载电路总谐波
        short DeltaKC2;                         //(D1312)05 20  C相负载电路二次谐波
        short DeltaKC3;                         //(D1313)05 21  C相负载电路三次谐波
        uint16_t DeltaP[4];                     //(D1314)05 22  负载电路功率差值     
        
        uint16_t Malignantloaden;               //(D1318)05 26  恶性负载功能使能
        uint16_t MalignantloadPmin;             //(D1319)05 27  恶性负载功能功率监测最小值
        uint16_t MalignantloadPmax;             //(D1320)05 28  恶性负载功能功率监测最大值
        uint16_t Malignantloadmin;              //(D1321)05 29  恶性负载功能总谐波监测最小值
        uint16_t Malignantloadmax;              //(D1322)05 2A  恶性负载功能总谐波监测最大值
        uint16_t MalignantloadSmax;             //(D1323)05 2B  恶性负载功能总谐波监测最小值
        uint16_t MalignantloadTmin;             //(D1324)05 2C  恶性负载功能总谐波监测最大值    
#endif        
#else
#endif

        /* 占用 624 half-word(16-bit) 624=1024-200-200
         * size 1248 byte(8-bit)
        * mb start addr is 0x0591(D1425), end addr is 0x0800(D2048)
        * must save 1248 byte, no use words set in Reserved[]
        * for example: If use two bytes, must set Reserved[1246]
        */                                      //MdAddr
        uint8_t idInfo[24];                     //(D1325)05 2D  ID序列号
        uint8_t macAddr[6];                     //(D1337)05 39  本机MAC地址
        uint8_t gatewayIP[4];                   //(D1340)05 3C  网关IP地址
        uint8_t maskIP[4];                      //(D1342)05 3E  子网掩码
        uint8_t localIP[4];                     //(D1344)05 40  本机IP地址
        uint8_t s0TargetIP[4];                  //(D1346)05 42  S0目标IP地址
        uint8_t s1TargetIP[4];                  //(D1348)05 44  S1目标IP地址（服务器IP）
        uint8_t mqttUserName[20];               //(D1350)05 46  用户名
        uint8_t mqttPassword[20];               //(D1360)05 50  密码
        uint8_t mqttPub[80];                    //(D1370)05 5A  发布主题
        uint8_t mqttSub[80];                    //(D1410)05 82  订阅主题
        uint8_t mqttAlarmPub[80];               //(D1450)05 AA  发布报警主题
        uint8_t DnsServerIP[4];                 //(D1490)05 D2  dns server ip
        uint8_t domainName[50];                 //(D1492)05 D4  域名

        uint8_t Reserved_Byte[36];              //(D1517)05 ED   [802]=(1248-(([D1647]-[D1425])*2))-2

        uint8_t EndmagicNum[2];                 //(D1535)05 FF  flash存储最后要保存magicNum, wirte flash时掉电可能会导致后面部分被擦除,

    } st;
} flash_param_t;


//不存入flash的变量
typedef union
{
    uint16_t param_buff[PARAM_HALFWORDSIZE];  //size: half-word(16-bit)
    struct
    {
        /* 占用 30 half-word(16-bit)
        * size 15 word(32-bit)
        * mb start addr is 0x0801(D2049), end addr is 0x081E(D2078)
        * must save 15 word, no use words set in Reserved[]
        * for example: If use two words, must set Reserved[13]
        */                                      //MdAddr
        uint32_t SecCnt;                        //(D2049)08 01  运行的秒数

        float fCali_Cur;                        //(D2051)08 03  校准电流值
        float fCali_Vol;                        //(D2053)08 05  校准电流值
        float fCali_Pow;                        //(D2055)08 07  校准功率值
        float fCali_PF;                         //(D2057)08 09  校准功率因数值
        float fConfig_Pstartup;                 //(D2059)08 0B  设置起动功率百分比
        
        float fClai_Err[4];                     //(D2061)08 0D  误差值A/B/C/总
        uint32_t Module_Reg32_reserve[3];       //(D2069)08 15  [8]=10-(([D2063]-[D2059])/2) 预留32位模组寄存器

        /* 占用 230 half-word(16-bit)
         * size 230 half-word(16-bit)
        * mb start addr is 0x081F(D2079), end addr is 0x0904(D2308)
        * must save 30 half-word, no use words set in Reserved[]
        * for example: If use two half-words, must set Reserved[28]
        */                                       //MdAddr
        uint16_t version[4];                     //(D2075)08 1B  版本号(主版本号.副版本号.日期月日.一日的临时版本号)
//        uint16_t aADCCh11Buf[KEEP_ADC_SIZE];     //(D1789)08 23  峰值保持CH11_ADC的值
//        uint16_t aADCCh10Buf[KEEP_ADC_SIZE];     //(D2113)08 41  峰值保持CH10_ADC的值
//        uint16_t usADCCh11Average;               //(D2143)08 5F  峰值平均值
//        uint16_t usADCCh10Average;               //(D2144)08 60  峰值平均值
        uint16_t State_NetLinkErr;              //(D2079)08 1F  网络连接错误状态        
        //uint16_t aADC1Val[ADC_CONVERT_SIZE];     //(D2145)08 61  DMA存放adc值
        uint16_t NetLink_State;                  //(D2080)08 20  网络连接状态
        uint16_t HW_ID;                          //(D2081)08 21  硬件ID
        uint16_t ntcADC1Val[4];                  //(D2082)08 22  存放ntc热敏电阻的ADC
        uint16_t AlmState;                       //(D2086)08 26  报警状态
        uint16_t SysState01;                     //(D2087)08 27  系统运行状态
        //uint16_t Reserved_HalfWord[154];         //(D2155)08 6B  [154]=230-([D2155]-[D2079])
        uint16_t State_Alarm[10];                //(D2088)08 28  报警状态
        uint16_t State_SystemErr;                //(D2098)08 32  系统错误状态
        uint16_t Meter_CF_Interval_Time[4];      //(D2099)08 33  CF脉冲频率（ms）
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
        uint16_t AlmOutput0;                     //(D2103)08 37
        uint16_t AlmOutput1;                     //(D2103)08 37
        uint16_t State_NtcConnect;               //(D2105)08 38  Ntc连接状态
        uint16_t AdcVal[8];                      //(D2106)08 39  采样AD值
#else
        uint16_t Reserved_HalfWord[143];         //(D2165)08 7C  [154]=230-([D2155]-[D2079])
#endif


        /* 占用 40 half-word(16-bit) 40=100-30-30
         * size 80 byte(8-bit)
        * mb start addr is 0x0905(D2309), end addr is 0x092C(D2348)
        * must save 80 byte, no use words set in Reserved[]
        * for example: If use two bytes, must set Reserved[78]
        */                                      //MdAddr
        uint8_t FromDnsServerIP[4];             //(D2114)08 42  IP from dns server
        uint8_t Reserved_Byte[2];               //(D2116)08 44  [76]=80-(([D2311]-[D2309])*2)

    } st;
} param_t;

//存放变量存入flash时的偏移
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
typedef struct
{
    uint32_t energy;    //电能数据

} flash_offset_t;
#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB || PROD_TYPE == PROD_ESA */

#define WORD_ByteH_ByteL       0x0101
#define WORD_ByteL_ByteH       0x0202
#define DWORD_WordH_WordL      0x0303  //高位在前
#define DWORD_WordL_WordH      0x0404  //低位在前

/* Exported constants --------------------------------------------------------*/
extern flash_param_t gFlashParam;
extern volatile param_t gParam;

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
extern flash_offset_t gflashOffset;
#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB || PROD_TYPE == PROD_ESA */

//extern LtList_st gLtList;
//extern uint16_t gLtElemIdx; //雷电流元素索引

//extern FS_Elem_st  gFS_Elem;
//extern FSS_Elem_st gFSS_Elem;

/* Private functions ---------------------------------------------------------*/
extern void bsp_erase_page(unsigned int pageAddress, unsigned short pageNum);
extern void PRINTF_FLASH(unsigned int addr, unsigned int len);

extern void Parameter_FlashWrite(unsigned int addr, void *pBuf, unsigned int len);
extern void Parameter_FlashRead(unsigned int addr, void *pBuf, unsigned int len);
extern void Parameter_FlashWrite_InOnePage(unsigned int addr, unsigned int len, void *pBuf, unsigned int *offset);
extern void Parameter_FlashRead_InOnePage(unsigned int addr, unsigned int len, void *pBuf, unsigned int *offset);


void Parameter_Init(void);

/* Thunder current monitor ----------------------------------------*/
//void PrintLtElem(LtElem_st LtElem);
//void PrintLtList(LtList_st LtList);
//void LtListFlashWrite(LtElem_st LtElem);
//void LtListFlashRead(uint16_t i, LtElem_st *LtElem);
//bool LtListFlashDelete(uint16_t i, LtElem_st *LtElem);
//void LtListInit(void);

#endif /* __APP_PARAMETER_H */
