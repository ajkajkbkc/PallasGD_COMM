
#ifndef __APP_COLLECT_H
#define __APP_COLLECT_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "main.h"


/* Private defines -----------------------------------------------------------*/

#define  MAX_CTRLP_NUM         1//360  //最多存储控制点数
#define  MAX_RTU_NUM           1//30   //最多连接终端个数

/* Production type -------------------------------------------- */
#define  TYPE_DES3        0  //海雷克3要素（特殊：读取指令不一样）
#define  TYPE_DES4        1
#define  TYPE_DES9        2
#define  TYPE_DESA        3
#define  TYPE_G1A3        4
#define  TYPE_M1AI        5
#define  TYPE_L2AI        6
#define  TYPE_LDA1        7
#define  TYPE_DEM2        8
#define  TYPE_DEM4        9
#define  TYPE_DEP4        10
#define  MAX_PRODEUCT_TYPE     11    //产品类型数量
/*
if u want to add new TYPE_XX, u must add attribute in the following functions:
1. gatewayparam_init
2. Handle_RtuCtrlP_AlmLimType
3. Handle_RtuCtrlP_PyldType
4. Handle_RtuCtrlP_CtrlPTypeStr
5. Handle_FindFlkRtu_RecvData
6. Handle_RtuType_ToSend
7. is_RtuReturn_protocol
*/


/* Rtu work state --------------------------------------------- */
#define RTU_STATE_NORMAL  0  //正常
#define RTU_STATE_LOSE    1  //失联

/* Alarm level ------------------------------------------------ */
#define MAX_ALARM_LEVEL   2
#define ALARM_LEVEL_1     0
#define ALARM_LEVEL_2     1
#define ALARM_LEVEL_3     2
#define ALARM_LEVEL_4     3

/* Valid Alarm Type ----------------------------------------------
    0: invalid   1: valid
    ALARM_1_VALID      //一级报警
    ALARM_2_VALID      //二级报警
    ALARM_3_VALID      //三级报警
    ALARM_4_VALID      //四级报警
 */
#define ALARM_NO_VALID              (0U)
#define ALARM_1_VALID_Pos           (0U)
#define ALARM_1_VALID_Msk           (0x1UL << ALARM_1_VALID_Pos)
#define ALARM_1_VALID               ALARM_1_VALID_Msk
#define ALARM_2_VALID_Pos           (1U)
#define ALARM_2_VALID_Msk           (0x1UL << ALARM_2_VALID_Pos)
#define ALARM_2_VALID               ALARM_2_VALID_Msk
#define ALARM_3_VALID_Pos           (2U)
#define ALARM_3_VALID_Msk           (0x1UL << ALARM_3_VALID_Pos)
#define ALARM_3_VALID               ALARM_3_VALID_Msk
#define ALARM_4_VALID_Pos           (3U)
#define ALARM_4_VALID_Msk           (0x1UL << ALARM_4_VALID_Pos)
#define ALARM_4_VALID               ALARM_4_VALID_Msk

/* Alarm State ------------------------------------------------ */
/*       采集Rtu数据判断报警            MQTT发送完报警             采集Rtu数据判断解除报警              MQTT发送完解除报警报警
   NORMAL------------------->ALARM_START-------------->ALARM_RUNING----------------------->ALARM_RELEASE---------------------
     ^-----------------------------------------------------------------------------------------------------------------------|
*/
#define STATE_NORMAL          0
#define STATE_ALARM_START     1
#define STATE_ALARM_RUNING    2
#define STATE_ALARM_RELEASE   3

/* Find Rtu time interval, unit: ms --------------------------- */
//#define  FIND_RTU_INTERVAL      200  //寻找RTU时，时间间隔(ms)
//#define  GET_RTU_INTERVAL       200  //获取RTU数据时，时间间隔(ms)
//#define  GET_ALL_RTU_INTERVAL   1000 //获取所有RTU数据时，时间间隔(ms)

/* Rtu lose max count ----------------------------------------- */
#define  RTU_LOSE_MAX_CNT       10    //连续x次没有收到RTU的数据算是RTU失联

/* Control point alarm limit type ----------------------------- */
#define  NO_ALMLIM                 0 //没有报警概念的数据，如采集次数
#define  HAVE_UP_DOWN_ALMLIM       1 //有报警上限和下限的模拟量
#define  HAVE_UP_ALMLIM            2 //只有报警上限的模拟量
#define  HAVE_IO_ALMLIM            3 //只有0和1的数字量


/* Exported types ------------------------------------------------------------*/

typedef struct _RTU_INFO
{
    uint8_t RtuIdNum;      //终端站号
    uint8_t RtuIdInfo[22]; //终端ID号
    uint8_t RtuType;       //终端类型
} rtu_info_t;  //RTU终端信息

typedef struct _RTU_INFO_PACKET
{
    uint32_t RtuNum;
    rtu_info_t Rtu_Info[MAX_RTU_NUM];
} rtu_info_packet_t;  //RTU信息包

typedef struct _RTU_WORKSTATE
{
    uint8_t RtuWorkState;  //工作状态
    uint8_t RtuLoseCnt;  //失联次数
} rtu_workstate_t;


typedef struct _CTRLPOINT_ALAM_LIMIT
{
    int16_t Alm1_Up;      //一级报警上限
    int16_t Alm1_Down;    //一级报警下限
    int16_t Alm2_Up;      //二级报警上限
    int16_t Alm2_Down;    //二级报警下限
    uint16_t ValidAlmType; //有效(已开启)的报警类型
} ctrlp_alarmlimit_t;  //控制点报警门限结构体

typedef struct _CTRLPOINT_INFO
{
    int16_t OldValue;                      //旧值
    int16_t NewValue;                      //新值
    uint8_t AlmState[MAX_ALARM_LEVEL];     //报警状态
} ctrlp_info_t; //控制点信息





/* Exported constants --------------------------------------------------------*/
extern const uint8_t CtrlPNum_Table[MAX_PRODEUCT_TYPE];

extern rtu_info_packet_t  gRtu_InfoPacket;   //要存入flash
extern rtu_workstate_t    gRtu_WorkState[MAX_RTU_NUM];
extern ctrlp_alarmlimit_t gCtrlP_AlmLim[MAX_CTRLP_NUM];//要存入flash
extern ctrlp_info_t       gCtrlP_Info[MAX_CTRLP_NUM];

extern uint8_t            gRtu_Num;
extern __IO uint16_t      gRtu_Idx;     //终端查询计数Index
extern uint16_t           gCtrlP_Num;   //已有的控制点数量
extern __IO uint16_t      gCtrlP_Idx;   //控制点查询计数Index

extern uint8_t gGetAtLeastOne;

/* Private functions ---------------------------------------------------------*/
void gatewayparam_init(void);
void Gateway_KeyTask(uint8_t operate);
void Handle_FindFlkRtu_RecvData(uint8_t *pBuf, uint16_t len);
void Handle_FindHlkRtu_RecvData(uint8_t *pBuf, uint16_t len);
bool is_RtuReturn_protocol(uint8_t *pBuf, uint16_t len);
void AutoFindRtu(void);

void osThreadNew_collectTask(void);

void CtrlPMutexInit(void);
int CtrlPMutexLock(void);
int CtrlPMutexUnlock(void);

#endif /* __APP_COLLECT_H */
