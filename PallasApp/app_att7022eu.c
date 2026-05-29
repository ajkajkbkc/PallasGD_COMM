
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "app_log.h"
#include "app_att7022eu.h"
#include "app_parameter.h"
#include "app_main.h"
#include "app_tool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "app_key.h"
#include "app_tm1650.h"
#include "app_oled.h"
#include "kalyke_monitor_task.h"
#include "module_ESE.h"
/*
 * Copyright (c) 2006-2018, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-22     Arrbow       first implementation
 */

/* Private includes ----------------------------------------------------------*/

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
//#include "FreeRTOS.h"
//#include "task.h"
//#include "semphr.h"
//#include "main.h"
//#include "pallas_es_log.h"
//#include "pallas_es_param.h"
//#include "module_ESM.h"
//#include "module_ESE.h"
//#include "module_ESC.h"

//#include <stdlib.h>
//#include <math.h>



/* Private define ------------------------------------------------------------*/
#define ATT7022_CSSet()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define ATT7022_CSClr()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

#define ATT7022_PHS_A   0
#define ATT7022_PHS_B   1
#define ATT7022_PHS_C   2
#define ATT7022_PHS_ALL 3

#define  _10_bit    0x400
#define  _12_bit    0x1000
#define  _13_bit    0x2000
#define  _15_bit    0x8000
#define  _16_bit    0x10000
#define  _20_bit    0x100000
#define  _21_bit    0x200000
#define  _23_bit    0x800000
#define  _24_bit    0x1000000

#define  PI         (3.1415927)

#define  ESM_KU     10
#define  ESM_KI     10

/* Private variables ---------------------------------------------------------*/
/* Definitions for att7022Task */
osThreadId_t att7022TaskHandle;
const osThreadAttr_t att7022Task_attributes =
{
    .name = "att7022Task",
    .priority = (osPriority_t) att7022TaskPriority,
    .stack_size = 1024
};

#if PRINT_LOG_OPEN == 1
static const char *TAG = "Att7022";
#endif

SemaphoreHandle_t gAtt7022Mutex;

meter_run_info_t gMeterRunInfo;

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA

static const uint16_t TAB_Mask[] = {0x0200, 0x0400, 0x0800, 0x0E00};

meter_param_t           gMeterParam;
meter_energy_param_t    gMeterEnergy;
meter_flag_param_t      gFlagMeterParam;

power_quality_param_t   gPowerQualityParam;

//#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB
sync_sampleD_n Sync_SampleD;  //同步数据缓存

short gActualData[SamplePointN];
compx_t gCompx[SamplePointN];
float gFFTData[SamplePointN];
//#endif

//#if PROD_TYPE == PROD_SFE
harmonic_param_t        gHarmonicParam;
//#endif


/************ const data of sin/cos ************/
const float cosf_tab[256] =
{
    0.9999, 0.9996, 0.9987, 0.9972, 0.9951, 0.9924, 0.9891, 0.9852, 0.9807, 0.9757,
    0.9700, 0.9637, 0.9569, 0.9495, 0.9415, 0.9329, 0.9238, 0.9142, 0.9039, 0.8932,
    0.8819, 0.8700, 0.8577, 0.8448, 0.8314, 0.8175, 0.8032, 0.7883, 0.7730, 0.7572,
    0.7409, 0.7242, 0.7071, 0.6895, 0.6715, 0.6531, 0.6343, 0.6152, 0.5956, 0.5758,
    0.5555, 0.5349, 0.5141, 0.4928, 0.4713, 0.4496, 0.4275, 0.4052, 0.3826, 0.3598,
    0.3368, 0.3136, 0.2902, 0.2667, 0.2429, 0.2191, 0.1950, 0.1709, 0.1467, 0.1224,
    0.0980,  0.0735, 0.0490, 0.0245, 0.0,   -0.0245, -0.0490, -0.0735, -0.0980, -0.1224,
    -0.1467, -0.1709, -0.1950, -0.2190, -0.2429, -0.2667, -0.2902, -0.3136, -0.3368, -0.3598,
    -0.3826, -0.4052, -0.4275, -0.4496, -0.4713, -0.4928, -0.5141, -0.5349, -0.5555, -0.5758,
    -0.5956, -0.6152, -0.6343, -0.6531, -0.6715, -0.6895, -0.7071, -0.7242, -0.7409, -0.7572,
    -0.7730, -0.7883, -0.8032, -0.8175, -0.8314, -0.8448, -0.8577, -0.8700, -0.8819, -0.8932,
    -0.9039, -0.9142, -0.9238, -0.9329, -0.9415, -0.9495, -0.9569, -0.9637, -0.9700, -0.9757,
    -0.9807, -0.9852, -0.9891, -0.9924, -0.9951, -0.9972, -0.9987, -0.9996, -0.9999, -0.9996,
    -0.9987, -0.9972, -0.9951, -0.9924, -0.9891, -0.9852, -0.9807, -0.9757, -0.9700, -0.9637,
    -0.9569, -0.9495, -0.9415, -0.9329, -0.9238, -0.9142, -0.9039, -0.8932, -0.8819, -0.8700,
    -0.8577, -0.8448, -0.8314, -0.8175, -0.8032, -0.7883, -0.7730, -0.7572, -0.7409, -0.7242,
    -0.7071, -0.6895, -0.6715, -0.6531, -0.6343, -0.6152, -0.5957, -0.5758, -0.5555, -0.5350,
    -0.5141, -0.4929, -0.4713, -0.4496, -0.4275, -0.4052, -0.3826, -0.3598, -0.3368, -0.3136,
    -0.2902, -0.2667, -0.2429, -0.2191, -0.1950, -0.1709, -0.1467, -0.1224, -0.0980, -0.0735,
    -0.0490, -0.0245, -0.0,   0.0245, 0.0490, 0.0735, 0.0980, 0.1224, 0.1467, 0.1709,
    0.1950, 0.2190, 0.2429, 0.2667, 0.2902, 0.3136, 0.3368, 0.3598, 0.3826, 0.4052,
    0.4275, 0.4496, 0.4713, 0.4928, 0.5140, 0.5349, 0.5555, 0.5758, 0.5956, 0.6152,
    0.6343, 0.6531, 0.6715, 0.6895, 0.7071, 0.7242, 0.7409, 0.7572, 0.7730, 0.7883,
    0.8032, 0.8175, 0.8314, 0.8448, 0.8577, 0.8700, 0.8819, 0.8932, 0.9039, 0.9142,
    0.9238, 0.9329, 0.9415, 0.9495, 0.9569, 0.9637, 0.9700, 0.9757, 0.9807, 0.9852,
    0.9891, 0.9924, 0.9951, 0.9972, 0.9987, 0.9996
};

const float sinf_tab[256] =
{
    0,     0.0245, 0.0490, 0.0735, 0.0980, 0.1224, 0.1467, 0.1709, 0.1950, 0.2191,
    0.2429, 0.2667, 0.2902, 0.3136, 0.3368, 0.3598, 0.3826, 0.4052, 0.4275, 0.4496,
    0.4713, 0.4928, 0.5141, 0.5349, 0.5555, 0.5758, 0.5956, 0.6152, 0.6343, 0.6531,
    0.6715, 0.6895, 0.7071, 0.7242, 0.7409, 0.7572, 0.7730, 0.7883, 0.8032, 0.8175,
    0.8314, 0.8448, 0.8577, 0.8700, 0.8819, 0.8932, 0.9039, 0.9142, 0.9238, 0.9329,
    0.9415, 0.9495, 0.9569, 0.9637, 0.9700, 0.9757, 0.9807, 0.9852, 0.9891, 0.9924,
    0.9951, 0.9972, 0.9987, 0.9996, 0.9999, 0.9996, 0.9987, 0.9972, 0.9951, 0.9924,
    0.9891, 0.9852, 0.9807, 0.9757, 0.9700, 0.9637, 0.9569, 0.9495, 0.9415, 0.9329,
    0.9238, 0.9142, 0.9039, 0.8932, 0.8819, 0.8700, 0.8577, 0.8448, 0.8314, 0.8175,
    0.8032, 0.7883, 0.7730, 0.7572, 0.7409, 0.7242, 0.7071, 0.6895, 0.6715, 0.6531,
    0.6343, 0.6152, 0.5957, 0.5758, 0.5555, 0.5349, 0.5141, 0.4929, 0.4713, 0.4496,
    0.4275, 0.4052, 0.3826, 0.3598, 0.3368, 0.3136, 0.2902, 0.2667, 0.2429, 0.2191,
    0.1950, 0.1709, 0.1467, 0.1224, 0.0980, 0.0735, 0.0490, 0.0245, 0.0,  -0.0245,
    -0.0490, -0.0735, -0.0980, -0.1224, -0.1467, -0.1709, -0.1950, -0.2190, -0.2429, -0.2667,
    -0.2902, -0.3136, -0.3368, -0.3598, -0.3826, -0.4052, -0.4275, -0.4496, -0.4713, -0.4928,
    -0.5141, -0.5349, -0.5555, -0.5758, -0.5956, -0.6152, -0.6343, -0.6531, -0.6715, -0.6895,
    -0.7071, -0.7242, -0.7409, -0.7572, -0.7730, -0.7883, -0.8032, -0.8175, -0.8314, -0.8448,
    -0.8577, -0.8700, -0.8819, -0.8932, -0.9039, -0.9142, -0.9238, -0.9329, -0.9415, -0.9495,
    -0.9569, -0.9637, -0.9700, -0.9757, -0.9807, -0.9852, -0.9891, -0.9924, -0.9951, -0.9972,
    -0.9987, -0.9996, -0.9999, -0.9996, -0.9987, -0.9972, -0.9951, -0.9924, -0.9891, -0.9852,
    -0.9807, -0.9757, -0.9700, -0.9637, -0.9569, -0.9495, -0.9415, -0.9329, -0.9238, -0.9142,
    -0.9039, -0.8932, -0.8819, -0.8700, -0.8577, -0.8448, -0.8314, -0.8175, -0.8032, -0.7883,
    -0.7730, -0.7572, -0.7409, -0.7242, -0.7071, -0.6895, -0.6715, -0.6531, -0.6343, -0.6152,
    -0.5957, -0.5758, -0.5555, -0.5350, -0.5141, -0.4929, -0.4714, -0.4496, -0.4275, -0.4052,
    -0.3826, -0.3598, -0.3368, -0.3136, -0.2902, -0.2667, -0.2429, -0.2191, -0.1950, -0.1709,
    -0.1467, -0.1224, -0.0980, -0.0735, -0.0490, -0.0245
};

const float FftCoefficient[31] = ////1~31 harmonic coefficient
{
    1.000000000000000,
    1.000925409832445,
    1.002469903639547,
    1.004636706352368,
    1.007430350532950,
    1.010856694162959,
    1.014922943701906,
    1.019637682557525,
    1.025010905145191,
    1.031054056748977,
    1.037780079434771,
    1.045203464305803,
    1.053340310433575,
    1.062208390842917,
    1.071827225979052,
    1.082218165137800,
    1.093404476397839,
    1.105411445656847,
    1.118266485442214,
    1.131999254242324,
    1.146641787187304,
    1.162228638999322,
    1.178797040233217,
    1.196387067939490,
    1.215041832005096,
    1.234807678564162,
    1.255734412022930,
    1.277875537412385,
    1.301288524970737,
    1.326035099068472,
    1.352181553824114
};

#endif



/* Private function prototypes -----------------------------------------------*/
void Att7022Task(void *argument);

void meter_cali_param_init(void);

float Read_Power_Factor(uint8_t phase, uint8_t *isNeg);


/* Private user code ---------------------------------------------------------*/


HAL_StatusTypeDef spi_write_and_read(uint8_t *data, uint16_t bytes_number)
{
    HAL_StatusTypeDef ret;

    if(gAtt7022Mutex == NULL)
    {
        ATT7022_CSClr();
        ret = HAL_SPI_TransmitReceive( &hspi2, data, data, bytes_number, HAL_MAX_DELAY );
        ATT7022_CSSet();
    }
    else
    {
        xSemaphoreTake(gAtt7022Mutex, portMAX_DELAY);
        ATT7022_CSClr();
        ret = HAL_SPI_TransmitReceive( &hspi2, data, data, bytes_number, HAL_MAX_DELAY );
        ATT7022_CSSet();
        xSemaphoreGive(gAtt7022Mutex);
    }

    return ret;
}

uint32_t att7022_read_register(uint8_t addr)
{
    uint32_t regVal;
    uint8_t data[4] = {0};

    data[0] = addr;
    spi_write_and_read(data, 4);
    regVal = data[1] << 16 | data[2] << 8 | (data[3] & 0xFF);

    return regVal;
}

void att7022_write_register(uint8_t addr, uint32_t regVal)
{
    uint8_t data[4];

    data[0] = addr | ATT7022_WRITE_REG;
    data[1] = regVal >> 16;
    data[2] = regVal >> 8;
    data[3] = regVal & 0xFFU;
    spi_write_and_read(data, 4);
}

static bool att7022_spi_is_ok(void)
{
    uint32_t regVal;
    uint8_t errtimes;

    errtimes = 0;
    while(errtimes++ < 10)
    {
        vTaskDelay(10);
        regVal = att7022_read_register(r_DeviceID);
        LOGI(TAG, "regVal = 0x%04X", regVal);
        if (0x7122A0 == regVal)
        {
            return true;
        }
    }

    return false;
}

/**
  * @brief  读取计量参数
  * @param  None
  * @retval 读取值
  */
