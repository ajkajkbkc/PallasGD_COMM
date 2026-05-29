
/* Private includes ----------------------------------------------------------*/
#include <string.h>

#include "app_payload.h"
#include "app_log.h"
#include "app_rtc.h"
#include "app_parameter.h"
#include "app_collect.h"
#include "app_tool.h"
#include "CJSON.h"

#include "FreeRTOS.h"
#include "app_oled.h"

#include "plc_netcfg.h"
#include "plc_element.h"
#include "kalyke_internet_task.h"
#include "app_att7022eu.h"
#include "module_ESE.h"
/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
__IO uint8_t gPyldRtu_Idx = 0;
__IO uint8_t gPyldP_Idx = 0;
__IO uint8_t gPyldOneP_Idx = 0;


/* Private function prototypes -----------------------------------------------*/



/* Private user code ---------------------------------------------------------*/



/**
  * @brief  判断控制点payload类型
  * @param  Rtu_Type: 产品类型
  * @param  OneRtuCtrlP_Idx: 各产品的第几个数据
  * @retval
  */
uint8_t Handle_RtuCtrlP_PyldType(uint8_t Rtu_Type, uint8_t OneRtuCtrlP_Idx)
{
    uint8_t ret;

    if( (Rtu_Type == TYPE_DES3) || (Rtu_Type == TYPE_DES4) || (Rtu_Type == TYPE_DES9) || (Rtu_Type == TYPE_DESA) )
    {
        if(OneRtuCtrlP_Idx < 3) //遥信 空开 接地
        {
            ret = PYLD_IO_TYPE;
        }
        else if(OneRtuCtrlP_Idx == 3) //雷击计数
        {
            ret = PYLD_X1_ANALOG_TYPE;
        }
        else if( (OneRtuCtrlP_Idx == 7) || (OneRtuCtrlP_Idx == 8) ) //温度
        {
            ret = PYLD_X10_N_ANALOG_TYPE;
        }
        else  //电压 电流 寿命
        {
            ret = PYLD_X10_ANALOG_TYPE;
        }
    }
    else if(Rtu_Type == TYPE_DEP4)
    {
        if(OneRtuCtrlP_Idx < 3) //遥信 空开 接地
        {
            ret = PYLD_IO_TYPE;
        }
        else if(OneRtuCtrlP_Idx == 3) //雷击计数
        {
            ret = PYLD_X1_ANALOG_TYPE;
        }
        else if(OneRtuCtrlP_Idx == 4) //温度
        {
            ret = PYLD_X10_N_ANALOG_TYPE;
        }
    }
    else if(Rtu_Type == TYPE_G1A3)
    {
        if( (OneRtuCtrlP_Idx == 0) || (OneRtuCtrlP_Idx == 1) ) //采集次数 采集频率
        {
            ret = PYLD_X1_ANALOG_TYPE;
        }
        else  //主地网 辅助地网1 辅助地网2
        {
            ret = PYLD_X100_ANALOG_TYPE;
        }
    }
    else if( (Rtu_Type == TYPE_M1AI) || (Rtu_Type == TYPE_L2AI) )
    {
        if((OneRtuCtrlP_Idx == 1) || (OneRtuCtrlP_Idx >= 4) ) //峰值 电荷量 单位能量
        {
            ret = PYLD_X10_ANALOG_TYPE;
        }
        else if(OneRtuCtrlP_Idx == 2)  //极性
        {
            ret = PYLD_IO_TYPE;
        }
        else //雷击次数 持续时间
        {
            ret = PYLD_X1_ANALOG_TYPE;
        }
    }
    else if( Rtu_Type == TYPE_LDA1 )
    {
        ret = PYLD_X10_ANALOG_TYPE;
    }
    else if( (Rtu_Type == TYPE_DEM2) || (Rtu_Type == TYPE_DEM4) )
    {
        if(OneRtuCtrlP_Idx < 4) //电压状态1 接地 脱扣1 脱扣2
        {
            ret = PYLD_IO_TYPE;
        }
        else if(OneRtuCtrlP_Idx < 5) //计数
        {
            ret = PYLD_X1_ANALOG_TYPE;
        }
        else if(OneRtuCtrlP_Idx < 6) //电流1
        {
            ret = PYLD_X10_ANALOG_TYPE;
        }
        else if(OneRtuCtrlP_Idx < 8) //温度1 温度2
        {
            ret = PYLD_X10_N_ANALOG_TYPE;
        }
        else if(OneRtuCtrlP_Idx < 9) //寿命
        {
            ret = PYLD_X10_ANALOG_TYPE;
        }
        else if(OneRtuCtrlP_Idx < 13)  //电压状态2 电压状态3 脱扣3 脱扣4
        {
            ret = PYLD_IO_TYPE;
        }
        else if(OneRtuCtrlP_Idx < 15)  //电流2 电流3
        {
            ret = PYLD_X10_ANALOG_TYPE;
        }
        else  //温度3 温度4
        {
            ret = PYLD_X10_N_ANALOG_TYPE;
        }
    }
    else
    {
        ret = 0xFF;
    }

    return ret;
}

/**
  * @brief  根据产品型号定义控制点数据类型
  * @param  Rtu_Type: 产品类型
  * @param  OneRtuCtrlP_Idx: 各产品的第几个数据
  * @param  *pCtrlPTypeStr: 返回的字符串
  * @retval None
  */
