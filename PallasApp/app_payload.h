
#ifndef __APP_PAYLOAD_H
#define __APP_PAYLOAD_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
/* Control point payload type ----------------------------- */
#define  PYLD_IO_TYPE            0 //如DES4的遥信、空开、接地，L1AI的峰值极性的数字量，按照IO量上传
#define  PYLD_X1_ANALOG_TYPE     1 //如DES4的雷击次数，L1AI的雷击次数、持续时间，G1A3的采集次数、采集频率（都是按值直接上传）
#define  PYLD_X10_ANALOG_TYPE    2 //如DES9的漏流
#define  PYLD_X10_N_ANALOG_TYPE  3 //有负值，如DES9的温度
#define  PYLD_X100_ANALOG_TYPE   4 //如G1A3的地网阻值

/* Exported types ------------------------------------------------------------*/
typedef struct PAYLOAD_STRING
{
    uint8_t IdStr[17];
    uint8_t RtuIdStr[20];
    uint8_t TerminalTypeStr[5];
    uint8_t MonitorTypeStr[2];
    uint8_t DataTypeStr[5];
    uint8_t ValueStr[10];
    uint8_t Alarm1Str[4];
    uint8_t Alarm2Str[4];
    uint8_t Alarm1UpStr[10];
    uint8_t Alarm1DownStr[10];
    uint8_t Alarm2UpStr[10];
    uint8_t Alarm2DownStr[10];
    uint8_t AlertStr[25];

} payload_string_t; //payload的文字



/* Exported constants --------------------------------------------------------*/
extern __IO uint8_t gPyldRtu_Idx;
extern __IO uint8_t gPyldP_Idx;
extern __IO uint8_t gPyldOneP_Idx;


/* Private functions ---------------------------------------------------------*/
void GetOnePayload(char *payload);
void kalyke_cycle_post_TEST(char *payload);
void UnpackPayLoad(char *payload);

#endif /* __APP_PAYLOAD_H */