unsigned int ATT7022_Get_MeterPara(unsigned char addr)
{
    uint32_t value = 0;

    if(PAR_READ_BIT(gParam.st.State_SystemErr, SysErr0_Meter_Msk))
    {
        return 0;
    }

    if(addr & 0x80)  //读校表寄存器
    {
        att7022_write_register(w_EnRdCali, 0x00005A);
        value = att7022_read_register(addr & 0x7F);
        att7022_write_register(w_EnRdCali, ~0x00005A);
    }
    else
    {
        value = att7022_read_register(addr);
    }

    return value;
}

/**
  * @brief  浮点型数据转换成寄存器值
  * @param  _reg_bit 多少位数据，如uint16_t的数据为_16_bit
  * @param  f_val 浮点型数据
  * @retval 寄存器值
  */
uint16_t float_data_to_register_value(uint32_t _reg_bit, float f_data)
{
    uint16_t reg_val;

    /* f_data >= 0, reg_val = INT[Ugain*_reg_bit]
       f_data <  0, reg_val = INT[(_reg_bit<<1)+(Ugain*_reg_bit)] */
    if(f_data < 0)
    {
        f_data *= (_reg_bit >> 1);
        f_data += (_reg_bit);
    }
    else
    {
        f_data *= (_reg_bit >> 1);
    }

    reg_val = (uint16_t)f_data;

    return reg_val;
}

/**
  * @brief  寄存器值转换成浮点型数据
  * @param  _reg_bit 多少位数据，如uint16_t的数据为_16_bit
  * @param  reg_val 寄存器值
  * @retval 浮点型数据
  */
float register_value_to_float_data(uint32_t _reg_bit, uint16_t reg_val)
{
    float f_data;

    if(reg_val & (_reg_bit >> 1))
    {
        f_data = (float)reg_val - (_reg_bit);
        f_data /= (_reg_bit >> 1);
    }
    else
    {
        f_data = (float)reg_val / (_reg_bit >> 1);
    }

    return f_data;
}

/**
  * @brief  修复计量模块
  * @param  None
  * @retval None
  */
void Repair_Metering(void)
{
    uint16_t param;
    uint8_t i, addr, first_run;

    att7022_write_register(w_SoftRst, 0x000000); //软复位
    vTaskDelay(50);
    att7022_write_register(w_EnRdCali, 0x00005A);
    first_run = gFlashParam.st.meter_cali[offset_StartSig] == 0xFFFF ? 1 : 0; //第一次启动?
    gMeterRunInfo.CheckSum0 = 0;
    gMeterRunInfo.CheckSum1 = 0;
    for(i = 0; i <= offset_OILevel; i++)
    {
        if(first_run) //读取计量芯片校表参数
        {
            gFlashParam.st.meter_cali[i] = att7022_read_register(ParamOffset_to_RegAddr(i));
        }
        else
        {
            addr = ParamOffset_to_RegAddr(i);
            param = att7022_read_register(addr);
            if(param != gFlashParam.st.meter_cali[i])
            {
                att7022_write_register(addr, gFlashParam.st.meter_cali[i]);
            }
        }
        if(i >= offset_ModeCfg && i <= offset_SAGLevel)
        {
            gMeterRunInfo.CheckSum0 += gFlashParam.st.meter_cali[i];
        }
        else if(i >= offset_Iregion1 && i <= offset_OILevel)
        {
            gMeterRunInfo.CheckSum1 += gFlashParam.st.meter_cali[i];
        }
    }
    att7022_write_register(w_EnRdCali, ~0x00005A);
    if(first_run)
    {
        LOGV(TAG, "first run init gFlashParam.st.meter_cali.");
        meter_cali_param_init();
        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
    }
    else
    {
        gMeterRunInfo.EmuWork = EMU_RUNNING;
    }
}

/**
  * @brief  读取电压
  * @param  phase 0/1/2 => A/B/C
  * @retval None
  */
float Read_Voltage(uint8_t phase)
{
    uint32_t itemp;
    float ftemp;

    itemp = att7022_read_register(r_UaRms + phase);  //A,B,C相电流
    ftemp = (float)itemp / _13_bit;

    return ftemp;
}

/**
  * @brief  校准电压偏移
  * @param  None
  * @retval None
  */