void Handle_RtuCtrlP_CtrlPTypeStr(uint8_t Rtu_Type, uint8_t OneRtuCtrlP_Idx, uint8_t *pCtrlPTypeStr)
{
    if( (Rtu_Type == TYPE_DES3) || (Rtu_Type == TYPE_DES4) || (Rtu_Type == TYPE_DES9) || (Rtu_Type == TYPE_DESA) )
    {
        if(0 == OneRtuCtrlP_Idx) //遥信
        {
            *pCtrlPTypeStr = 'Y';
        }
        else if(1 == OneRtuCtrlP_Idx) //空开
        {
            *pCtrlPTypeStr = 'K';
        }
        else if(2 == OneRtuCtrlP_Idx) //接地
        {
            *pCtrlPTypeStr = 'G';
        }
        else if(3 == OneRtuCtrlP_Idx) //雷击次数
        {
            *pCtrlPTypeStr = 'J';
        }
        else if(7 > OneRtuCtrlP_Idx) //电流
        {
            *pCtrlPTypeStr = 'A';
        }
        else if(9 > OneRtuCtrlP_Idx) //温度
        {
            *pCtrlPTypeStr = 'T';
        }
        else if(12 > OneRtuCtrlP_Idx)//电压
        {
            *pCtrlPTypeStr = 'V';
        }
        else //寿命
        {
            *pCtrlPTypeStr = 'P';
        }
    }
    else if(Rtu_Type == TYPE_DEP4)
    {
        if(0 == OneRtuCtrlP_Idx) //遥信
        {
            *pCtrlPTypeStr = 'Y';
        }
        else if(1 == OneRtuCtrlP_Idx) //空开
        {
            *pCtrlPTypeStr = 'K';
        }
        else if(2 == OneRtuCtrlP_Idx) //接地
        {
            *pCtrlPTypeStr = 'G';
        }
        else if(3 == OneRtuCtrlP_Idx) //雷击次数
        {
            *pCtrlPTypeStr = 'J';
        }
        else if(4 == OneRtuCtrlP_Idx) //温度
        {
            *pCtrlPTypeStr = 'T';
        }
    }
    else if(Rtu_Type == TYPE_G1A3)
    {
        if(0 == OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "DJ", 2); //采集次数
        }
        else if(1 == OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "MIN", 3); //采集频率
        }
        else if(2 == OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "DR", 2); //主地网
        }
        else if(3 == OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "ZR1", 3); //辅助地网1
        }
        else
        {
            memcpy(pCtrlPTypeStr, "ZR2", 3); //辅助地网2
        }
    }
    else if( (Rtu_Type == TYPE_M1AI) || (Rtu_Type == TYPE_L2AI) )
    {
        if(0 == OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "LJ", 2); //次数
        }
        else if(1 == OneRtuCtrlP_Idx)
        {
            if(Rtu_Type == TYPE_L2AI)
            {
                memcpy(pCtrlPTypeStr, "F", 2); //峰值
            }
            else
            {
                memcpy(pCtrlPTypeStr, "LF", 2); //峰值
            }
        }
        else if(2 == OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "JX", 2); //极性
        }
        else if(3 == OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "LT", 2); //持续时间
        }
        else if(4 == OneRtuCtrlP_Idx)
        {
            *pCtrlPTypeStr = 'Q';           //电荷量
        }
        else
        {
            memcpy(pCtrlPTypeStr, "WR", 2); //单位能量
        }
    }
    else if(Rtu_Type == TYPE_LDA1)
    {
        memcpy(pCtrlPTypeStr, "LA", 2);     //漏电流
    }
    else if( (Rtu_Type == TYPE_DEM2) || (Rtu_Type == TYPE_DEM4) )
    {
        if(1 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "VS", 2); //电压状态1
        }
        else if(2 > OneRtuCtrlP_Idx)
        {
            *pCtrlPTypeStr = 'G';           //接地
        }
        else if(4 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "SW", 2); //脱扣1 脱扣2
        }
        else if(5 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "J", 1); //计数
        }
        else if(6 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "A", 1); //电流1
        }
        else if(8 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "T", 1); //温度1 温度2
        }
        else if(9 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "P", 1); //寿命
        }
        else if(11 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "VS", 2); //电压状态2 电压状态3
        }
        else if(13 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "SW", 2); //脱扣3 脱扣4
        }
        else if(15 > OneRtuCtrlP_Idx)
        {
            memcpy(pCtrlPTypeStr, "A", 1); //电流2 电流3
        }
        else
        {
            memcpy(pCtrlPTypeStr, "T", 1); //温度3 温度4
        }
    }
    else
    {
        *pCtrlPTypeStr = 'X';
        LOGE("payload", "unkown Rtu type in %s()", __func__);
    }
}

/**
  * @brief  获取控制点值字符串
  * @param  Rtu_Type: 产品类型
  * @param  OneRtuCtrlP_Idx: 各产品的第几个数据
  * @param  CtrlP_Idx: 输入总控制点中的第几个控制点
  * @param  *pMonitorTypeStr: 返回该控制点的模拟类型字符串
  * @param  *pValueStr: 返回该控制点的值字符串
  * @retval None
  */
void Handle_RtuCtrlP_CtrlPValStr(uint8_t Rtu_Type, uint8_t OneRtuCtrlP_Idx, uint16_t CtrlP_Idx, uint8_t *pMonitorTypeStr, uint8_t *pValueStr)
{
    uint8_t tempStr[12];
    uint8_t len;
    uint16_t temp;

    if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_IO_TYPE)
    {
        pMonitorTypeStr[0] = 'D';
        len = Val_To_String(0, gCtrlP_Info[CtrlP_Idx].NewValue, tempStr);
        memcpy(pValueStr, tempStr, len);
    }
    else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X1_ANALOG_TYPE)
    {
        pMonitorTypeStr[0] = 'A';
        len = Val_To_String(0, gCtrlP_Info[CtrlP_Idx].NewValue, tempStr);
        memcpy(pValueStr, tempStr, len);
    }
    else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X10_ANALOG_TYPE)
    {
        pMonitorTypeStr[0] = 'A';
        len = ValDivideBy10_To_String(0, gCtrlP_Info[CtrlP_Idx].NewValue, tempStr);
        memcpy(pValueStr, tempStr, len);
    }
    else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X10_N_ANALOG_TYPE)
    {
        pMonitorTypeStr[0] = 'A';
        if(gCtrlP_Info[CtrlP_Idx].NewValue < 0) //采样值为负值
        {
            temp = -gCtrlP_Info[CtrlP_Idx].NewValue;
            len = ValDivideBy10_To_String(0, temp, tempStr);
            pValueStr[0] = '-';
            memcpy(&pValueStr[1], tempStr, len);
        }
        else
        {
            len = ValDivideBy10_To_String(0, gCtrlP_Info[CtrlP_Idx].NewValue, tempStr);
            memcpy(pValueStr, tempStr, len);
        }
    }
    else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X100_ANALOG_TYPE)
    {
        pMonitorTypeStr[0] = 'A';
        len = ValDivideBy100_To_String(gCtrlP_Info[CtrlP_Idx].NewValue, tempStr);
        memcpy(pValueStr, tempStr, len);
    }
    else
    {
        pMonitorTypeStr[0] = 'X';
        len = Val_To_String(0, gCtrlP_Info[CtrlP_Idx].NewValue, tempStr);
        memcpy(pValueStr, tempStr, len);
        LOGE("payload", "unkown payload type in %s()", __func__);
    }
}

