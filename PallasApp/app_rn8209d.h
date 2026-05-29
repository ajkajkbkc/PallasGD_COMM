///*
// * Copyright (c) 2006-2018, Fexlink Development Team
// *
// * SPDX-License-Identifier: Apache-2.0
// *
// * Change Logs:
// * Date           Author       Notes
// * 2022-07-02     Arrbow       first implementation
// */

//#ifndef __PALLAS_ES_RN8209D_H
//#define __PALLAS_ES_RN8209D_H

///* Includes ------------------------------------------------------------------*/
//#include "app_opts.h"

//#if  PROD_TYPE == PROD_FSS
//#include <stdbool.h>
//#include "main.h"


///* Private defines -----------------------------------------------------------*/

///***************************** read or write *****************************/
//#define RN8209D_READ_REG    0x00
//#define RN8209D_WRITE_REG   0x80

///*********************** 校表参数和计量控制寄存器 ************************/
//#define rw_SYSCON           0x00
//#define rw_EMUCON           0x01
//#define rw_HFConst          0x02
//#define rw_PStart           0x03
//#define rw_QStart           0x04
//#define rw_GPQA             0x05
//#define rw_GPQB             0x06
//#define rw_PhsA             0x07
//#define rw_PhsB             0x08
//#define rw_QPhsCal          0x09
//#define rw_APOSA            0x0A
//#define rw_APOSB            0x0B
//#define rw_RPOSA            0x0C
//#define rw_RPOSB            0x0D
//#define rw_IARMSOS          0x0E
//#define rw_IBRMSOS          0x0F
//#define rw_IBGain           0x10

////#define rw_D2FPL           0x11
////#define rw_D2FPH           0x12
////#define rw_DCIAH           0x13
////#define rw_DCIBH           0x14
////#define rw_DCUH            0x15
////#define rw_DCL             0x16
////#define rw_EMUCON2         0x17

///************************* 计量参数和状态寄存器 **************************/
//#define rw_PFCnt            0x20
//#define rw_QFCnt            0x21
//#define r_IARMS             0x22
//#define r_IBRMS             0x23
//#define r_URMS              0x24
//#define r_UFreq             0x25
//#define r_PowerPA           0x26
//#define r_PowerPB           0x27
//#define r_PowerQ            0x28
//#define r_EnergyP           0x29
//#define r_EnergyP2          0x2A
//#define r_EnergyQ           0x2B
//#define r_EnergyQ2          0x2C
//#define r_EMUStatus         0x2D

////#define r_SPL_IA          0x30
////#define r_SPL_IB          0x31
////#define r_SPL_U           0x32
////#define r_UFREQ2          0x35

///****************************** 中断寄存器 *******************************/
//#define rw_IE               0x40
//#define r_IF                0x41
//#define r_RIF               0x42

///*************************** 系统状态寄存器 ******************************/
//#define r_SysStatus         0x43
//#define r_LastRData         0x44
//#define r_LastWData         0x45

//#define r_DeciveID          0x7F

//#define w_Cmd               0xEA    //命令寄存器

////向命令寄存器写入值
//#define w_Cmd_Reset         0xFA
//#define w_Cmd_Write_Enable  0xE5
//#define w_Cmd_Write_Protect 0xDC
//#define w_Cmd_PathA         0x5A
//#define w_Cmd_PathB         0xA5


//typedef enum
//{
//    rn_offset_SYSCON  = 0,
//    rn_offset_EMUCON  = 1,
//    rn_offset_HFConst = 2,
//    rn_offset_PStart  = 3,
//    rn_offset_QStart  = 4,
//    rn_offset_GPQA    = 5,
//    rn_offset_GPQB    = 6,
//    rn_offset_PhsA    = 7,
//    rn_offset_PhsB    = 8,
//    rn_offset_QPhsCal = 9,
//    rn_offset_APOSA   = 10,
//    rn_offset_APOSB   = 11,
//    rn_offset_RPOSA   = 12,
//    rn_offset_RPOSB   = 13,
//    rn_offset_IARMSOS = 14,
//    rn_offset_IBRMSOS = 15,
//    rn_offset_IBGain  = 16,

//} rn_cali_offset_e;



////EMU工作状态定义
//typedef enum
//{
//    rn_EMU_INIT              = 0,   //初始化
//    rn_EMU_RUNNING           = 1,   //运行中

//    rn_EMU_CALI_OFFSET_CUR   = 10,  //校准电流偏移
//    rn_EMU_CALI_N_CUR        = 11,  //校准电流比例系数N
//    rn_EMU_CALI_GAIN_CUR     = 12,  //校准电流增益

//    rn_EMU_CALI_OFFSET_VOL   = 20,  //校准电压偏移
//    rn_EMU_CALI_GAIN_VOL     = 21,  //校准电压增益

//} rn_emu_work_e;




///* Exported types ------------------------------------------------------------*/
//typedef struct
//{
//    unsigned int CheckSum0;    //计量芯片校验和
//    unsigned char EmuWork;     //工作状态

//} rn_meter_run_info_t;




///* Exported constants --------------------------------------------------------*/
//extern rn_meter_run_info_t rn_gMeterRunInfo;



///* Private functions ---------------------------------------------------------*/
//extern void osThreadNew_rn8209dTask(void);

//extern unsigned int RN8209D_Get_MeterPara(unsigned char addr);


//#endif /* PROD_TYPE == PROD_FSS */

//#endif /* __PALLAS_ES_RN8209D_H */