void Cali_Offset_Voltage(void)
{
    uint8_t i, j;
    uint16_t times;
    uint32_t Vol[3];

    times = 6;

    for(i = 0; i < 3; i++)
    {
        Vol[i] = 0;
        gFlashParam.st.meter_cali[offset_UgainA + i] = 0;
        gFlashParam.st.meter_cali[offset_UoffsetA + i] = 0;
    }
    Repair_Metering();
    gMeterRunInfo.EmuWork = EMU_CALI_OFFSET_VOL;
    vTaskDelay(2000);  //清零增益后需等待

    //分相
    for(i = 0; i < times; i++)
    {
        for(j = 0; j < 3; j++)
        {
            Vol[j] += att7022_read_register(r_UaRms + j);
        }
        vTaskDelay(100);
    }
    for(i = 0; i < 3; i++)
    {
        Vol[i] /= times;  //取平均值
        //UrmsOffset=(Urms^2)/(2^15)
        Vol[i] *= Vol[i];
        Vol[i] >>= 15;
        gFlashParam.st.meter_cali[offset_UoffsetA + i] = (uint16_t)Vol[i];
    }
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准电压增益
  * @param  fCali_Vol 标准源输出电压值
  * @retval None
  */
void Cali_Gain_Voltage(float fCali_Vol)
{
    uint8_t i, j;
    uint16_t times;
    float fVol[3];

    times = 6;

    //复位增益和偏移寄存器
    for(i = 0; i < 3; i++)
    {
        gFlashParam.st.meter_cali[offset_UgainA + i] = 0;
        fVol[i] = 0;
    }
    Repair_Metering();
    gMeterRunInfo.EmuWork = EMU_CALI_GAIN_VOL;
    vTaskDelay(2000);  //清零增益后需等待

    //多次采
    for(i = 0; i < times; i++)
    {
        for(j = 0; j < 3; j++)
        {
            fVol[j] += Read_Voltage(j);
        }
        vTaskDelay(100);
    }

    //计算
    for(i = 0; i < 3; i++)
    {
        fVol[i] /= (float)times;  //取平均值
        /* 被测量示值Ut   被测量真值U0
           Ugain = U0/Ut - 1
           Ugain >= 0, GU1 = INT[Ugain*2^15]
           Ugain <  0, GU1 = INT[2^16+Ugain*2^15] */
        fVol[i] = fCali_Vol / fVol[i];
        fVol[i] -= 1;
        gFlashParam.st.meter_cali[offset_UgainA + i] = float_data_to_register_value(_16_bit, fVol[i]);
    }
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准电压增益（使用误差err校准）
  * @param  fErr 相对误差值
  * @retval None
  */
void Cali_Gain_Voltage_Use_Err(float *pfErr)
{
    uint8_t i;
    uint16_t utemp;
    float ftemp;

    /* 相对误差Err  已有增益Xgain0  新的增益Xgain1
    Xgain1 = (Xgain0-Err)/(Err+1)   */
    for(i = 0; i < 3; i++)
    {
        //取 Xgain0
        utemp = gFlashParam.st.meter_cali[offset_UgainA + i];
        ftemp = register_value_to_float_data(_16_bit, utemp);

        //计算出 Xgain1
        ftemp -= pfErr[i];
        ftemp = ftemp / (pfErr[i] + 1);

        //写 Xgain1
        gFlashParam.st.meter_cali[offset_UgainA + i] = float_data_to_register_value(_16_bit, ftemp);
    }

    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  读取电流
  * @param  phase 0/1/2 => A/B/C
  * @retval None
  */
float Read_Current(uint8_t phase)
{
    uint32_t itemp;
    float ftemp;

    itemp = att7022_read_register(r_IaRms + phase);  //A,B,C相电流
    ftemp = (float)itemp / _13_bit;
    ftemp /= gFlashParam.st.N_Ib_Vi[phase];

    return ftemp;
}
/* ---------------------------------------------------------------------
    互感器参数1A/0.5mA  并联电阻R=200Ω  设Ib=3000mA   则：
    3000mA/1.5mA  =>  Vi=1.5mA*200Ω=300mV  =>  Ib/Vi=3000mA/300mV
    电流2倍增益  =>  Ib/Vi=3000mA/600mV
    N为比例系数  Ireg为电流有效值寄存器  It为实际电流  N = Ireg/It
   --------------------------------------------------------------------- */
float Read_Current_LEAK(uint8_t phase)
{
    uint32_t itemp;
    float ftemp;

    itemp = att7022_read_register(r_InRms + phase);  //剩余电流
    //ftemp = (float)itemp;
    ftemp = (float)itemp / _13_bit;
    ftemp /= 1.122;
    //1: 1122
    //2: 2243
    //3: 3363
    //4: 4478
    //5: 5601
    //6: 6722
    
    return ftemp;
}

/**
  * @brief  校准电流偏移
  * @param  None
  * @retval None
  */
void Cali_Offset_Current(void)
{
    uint8_t i, j;
    uint16_t times;
    uint32_t Cur[3], Curt;

    times = 6;

    Curt = 0;
    gFlashParam.st.meter_cali[offset_IRmsoffsetT + i] = 0;
    for(i = 0; i < 3; i++)
    {
        Cur[i] = 0;
        gFlashParam.st.meter_cali[offset_IgainA + i] = 0;
        gFlashParam.st.meter_cali[offset_IoffsetA + i] = 0;
    }
    Repair_Metering();
    gMeterRunInfo.EmuWork = EMU_CALI_OFFSET_CUR;
    vTaskDelay(2000);  //清零增益后需等待

    //在分相有效值增益校正之前，进行ItRmsoffset校正(校表参数0x6A)。
    for(i = 0; i < times; i++)
    {
        Curt += att7022_read_register(r_ItRms);
        vTaskDelay(100);
    }
    Curt /= times;  //取平均值
    Curt *= Curt;
    Curt >>= 15;
    gFlashParam.st.meter_cali[offset_IRmsoffsetT] = (uint16_t)Curt;

    //分相
    for(i = 0; i < times; i++)
    {
        for(j = 0; j < 3; j++)
        {
            Cur[j] += att7022_read_register(r_IaRms + j);
        }
        vTaskDelay(100);
    }
    for(i = 0; i < 3; i++)
    {
        Cur[i] /= times;  //取平均值
        //IrmsOffset=(Irms^2)/(2^15)
        Cur[i] *= Cur[i];
        Cur[i] >>= 15;
        gFlashParam.st.meter_cali[offset_IoffsetA + i] = (uint16_t)Cur[i];
    }
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准电流比例系数N
  * @param  fCali_Cur 标准源输出电流值
  * @retval None
  */
void Cali_N_Current(float fCali_Cur)
{
    uint8_t i, j;
    uint16_t times;
    uint32_t utemp;
    float fCur[3];

    times = 6;

    for(i = 0; i < 3; i++)
    {
        fCur[i] = 0;
    }
    //多次采电流
    for(i = 0; i < times; i++)
    {
        for(j = 0; j < 3; j++)
        {
            utemp = att7022_read_register(r_IaRms + j);
            fCur[j] += (float)utemp / _13_bit;
        }
        vTaskDelay(100);
    }

    //计算
    for(i = 0; i < 3; i++)
    {
        fCur[i] /= times;  //取平均值
        gFlashParam.st.N_Ib_Vi[i] = fCur[i] / fCali_Cur;
    }
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准电流增益
  * @param  fCali_Cur 标准源输出电流值
  * @retval None
  */
void Cali_Gain_Current(float fCali_Cur)
{
    uint8_t i, j;
    uint16_t times;
    float fCur[3];

    times = 6;

    //复位增益和偏移寄存器
    for(i = 0; i < 3; i++)
    {
        gFlashParam.st.meter_cali[offset_IgainA + i] = 0;
        fCur[i] = 0;
    }
    Repair_Metering();
    gMeterRunInfo.EmuWork = EMU_CALI_GAIN_CUR;
    vTaskDelay(2000);  //清零增益后需等待

    for(j = 0; j < 3; j++)
    {
        Read_Current(j);
    }

    //多次采电流
    for(i = 0; i < times; i++)
    {
        for(j = 0; j < 3; j++)
        {
            fCur[j] += Read_Current(j);
        }
        vTaskDelay(100);
    }

    //计算
    for(i = 0; i < 3; i++)
    {
        fCur[i] /= (float)times;  //取平均值
        /* 被测量示值It   被测量真值I0
           Igain = I0/It - 1
           Igain >= 0, GI1 = INT[Igain*2^15]
           Igain <  0, GI1 = INT[2^16+Igain*2^15] */
        fCur[i] = fCali_Cur / fCur[i];
        fCur[i] -= 1;
        gFlashParam.st.meter_cali[offset_IgainA + i] = float_data_to_register_value(_16_bit, fCur[i]);
    }
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准电流增益（使用误差err校准）
  * @param  fErr 相对误差值
  * @retval None
  */
void Cali_Gain_Current_Use_Err(float *pfErr)
{
    uint8_t i;
    uint16_t utemp;
    float ftemp;

    /* 相对误差Err  已有增益Xgain0  新的增益Xgain1
    Xgain1 = (Xgain0-Err)/(Err+1)   */
    for(i = 0; i < 3; i++)
    {
        //取 Xgain0
        utemp = gFlashParam.st.meter_cali[offset_IgainA + i];
        ftemp = register_value_to_float_data(_16_bit, utemp);

        //计算出 Xgain1
        ftemp -= pfErr[i];
        ftemp = ftemp / (pfErr[i] + 1);

        //写 Xgain1
        gFlashParam.st.meter_cali[offset_IgainA + i] = float_data_to_register_value(_16_bit, ftemp);
    }

    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}


#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
/**
  * @brief  读取功率
  * @param  phase 0/1/2/3 => A/B/C/总
  * @param  powType 0/1/2 => 有功/无功/视在
  * @param  isNeg 0/1 => 返回的 正/负
  * @retval 返回功率值
  */
float Read_Power(uint8_t phase, uint8_t powType, uint8_t *isNeg)
{
    uint32_t itemp;
    float ftemp;
    uint8_t btemp;

    if(powType == 0)
    {
        itemp = att7022_read_register(r_Pa + phase);
    }
    else if(powType == 1)
    {
        itemp = att7022_read_register(r_Qa + phase);
    }
    else
    {
        itemp = att7022_read_register(r_Sa + phase);
    }

    if (itemp & _23_bit)
    {
        itemp = _24_bit - itemp;
        btemp = 1;
    }
    else
    {
        btemp = 0;
    }

    if (phase == 3)                    //合相
    {
        itemp <<= 1;                   //加倍
    }

    ftemp = (float)itemp * gFlashParam.st.K_Pow;
    if(btemp)
    {
        ftemp = -ftemp;
    }

    if(isNeg != NULL)
    {
        *isNeg = btemp;
    }

    return ftemp;
}

/**
  * @brief  校准功率增益
  * @param  fCali_Pow 标准源功率因数为1.0时输出功率值
  * @retval None
  */
void Cali_Gain_Power(float fCali_Pow)
{
    uint8_t i, j;
    uint16_t times;
    float fPow[3];

    times = 6;

    //复位增益寄存器
    for(i = 0; i < 3; i++)
    {
        gFlashParam.st.meter_cali[offset_PgainA + i] = 0;
        gFlashParam.st.meter_cali[offset_QgainA + i] = 0;
        gFlashParam.st.meter_cali[offset_SgainA + i] = 0;
        gFlashParam.st.meter_cali[offset_PhSregApq0 + i] = 0;
        gFlashParam.st.meter_cali[offset_PhSregApq1 + i] = 0;
        fPow[i] = 0;
    }
    Repair_Metering();
    gMeterRunInfo.EmuWork = EMU_CALI_GAIN_POW;
    vTaskDelay(2000);  //清零增益后需等待

    for(j = 0; j < 3; j++)  //第一次数据不要
    {
        Read_Power(j, 0, NULL);
        Read_Power_Factor(j, NULL);
    }
    vTaskDelay(100);
    //多次采
    for(i = 0; i < times; i++)
    {
        for(j = 0; j < 3; j++)
        {
            fPow[j] += Read_Power(j, 0, NULL);
        }
        vTaskDelay(100);
    }

    //计算
    for(i = 0; i < 3; i++)
    {
        fPow[i] /= (float)times;  //取平均值
        /* 被测量示值Pt   被测量真值P0
           err = Pt/P0-1
           Pgain = -err/(1+err) = P0/Pt-1
           Pgain >= 0, GP1 = INT[Pgain*2^15]
           Pgain <  0, GP1 = INT[2^16+Pgain*2^15] */
        fPow[i] = fCali_Pow / fPow[i];
        fPow[i] -= 1;
        gFlashParam.st.meter_cali[offset_PgainA + i] = float_data_to_register_value(_16_bit, fPow[i]);  //有功和无功功率增益写入同一值 选择PQS计算视在功率可不校正
        gFlashParam.st.meter_cali[offset_QgainA + i] = gFlashParam.st.meter_cali[offset_PgainA + i];
    }
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准功率增益（使用误差err校准）
  * @param  fErr 相对误差值
  * @retval None
  */
void Cali_Gain_Power_Use_Err(float *pfErr)
{
    uint8_t i;
    uint16_t utemp;
    float ftemp;

    /* 相对误差Err  已有增益Xgain0  新的增益Xgain1
    Xgain1 = (Xgain0-Err)/(Err+1)   */
    for(i = 0; i < 3; i++)
    {
        //取 Xgain0
        utemp = gFlashParam.st.meter_cali[offset_PgainA + i];
        ftemp = register_value_to_float_data(_16_bit, utemp);

        //计算出 Xgain1
        ftemp -= pfErr[i];
        ftemp = ftemp / (pfErr[i] + 1);

        //写 Xgain1
        gFlashParam.st.meter_cali[offset_PgainA + i] = float_data_to_register_value(_16_bit, ftemp);
        gFlashParam.st.meter_cali[offset_QgainA + i] = gFlashParam.st.meter_cali[offset_PgainA + i];
    }

    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准功率增益（已知 Ugain 和 Igain 时使用公式计算）
  * @param  None
  * @retval None
  */
void Cali_Gain_Power_Use_Formula(uint16_t HFconst, uint16_t EC, float *pN_Ib)
{
    uint8_t i;
    uint16_t utemp;
    float ftemp, fUgain, fIgain;

    /* 电压增益Ugain  电流增益Igain
    (Pgain+1) = (Ugain+1)*(Igain+1)*HFconst*EC*2^20 / (2.592*10^10*N)   */
    for(i = 0; i < 3; i++)
    {
        utemp = gFlashParam.st.meter_cali[offset_UgainA + i];
        fUgain = register_value_to_float_data(_16_bit, utemp);

        utemp = gFlashParam.st.meter_cali[offset_IgainA + i];
        fIgain = register_value_to_float_data(_16_bit, utemp);

        ftemp = EC * 0.01024; // 2^10/10^5=0.01024
        ftemp *= HFconst;
        ftemp *= 0.01024;
#if EMU_FREQ == 0
        ftemp /= 2.592;
#else
        ftemp /= 5.184;
#endif
        ftemp /= pN_Ib[i];
        ftemp *= (fIgain + 1);
        ftemp *= (fUgain + 1);
        ftemp -= 1;

        //写 Pgain
        gFlashParam.st.meter_cali[offset_PgainA + i] = float_data_to_register_value(_16_bit, ftemp);
        gFlashParam.st.meter_cali[offset_QgainA + i] = gFlashParam.st.meter_cali[offset_PgainA + i];
    }

    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  读取功率因数
  * @param  phase 0/1/2/3 => A/B/C/总
  * @param  isNeg 0/1 => 返回的 正/负
  * @retval 返回功率因数值
  */
float Read_Power_Factor(uint8_t phase, uint8_t *isNeg)
{
    uint32_t itemp;
    float ftemp;
    uint8_t btemp;

    itemp = att7022_read_register(r_Pfa + phase);

    if (itemp & _23_bit)
    {
        itemp = _24_bit - itemp;
        btemp = 1;
    }
    else
    {
        btemp = 0;
    }

    ftemp = (float)itemp / _23_bit;
    if(ftemp >= 1.000)
    {
        ftemp = 1.000;
    }
    else if(btemp)
    {
        ftemp = -ftemp;
    }

    if(isNeg != NULL)
    {
        *isNeg = btemp;
    }

    return ftemp;
}

/**
  * @brief  校准相位
  * @param  fCali_PF 标准源输出功率因数
  * @retval None
  */
void Cali_Gain_Power_Factor(float fCali_PF)
{
    uint8_t i, j;
    uint16_t times;
    float fPF[3];

    times = 6;

    //复位增益寄存器
    for(i = 0; i < 3; i++)
    {
        gFlashParam.st.meter_cali[offset_PhSregApq0 + i] = 0;
        gFlashParam.st.meter_cali[offset_PhSregApq1 + i] = 0;
        fPF[i] = 0;
    }
    Repair_Metering();
    gMeterRunInfo.EmuWork = EMU_CALI_GAIN_PHASE;
    vTaskDelay(2000);  //清零增益后需等待

    for(j = 0; j < 3; j++)  //第一次数据不要
    {
        Read_Power_Factor(j, NULL);
    }
    vTaskDelay(100);
    //多次采
    for(i = 0; i < times; i++)
    {
        for(j = 0; j < 3; j++)
        {
            fPF[j] += Read_Power_Factor(j, NULL);
        }
        vTaskDelay(100);
    }

    //计算
    for(i = 0; i < 3; i++)
    {
        fPF[i] /= (float)times;  //取平均值
        /* 被测量示值PFt   被测量真值PF0
           err = PFt/PF0-1
           PFgain = -err/1.732
           PFgain >= 0, GPF1 = INT[PFgain*2^15]
           PFgain <  0, GPF1 = INT[2^16+PFgain*2^15] */
        fPF[i] = fPF[i] / fCali_PF;
        fPF[i] -= 1;
        fPF[i] = -fPF[i];
        fPF[i] /= 1.732;
        if(fPF[i] < 0)
        {
            fPF[i] *= _15_bit;
            fPF[i] += _16_bit;
        }
        else
        {
            fPF[i] *= _15_bit;
        }

        gFlashParam.st.meter_cali[offset_PhSregApq0 + i] = (uint16_t)fPF[i];
        gFlashParam.st.meter_cali[offset_PhSregApq1 + i] = (uint16_t)fPF[i];
    }
    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准相位（使用误差err校准）
  * @param  fErr 相对误差值
  * @retval None
  */
void Cali_Gain_Power_Factor_Use_Err(float *pfErr)
{
    uint8_t i;
    uint16_t utemp;
    float ftemp, ftemp1;

    /* 相对误差Err  已有增益PFgain0  新的增益PFgain1
    PFgain1 = [(Err+1)*PFgain0]-(Err/1.732)   */
    for(i = 0; i < 3; i++)
    {
        //取 PFgain0
        utemp = gFlashParam.st.meter_cali[offset_PhSregApq0 + i];

        if(utemp & _15_bit)
        {
            ftemp = (float)utemp - _16_bit;
            ftemp /= _15_bit;
        }
        else
        {
            ftemp = (float)utemp / _15_bit;
        }

        //计算出 PFgain1
        ftemp1 = 1 + pfErr[i];
        ftemp *= ftemp1;
        ftemp1 = pfErr[i] / 1.732;
        ftemp -= ftemp1;

        //写 PFgain1
        if(ftemp < 0)
        {
            ftemp *= _15_bit;
            ftemp += _16_bit;
        }
        else
        {
            ftemp *= _15_bit;
        }

        gFlashParam.st.meter_cali[offset_PhSregApq0 + i] = (uint16_t)ftemp;
        gFlashParam.st.meter_cali[offset_PhSregApq1 + i] = (uint16_t)ftemp;
    }

    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  读取电压频率
  * @param  None
  * @retval 返回值
  */
float Read_Voltage_Frequency(void)
{
    uint32_t itemp;
    float ftemp;

    itemp = att7022_read_register(r_Freq);
    ftemp = (float)itemp / _13_bit;

    return ftemp;
}

/**
  * @brief  读取角度
  * @param  phase 0/1/2 => A/B/C
  * @param  angleType 0/1 => 电压/电流
  * @param  retType 0/1 => 返回类型 角度/弧度
  * @param  isNeg 0/1 => 返回的 正/负
  * @retval 返回角度/弧度值（角度=弧度*180）
  */
float Read_Angle(uint8_t phase, uint8_t angleType, uint8_t retType, uint8_t *isNeg)
{
    uint32_t itemp;
    float ftemp;
    uint8_t btemp;

    if(angleType == 0)
    {
        itemp = att7022_read_register(r_YUaUb + phase);
    }
    else
    {
        itemp = att7022_read_register(r_Pga + phase);
    }
    itemp = itemp & 0x1FFFFF; //21位有效数据，补码形式

    if(itemp & _20_bit)
    {
        itemp = _21_bit - itemp;
        btemp = 1;
    }
    else
    {
        btemp = 0;
    }

    ftemp = (float)itemp / _20_bit;
    if(btemp)
    {
        ftemp = -ftemp;
    }

    if(ftemp < 0)  //角度去除负数
    {
        ftemp += 2;
    }

    if(retType == 0)
    {
        ftemp *= 180.0;
    }
    else
    {
        ftemp *= PI;
    }

    if(isNeg != NULL)
    {
        *isNeg = btemp;
    }

    return ftemp;
}

/**
  * @brief  设置起动功率百分比
  * @param  percent 配置百分比
  * @retval None
  */
void Config_PStartup(float percent)
{
    //uint32_t itemp;
    float ftemp;


    /* Pstartup = INT[0.6*Ub*Ib*HFconst*EC*k%*2^23/(2.592*10^10)]
            = INT[0.6*220*2^23*Ib*HFconst*EC*k%/(2.592*10^10)]
            = INT[0.04272*Ib*HFconst*EC*k%]  */
    if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA1 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB1 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE1)
    {
#if EMU_FREQ == 0
        ftemp = 0.2136; //0.04272 * 5
#else
        ftemp = 0.1068;
#endif
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA2 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB2 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB2)
    {
#if EMU_FREQ == 0
        ftemp = 4.2720; //0.04272 * 100
#else
        ftemp = 2.136;
#endif
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA3 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB3 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE3)
    {
#if EMU_FREQ == 0
        ftemp = 8.5440; //0.04272 * 200
#else
        ftemp = 4.2720;
#endif
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA4 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB4 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE4)
    {
#if EMU_FREQ == 0
        ftemp = 17.0880; //0.04272 * 400
#else
        ftemp = 8.5440;
#endif
    }

    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA5 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB5 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE5)
    {
#if EMU_FREQ == 0
        ftemp = 25.6319; //0.04272 * 600
#else
        ftemp = 12.8160;
#endif
    }
    else
    {
#if EMU_FREQ == 0
        ftemp = 42.720; //0.04272 * 1000
#else
        ftemp = 21.360;
#endif
    }
    ftemp *= gFlashParam.st.meter_cali[offset_HFConst];
    ftemp *= gFlashParam.st.EC;
    ftemp *= percent;

    gFlashParam.st.meter_cali[offset_Pstartup] = (uint16_t)ftemp;

    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

    gMeterRunInfo.EmuWork = EMU_INIT;
}

/**
  * @brief  校准整体
  * @param  None
  * @retval None
  */
void Cali_I_U_P_PF_T0(float fCali_Cur, float fCali_Vol, float fCali_PF)
{
    uint8_t i, j;
    uint16_t times;
    float fCur[3], fVol[3], fPF[3];

    times = 6;

    //复位增益和偏移寄存器
    for(i = 0; i < 3; i++)
    {
        gFlashParam.st.meter_cali[offset_IgainA + i] = 0;
        gFlashParam.st.meter_cali[offset_UgainA + i] = 0;
        gFlashParam.st.meter_cali[offset_PhSregApq0 + i] = 0;
        gFlashParam.st.meter_cali[offset_PhSregApq1 + i] = 0;

        fCur[i] = 0;
        fVol[i] = 0;
        fPF[i] = 0;
    }
    Repair_Metering();
    gMeterRunInfo.EmuWork = EMU_CALI_U_I_P_PF;
    vTaskDelay(2000);  //清零增益后需等待

    for(j = 0; j < 3; j++)
    {
        Read_Current(j);
        Read_Voltage(j);
        Read_Power_Factor(j, NULL);
    }

    //多次采
    for(i = 0; i < times; i++)
    {
        for(j = 0; j < 3; j++)
        {
            fCur[j] += Read_Current(j);
            fVol[j] += Read_Voltage(j);
            fPF[j] += Read_Power_Factor(j, NULL);
        }
        vTaskDelay(100);
    }

    //计算
    for(i = 0; i < 3; i++)
    {
        //I
        fCur[i] /= (float)times;  //取平均值
        /* 被测量示值It   被测量真值I0
           Igain = I0/It - 1
           Igain >= 0, GI1 = INT[Igain*2^15]
           Igain <  0, GI1 = INT[2^16+Igain*2^15] */
        fCur[i] = fCali_Cur / fCur[i];
        fCur[i] -= 1;
        gFlashParam.st.meter_cali[offset_IgainA + i] = float_data_to_register_value(_16_bit, fCur[i]);

        //U
        fVol[i] /= (float)times;  //取平均值
        /* 被测量示值Ut   被测量真值U0
           Ugain = U0/Ut - 1
           Ugain >= 0, GU1 = INT[Ugain*2^15]
           Ugain <  0, GU1 = INT[2^16+Ugain*2^15] */
        fVol[i] = fCali_Vol / fVol[i];
        fVol[i] -= 1;
        gFlashParam.st.meter_cali[offset_UgainA + i] = float_data_to_register_value(_16_bit, fVol[i]);

        //PF
        fPF[i] /= (float)times;  //取平均值
        /* 被测量示值PFt   被测量真值PF0
           err = PFt/PF0-1
           PFgain = -err/1.732
           PFgain >= 0, GPF1 = INT[PFgain*2^15]
           PFgain <  0, GPF1 = INT[2^16+PFgain*2^15] */
        fPF[i] = fPF[i] / fCali_PF;
        fPF[i] -= 1;
        fPF[i] = -fPF[i];
        fPF[i] /= 1.732;
        if(fPF[i] < 0)
        {
            fPF[i] *= _15_bit;
            fPF[i] += _16_bit;
        }
        else
        {
            fPF[i] *= _15_bit;
        }

        gFlashParam.st.meter_cali[offset_PhSregApq0 + i] = (uint16_t)fPF[i];
        gFlashParam.st.meter_cali[offset_PhSregApq1 + i] = (uint16_t)fPF[i];
    }

    Cali_Gain_Power_Use_Formula(gFlashParam.st.meter_cali[offset_HFConst], gFlashParam.st.EC, gFlashParam.st.N_Ib_Vi);
}

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB
/**
  * @brief  读取基波电压有效值
  * @param  phase 0/1/2 => A/B/C
  * @retval 基波电压有效值
  */
float Read_Voltage_FundamentalWave(uint8_t phase)
{
    uint32_t itemp;
    float ftemp;

    itemp = att7022_read_register(r_LineUaRms + phase);
    ftemp = (float)itemp / _13_bit;

    return ftemp;
}

/**
  * @brief  读取基波电流有效值
  * @param  phase 0/1/2 => A/B/C
  * @retval 基波电流有效值
  */
float Read_Current_FundamentalWave(uint8_t phase)
{
    uint32_t itemp;
    float ftemp;

    itemp = att7022_read_register(r_LineIaRms + phase);
    ftemp = (float)itemp / _13_bit;
    ftemp /= gFlashParam.st.N_Ib_Vi[phase];

    return ftemp;
}

/**
  * @brief  复数加法 compx1+compx2
  * @param  compx1
  * @param  compx2
  * @retval 复数结果
  */
compx_t Complex_Add_Complex(compx_t X1, compx_t X2)
{
    compx_t X3;

    X3.real = X1.real + X2.real;
    X3.imag = X1.imag + X2.imag;

    return X3;
}

/**
  * @brief  复数乘法 compx1*compx2
  * @param  compx1
  * @param  compx2
  * @retval 复数结果
  */
compx_t Complex_Multi_Complex(compx_t X1, compx_t X2)
{
    compx_t X3;

    X3.real = X1.real * X2.real - X1.imag * X2.imag;
    X3.imag = X1.real * X2.imag + X1.imag * X2.real;

    return X3;
}

/**
  * @brief  复数计算幅值
  * @param  compx 输入复数
  * @retval 幅值
  */
float Compx_Calculate_Amplitude(compx_t X)
{
    return sqrt(X.real * X.real + X.imag * X.imag);
}

/**
  * @brief  复数计算相位
  * @param  compx 输入复数
  * @retval 相位
  */
float Compx_Calculate_Phase(compx_t X)
{
    return atan2(X.imag, X.real);
}

/**
  * @brief  已知幅值和相角计算正负零序
  * @param  pRms 幅值
  * @param  pAngle 相角
  * @param  pE 返回的零/正/负序
  * @retval None
  */
void Calculate_Positive_Negative_Zero_Sequence(float *pRms, float *pAngle, float *pE)
{
    uint8_t i;
    float ftemp;
    compx_t compx1;
    compx_t compx2;
    compx_t compxRms[3];  //有效值

    /* U[i]   = |U[i]|cosa + i|U[i]|sina
       U[i].r = |U[i]|cosa
       U[i].i = |U[i]|sina  */

    //计算有效值
    for(i = 0; i < 3; i++)
    {
        //计算弧度
        ftemp = pAngle[i] / 180.0;
        ftemp *= PI;

        compxRms[i].real = pRms[i] * cos(ftemp);
        compxRms[i].imag = pRms[i] * sin(ftemp);
    }

    /* 计算零序 3*U0 = (U[0] + U[1] + U[2])
                = (U[0].r+U[1].r+U[2].r) + i(U[0].i+U[1].i+U[2].i)    */
    compx1.real = 0;
    compx1.imag = 0;
    for(i = 0; i < 3; i++)
    {
        compx1 = Complex_Add_Complex(compx1, compxRms[i]);
    }
    compx1.real /= 3;
    compx1.imag /= 3;
    pE[0] = Compx_Calculate_Amplitude(compx1);

    /* 计算正序 3*U1 = U[0] + [-1/2+i(sqr(3)/2)]*U[1] + [-1/2-i(sqr(3)/2)]*U[2]   其中sqr(3)/2 = 0.866 */
    /* 计算负序 3*U2 = U[0] + [-1/2-i(sqr(3)/2)]*U[1] + [-1/2+i(sqr(3)/2)]*U[2]   其中sqr(3)/2 = 0.866 */
    compx1.real = compxRms[0].real;
    compx1.imag = compxRms[0].imag;
    compx2.real = -0.5;
    compx2.imag = 0.866;
    compx2 = Complex_Multi_Complex(compxRms[1], compx2);
    compx1.real += compx2.real;
    compx1.imag += compx2.imag;
    compx2.real = -0.5;
    compx2.imag = -0.866;
    compx2 = Complex_Multi_Complex(compxRms[2], compx2);
    compx1.real += compx2.real;
    compx1.imag += compx2.imag;
    compx1.real /= 3;
    compx1.imag /= 3;
    pE[1] = Compx_Calculate_Amplitude(compx1);

    compx1.real = compxRms[0].real;
    compx1.imag = compxRms[0].imag;
    compx2.real = -0.5;
    compx2.imag = -0.866;
    compx2 = Complex_Multi_Complex(compxRms[1], compx2);
    compx1.real += compx2.real;
    compx1.imag += compx2.imag;
    compx2.real = -0.5;
    compx2.imag = 0.866;
    compx2 = Complex_Multi_Complex(compxRms[2], compx2);
    compx1.real += compx2.real;
    compx1.imag += compx2.imag;
    compx1.real /= 3;
    compx1.imag /= 3;
    pE[2] = Compx_Calculate_Amplitude(compx1);
}

/**
  * @brief  报警判断三相不平衡度
  * @param  None
  * @retval None
  */
void alarm_imb_process(void)
{
    float fUI_THUp[2];

    /* 电压 电流不平衡度 */
    fUI_THUp[0] = gFlashParam.st.eUI_THUp[0] / 100.0;
    fUI_THUp[1] = gFlashParam.st.eUI_THUp[1] / 100.0;
    if(gPowerQualityParam.Volt_Imbalance[0] > fUI_THUp[0] || gPowerQualityParam.Volt_Imbalance[1] > fUI_THUp[1])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_VOL_IMB_Msk);
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOL_IMB_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_VOL_IMB_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_VOL_IMB_Msk);
        }
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_VOL_IMB_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOL_IMB_Msk);
        }
    }
    
    fUI_THUp[0] = gFlashParam.st.eUI_THUp[2] / 100.0;
    fUI_THUp[1] = gFlashParam.st.eUI_THUp[3] / 100.0;
    if(gPowerQualityParam.Curr_Imbalance[0] > fUI_THUp[0] || gPowerQualityParam.Curr_Imbalance[1] > fUI_THUp[1])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_CUR_IMB_Msk);
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CUR_IMB_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_CUR_IMB_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_CUR_IMB_Msk);
        }
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_CUR_IMB_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CUR_IMB_Msk);
        }
    }
}