/**
  * @brief  获取控制点报警信息字符串
  * @param  Rtu_Type: 产品类型
  * @param  OneRtuCtrlP_Idx: 各产品的第几个数据
  * @param  CtrlP_Idx: 输入总控制点中的第几个控制点
  * @param  Alarm1Str: 返回该控制点的报警信息(level 1)
  * @param  Alarm1UpStr: 返回该控制点的报警上限(level 1)
  * @param  Alarm1DownStr: 返回该控制点的报警下限(level 1)
  * @param  Alarm2Str: 返回该控制点的报警信息(level 2)
  * @param  Alarm2UpStr: 返回该控制点的报警上限(level 2)
  * @param  Alarm2DownStr: 返回该控制点的报警下限(level 2)
  * @retval None
  */
void Handle_RtuCtrlP_CtrlPAlmStr(uint8_t Rtu_Type, uint8_t OneRtuCtrlP_Idx, uint16_t CtrlP_Idx,
                                 uint8_t *Alarm1Str, uint8_t *Alarm1UpStr, uint8_t *Alarm1DownStr,
                                 uint8_t *Alarm2Str, uint8_t *Alarm2UpStr, uint8_t *Alarm2DownStr)
{
    uint8_t tempStr[12];
    uint8_t len;
    uint16_t temp;

    //"D"
    if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_IO_TYPE)
    {
        //alarm 1
        if(PAR_READ_BIT(gCtrlP_AlmLim[CtrlP_Idx].ValidAlmType, ALARM_1_VALID))
        {
            if(gCtrlP_AlmLim[CtrlP_Idx].Alm1_Up == 0)
            {
                memcpy(Alarm1Str, "0", 1);
            }
            else
            {
                memcpy(Alarm1Str, "1", 1);
            }
        }
        else
        {
            memcpy(Alarm1Str, "N/A", 3);
        }
        //alarm 2
        if(PAR_READ_BIT(gCtrlP_AlmLim[CtrlP_Idx].ValidAlmType, ALARM_2_VALID))
        {
            if(gCtrlP_AlmLim[CtrlP_Idx].Alm2_Up == 0)
            {
                memcpy(Alarm2Str, "0", 1);
            }
            else
            {
                memcpy(Alarm2Str, "1", 1);
            }
        }
        else
        {
            memcpy(Alarm2Str, "N/A", 3);
        }
    }
    //"A"
    else
    {
        //alarm 1
        if(PAR_READ_BIT(gCtrlP_AlmLim[CtrlP_Idx].ValidAlmType, ALARM_1_VALID))
        {
            if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X100_ANALOG_TYPE)
            {
                len = ValDivideBy100_To_String(gCtrlP_AlmLim[CtrlP_Idx].Alm1_Up, tempStr);
                memcpy(Alarm1UpStr, tempStr, len);
                len = ValDivideBy100_To_String(gCtrlP_AlmLim[CtrlP_Idx].Alm1_Down, tempStr);
                memcpy(Alarm1DownStr, tempStr, len);
            }
            else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X10_N_ANALOG_TYPE)
            {
                if(gCtrlP_AlmLim[CtrlP_Idx].Alm1_Up < 0) //值为负值
                {
                    temp = - gCtrlP_AlmLim[CtrlP_Idx].Alm1_Up;
                    len = ValDivideBy10_To_String(0, temp, tempStr);
                    Alarm1UpStr[0] = '-';
                    memcpy(&Alarm1UpStr[1], tempStr, len);
                }
                else
                {
                    len = ValDivideBy10_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm1_Up, tempStr);
                    memcpy(Alarm1UpStr, tempStr, len);
                }

                if(gCtrlP_AlmLim[CtrlP_Idx].Alm1_Down < 0) //值为负值
                {
                    temp = - gCtrlP_AlmLim[CtrlP_Idx].Alm1_Down;
                    len = ValDivideBy10_To_String(0, temp, tempStr);
                    Alarm1DownStr[0] = '-';
                    memcpy(&Alarm1DownStr[1], tempStr, len);
                }
                else
                {
                    len = ValDivideBy10_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm1_Down, tempStr);
                    memcpy(Alarm1DownStr, tempStr, len);
                }
            }
            else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X10_ANALOG_TYPE)
            {
                len = ValDivideBy10_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm1_Up, tempStr);
                memcpy(Alarm1UpStr, tempStr, len);
                len = ValDivideBy10_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm1_Down, tempStr);
                memcpy(Alarm1DownStr, tempStr, len);
            }
            else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X1_ANALOG_TYPE)
            {
                len = Val_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm1_Up, tempStr);
                memcpy(Alarm1UpStr, tempStr, len);
                len = Val_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm1_Down, tempStr);
                memcpy(Alarm1DownStr, tempStr, len);
            }
        }
        else
        {
            memcpy(Alarm1UpStr, "N/A", 3);
            memcpy(Alarm1DownStr, "N/A", 3);
        }

        //alarm 2
        if(PAR_READ_BIT(gCtrlP_AlmLim[CtrlP_Idx].ValidAlmType, ALARM_2_VALID))
        {
            if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X100_ANALOG_TYPE)
            {
                len = ValDivideBy100_To_String(gCtrlP_AlmLim[CtrlP_Idx].Alm2_Up, tempStr);
                memcpy(Alarm2UpStr, tempStr, len);
                len = ValDivideBy100_To_String(gCtrlP_AlmLim[CtrlP_Idx].Alm2_Down, tempStr);
                memcpy(Alarm2DownStr, tempStr, len);
            }
            else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X10_N_ANALOG_TYPE)
            {
                if(gCtrlP_AlmLim[CtrlP_Idx].Alm2_Up < 0) //值为负值
                {
                    temp = - gCtrlP_AlmLim[CtrlP_Idx].Alm2_Up;
                    len = ValDivideBy10_To_String(0, temp, tempStr);
                    Alarm2UpStr[0] = '-';
                    memcpy(&Alarm2UpStr[1], tempStr, len);
                }
                else
                {
                    len = ValDivideBy10_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm2_Up, tempStr);
                    memcpy(Alarm2UpStr, tempStr, len);
                }

                if(gCtrlP_AlmLim[CtrlP_Idx].Alm2_Down < 0) //值为负值
                {
                    temp = - gCtrlP_AlmLim[CtrlP_Idx].Alm2_Down;
                    len = ValDivideBy10_To_String(0, temp, tempStr);
                    Alarm2DownStr[0] = '-';
                    memcpy(&Alarm2DownStr[1], tempStr, len);
                }
                else
                {
                    len = ValDivideBy10_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm2_Down, tempStr);
                    memcpy(Alarm2DownStr, tempStr, len);
                }
            }
            else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X10_ANALOG_TYPE)
            {
                len = ValDivideBy10_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm2_Up, tempStr);
                memcpy(Alarm2UpStr, tempStr, len);
                len = ValDivideBy10_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm2_Down, tempStr);
                memcpy(Alarm2DownStr, tempStr, len);
            }
            else if(Handle_RtuCtrlP_PyldType(Rtu_Type, OneRtuCtrlP_Idx) == PYLD_X1_ANALOG_TYPE)
            {
                len = Val_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm2_Up, tempStr);
                memcpy(Alarm2UpStr, tempStr, len);
                len = Val_To_String(0, gCtrlP_AlmLim[CtrlP_Idx].Alm2_Down, tempStr);
                memcpy(Alarm2DownStr, tempStr, len);
            }
        }
        else
        {
            memcpy(Alarm2UpStr, "N/A", 3);
            memcpy(Alarm2DownStr, "N/A", 3);
        }
    }
}