/**
  * @brief  计算三相不平衡度
  * @param  None
  * @retval None
  */
void Calculate_Phase_Unbalance(void)
{
    //uint8_t i;

    //计算零正负序
    Calculate_Positive_Negative_Zero_Sequence(gPowerQualityParam.Fundamental_U, gPowerQualityParam.Fundamental_UA, gPowerQualityParam.Volt_E);
    Calculate_Positive_Negative_Zero_Sequence(gPowerQualityParam.Fundamental_I, gPowerQualityParam.Fundamental_IA, gPowerQualityParam.Curr_E);

    /* 零序不平衡度 E0=U0/U1    负序不平衡度 E2=U2/U1   */
    if(gPowerQualityParam.Volt_E[1] == 0)
    {
        gPowerQualityParam.Volt_Imbalance[0] = 0;
        gPowerQualityParam.Volt_Imbalance[1] = 0;
    }
    else
    {
        gPowerQualityParam.Volt_Imbalance[0] = gPowerQualityParam.Volt_E[0] / gPowerQualityParam.Volt_E[1];
        gPowerQualityParam.Volt_Imbalance[1] = gPowerQualityParam.Volt_E[2] / gPowerQualityParam.Volt_E[1];
        if(gPowerQualityParam.Volt_Imbalance[0] > 1)
        {
            gPowerQualityParam.Volt_Imbalance[0] = 1;
        }
        if(gPowerQualityParam.Volt_Imbalance[1] > 1)
        {
            gPowerQualityParam.Volt_Imbalance[1] = 1;
        }
        gPowerQualityParam.Volt_Imbalance[0] *= 100;
        gPowerQualityParam.Volt_Imbalance[1] *= 100;
    }
    if(gPowerQualityParam.Curr_E[1] == 0)
    {
        gPowerQualityParam.Curr_Imbalance[0] = 0;
        gPowerQualityParam.Curr_Imbalance[1] = 0;
    }
    else
    {
        gPowerQualityParam.Curr_Imbalance[0] = gPowerQualityParam.Curr_E[0] / gPowerQualityParam.Curr_E[1];
        gPowerQualityParam.Curr_Imbalance[1] = gPowerQualityParam.Curr_E[2] / gPowerQualityParam.Curr_E[1];
        if(gPowerQualityParam.Curr_Imbalance[0] > 1)
        {
            gPowerQualityParam.Curr_Imbalance[0] = 1;
        }
        if(gPowerQualityParam.Curr_Imbalance[1] > 1)
        {
            gPowerQualityParam.Curr_Imbalance[1] = 1;
        }
        gPowerQualityParam.Curr_Imbalance[0] *= 100;
        gPowerQualityParam.Curr_Imbalance[1] *= 100;
    }
}

/**
  * @brief  自动同步采样初始化
            Femu时钟频率Femu=921.6kHz  ADC采样频率Fs=3.2kHz  一周期固定点N=64
  * @param  None
  * @retval None
  */