/**
  * @brief  获取报警状态字符串
  * @param  Rtu_Idx: 终端查询计数Index
  * @param  CtrlP_Idx: 输入总控制点中的第几个控制点
  * @param  AlertStr: 报警状态信息
  * @retval None
  */
void Handle_RtuCtrlP_CtrlPAltStr(uint8_t Rtu_Idx, uint16_t CtrlP_Idx, uint8_t *AlertStr)
{
    uint8_t len = 0;

    if(gRtu_WorkState[Rtu_Idx].RtuWorkState == RTU_STATE_LOSE) //掉线
    {
        memcpy(AlertStr, "ERROR", 5);
        return ;
    }

    if( (gCtrlP_Info[CtrlP_Idx].AlmState[ALARM_LEVEL_1] == STATE_NORMAL) || (gCtrlP_Info[CtrlP_Idx].AlmState[ALARM_LEVEL_1] == STATE_ALARM_RELEASE) )
    {
        if( (gCtrlP_Info[CtrlP_Idx].AlmState[ALARM_LEVEL_2] == STATE_NORMAL) || (gCtrlP_Info[CtrlP_Idx].AlmState[ALARM_LEVEL_2] == STATE_ALARM_RELEASE) )
        {
            memcpy(AlertStr, "NORMAL", 6);
            return ;
        }
    }

    if( (gCtrlP_Info[CtrlP_Idx].AlmState[ALARM_LEVEL_1] == STATE_ALARM_RUNING) || (gCtrlP_Info[CtrlP_Idx].AlmState[ALARM_LEVEL_1] == STATE_ALARM_START) )
    {
        memcpy(&AlertStr[len], "ALARM1", 6);
        len += 6;
    }
    if( (gCtrlP_Info[CtrlP_Idx].AlmState[ALARM_LEVEL_2] == STATE_ALARM_RUNING) || (gCtrlP_Info[CtrlP_Idx].AlmState[ALARM_LEVEL_2] == STATE_ALARM_START) )
    {
        if (0 != len)
        {
            AlertStr[len] = ',';
            len++;
        }
        memcpy(&AlertStr[len], "ALARM2", 6);
        len += 6;
    }
}


/**
  * @brief  装载一个值
  * @param  输入数值
  * @retval None
  */