void ADC_Auto_Sync_Init(void)
{
    att7022_write_register(w_EnWrCali, 0x00005A);
    att7022_write_register(w_SyncStr, 0x000000);  //停止同步数据功能
    att7022_write_register(w_SyncStr, 0x000002);  //开启自动同步数据功能
    att7022_write_register(w_EnWrCali, ~0x00005A);
}

/**
  * @brief  手动同步采样初始化
            Femu时钟频率Femu=1.8432MHz  ADC采样频率Fs=6.4kHz  一周期固定点N=128
  * @param  None
  * @retval None
  */
void ADC_Manual_Sync_Init(void)
{
    uint32_t flag_buf;

    att7022_write_register(w_EnWrCali, 0x00005A);
    att7022_write_register(w_SyncStr, 0x00000D);  //停止同步数据功能
    flag_buf = att7022_read_register(r_Freq);     //read freq
    flag_buf = (14400 * 8192) / flag_buf;         //计算同步系数
    att7022_write_register(w_SyncSet, flag_buf);  //写入同步系数
    att7022_write_register(w_SyncStr, 0x00000F);  //开启手动同步
    att7022_write_register(w_EnWrCali, ~0x00005A);
}

//found the max data
short max_find(short *x, unsigned char num)
{
    short iax, maxret, tmp;

    tmp = abs(x[0]);
    for(iax = 0; iax < num; iax++) //one wave 128 points
    {
        if((x[iax] < 0) && ((tmp + x[iax]) < 0))
        {
            maxret = -x[iax];
            tmp = maxret;
        }
        if((x[iax] > 0) && ((tmp - x[iax]) < 0))
        {
            maxret = x[iax];
            tmp = maxret;
        }
    }
    return(maxret);
}

/**
  * @brief  FFT程序
  * @param  xin 输入/输出序列
  * @param  N 输入点数, N需满足二次幂8、16、32、64、128、...
  * @retval None
  */
void FFT(compx_t *pX, short N)
{
    uint8_t f, i, j, k, L, m;
    uint16_t le, B, ip;
    compx_t w, t;

    //倒序排序
    j = N / 2;
    for(i = 1; i <= (N - 2); i++)
    {
        if(i < j)
        {
            t = pX[j];
            pX[j] = pX[i];
            pX[i] = t;
        }
        k = N / 2;
        while(j >= k)
        {
            j = j - k;
            k = k / 2;
        }
        j = j + k;
    }

    //蝶形运算
    f = N;
    for(m = 1; (f = f / 2) != 1; m++);  //m ranks for FFT

    for(L = 1; L <= m; L++)	//rank: 1--m
    {
        le = (unsigned short)pow(2, L);
        B = le / 2;

        for(j = 0; j <= B - 1; j++)
        {
            w.real = cosf_tab[(256 * j) >> L];
            w.imag = -sinf_tab[(256 * j) >> L];

            for(i = j; i <= N - 1; i = i + le)
            {
                ip = i + B;
                t = Complex_Multi_Complex(pX[ip], w);

                pX[ip].real = pX[i].real - t.real;
                pX[ip].imag = pX[i].imag - t.imag;
                pX[i].real = pX[i].real + t.real;
                pX[i].imag = pX[i].imag + t.imag;
            }
        }
    }
    /***** FFT done****/
}

/**
  * @brief  采样数据处理
  * @param  None
  * @retval None
  */
void Sample_Data_Handle(void)
{
    uint8_t i, k;
    int16_t imax;
    int32_t avg;
//    float fft_result;
#if PROD_TYPE == PROD_SFE
    float fundamentalWave, oneFFTData, precentSend;
#endif    
    float ftemp, fphase;
    compx_t fundamentalPhase[6];

    for(k = 0; k < 6; k++)  //从7个通道中获取一个通道数据
    {
        avg = 0;
        for(i = 0; i < SamplePointN; i++)
        {
            gActualData[i] = Sync_SampleD.D[k][i];
            avg += gActualData[i];
        }
        avg /= SamplePointN;

        for(i = 0; i < SamplePointN; i++)
        {
            gActualData[i] -= avg;  //把直流电擦除
        }

        imax = max_find(gActualData, SamplePointN);
        for(i = 0; i < SamplePointN; i++)
        {
            gCompx[i].real = gActualData[i];
            gCompx[i].real = gCompx[i].real / imax;   //减小幅值范围
            gCompx[i].imag = 0;
        }

        FFT(gCompx, SamplePointN);

        fundamentalPhase[k] = gCompx[1];  //保存各通道基波相位

#if PROD_TYPE == PROD_SFE
        for(i = 0; i < (SamplePointN >> 1); i++)  //FFT结果镜像(共轭)，所以只需要一半
        {
            gFFTData[i]  = Compx_Calculate_Amplitude(gCompx[i]);
            //gFFTData[i] = fft_result * 2 / SamplePointN;  //减小幅值范围
        }

        for(i = 0; i < 32; i++)
        {
            gFFTData[i + 1] *= FftCoefficient[i]; //将FFT结果乘以FFT系数
        }

        fundamentalWave = gFFTData[1];
        for(i = 2; i < 32; i++)
        {
            oneFFTData = gFFTData[i];
#if 0  //参考代码
            if(fundamentalWave > 1)  //如果基波很小，忽略谐波的计算，将其计算为0
            {
                precentSend = oneFFTData / fundamentalWave;  //计算除0次外的谐波
                if(k < 3)  //U
                {
                    gHarmonicParam.Volt_2_31[i - 2][k] = (int16_t)(precentSend * 10000);
                }
                else  //I
                {
                    gHarmonicParam.Curr_2_31[i - 2][k - 3] = (int16_t)(precentSend * 10000);
                }
            }
            else
            {
                if(k < 3)
                {
                    gHarmonicParam.Volt_2_31[i - 2][k] = 0;
                }
                else
                {
                    gHarmonicParam.Curr_2_31[i - 2][k - 3] = 0;
                }
            }
#else
            if(k < 3)
            {
                if(gMeterParam.Volt[k] == 0)
                {
                    gHarmonicParam.Volt_2_31[i - 2][k] = 0;
                }
                else
                {
                    precentSend = oneFFTData / fundamentalWave;  //计算除0次外的谐波
                    gHarmonicParam.Volt_2_31[i - 2][k] = (uint16_t)(precentSend * 10000);
                }
            }
            else
            {
                if(gMeterParam.Curr[k - 3] == 0)
                {
                    gHarmonicParam.Curr_2_31[i - 2][k - 3] = 0;
                }
                else
                {
                    precentSend = oneFFTData / fundamentalWave;  //计算除0次外的谐波
                    gHarmonicParam.Curr_2_31[i - 2][k - 3] = (uint16_t)(precentSend * 10000);
                    
                    gFlashParam.st.DeltaKA2 = gHarmonicParam.Curr_2_31[0][0];
                    gFlashParam.st.DeltaKA3 = gHarmonicParam.Curr_2_31[1][0];
                    
                    gFlashParam.st.DeltaKB2 = gHarmonicParam.Curr_2_31[0][1];
                    gFlashParam.st.DeltaKB3 = gHarmonicParam.Curr_2_31[1][1];
                    
                    gFlashParam.st.DeltaKC2 = gHarmonicParam.Curr_2_31[0][2];
                    gFlashParam.st.DeltaKC3 = gHarmonicParam.Curr_2_31[1][2];
                }
            }
#endif
        }
#endif /* PROD_TYPE == PROD_ESE */
        //vTaskDelay(1);
    }

    //计算基波相角
    gPowerQualityParam.Fundamental_UA[0] = 0;
    fphase = Compx_Calculate_Phase(fundamentalPhase[0]);  //A相电压相位
    for(k = 1; k < 6; k++)
    {
        ftemp = Compx_Calculate_Phase(fundamentalPhase[k]);
        ftemp = fphase - ftemp;  //以A相电压为基准
        ftemp = ftemp / PI;  //弧度->角度
        if(ftemp < 0)
        {
            ftemp += 2;
        }
        ftemp = ftemp * 180;

        if(k < 3)
        {
            gPowerQualityParam.Fundamental_UA[k] = ftemp;
        }
        else
        {
            gPowerQualityParam.Fundamental_IA[k - 3] = ftemp;
        }
    }
}

/**
  * @brief  谐波处理循环（500ms调用一次该函数）
  * @param  None
  * @retval None
  */
void Do_Harmonic_Loop(void)
{
    static uint8_t LoopState = 3;
    uint8_t i, j;

    switch(LoopState)
    {
    case 0:  //初始化同步采样
        //ADC_Auto_Sync_Init();
        ADC_Manual_Sync_Init();
        LoopState++;
        break;

    case 1:  //采样数据获取
        att7022_write_register(w_BuffSet, 0x000023);  //发送读取缓存指针，丢弃前35个不准的点
        for(i = 0; i < 146; i++)
        {
            for(j = 0; j < 7; j++)
            {
                Sync_SampleD.D[j][i] = att7022_read_register(r_WaveBuff);
            }
        }
        LoopState++;
        break;
    
//        att7022_write_register(w_BuffSet, 0x000023);  //发送读取缓存指针，丢弃前35个不准的点
//        for(i = 0; i < 30; i++)
//        {
//            for(j = 0; j < 7; j++)
//            {
//                Sync_SampleD.D[j][i] = att7022_read_register(r_WaveBuff);
//            }
//        }
//        LoopState++;
//        break;
        
    case 2:  //采样数据处理
        Sample_Data_Handle();
        LoopState++;
        break;

    case 3:  //计算三相不平衡度
        for(i = 0; i < 3; i++)  //采样基波有效值
        {
            gPowerQualityParam.Fundamental_U[i] = Read_Voltage_FundamentalWave(i);
            if(gPowerQualityParam.Fundamental_U[i] < gFlashParam.st.Vol_Minimun)
            {
                gPowerQualityParam.Fundamental_U[i] = 0;
            }

            gPowerQualityParam.Fundamental_I[i] = Read_Current_FundamentalWave(i);
            if(gPowerQualityParam.Fundamental_I[i] < gFlashParam.st.Cur_Minimun)
            {
                gPowerQualityParam.Fundamental_I[i] = 0;
            }
        }
        Calculate_Phase_Unbalance();
        alarm_imb_process();
        LoopState++;
        break;

    default:  //等待下一次谐波采样计算
        if(LoopState >= 4)  //前提N>=4，可延时(N+1)*500ms
        {
            LoopState = 0;
        }
        else
        {
            LoopState++;
        }
        break;
    }
}

#if PROD_TYPE == PROD_SFE
/**
  * @brief  报警判断谐波
  * @param  None
  * @retval None
  */
void alarm_THD_process(void)
{
    uint8_t i, j, flgU, flgUA, flgUB, flgUC, flgUAx, flgUBx, flgUCx, flgI, flgIA, flgIB, flgIC, flgIAx, flgIBx, flgICx;

    /* 电压 电流谐波 */
    flgU = 0;
		flgUA = 0;
		flgUB = 0;
		flgUC = 0;
		flgUAx = 0;
		flgUBx = 0;
		flgUCx = 0;
	
		flgI = 0;
    flgIA = 0;
		flgIB = 0;
		flgIC = 0;
		flgIAx = 0;
		flgIBx = 0;
		flgICx = 0;
	
    for(i = 0; i < 3; i++)
    {
        if(gHarmonicParam.Volt_1[i] > gFlashParam.st.THD_THUp[0]) //判断总谐波畸变率
        {
            flgU = 1;
						if(i == 0)
						{
								flgUA = 1;
						}
						else if(i == 1)
						{
								flgUB = 1;
						}
						else if(i == 2)
						{
								flgUC = 1;
						}
        }
        else
        {
            for(j = 0; j < 30; j++)
            {
                if(j % 2)  //判断奇次谐波含有率
                {
                    if(gHarmonicParam.Volt_2_31[j][i] > gFlashParam.st.HR_THUp[0])
                    {
												if(i == 0)
												{		
														flgUAx = 1;
														UAHarmonici = i;
														UAHarmonicj = j;
												}
												else if(i == 1)
												{	
														flgUBx = 1;
														UBHarmonici = i;
														UBHarmonicj = j;
												}
												else if(i == 2)
												{
														flgUCx = 1;
														UCHarmonici = i;
														UCHarmonicj = j;
												}
                        break;
                    }
                }
                else  //判断偶次谐波含有率
                {
                    if(gHarmonicParam.Volt_2_31[j][i] > gFlashParam.st.HR_THUp[1])
                    {
												if(i == 0)
												{		
														flgUAx = 1;
														UAHarmonici = i;
														UAHarmonicj = j;
												}
												else if(i == 1)
												{	
														flgUBx = 1;
														UBHarmonici = i;
														UBHarmonicj = j;
												}
												else if(i == 2)
												{
														flgUCx = 1;
														UCHarmonici = i;
														UCHarmonicj = j;
												}
                        break;
                    }
                }
            }
        }

        if(gHarmonicParam.Curr_1[i] > gFlashParam.st.THD_THUp[1]) //判断总谐波畸变率
        {
            flgI = 1;
						if(i == 0)
						{
								flgIA = 1;
						}
						else if(i == 1)
						{
								flgIB = 1;
						}
						else if(i == 2)
						{
								flgIC = 1;
						}
        }
        else
        {
            for(j = 0; j < 30; j++)
            {
                if(j % 2)  //判断奇次谐波含有率
                {
                    if(gHarmonicParam.Curr_2_31[j][i] > gFlashParam.st.HR_THUp[2])
                    {
												if(i == 0)
												{		
														flgIAx = 1;
														IAHarmonici = i;
														IAHarmonicj = j;
												}
												else if(i == 1)
												{	
														flgIBx = 1;
														IBHarmonici = i;
														IBHarmonicj = j;
												}
												else if(i == 2)
												{
														flgICx = 1;
														ICHarmonici = i;
														ICHarmonicj = j;
												}
                        break;
                    }
                }
                else  //判断偶次谐波含有率
                {
                    if(gHarmonicParam.Curr_2_31[j][i] > gFlashParam.st.HR_THUp[3])
                    {
												if(i == 0)
												{		
														flgIAx = 1;
														IAHarmonici = i;
														IAHarmonicj = j;
												}
												else if(i == 1)
												{	
														flgIBx = 1;
														IBHarmonici = i;
														IBHarmonicj = j;
												}
												else if(i == 2)
												{
														flgICx = 1;
														ICHarmonici = i;
														ICHarmonicj = j;
												}
                        break;
                    }
                }
            }
        }
    }

    if(flgU)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_VOL_THD_Msk);
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOL_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_VOL_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_VOL_THD_Msk);
        }
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_VOL_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOL_THD_Msk);
        }
    }
		
		if(flgUA)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLA_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_VOLA_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLA_THD_Msk);
        }
    }

		if(flgUB)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLB_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_VOLB_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLB_THD_Msk);
        }
    }
		
		if(flgUC)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLC_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_VOLC_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLC_THD_Msk);
        }
    }
		
		if(flgUAx)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLAx_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_VOLAx_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLAx_THD_Msk);
        }
    }

		if(flgUBx)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLBx_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_VOLBx_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLBx_THD_Msk);
        }
    }
		
		if(flgUCx)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLCx_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_VOLCx_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLCx_THD_Msk);
        }
    }
		
    if(flgI)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_CUR_THD_Msk);
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CUR_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_CUR_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_CUR_THD_Msk);
        }
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_CUR_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CUR_THD_Msk);
        }
    }
		
		if(flgIA)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURA_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_CURA_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURA_THD_Msk);
        }
    }

		if(flgIB)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURB_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_CURB_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURB_THD_Msk);
        }
    }
		
		if(flgIC)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURC_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_CURC_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURC_THD_Msk);
        }
    }
		
		if(flgIAx)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURAx_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_CURAx_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURAx_THD_Msk);
        }
    }

		if(flgIBx)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURBx_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_CURBx_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURBx_THD_Msk);
        }
    }
		
		if(flgICx)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURCx_THD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm2], StateAlm2_CURCx_THD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURCx_THD_Msk);
        }
    }
}