void GetOnePayload(char *payload)
{
    LOGV("payload", "Enter %s()", __func__);
    payload_string_t pyldStr;
    rtc_datetime_t datetime;
    uint8_t cyBuf[12];

    memset(&pyldStr, '\0', sizeof(pyldStr));

    //KSDC120200630001
    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);

    //KSDDES4200630001_01
    memcpy(pyldStr.RtuIdStr, (uint8_t *)&gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuIdInfo, 16);
    pyldStr.RtuIdStr[16] = '_';
    sprintf((char *)cyBuf, "%02u", gPyldOneP_Idx + 1);
    memcpy(&pyldStr.RtuIdStr[17], cyBuf, 2);

    //DES4
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuIdInfo[18], 4);

    //K
    Handle_RtuCtrlP_CtrlPTypeStr(gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType, gPyldOneP_Idx, pyldStr.DataTypeStr);

    //D 0
    Handle_RtuCtrlP_CtrlPValStr(gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType, gPyldOneP_Idx, gPyldP_Idx, pyldStr.MonitorTypeStr, pyldStr.ValueStr);

    //N/A
    Handle_RtuCtrlP_CtrlPAlmStr(gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType, gPyldOneP_Idx, gPyldP_Idx,
                                pyldStr.Alarm1Str, pyldStr.Alarm1UpStr, pyldStr.Alarm1DownStr,
                                pyldStr.Alarm2Str, pyldStr.Alarm2UpStr, pyldStr.Alarm2DownStr);

    //NORMAL
    Handle_RtuCtrlP_CtrlPAltStr(gPyldRtu_Idx, gPyldP_Idx, pyldStr.AlertStr);

    RTC_GetDatetime(&datetime);

    if(Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType, gPyldOneP_Idx) == PYLD_IO_TYPE)
    {
        sprintf(payload, "{\"header\":{\"client\":\"%s\",\"time\":\"%d/%d/%d %d:%d:%d\"},\
\"data\":{\"id\":\"%s\",\
\"terminal_type\":\"%s\",\
\"monitor_type\":\"%s\",\
\"data_type\":\"%s\",\
\"value\":\"%s\",\
\"alarm\":\
{\"alarm1\":\"%s\",\
\"alarm2\":\"%s\"},\
\"alert\":\"%s\"}}\0",
                (unsigned char *)pyldStr.IdStr, datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second, \
                (unsigned char *)pyldStr.RtuIdStr, \
                (unsigned char *)pyldStr.TerminalTypeStr, \
                (unsigned char *)pyldStr.MonitorTypeStr, \
                (unsigned char *)pyldStr.DataTypeStr, \
                (unsigned char *)pyldStr.ValueStr, \
                (unsigned char *)pyldStr.Alarm1Str, \
                (unsigned char *)pyldStr.Alarm2Str, \
                (unsigned char *)pyldStr.AlertStr);
    }
    else
    {
        sprintf(payload, "{\"header\":{\"client\":\"%s\",\"time\":\"%d/%d/%d %d:%d:%d\"},\
\"data\":{\"id\":\"%s\",\
\"terminal_type\":\"%s\",\
\"monitor_type\":\"%s\",\
\"data_type\":\"%s\",\
\"value\":\"%s\",\
\"alarm\":\
{\"alarm1_up\":\"%s\",\
\"alarm1_down\":\"%s\",\
\"alarm2_up\":\"%s\",\
\"alarm2_down\":\"%s\"},\
\"alert\":\"%s\"}}\0",
                (unsigned char *)pyldStr.IdStr, datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second, \
                (unsigned char *)pyldStr.RtuIdStr, \
                (unsigned char *)pyldStr.TerminalTypeStr, \
                (unsigned char *)pyldStr.MonitorTypeStr, \
                (unsigned char *)pyldStr.DataTypeStr, \
                (unsigned char *)pyldStr.ValueStr, \
                (unsigned char *)pyldStr.Alarm1UpStr, \
                (unsigned char *)pyldStr.Alarm1DownStr, \
                (unsigned char *)pyldStr.Alarm2UpStr, \
                (unsigned char *)pyldStr.Alarm2DownStr, \
                (unsigned char *)pyldStr.AlertStr);
    }
}