/**
  * @brief  恶性负载报警判断（500ms调用一次该函数）
            计算总谐波畸变率  THDu = sqrt[(Urms^2-Ufundamental^2)/Ufundamental^2]
  * @param  None
  * @retval None
  */

void alarm_bike_process(void)
{
    uint8_t i, flgI, THDUaSC, THDUaEA, THDUaVC, THDUbSC, THDUbEA, THDUbVC, THDUcSC, THDUcEA, THDUcVC;
    
    flgI = 0;
	
		THDUaSC = 0;
		THDUaEA = 0;
		THDUaVC = 0;
		
		THDUbSC = 0;
		THDUbEA = 0;
		THDUbVC = 0;
	
		THDUcSC = 0;
		THDUcEA = 0;
		THDUcVC = 0;
    
    for(i = 0; i < 3; i++)
    {
        if(gHarmonicParam.Curr_1[i] > gFlashParam.st.Malignantloadmin && gHarmonicParam.Curr_1[i] < gFlashParam.st.Malignantloadmax) //判断总谐波畸变率
        {            
            if(gHarmonicParam.Curr_2_31[0][i] < gFlashParam.st.MalignantloadSmax)
            {
                if(gHarmonicParam.Curr_2_31[1][i] > gFlashParam.st.MalignantloadTmin)
                {
                    if(gHarmonicParam.Curr_2_31[1][i] > gHarmonicParam.Curr_2_31[3][i] && gHarmonicParam.Curr_2_31[3][i] > gHarmonicParam.Curr_2_31[5][i] && gHarmonicParam.Curr_2_31[5][i] > gHarmonicParam.Curr_2_31[0][i])
                    {                                    
                        flgI = 1;
                    }
                }                    
            }               
        }
    }

		if(flgI)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_LOAD_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_LOAD_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_LOAD_Msk);
        }
    }
		
		for(i = 0; i < 3; i++)
    {
				if(gHarmonicParam.Volt_1[i] > 300 && gHarmonicParam.Volt_1[i] < 500) //短路故障电压总谐波畸变率3%-5%
        {
						if(gHarmonicParam.Curr_1[i] > 500 && gHarmonicParam.Curr_1[i] < 800) //短路故障电流总谐波畸变率5%-15%
						{      
								if(gHarmonicParam.Curr_2_31[0][i] > 1000 && gHarmonicParam.Curr_2_31[0][i] < 1500) //二次谐波含有量10%-15%
								{  
										if(i==0)
										{
												THDUaSC = 1;
										}
										else if(i==1)
										{
												THDUbSC = 1;
										}
										else if(i==2)
										{
												THDUcSC = 1;
										}             
								}               
						}
				}
    }				
		
		for(i = 0; i < 3; i++)
    {
				if(gHarmonicParam.Volt_1[i] > 500 && gHarmonicParam.Volt_1[i] < 1000) //电弧故障电压总谐波畸变率5%-10%
        {
						if(gHarmonicParam.Curr_1[i] > 1500 && gHarmonicParam.Curr_1[i] < 2500) //电弧故障电流总谐波畸变率15%-25%
						{   
								if(gHarmonicParam.Curr_2_31[1][i] > 2000 && gHarmonicParam.Curr_2_31[1][i] < 3000) //三次谐波含有量20%-30%
								{                                
										if(gHarmonicParam.Curr_2_31[3][i] > 1000 && gHarmonicParam.Curr_2_31[3][i] < 1500) //五次谐波含有量10%-15%
										{      
												if(i==0)
												{
														THDUaEA = 1;
												}
												else if(i==1)
												{
														THDUbEA = 1;
												}
												else if(i==2)
												{
														THDUcEA = 1;
												}             
										}
								}               
					}
				}
    }
		
		for(i = 0; i < 3; i++)
    {
				if(gHarmonicParam.Volt_1[i] > 400 && gHarmonicParam.Volt_1[i] < 700) //虚接打火故障电压总谐波畸变率4%-7%
        {
						if(gHarmonicParam.Curr_1[i] > 800 && gHarmonicParam.Curr_1[i] < 1200) //虚接打火故障电流总谐波畸变率8%-12%
						{           
								if(gHarmonicParam.Curr_2_31[1][i] > 800 && gHarmonicParam.Curr_2_31[1][i] < 1500) //三次谐波含有量8%-15%
								{  
										if(gHarmonicParam.Curr_2_31[3][i] > 300 && gHarmonicParam.Curr_2_31[3][i] < 800) //五次谐波含有量3%-8%
										{
												if(i==0)
												{
														THDUaVC = 1;
												}
												else if(i==1)
												{
														THDUbVC = 1;
												}
												else if(i==2)
												{
														THDUcVC = 1;
												}
										}										
								}               
					}
				}
    }		

		if(THDUaSC)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaSC_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUaSC_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaSC_Msk);
        }
    }
		
		if(THDUbSC)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbSC_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUbSC_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbSC_Msk);
        }
    }
		
		if(THDUcSC)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcSC_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUcSC_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcSC_Msk);
        }
    }
		
		if(THDUaEA)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaEA_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUaEA_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaEA_Msk);
        }
    }
		
		if(THDUbEA)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbEA_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUbEA_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbEA_Msk);
        }
    }
		
		if(THDUcEA)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcEA_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUcEA_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcEA_Msk);
        }
    }
		
		if(THDUaVC)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaVC_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUaVC_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaVC_Msk);
        }
    }
		
		if(THDUbVC)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbVC_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUbVC_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbVC_Msk);
        }
    }
		
		if(THDUcVC)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcVC_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm4], StateAlm4_THDUcVC_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcVC_Msk);
        }
    }
}

void Do_elecbike_detect(void)
{
    static uint8_t LoopState = 0;
    static uint8_t detectState = 0;
    
    static float fx[6];  //基波数据滞后有效值数据约500ms，所以需保存
    uint8_t i;
    
    float ftemp1, ftemp2;
    float ftempU, ftempI, ftempP;

    switch(LoopState)
    {
    case 0:  //采样有效值
        for (i = 0; i < 3; i++)
        {            
            ftempU = Read_Voltage(i);
            if(ftempU != 0)
            {
                fx[i] = ftempU;
            }

            ftempI = Read_Current(i);
            if(ftempI != 0)
            {
                fx[i + 3] = ftempI;  
            }                      

//            if((fx[i] * fx[i + 3]) != 0 && (fx[i] * fx[i + 3]) !=(gMeterParam.PowP[i] * 1000.0))
//            {                
                ftempP = (fx[i] * fx[i + 3]) - (gMeterParam.PowP[i]);

                gFlashParam.st.DeltaP[i] = ftempP;
                
                gFlashParam.st.DeltaP[3] = (fx[0] * fx[3] + fx[1] * fx[4] + fx[2] * fx[5]) - (gMeterParam.PowP[3]);
            
                if(gFlashParam.st.DeltaP[i] !=0 && gFlashParam.st.DeltaP[i] > gFlashParam.st.MalignantloadPmin && gFlashParam.st.DeltaP[i] < gFlashParam.st.MalignantloadPmax) 
                {
                    detectState = 1;                                              
                }                
                else if(gFlashParam.st.DeltaP[3] !=0 && gFlashParam.st.DeltaP[3] > gFlashParam.st.MalignantloadPmin && gFlashParam.st.DeltaP[3] < gFlashParam.st.MalignantloadPmax) 
                {
                    detectState = 1;                                              
                }
//            }
        }
        
        if(detectState == 1)
        {
            LoopState++;           
        }    
        break;

    case 1:  //采样基波有效值
        for(i = 0; i < 3; i++)
        {
            gPowerQualityParam.Fundamental_I[i] = Read_Current_FundamentalWave(i);
            if(gPowerQualityParam.Fundamental_I[i] < gFlashParam.st.Cur_Minimun)
            {
                gPowerQualityParam.Fundamental_I[i] = 0;
            }
        }
        if(detectState == 1)
        {
            LoopState++;          
        }    
        break;

    case 2:  //采样数据处理
        for(i = 0; i < 3; i++)
        {
            ftemp1 = fx[i + 3] * fx[i + 3];
            ftemp2 = gPowerQualityParam.Fundamental_I[i] * gPowerQualityParam.Fundamental_I[i];
            if(ftemp1 < ftemp2)
            {
                continue;
            }
            ftemp1 -= ftemp2;
            ftemp1 /= ftemp2;
            ftemp1 = sqrt(ftemp1);
            gHarmonicParam.Curr_1[i] = (int16_t)(ftemp1 * 10000);
            gFlashParam.st.DeltaKA0 = gHarmonicParam.Curr_1[0];
            gFlashParam.st.DeltaKB0 = gHarmonicParam.Curr_1[1];
            gFlashParam.st.DeltaKC0 = gHarmonicParam.Curr_1[2];
        }
        alarm_bike_process();
        
        if(detectState == 1)
        {
            LoopState++;          
        }
        break;

    default:  //等待下一次谐波采样计算
        if(LoopState >= 3)  //前提N>=3，可延时(N+1)*500ms
        {
            LoopState = 0;   
            detectState = 0;           
        }
        else
        {
            LoopState++;
        }
        break;
    }
}

/**
  * @brief  总谐波畸变率循环（500ms调用一次该函数）
            计算总谐波畸变率  THDu = sqrt[(Urms^2-Ufundamental^2)/Ufundamental^2]
  * @param  None
  * @retval None
  */
void Do_THDx_Loop(void)
{
    static uint8_t LoopState = 0;
    static float fX[6];  //基波数据滞后有效值数据约500ms，所以需保存
    uint8_t i;
//    uint32_t itemp;
    float ftemp1, ftemp2;

    switch(LoopState)
    {
    case 0:  //采样有效值
        for (i = 0; i < 3; i++)
        {
            ftemp1 = Read_Voltage(i);
            if(ftemp1 < gFlashParam.st.Vol_Minimun)
            {
                ftemp1 = 0;
            }
            fX[i] = ftemp1;

            ftemp1 = Read_Current(i);
            if(ftemp1 < gFlashParam.st.Cur_Minimun)
            {
                ftemp1 = 0;
            }
            fX[i + 3] = ftemp1;
        }
        LoopState++;
        break;

    case 1:  //采样基波有效值
        for(i = 0; i < 3; i++)
        {
            gPowerQualityParam.Fundamental_U[i] = Read_Voltage_FundamentalWave(i);
            if(gPowerQualityParam.Fundamental_U[i] < gFlashParam.st.Vol_Minimun)
            {
                gPowerQualityParam.Fundamental_U[i] = 0;
            }

            gPowerQualityParam.Fundamental_I[i] = Read_Current_FundamentalWave(i);
            if(gPowerQualityParam.Fundamental_I[i] < gFlashParam.st.Cur_Minimun)
            {
                gPowerQualityParam.Fundamental_I[i] = 0;
            }
        }
        LoopState++;
        break;

    case 2:  //采样数据处理
        for(i = 0; i < 3; i++)
        {
            ftemp1 = fX[i] * fX[i];
            ftemp2 = gPowerQualityParam.Fundamental_U[i] * gPowerQualityParam.Fundamental_U[i];
            if(ftemp1 < ftemp2)  //避免数值突然下降导致结果波动大
            {
                continue;
            }
            ftemp1 -= ftemp2;
            ftemp1 /= ftemp2;
            ftemp1 = sqrt(ftemp1);
            gHarmonicParam.Volt_1[i] = (int16_t)(ftemp1 * 10000);

            ftemp1 = fX[i + 3] * fX[i + 3];
            ftemp2 = gPowerQualityParam.Fundamental_I[i] * gPowerQualityParam.Fundamental_I[i];
            if(ftemp1 < ftemp2)
            {
                continue;
            }
            ftemp1 -= ftemp2;
            ftemp1 /= ftemp2;
            ftemp1 = sqrt(ftemp1);
            gHarmonicParam.Curr_1[i] = (int16_t)(ftemp1 * 10000);
        }
        alarm_THD_process();
        LoopState++;
        break;

    default:  //等待下一次谐波采样计算
        if(LoopState >= 3)  //前提N>=3，可延时(N+1)*500ms
        {
            LoopState = 0;
        }
        else
        {
            LoopState++;
        }
        break;
    }
}
#endif /* PROD_TYPE == PROD_ESE */

#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB */

void Read_MeterPara_LEAK(void)
{
    float ftemp;

    //读取剩余电流-----------------------------------------------------------
    ftemp = Read_Current_LEAK(0);
//        if(ftemp < gFlashParam.st.Cur_Minimun)
//        {
//            ftemp = 0;
//            gFlagMeterParam.FlagUI |= 0x10 << i;                   //无电流
//        }
    //gMeterParam.Cur = (uint16_t)(ftemp * 10);
    gMeterParam.Cur = (ftemp * 10);
    
    if(gMeterParam.Cur > gFlashParam.st.Cur_THUp)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Msk);
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_CUR_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm1], StateAlm1_CUR_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Msk);
        }
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_CUR_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_CUR_Msk);
        }
    }
}

/**
  * @brief  读取计量参数
  * @param  None
  * @retval None
  */
void Read_MeterPara(void)
{
    static uint8_t flashcnt = 0;
    uint8_t i, isNeg, idx;
    uint32_t itemp;
    float ftemp;

    gFlagMeterParam.SFlag = att7022_read_register(r_SFlag);    //读取EMU状态
    gFlagMeterParam.FlagUI = 0x00;                             //电压电流标志(正常)
    //读取A,B,C相电压------------------------------------------------------------------
    for (i = 0; i < 3; i++)
    {
        ftemp = Read_Voltage(i);
        if(ftemp < gFlashParam.st.Vol_Minimun)
        {
            ftemp = 0;
            gFlagMeterParam.FlagUI |= 0x01 << i;                   //无电压
        }
        gMeterParam.Volt[i] = ftemp;
    }
    //读取A,B,C相电流------------------------------------------------------------------
    for (i = 0; i < 3; i++)
    {
        ftemp = Read_Current(i);
        if(ftemp < gFlashParam.st.Cur_Minimun)
        {
            ftemp = 0;
            gFlagMeterParam.FlagUI |= 0x10 << i;                   //无电流
        }
        gMeterParam.Curr[i] = ftemp;
    }
    //读取A,B,C,合相有功功率------------------------------------------------------------
    for (i = 0; i < 4; i++)
    {
        if ((gFlagMeterParam.SFlag & TAB_Mask[i]) != TAB_Mask[i]) //启动
        {
            ftemp = Read_Power(i, 0, &isNeg);
            if ((ftemp < gFlashParam.st.Pow_Minimun && isNeg == 0) || (-ftemp < gFlashParam.st.Pow_Minimun && isNeg == 1))
            {
                ftemp = 0;
                isNeg = 0;
            }
        }
        else
        {
            ftemp = 0;
            isNeg = 0;
        }
        gMeterParam.PowP[i] = ftemp;

        if(isNeg)
        {
            gFlagMeterParam.PowQuad |=  (0x01 << i);
        }
        else
        {
            gFlagMeterParam.PowQuad &= ~(0x01 << i);
        }
    }
    //读取A,B,C,合相无功功率-------------------------------------------------------------
    for (i = 0; i < 4; i++)
    {
        if ((gFlagMeterParam.SFlag & TAB_Mask[i]) != TAB_Mask[i]) //启动
        {
            ftemp = Read_Power(i, 1, &isNeg);
            if ((ftemp < gFlashParam.st.Pow_Minimun && isNeg == 0) || (-ftemp < gFlashParam.st.Pow_Minimun && isNeg == 1))
            {
                ftemp = 0;
                isNeg = 0;
            }
        }
        else
        {
            ftemp = 0;
            isNeg = 0;
        }
        gMeterParam.PowQ[i] = ftemp;

        if(isNeg)
        {
            gFlagMeterParam.PowQuad |=  (0x10 << i);
        }
        else
        {
            gFlagMeterParam.PowQuad &= ~(0x10 << i);
        }
    }
    //读取A,B,C,合相视在功率------------------------------------------------------------
    for (i = 0; i < 4; i++)
    {
        if ((gFlagMeterParam.SFlag & TAB_Mask[i]) != TAB_Mask[i]) //启动
        {
            ftemp = Read_Power(i, 2, &isNeg);
            if (ftemp < gFlashParam.st.Pow_Minimun)
            {
                ftemp = 0;
                isNeg = 0;
            }
        }
        else
        {
            ftemp = 0;
            isNeg = 0;
        }
        gMeterParam.PowS[i] = ftemp;
    }
    //读取A,B,C,合相功率因数-------------------------------------------------------------
    for (i = 0; i < 4; i++)
    {
        if ((gFlagMeterParam.SFlag & TAB_Mask[i]) != TAB_Mask[i]) //启动
        {
            ftemp = Read_Power_Factor(i, &isNeg);
        }
        else
        {
            ftemp = 1.000;
        }
        gMeterParam.Pf[i] = ftemp;
    }
    //线频率（f=Freq/2^13,单位Hz）-------------------------------------------------------
    gMeterParam.Freq = Read_Voltage_Frequency();
    
    if(gMeterParam.Freq > gFlashParam.st.FREQ_Up*0.01 || gMeterParam.Freq < gFlashParam.st.FREQ_Down*0.01)
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_DUF_Msk);
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DUF_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm1], StateAlm1_DUF_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_DUF_Msk);
        }
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_DUF_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DUF_Msk);
        }
    }
    //有功电能----------------------------------------------------------------------------
    for(i = 0; i < 4; i++)
    {
        itemp = att7022_read_register(r_Epa + i);
        ftemp = (float)itemp / gFlashParam.st.EC;
        if(gFlagMeterParam.PowQuad & (0x01 << i)) //反向
        {
            gMeterEnergy.EPE[i] += ftemp;
        }
        else //正向
        {
            gMeterEnergy.EPI[i] += ftemp;
        }
    }
    //全波无功电能------------------------------------------------------------------------
    for(i = 0; i < 4; i++)
    {
        itemp = att7022_read_register(r_Eqa + i);
        ftemp = (float)itemp / gFlashParam.st.EC;
        if(gFlagMeterParam.PowQuad & (0x10 << i)) //反向
        {
            gMeterEnergy.EQC[i] += ftemp;
        }
        else //正向
        {
            gMeterEnergy.EQL[i] += ftemp;
        }
    }
    flashcnt++;
    if(flashcnt >= 100)
    {
        Parameter_FlashWrite_InOnePage(PAR_RTU_INFO_SAVE_ADDR, sizeof(gMeterEnergy), &gMeterEnergy, &gflashOffset.energy); //存入flash
        flashcnt = 0;
    }
    //电压夹角计算-----------------------------------------------------------------------
    for(i = 0; i < 3; i++)
    {
        gPowerQualityParam.Volt_Angle[i] = Read_Angle(i, 0, 0, &isNeg);
    }
    //电流夹角计算-----------------------------------------------------------------------
    for(i = 0; i < 3; i++)
    {
        gPowerQualityParam.Curr_Angle[i] = Read_Angle(i, 1, 0, &isNeg);
    }
    //电压夹角计算线电压------------------------------------------------------------------
    for(i = 0; i < 3; i++)
    {
        //Uab^2 = Ua^2 - 2*UaUbcos∠UaUb + Ub^2 相电压UaUb与线电压Uab的关系
        idx = i == 2 ? 0 : i + 1;
        ftemp = (float)gPowerQualityParam.Volt_Angle[i] - gPowerQualityParam.Volt_Angle[idx];
        ftemp /= 180.0;
        ftemp *= PI;  //计算弧度
        ftemp = cos(ftemp);
        ftemp *= gMeterParam.Volt[i];
        ftemp *= gMeterParam.Volt[idx];
        ftemp *= 2;
        ftemp = (gMeterParam.Volt[i] * gMeterParam.Volt[i]) - ftemp;
        ftemp += (gMeterParam.Volt[idx] * gMeterParam.Volt[idx]);
        ftemp = sqrt(ftemp);
        gMeterParam.VoltL[i] = ftemp;
    }
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB
    //电压、线电压、频率偏差计算 ΔY=(Y-Yn)/Yn,Y实际值,Yn电网标称值--------------------------
    for(i = 0; i < 3; i++)
    {
        ftemp = (float)gMeterParam.Volt[i] - C_100Un_220V;
        ftemp /= (float)C_100Un_220V;
        ftemp *= 100.0;
        gPowerQualityParam.Volt_Deviation[i] = ftemp;

        ftemp = (float)gMeterParam.VoltL[i] - C_100LUn_380V;
        ftemp /= (float)C_100LUn_380V;
        ftemp *= 100.0;
        gPowerQualityParam.VoltL_Deviation[i] = ftemp;
    }
    ftemp = (float)gMeterParam.Freq - C_100F_50Hz;
    ftemp /= (float)C_100F_50Hz;
    ftemp *= 100.0;
    gPowerQualityParam.Freq_Deviation = ftemp;

    //同步采样数据谐波处理（包括三相不平衡）--------------------------------------------------
    Do_Harmonic_Loop();

    //计算总谐波畸变率----------------------------------------------------------------------
#if PROD_TYPE == PROD_SFE
    Do_THDx_Loop();
    if(gFlashParam.st.Malignantloaden == 1)
    {
        Do_elecbike_detect();
    }
#endif /* PROD_TYPE == PROD_ESE */

#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB */
}

/**
  * @brief  清除电能参数，bit位为1时清零
  * @param  clr 清除标志位：
  *            @arg bit15: 合相反向无功电能
  *            @arg bit14: C相反向无功电能
  *            @arg bit13: B相反向无功电能
  *            @arg bit12: A相反向无功电能
  *            @arg bit11: 合相正向无功电能
  *            @arg bit10: C相正向无功电能
  *            @arg bit9: B相正向无功电能
  *            @arg bit8: A相正向无功电能
  *            @arg bit7: 合相反向有功电能
  *            @arg bit6: C相反向有功电能
  *            @arg bit5: B相反向有功电能
  *            @arg bit4: A相反向有功电能
  *            @arg bit3: 合相正向有功电能
  *            @arg bit2: C相正向有功电能
  *            @arg bit1: B相正向有功电能
  *            @arg bit0: A相正向有功电能
  * @retval None
  */
void meter_clear_energy(unsigned short clr)
{
    uint8_t i;

    for(i = 0; i < 4; i++)
    {
        if(clr & (0x0001 << i))
        {
            gMeterEnergy.EPI[i] = 0;
        }
        if(clr & (0x0010 << i))
        {
            gMeterEnergy.EPE[i] = 0;
        }
        if(clr & (0x0100 << i))
        {
            gMeterEnergy.EQL[i] = 0;
        }
        if(clr & (0x1000 << i))
        {
            gMeterEnergy.EQC[i] = 0;
        }
    }
    Parameter_FlashWrite_InOnePage(PAR_RTU_INFO_SAVE_ADDR, sizeof(gMeterEnergy), &gMeterEnergy, &gflashOffset.energy); //存入flash
}

/**
  * @brief  初始化计量校表参数
  * @param  None
  * @retval None
  */