#if 0
void kalyke_cycle_post_TEST(mqtt_config_array_hanyu_st *pCfgs,rtc_datetime_t *pTime, uint32_t milliSecond)
{
    LOGV("Kalyke_monitor", "Enter %s(), report_cycle = %u, reportContentLen = %u", __func__, pCfgs->report_cycle, pCfgs->reportContentLen);
    char *tempStr = pvPortMalloc(1024);
    char *mqttBuf = pvPortMalloc(4096);
    memset(mqttBuf, 0, 4096);
    LOGV("Kalyke_monitor", "Free heap: %u", xPortGetFreeHeapSize());
//    snvs_hp_rtc_datetime_t rtcDate;
//    kalyke_SNVS_HP_RTC_GetDatetime(&rtcDate);
    rtc_datetime_t rtcDate;
    RTC_GetDatetime(&rtcDate);

    if (memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0)
    {
        mqtt_config_array_st *pRC = pCfgs->pReportContent;

        for(int i = 0; i < pCfgs->reportContentLen; i++)  //所有数据点都是SYM_CFG 则不需要推送
        {
            if(pRC[i].sym != SYM_CFG)
            {
                break;
            }
            if(i >= pCfgs->reportContentLen - 1) //直到最后一个数据点都是SYM_CFG
            {
                vPortFree(tempStr);
                vPortFree(mqttBuf);
                LOGD("Kalyke_monitor", "Leave %s(), mqttBuf = %s", __func__, mqttBuf);
                return;
            }
        }

        strcpy(mqttBuf, "{");
        sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
        strcat(mqttBuf, tempStr);

        if (memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0)
        {
            char device_id[24] = {0};
            memcpy(device_id, gFlashParam.st.idInfo, 16);
//            sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":%d,", device_id,pCfgs->slave_id);
            sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":%d,\"slave_name\":\"%s\",", device_id, pCfgs->slave_id, pCfgs->slave_name);
            strcat(mqttBuf, tempStr);
        }

        strcat(mqttBuf, "\"data\":[");
        for (int i = 0; i < pCfgs->reportContentLen; i++)
        {
            if (pRC[i].sym == SYM_CFG)  //配置信息不需要上传
            {
                continue;
            }

            if (pRC[i].dataType == DTYPE_F32)  //(strcmp(pRC[i].dataType, "float32") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%.3f},", pRC[i].name, getElementValue_float(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_I16)  //(strcmp(pRC[i].dataType, "int16") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", pRC[i].name, getElementValue_int16(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_I32)  //(strcmp(pRC[i].dataType, "int32") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", pRC[i].name, getElementValue_int32(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_BOOL)  //(strcmp(pRC[i].dataType, "bool") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", pRC[i].name, getElementValue_bool(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_STRING)  //(strcmp(pRC[i].dataType, "string") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":\"%s\"},", pRC[i].name, getElementValue_string(&pRC[i]));
            }
            else // int16 default
            {
                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
                tempStr[0] = '\0';
            }
            strcat(mqttBuf, tempStr);
        }
        int len = strlen(mqttBuf);
        if (pCfgs->reportContentLen > 0) 
            mqttBuf[len - 1] = '\0';
        strcat(mqttBuf, "]}\r\n");
        
        vPortFree(tempStr);
        kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuf, 0);
    }

    else if (memcmp(g_plc_netcfg.mqtt.vender, "NFYMQTT", 7) == 0)
    {
        mqtt_config_array_st *pRC = pCfgs->pReportContent;

        for(int i = 0; i < pCfgs->reportContentLen; i++)  //所有数据点都是SYM_CFG 则不需要推送
        {
            if(pRC[i].sym != SYM_CFG)
            {
                break;
            }
            if(i >= pCfgs->reportContentLen - 1) //直到最后一个数据点都是SYM_CFG
            {
                vPortFree(tempStr);
                vPortFree(mqttBuf);
                LOGD("Kalyke_monitor", "Leave %s(), mqttBuf = %s", __func__, mqttBuf);
                return;
            }
        }
			
        strcpy(mqttBuf, "{");

        payload_string_t pyldStr;
        rtc_datetime_t datetime;
        RTC_GetDatetime(&datetime);
        
        char device_id[24] = {0};
        memcpy(device_id, pyldStr.IdStr, 16);
        
        sprintf(tempStr, "\"deviceId\":\"%s\",", device_id);
        strcat(mqttBuf, tempStr);
        
        sprintf(tempStr, "\"timestamp\":\"%d/%d/%d %d:%d:%d,", datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
        strcat(mqttBuf, tempStr);
        
        sprintf(tempStr, "\"headers\":\{},");
        strcat(mqttBuf, tempStr);
        
        sprintf(tempStr, "\"messageId\":\"\",");
        strcat(mqttBuf, tempStr);
        
        sprintf(tempStr, "\"properties\":\{}");
        strcat(mqttBuf, tempStr);
        
        strcat(mqttBuf, "\"properties\":{");
        for (int i = 0; i < pCfgs->reportContentLen; i++)
        {
            if (pRC[i].sym == SYM_CFG)  //配置信息不需要上传
            {
                continue;
            }

            if (pRC[i].dataType == DTYPE_F32)  //(strcmp(pRC[i].dataType, "float32") == 0)
            {
                sprintf(tempStr, "\"%s\":%.3f,", pRC[i].name, getElementValue_float(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_I16)  //(strcmp(pRC[i].dataType, "int16") == 0)
            {
                sprintf(tempStr, "\"%s\":%d,", pRC[i].name, getElementValue_int16(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_I32)  //(strcmp(pRC[i].dataType, "int32") == 0)
            {
                sprintf(tempStr, "\"%s\":%d,", pRC[i].name, getElementValue_int32(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_BOOL)  //(strcmp(pRC[i].dataType, "bool") == 0)
            {
                sprintf(tempStr, "\"%s\":%d,", pRC[i].name, getElementValue_bool(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_STRING)  //(strcmp(pRC[i].dataType, "string") == 0)
            {
                sprintf(tempStr, "\"%s\":%s,", pRC[i].name, getElementValue_string(&pRC[i]));
            }
            else // int16 default
            {
                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
                tempStr[0] = '\0';
            }
            strcat(mqttBuf, tempStr);
        }
        int len = strlen(mqttBuf);
        if (pCfgs->reportContentLen > 0) 
            mqttBuf[len - 1] = '\0';
        strcat(mqttBuf, "}\r\n");
        
        vPortFree(tempStr);
    
        char device_topic[CONFIG_MQTT_MAX_LWT_TOPIC];
        sprintf(device_topic, "/%s/%s%s%d/properties/report", device_id, device_id, pCfgs->slave_name, pCfgs->slave_id);
        kalyke_mqtt_publish(device_topic, mqttBuf, 0);
    }
    
    LOGD("Kalyke_monitor", "Leave %s(), mqttBuf = %s", __func__, mqttBuf);
    vPortFree(mqttBuf);
 
#else
void kalyke_cycle_post_TEST(char *payload)
{
    payload_string_t pyldStr;
    rtc_datetime_t datetime;
    RTC_GetDatetime(&datetime);
//    uint8_t cyBuf[12];

    memset(&pyldStr, '\0', sizeof(pyldStr));

    //KSDC120200630001
    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    //KSDDES4200630001_01
//    memcpy(pyldStr.RtuIdStr, (uint8_t *)gFlashParam.st.idNum, 1);
//    pyldStr.RtuIdStr[16] = '_';
//    sprintf((char *)cyBuf, "%02u", gPyldOneP_Idx + 1);
//    memcpy(&pyldStr.RtuIdStr[17], cyBuf, 2);
    
    //DES4
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuIdInfo[18], 4);

    //K
    Handle_RtuCtrlP_CtrlPTypeStr(gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType, gPyldOneP_Idx, pyldStr.DataTypeStr);

    //D 0
    Handle_RtuCtrlP_CtrlPValStr(gRtu_InfoPacket.Rtu_Info[gPyldRtu_Idx].RtuType, gPyldOneP_Idx, gPyldP_Idx, pyldStr.MonitorTypeStr, pyldStr.ValueStr);
    
    sprintf(payload, "{\"header\":{\"device_id\":\"%s\",\"time\":\"%d/%d/%d %d:%d:%d\",\"slave_id\":\"%d\",\"slave_name\":\"%s\"},\
\"data\":{\"%s\":\"%f\",\
\"%s\":\"%f\",\
\"%s\":\"%f\",\
\"%s\":\"%f\",\
\"%s\":\"%f\",\
\"%s\":\"%f\",\
\"%s\":\"%f\",\
\"%s\":\"%d\",\
\"%s\":\"%d\",\
\"%s\":\"%d\",\
\"%s\":\"%d\"}}",
                (unsigned char *)pyldStr.IdStr, datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second, gFlashParam.st.idNum,(unsigned char *)pyldStr.TerminalTypeStr,  \
                "L1", gMeterParam.Volt[0],  \
                "L2", gMeterParam.Volt[1],  \
                "L3", gMeterParam.Volt[2],  \
                "I1", gMeterParam.VoltL[0],  \
                "I2", gMeterParam.VoltL[1],  \
                "I3", gMeterParam.VoltL[2],  \
                "I4", gMeterParam.Cur,  \
                "T1", gESE_Elem.st.Tmp[0],  \
                "T2", gESE_Elem.st.Tmp[1],  \
                "T3", gESE_Elem.st.Tmp[2],  \
                "T4", gESE_Elem.st.Tmp[3]);

#endif
                
}

/**
  * @brief  解析数据包
  * @param  输入数值
  * @retval None
  */
/*{
    "header":{
        "client":"KSDC120201111001",
        "command":"RESET",
        "token":"10ad6cee-ab89-40ba-a104-00e4562c3053",
        "time":"2020-12-29 09:55:50",
        "keep-alive":"ON/OFF",
        "rate":"1"
    },
    "data":{
        "ctrlPoint":"KSDDES9200722005_02",
        "alarm1":"1",
        "alarm2":"1",
        "alarm3":"N/A",
        "alarm4":"N/A"
    }
}*/
void UnpackPayLoad(char *payload)
{
    uint8_t cyBuf[20];
    uint8_t cyRtuIdStr[20];
    uint8_t cyTempBuf[20];

    cJSON *root;
    cJSON *cChild;
    cJSON *cTemp;

    unsigned int year;
    unsigned int month;
    unsigned int day;
    unsigned int hour;
    unsigned int minute;
    unsigned int second;
    rtc_datetime_t datetime;

    char *strValue;
    float fTemp;

    uint8_t rtu_idx = 0;
    uint8_t onertu_ctrlP_idx = 0;
    uint16_t i;

    LOGW("payload", "Handle data, Parse Json Object   -----------------------------------------------------------------\r\n");
    root = cJSON_Parse(payload);
    if(root == NULL)
    {
        LOGE("payload", "cJSON_Parse(payload) return NULL");
        cJSON_Delete(root);
        return;
    }
    LOGW("payload", "receive data.");

    /* header ----------------------------------------------------------------------------------------- */
    cChild = cJSON_GetObjectItem(root, "header");
    if(cChild == NULL)
    {
        LOGE("payload", "cJSON_GetObjectItem(root, \"header\") return NULL");
        cJSON_Delete(root);
        return;
    }

    /* client ------------------------------------------ */
    strValue = cJSON_GetObjectItem(cChild, "client")->valuestring;
    memset(cyRtuIdStr, '\0', sizeof(cyRtuIdStr) );
    memset(cyBuf, '\0', sizeof(cyBuf) );
    memcpy(cyRtuIdStr, gFlashParam.st.idInfo, 16);
    memcpy(cyBuf, strValue, 16);
    LOGV("payload", "client = %s, this = %s", strValue, cyRtuIdStr);
    if(0 != memcmp(cyBuf, cyRtuIdStr, 16))
    {
        LOGE("payload", "client ID mismatch.");
        cJSON_Delete(root);
        return;
    }
    LOGW("payload", "client ID mismatch.");

    /* command ------------------------------------------ */
    strValue = cJSON_GetObjectItem(cChild, "command")->valuestring;
    memset(cyBuf, '\0', sizeof(cyBuf) );
    memcpy(cyBuf, strValue, 5);
    LOGV("payload", "command = %s", cyBuf);
    if(0 != memcmp(cyBuf, "RESET", 5))
    {
        LOGE("payload", "command error, is not RESET.");
        cJSON_Delete(root);
        return;
    }
    LOGW("payload", "command right, is RESET.");

    /* time  -------------------------------------------- */
    cTemp = cJSON_GetObjectItem(cChild, "time");
    if(cTemp == NULL)
    {
        LOGE("payload", "cJSON_GetObjectItem(root, \"time\") return NULL");
    }
    else
    {
        strValue = cTemp->valuestring;
        LOGV("payload", "time = %s", strValue);

        sscanf(strValue, "%u-%u-%u %u:%u:%u", &year, &month, &day, &hour, &minute, &second);
        datetime.year   = year;
        datetime.month  = month;
        datetime.day    = day;
        datetime.hour   = hour;
        datetime.minute = minute;
        datetime.second = second;
        LOGW("payload", "%u-%u-%u %u:%u:%u", datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
        RTC_SetDatetime(&datetime);
    }

    /* rate  -------------------------------------------- */
    cTemp = cJSON_GetObjectItem(cChild, "rate");
    if(cTemp == NULL)
    {
        LOGE("payload", "cJSON_GetObjectItem(root, \"time\") return NULL");
    }
    else
    {
        strValue = cTemp->valuestring;
        LOGV("payload", "rate = %s", strValue);

        sscanf(strValue, "%f", &fTemp);
        LOGW("payload", "fTemp = %f", fTemp);
        if(fTemp != 0)
        {
            gFlashParam.st.mqttPublishInterval = fTemp;
            LOGW("payload", "gVenusParam.mqttPublishInterval = %d", gFlashParam.st.mqttPublishInterval);
            Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        }
    }

    /* data ----------------------------------------------------------------------------------------- */
    cChild = cJSON_GetObjectItem(root, "data");
    if(cChild == NULL)
    {
        LOGE("payload", "cJSON_GetObjectItem(root, \"data\") return NULL");
        cJSON_Delete(root);
        return;
    }

    /* ctrlPoint  -------------------------------------------- */
    cTemp = cJSON_GetObjectItem(cChild, "ctrlPoint");
    if (cTemp == NULL)
    {
        LOGE("payload", "have no ctrlPoint");
        cJSON_Delete(root);
        return;
    }

    strValue = cTemp->valuestring;
    memset(cyTempBuf, '\0', sizeof(cyTempBuf) );
    memcpy(cyTempBuf, strValue, 19);
    LOGV("payload", "ctrlPoint = %s", cyTempBuf);

    for(i = 0; i < MAX_CTRLP_NUM; i++) //find control point
    {
        memset(cyRtuIdStr, '\0', sizeof(cyRtuIdStr) );
        memcpy(cyRtuIdStr, gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuIdInfo, 16);
        cyRtuIdStr[16] = '_';
        sprintf((char *)cyBuf, "%02u", onertu_ctrlP_idx + 1);
        memcpy(&cyRtuIdStr[17], cyBuf, 2);

        if(0 == memcmp(cyTempBuf, cyRtuIdStr, 19)) //find it
        {
            LOGW("payload", "find it ctrlPoint = %s", cyRtuIdStr);

            gCtrlP_AlmLim[i].ValidAlmType = ALARM_NO_VALID;
            //"D"
            if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_IO_TYPE )
            {
                LOGW("payload", "Ctrl Point TYPE = DC");

                /* alarm1  -------------------------------------------- */
                cTemp = cJSON_GetObjectItem(cChild, "alarm1");
                if(cTemp == NULL)
                {
                    LOGE("payload", "cJSON_GetObjectItem(root, \"alarm1\") return NULL");
                }
                else
                {
                    strValue = cTemp->valuestring;
                    LOGV("payload", "alarm1 = %s", strValue);

                    if (0 != memcmp(strValue, "N/A", 3) )
                    {
                        fTemp = 0;
                        sscanf(strValue, "%f", &fTemp);
                        gCtrlP_AlmLim[i].Alm1_Up = fTemp;
                        PAR_SET_BIT(gCtrlP_AlmLim[i].ValidAlmType, ALARM_1_VALID);
                    }
                }

                /* alarm2  -------------------------------------------- */
                cTemp = cJSON_GetObjectItem(cChild, "alarm2");
                if(cTemp == NULL)
                {
                    LOGE("payload", "cJSON_GetObjectItem(root, \"alarm2\") return NULL");
                }
                else
                {
                    strValue = cTemp->valuestring;
                    LOGV("payload", "alarm2 = %s", strValue);

                    if (0 != memcmp(strValue, "N/A", 3) )
                    {
                        fTemp = 0;
                        sscanf(strValue, "%f", &fTemp);
                        gCtrlP_AlmLim[i].Alm2_Up = fTemp;
                        PAR_SET_BIT(gCtrlP_AlmLim[i].ValidAlmType, ALARM_2_VALID);
                    }
                }
            }
            //"A"
            else
            {
                LOGW("payload", "Ctrl Point TYPE = AC");

                /* alarm1_up  -------------------------------------------- */
                cTemp = cJSON_GetObjectItem(cChild, "alarm1_up");
                if(cTemp == NULL)
                {
                    LOGE("payload", "cJSON_GetObjectItem(root, \"alarm1_up\") return NULL");
                }
                else
                {
                    strValue = cTemp->valuestring;
                    LOGV("payload", "alarm1_up = %s", strValue);

                    if (0 != memcmp(strValue, "N/A", 3) )
                    {
                        fTemp = 0;
                        sscanf(strValue, "%f", &fTemp);
                        if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X1_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm1_Up = fTemp;
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X10_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm1_Up = (fTemp * 10);
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X10_N_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm1_Up = (fTemp * 10);
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X100_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm1_Up = (fTemp * 100);
                        }
                        PAR_SET_BIT(gCtrlP_AlmLim[i].ValidAlmType, ALARM_1_VALID);
                    }
                }

                /* alarm1_down  -------------------------------------------- */
                cTemp = cJSON_GetObjectItem(cChild, "alarm1_down");
                if(cTemp == NULL)
                {
                    LOGE("payload", "cJSON_GetObjectItem(root, \"alarm1_down\") return NULL");
                }
                else
                {
                    strValue = cTemp->valuestring;
                    LOGV("payload", "alarm1_down = %s", strValue);

                    if (0 != memcmp(strValue, "N/A", 3) )
                    {
                        fTemp = 0;
                        sscanf(strValue, "%f", &fTemp);
                        if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X1_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm1_Down = fTemp;
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X10_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm1_Down = (fTemp * 10);
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X10_N_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm1_Down = (fTemp * 10);
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X100_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm1_Down = (fTemp * 100);
                        }
                        PAR_SET_BIT(gCtrlP_AlmLim[i].ValidAlmType, ALARM_1_VALID);
                    }
                }

                /* alarm2_up  -------------------------------------------- */
                cTemp = cJSON_GetObjectItem(cChild, "alarm2_up");
                if(cTemp == NULL)
                {
                    LOGE("payload", "cJSON_GetObjectItem(root, \"alarm2_up\") return NULL");
                }
                else
                {
                    strValue = cTemp->valuestring;
                    LOGV("payload", "alarm2_up = %s", strValue);

                    if (0 != memcmp(strValue, "N/A", 3) )
                    {
                        fTemp = 0;
                        sscanf(strValue, "%f", &fTemp);
                        if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X1_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm2_Up = fTemp;
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X10_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm2_Up = (fTemp * 10);
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X10_N_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm2_Up = (fTemp * 10);
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X100_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm2_Up = (fTemp * 100);
                        }
                        PAR_SET_BIT(gCtrlP_AlmLim[i].ValidAlmType, ALARM_2_VALID);
                    }
                }

                /* alarm2_down  -------------------------------------------- */
                cTemp = cJSON_GetObjectItem(cChild, "alarm2_down");
                if(cTemp == NULL)
                {
                    LOGE("payload", "cJSON_GetObjectItem(root, \"alarm2_down\") return NULL");
                }
                else
                {
                    strValue = cTemp->valuestring;
                    LOGV("payload", "alarm2_down = %s", strValue);

                    if (0 != memcmp(strValue, "N/A", 3) )
                    {
                        fTemp = 0;
                        sscanf(strValue, "%f", &fTemp);
                        if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X1_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm2_Down = fTemp;
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X10_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm2_Down = (fTemp * 10);
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X10_N_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm2_Down = (fTemp * 10);
                        }
                        else if( Handle_RtuCtrlP_PyldType(gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType, onertu_ctrlP_idx) == PYLD_X100_ANALOG_TYPE )
                        {
                            gCtrlP_AlmLim[i].Alm2_Down = (fTemp * 100);
                        }
                        PAR_SET_BIT(gCtrlP_AlmLim[i].ValidAlmType, ALARM_2_VALID);
                    }
                }
            }

//            Parameter_FlashWrite(PAR_CTRL_INFO_SAVE_ADDR, &gCtrlP_AlmLim, sizeof(gCtrlP_AlmLim));  //save alarm limit
        }  /* if(0 == memcmp(cyTempBuf, cyRtuIdStr, 19)) */

        onertu_ctrlP_idx++;
        if(onertu_ctrlP_idx == CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[rtu_idx].RtuType])
        {
            rtu_idx++;
            if(rtu_idx == gRtu_Num)
            {
                break;
            }
            onertu_ctrlP_idx = 0;
        }
    } /* End of for(i = 0; i < MAX_CTRLP_NUM; i++) */

    cJSON_Delete(root);
}