void meter_cali_param_init(void)
{
    /*  Femu = 1.8432MHz */
    gFlashParam.st.meter_cali[offset_ModeCfg] = 0xB8FF;

    gFlashParam.st.meter_cali[offset_PGACtrl] = 0x0000;

    /*  视在功率/能量寄存器采用PQS方式计量
        三相四线使用代数和累加方式，能量寄存器读后清零
        开启基波/谐波计量功能 */
    gFlashParam.st.meter_cali[offset_EMUCfg] = 0x3CC4;

    gFlashParam.st.meter_cali[offset_ModuleCfg] = 0x3427;

    /*  外部引脚SEL=1时为三相四线制
        使用新的角度算法 选择基波测量 */
    gFlashParam.st.meter_cali[offset_EMCfg] = 0x0008;

    if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA1 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB1 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE1)
    {
        gFlashParam.st.meter_cali[offset_HFConst] = HFconst_EC6400_Ib5A;

        gFlashParam.st.meter_cali[offset_PgainA] = 0x0FDE;
        gFlashParam.st.meter_cali[offset_PgainB] = 0x0FAB;
        gFlashParam.st.meter_cali[offset_PgainC] = 0x0F6F;
        gFlashParam.st.meter_cali[offset_QgainA] = 0x0FDE;
        gFlashParam.st.meter_cali[offset_QgainB] = 0x0FAB;
        gFlashParam.st.meter_cali[offset_QgainC] = 0x0F6F;

        gFlashParam.st.meter_cali[offset_PhSregApq0] = 0xFF9B;
        gFlashParam.st.meter_cali[offset_PhSregBpq0] = 0xFF90;
        gFlashParam.st.meter_cali[offset_PhSregCpq0] = 0xFEB3;
        gFlashParam.st.meter_cali[offset_PhSregApq1] = 0xFF9B;
        gFlashParam.st.meter_cali[offset_PhSregBpq1] = 0xFF90;
        gFlashParam.st.meter_cali[offset_PhSregCpq1] = 0xFEB3;

        gFlashParam.st.meter_cali[offset_UgainA] = 0xDF7F;
        gFlashParam.st.meter_cali[offset_UgainB] = 0xDF7C;
        gFlashParam.st.meter_cali[offset_UgainC] = 0xDF2E;

        gFlashParam.st.meter_cali[offset_IgainA] = 0x0814;
        gFlashParam.st.meter_cali[offset_IgainB] = 0x07E8;
        gFlashParam.st.meter_cali[offset_IgainC] = 0x081E;

        gFlashParam.st.meter_cali[offset_Pstartup] = REG_Pstartup_Ib5A;
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA2 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB2 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE2)
    {
        gFlashParam.st.meter_cali[offset_HFConst] = HFconst_EC400_Ib100A;

        gFlashParam.st.meter_cali[offset_PgainA] = 0x103E;
        gFlashParam.st.meter_cali[offset_PgainB] = 0x1019;
        gFlashParam.st.meter_cali[offset_PgainC] = 0x0FAC;
        gFlashParam.st.meter_cali[offset_QgainA] = 0x103E;
        gFlashParam.st.meter_cali[offset_QgainB] = 0x1019;
        gFlashParam.st.meter_cali[offset_QgainC] = 0x0FAC;

        gFlashParam.st.meter_cali[offset_PhSregApq0] = 0x0086;
        gFlashParam.st.meter_cali[offset_PhSregBpq0] = 0x00B2;
        gFlashParam.st.meter_cali[offset_PhSregCpq0] = 0x0014;
        gFlashParam.st.meter_cali[offset_PhSregApq1] = 0x0086;
        gFlashParam.st.meter_cali[offset_PhSregBpq1] = 0x00B2;
        gFlashParam.st.meter_cali[offset_PhSregCpq1] = 0x0014;

        gFlashParam.st.meter_cali[offset_UgainA] = 0xDF74;
        gFlashParam.st.meter_cali[offset_UgainB] = 0xDF6D;
        gFlashParam.st.meter_cali[offset_UgainC] = 0xDF2B;

        gFlashParam.st.meter_cali[offset_IgainA] = 0x086A;
        gFlashParam.st.meter_cali[offset_IgainB] = 0x0851;
        gFlashParam.st.meter_cali[offset_IgainC] = 0x0848;

        gFlashParam.st.meter_cali[offset_Pstartup] = REG_Pstartup_Ib100A;
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA3 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB3 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE3)
    {
        gFlashParam.st.meter_cali[offset_HFConst] = HFconst_EC300_Ib200A;

        gFlashParam.st.meter_cali[offset_PgainA] = 0xDFD8;
        gFlashParam.st.meter_cali[offset_PgainB] = 0xE038;
        gFlashParam.st.meter_cali[offset_PgainC] = 0xDFA1;
        gFlashParam.st.meter_cali[offset_QgainA] = 0xDFD8;
        gFlashParam.st.meter_cali[offset_QgainB] = 0xE038;
        gFlashParam.st.meter_cali[offset_QgainC] = 0xDFA1;

        gFlashParam.st.meter_cali[offset_PhSregApq0] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregBpq0] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregCpq0] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregApq1] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregBpq1] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregCpq1] = 0x01FE;

        gFlashParam.st.meter_cali[offset_UgainA] = 0xDF82;
        gFlashParam.st.meter_cali[offset_UgainB] = 0xDFB8;
        gFlashParam.st.meter_cali[offset_UgainC] = 0xDF63;

        gFlashParam.st.meter_cali[offset_IgainA] = 0x0808;
        gFlashParam.st.meter_cali[offset_IgainB] = 0x0836;
        gFlashParam.st.meter_cali[offset_IgainC] = 0x07D8;

        gFlashParam.st.meter_cali[offset_Pstartup] = REG_Pstartup_Ib200A;
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA4 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB4 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE4)
    {
        gFlashParam.st.meter_cali[offset_HFConst] = HFconst_EC100_Ib400A;

        gFlashParam.st.meter_cali[offset_PgainA] = 0x0FA0;
        gFlashParam.st.meter_cali[offset_PgainB] = 0x0F56;
        gFlashParam.st.meter_cali[offset_PgainC] = 0x0F5E;
        gFlashParam.st.meter_cali[offset_QgainA] = 0x0FA0;
        gFlashParam.st.meter_cali[offset_QgainB] = 0x0F56;
        gFlashParam.st.meter_cali[offset_QgainC] = 0x0F5E;

        gFlashParam.st.meter_cali[offset_PhSregApq0] = 0x023B;
        gFlashParam.st.meter_cali[offset_PhSregBpq0] = 0x0259;
        gFlashParam.st.meter_cali[offset_PhSregCpq0] = 0x0233;
        gFlashParam.st.meter_cali[offset_PhSregApq1] = 0x023B;
        gFlashParam.st.meter_cali[offset_PhSregBpq1] = 0x0259;
        gFlashParam.st.meter_cali[offset_PhSregCpq1] = 0x0233;

        gFlashParam.st.meter_cali[offset_UgainA] = 0xDF43;
        gFlashParam.st.meter_cali[offset_UgainB] = 0xDF31;
        gFlashParam.st.meter_cali[offset_UgainC] = 0xDF25;

        gFlashParam.st.meter_cali[offset_IgainA] = 0x0812;
        gFlashParam.st.meter_cali[offset_IgainB] = 0x07D9;
        gFlashParam.st.meter_cali[offset_IgainC] = 0x0809;

        gFlashParam.st.meter_cali[offset_Pstartup] = REG_Pstartup_Ib400A;
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA5 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB5 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE5)
    {
        gFlashParam.st.meter_cali[offset_HFConst] = HFconst_EC60_Ib600A;

        gFlashParam.st.meter_cali[offset_PgainA] = 0x119F;
        gFlashParam.st.meter_cali[offset_PgainB] = 0x0FD2;
        gFlashParam.st.meter_cali[offset_PgainC] = 0x1079;
        gFlashParam.st.meter_cali[offset_QgainA] = 0x119F;
        gFlashParam.st.meter_cali[offset_QgainB] = 0x0FD2;
        gFlashParam.st.meter_cali[offset_QgainC] = 0x1079;

        gFlashParam.st.meter_cali[offset_PhSregApq0] = 0x0206;
        gFlashParam.st.meter_cali[offset_PhSregBpq0] = 0x018E;
        gFlashParam.st.meter_cali[offset_PhSregCpq0] = 0x0118;
        gFlashParam.st.meter_cali[offset_PhSregApq1] = 0x0206;
        gFlashParam.st.meter_cali[offset_PhSregBpq1] = 0x018E;
        gFlashParam.st.meter_cali[offset_PhSregCpq1] = 0x0118;

        gFlashParam.st.meter_cali[offset_UgainA] = 0xDF79;
        gFlashParam.st.meter_cali[offset_UgainB] = 0xDF6F;
        gFlashParam.st.meter_cali[offset_UgainC] = 0xDF2C;

        gFlashParam.st.meter_cali[offset_IgainA] = 0x09B6;
        gFlashParam.st.meter_cali[offset_IgainB] = 0x0810;
        gFlashParam.st.meter_cali[offset_IgainC] = 0x090E;

        gFlashParam.st.meter_cali[offset_Pstartup] = REG_Pstartup_Ib600A;
    }
    else
    {
        gFlashParam.st.meter_cali[offset_HFConst] = HFconst_EC40_Ib1000A;

        gFlashParam.st.meter_cali[offset_PgainA] = 0x0E38;
        gFlashParam.st.meter_cali[offset_PgainB] = 0x1074;
        gFlashParam.st.meter_cali[offset_PgainC] = 0x0DCB;
        gFlashParam.st.meter_cali[offset_QgainA] = 0x0E38;
        gFlashParam.st.meter_cali[offset_QgainB] = 0x1074;
        gFlashParam.st.meter_cali[offset_QgainC] = 0x0DCB;

        gFlashParam.st.meter_cali[offset_PhSregApq0] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregBpq0] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregCpq0] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregApq1] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregBpq1] = 0x01FE;
        gFlashParam.st.meter_cali[offset_PhSregCpq1] = 0x01FE;

        gFlashParam.st.meter_cali[offset_UgainA] = 0xDF68;
        gFlashParam.st.meter_cali[offset_UgainB] = 0xDF43;
        gFlashParam.st.meter_cali[offset_UgainC] = 0xDF21;

        gFlashParam.st.meter_cali[offset_IgainA] = 0x06BC;
        gFlashParam.st.meter_cali[offset_IgainB] = 0x08E5;
        gFlashParam.st.meter_cali[offset_IgainC] = 0x06BC;

        gFlashParam.st.meter_cali[offset_Pstartup] = REG_Pstartup_Ib1000A;
    }


}

/**
  * @brief  初始化计量任务
  * @param  None
  * @retval None
  */
void Init_TaskMetering(void)
{
    gMeterRunInfo.EmuWork = EMU_INIT;
    gMeterRunInfo.CheckSum0 += 0x55555555;
    gMeterRunInfo.CheckSum1 += 0x55555555;

    gFlagMeterParam.SFlag = 0x0E00;
    gFlagMeterParam.FlagUI = 0x00;
    gFlagMeterParam.PowQuad = 0x00;

}

/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_att7022euTask(void)
{
    gAtt7022Mutex = xSemaphoreCreateMutex();
    att7022TaskHandle = osThreadNew(Att7022Task, NULL, &att7022Task_attributes);
}

/**
  * @brief  Function implementing the uartTask thread.
  * @param  argument: Not used
  * @retval None
  */
void Att7022Task(void *argument)
{
    LOGD(TAG, "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());
    uint8_t i;
    float fErr[3];

    Init_TaskMetering();

    att7022_write_register(w_SoftRst, 0x000000); //软复位
    vTaskDelay(50);
    if(!att7022_spi_is_ok())
    {
        LOGE(TAG, "Delete ATT7022Task");
        PAR_SET_BIT(gParam.st.State_SystemErr, SysErr0_Meter_Msk);
        vTaskDelete(NULL); //删除任务
    }
    PAR_CLEAR_BIT(gParam.st.State_SystemErr, SysErr0_Meter_Msk);

    for(;;)
    {
        switch(gMeterRunInfo.EmuWork)
        {
        case EMU_INIT:
            Repair_Metering();
            break;

        case EMU_RUNNING:
            Read_MeterPara();
            Read_MeterPara_LEAK();
            break;


        case EMU_CONFIG_ISTARTUP:
            Config_PStartup(gParam.st.fConfig_Pstartup);
            break;


        case EMU_CALI_OFFSET_CUR:
            Cali_Offset_Current();
            break;

        case EMU_CALI_N_CUR:
            Cali_N_Current(gParam.st.fCali_Cur);
            break;

        case EMU_CALI_GAIN_CUR:
            Cali_Gain_Current(gParam.st.fCali_Cur);
            break;

        case EMU_CALI_GAIN_CUR_USE_ERR:
            for(i = 0; i < 3; i++)
            {
                fErr[i] = gParam.st.fClai_Err[i];
            }
            Cali_Gain_Current_Use_Err(fErr);
            break;


        case EMU_CALI_OFFSET_VOL:
            Cali_Offset_Voltage();
            break;

        case EMU_CALI_GAIN_VOL:
            Cali_Gain_Voltage(gParam.st.fCali_Vol);
            break;

        case EMU_CALI_GAIN_VOL_USE_ERR:
            for(i = 0; i < 3; i++)
            {
                fErr[i] = gParam.st.fClai_Err[i];
            }
            Cali_Gain_Voltage_Use_Err(fErr);
            break;


        case EMU_CALI_GAIN_POW:
            Cali_Gain_Power(gParam.st.fCali_Pow);
            break;

        case EMU_CALI_GAIN_POW_USE_ERR:
            for(i = 0; i < 3; i++)
            {
                fErr[i] = gParam.st.fClai_Err[i];
            }
            Cali_Gain_Power_Use_Err(fErr);
            break;

        case EMU_CALI_GAIN_POW_USE_FORMULA:
            Cali_Gain_Power_Use_Formula(gFlashParam.st.meter_cali[offset_HFConst], gFlashParam.st.EC, gFlashParam.st.N_Ib_Vi);
            break;


        case EMU_CALI_GAIN_PHASE:
            Cali_Gain_Power_Factor(gParam.st.fCali_PF);
            break;

        case EMU_CALI_GAIN_PHASE_USE_ERR:
            for(i = 0; i < 3; i++)
            {
                fErr[i] = gParam.st.fClai_Err[i];
            }
            Cali_Gain_Power_Factor_Use_Err(fErr);
            break;

        case EMU_CALI_U_I_P_PF:
            Cali_I_U_P_PF_T0(gParam.st.fCali_Cur, gParam.st.fCali_Vol, gParam.st.fCali_PF);
            break;

        default:
            gMeterRunInfo.EmuWork = EMU_INIT;
            break;
        }

        osDelay(100);
    }
}
#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB || PROD_TYPE == PROD_ESA */


#endif /* PROD_TYPE == PROD_ESM || PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESC || PROD_TYPE == PROD_ESB || PROD_TYPE == PROD_ESA */
