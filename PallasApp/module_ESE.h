/*
 * Copyright (c) 2006-2018, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-28     Arrbow       first implementation
 */

#ifndef __MODULE_ESE_H
#define __MODULE_ESE_H

/* Includes ------------------------------------------------------------------*/
#include "app_opts.h"


#include "main.h"


/* Private defines -----------------------------------------------------------*/
#define HW_VERSION                (200)


#define EMU_FREQ                  (1)     //0: Femu=921.6kHz  1: Femu=1.8432MHz

/**
  @brief 数据大小
  */


/**
  @brief 标志位定义
  */
#define StateAlm0                 0       //gFlashParam.st.State_Alarm[0]
#define StateAlm0_DI_Msk          0x0001  //四路输入异常
#define StateAlm0_DO_Msk          0x0002  //两路输出异常
#define StateAlm0_TMP_Msk         0x0004  //四路温度异常
#define StateAlm0_CUR_Msk         0x0008  //剩余电流异常
#define StateAlm0_VOL_Up_Msk      0x0010  //三相过压
#define StateAlm0_VOL_Down_Msk    0x0020  //三相欠压
#define StateAlm0_CUR_Up_Msk      0x0040  //三相过流
#define StateAlm0_CUR_Down_Msk    0x0080  //三相欠流
#define StateAlm0_POW_Up_Msk      0x0100  //三相过功率异常
#define StateAlm0_POW_Down_Msk    0x0200  //三相欠功率异常
#define StateAlm0_DUF_Msk         0x0400  //频率偏差异常
#define StateAlm0_VOL_IMB_Msk     0x0800  //电压不平衡(零序或负序不平衡度超限)
#define StateAlm0_CUR_IMB_Msk     0x1000  //电流不平衡
#define StateAlm0_VOL_THD_Msk     0x2000  //电压谐波(总、奇次或偶次谐波超限)
#define StateAlm0_CUR_THD_Msk     0x4000  //电流谐波
#define StateAlm0_LOAD_Msk        0x8000  //恶性负载异常

#define StateAlm0_DIDO_Msk        (StateAlm0_DI_Msk | StateAlm0_DO_Msk)
#define StateAlm0_VOL_CUR_Msk     (StateAlm0_VOL_Up_Msk | StateAlm0_VOL_Down_Msk | StateAlm0_CUR_Up_Msk | StateAlm0_CUR_Down_Msk | StateAlm0_CUR_Msk)
#define StateAlm0_POW_Msk         (StateAlm0_POW_Up_Msk | StateAlm0_POW_Down_Msk)
#define StateAlm0_IMB_Msk         (StateAlm0_VOL_IMB_Msk | StateAlm0_CUR_IMB_Msk)
#define StateAlm0_THD_Msk         (StateAlm0_VOL_THD_Msk | StateAlm0_CUR_THD_Msk)
#define StateAlm0_IMB_THD_Msk     (StateAlm0_IMB_Msk | StateAlm0_THD_Msk)
#if PROD_TYPE == PROD_SFA
#define StateAlm0_ALL_Msk         (StateAlm0_DIDO_Msk | StateAlm0_VOL_CUR_Msk | StateAlm0_POW_Msk | StateAlm0_TMP_Msk | StateAlm0_DUF_Msk)
#elif PROD_TYPE == PROD_SFB
#define StateAlm0_ALL_Msk         (StateAlm0_DIDO_Msk | StateAlm0_VOL_CUR_Msk | StateAlm0_POW_Msk | StateAlm0_IMB_Msk | StateAlm0_TMP_Msk | StateAlm0_DUF_Msk)
#else
#define StateAlm0_ALL_Msk         (StateAlm0_DIDO_Msk | StateAlm0_VOL_CUR_Msk | StateAlm0_POW_Msk | StateAlm0_IMB_THD_Msk | StateAlm0_TMP_Msk | StateAlm0_DUF_Msk | StateAlm0_LOAD_Msk)
#endif

#define StateAlm1                 1       //gFlashParam.st.State_Alarm[1]
#define StateAlm1_VOL1_Up_Msk     0x0001  //A相过压
#define StateAlm1_VOL2_Up_Msk     0x0002  //B相过压
#define StateAlm1_VOL3_Up_Msk     0x0004  //C相过压
#define StateAlm1_VOL1_Down_Msk   0x0008  //A相欠压
#define StateAlm1_VOL2_Down_Msk   0x0010  //B相欠压
#define StateAlm1_VOL3_Down_Msk   0x0020  //C相欠压
#define StateAlm1_CUR1_Up_Msk     0x0040  //A相过流
#define StateAlm1_CUR2_Up_Msk     0x0080  //B相过流
#define StateAlm1_CUR3_Up_Msk     0x0100  //C相过流
#define StateAlm1_CUR1_Down_Msk   0x0200  //A相欠流
#define StateAlm1_CUR2_Down_Msk   0x0400  //B相欠流
#define StateAlm1_CUR3_Down_Msk   0x0800  //C相欠流
#define StateAlm1_CUR_Msk         0x1000  //剩余电流
#define StateAlm1_DUF_Msk         0x2000  //频率偏差异常

#define StateAlm1_VOL_Up_Msk      (StateAlm1_VOL1_Up_Msk | StateAlm1_VOL2_Up_Msk | StateAlm1_VOL3_Up_Msk)
#define StateAlm1_VOL_Down_Msk    (StateAlm1_VOL1_Down_Msk | StateAlm1_VOL2_Down_Msk | StateAlm1_VOL3_Down_Msk)
#define StateAlm1_CUR_Up_Msk      (StateAlm1_CUR1_Up_Msk | StateAlm1_CUR2_Up_Msk | StateAlm1_CUR3_Up_Msk)
#define StateAlm1_CUR_Down_Msk    (StateAlm1_CUR1_Down_Msk | StateAlm1_CUR2_Down_Msk | StateAlm1_CUR3_Down_Msk)
#define StateAlm1_VOL1_Msk        (StateAlm1_VOL_Up_Msk | StateAlm1_VOL_Down_Msk)
#define StateAlm1_CUR1_Msk        (StateAlm1_CUR_Up_Msk | StateAlm1_CUR_Down_Msk)
#define StateAlm1_ALL_Msk         (StateAlm1_VOL1_Msk | StateAlm1_CUR1_Msk | StateAlm1_CUR_Msk | StateAlm1_DUF_Msk)

#define StateAlm2                 2       //gFlashParam.st.State_Alarm[2]
#define StateAlm2_VOL_IMB_Msk     0x0001  //电压不平衡(零序或负序不平衡度超限)
#define StateAlm2_CUR_IMB_Msk     0x0002  //电流不平衡
#define StateAlm2_VOL_THD_Msk     0x0004  //电压谐波(总谐波超限)
#define StateAlm2_CUR_THD_Msk     0x0008  //电流谐波(总谐波超限)
#if PROD_TYPE == PROD_SFE
#define StateAlm2_VOLA_THD_Msk    0x0010  //A电压总谐波超限
#define StateAlm2_VOLB_THD_Msk    0x0020  //B电压总谐波超限
#define StateAlm2_VOLC_THD_Msk    0x0040  //C电压总谐波超限
#define StateAlm2_VOLAx_THD_Msk   0x0080  //A电压分次谐波(奇次或偶次谐波超限)
#define StateAlm2_VOLBx_THD_Msk   0x0100  //B电压分次谐波(奇次或偶次谐波超限)
#define StateAlm2_VOLCx_THD_Msk   0x0200  //C电压分次谐波(奇次或偶次谐波超限)
#define StateAlm2_CURA_THD_Msk    0x0400  //A电流总谐波超限
#define StateAlm2_CURB_THD_Msk    0x0800  //B电流总谐波超限
#define StateAlm2_CURC_THD_Msk    0x1000  //C电流总谐波超限
#define StateAlm2_CURAx_THD_Msk   0x2000  //A电流分次谐波(奇次或偶次谐波超限)
#define StateAlm2_CURBx_THD_Msk   0x4000  //B电流分次谐波(奇次或偶次谐波超限)
#define StateAlm2_CURCx_THD_Msk   0x8000  //C电流分次谐波(奇次或偶次谐波超限)
#elif PROD_TYPE == PROD_SFA || PROD_TYPE == PROD_SFB
#define StateAlm2_POW1_Up_Msk     0x0010  //A相过功率
#define StateAlm2_POW2_Up_Msk     0x0020  //B相过功率
#define StateAlm2_POW3_Up_Msk     0x0040  //C相过功率
#define StateAlm2_POW1_Down_Msk   0x0080  //A相欠功率
#define StateAlm2_POW2_Down_Msk   0x0100  //B相欠功率
#define StateAlm2_POW3_Down_Msk   0x0200  //C相欠功率
#endif

#if PROD_TYPE == PROD_SFA || PROD_TYPE == PROD_SFB
#define StateAlm2_POW_Up_Msk      (StateAlm2_POW1_Up_Msk | StateAlm2_POW2_Up_Msk | StateAlm2_POW3_Up_Msk)
#define StateAlm2_POW_Down_Msk    (StateAlm2_POW1_Down_Msk | StateAlm2_POW2_Down_Msk | StateAlm2_POW3_Down_Msk)
#endif
#define StateAlm2_IMB_Msk         (StateAlm2_VOL_IMB_Msk | StateAlm2_CUR_IMB_Msk)
#define StateAlm2_THD_Msk         (StateAlm2_VOL_THD_Msk | StateAlm2_CUR_THD_Msk)
#define StateAlm2_VOLTHD_Msk      (StateAlm2_VOLA_THD_Msk | StateAlm2_VOLB_THD_Msk | StateAlm2_VOLC_THD_Msk | StateAlm2_VOLAx_THD_Msk | StateAlm2_VOLBx_THD_Msk | StateAlm2_VOLCx_THD_Msk)
#define StateAlm2_CURTHD_Msk      (StateAlm2_CURA_THD_Msk | StateAlm2_CURB_THD_Msk | StateAlm2_CURC_THD_Msk | StateAlm2_CURAx_THD_Msk | StateAlm2_CURBx_THD_Msk | StateAlm2_CURCx_THD_Msk)
#define StateAlm2_IMB_THD_Msk     (StateAlm2_VOLTHD_Msk | StateAlm2_CURTHD_Msk)
#if PROD_TYPE == PROD_SFA
#define StateAlm2_ALL_Msk         (StateAlm2_POW_Up_Msk | StateAlm2_POW_Down_Msk)
#elif PROD_TYPE == PROD_SFB
#define StateAlm2_ALL_Msk         (StateAlm2_POW_Up_Msk | StateAlm2_POW_Down_Msk | StateAlm2_IMB_Msk)
#else
#define StateAlm2_ALL_Msk         (StateAlm2_IMB_Msk | StateAlm2_THD_Msk | StateAlm2_IMB_THD_Msk)
#endif

#define StateAlm3                 3       //gFlashParam.st.State_Alarm[3]
#define StateAlm3_DI1_Msk         0x0001  //DI1异常
#define StateAlm3_DI2_Msk         0x0002  //DI2异常
#define StateAlm3_DI3_Msk         0x0004  //DI3异常
#define StateAlm3_DI4_Msk         0x0008  //DI4异常
#define StateAlm3_DO1_Msk         0x0010  //DO1异常
#define StateAlm3_DO2_Msk         0x0020  //DO2异常
#define StateAlm3_TMP1_Msk        0x0040  //温度1异常
#define StateAlm3_TMP2_Msk        0x0080  //温度2异常
#define StateAlm3_TMP3_Msk        0x0100  //温度3异常
#define StateAlm3_TMP4_Msk        0x0200  //温度4异常

#define StateAlm3_DI_Msk          (StateAlm3_DI1_Msk | StateAlm3_DI2_Msk | StateAlm3_DI3_Msk | StateAlm3_DI4_Msk)
#define StateAlm3_DO_Msk          (StateAlm3_DO1_Msk | StateAlm3_DO2_Msk)
#define StateAlm3_TMP_Msk         (StateAlm3_TMP1_Msk | StateAlm3_TMP2_Msk | StateAlm3_TMP3_Msk | StateAlm3_TMP4_Msk)
#define StateAlm3_ALL_Msk         (StateAlm3_DI_Msk | StateAlm3_DO_Msk | StateAlm3_TMP_Msk)


#define StateAlm4                 4       //gFlashParam.st.State_Alarm[4]
#define StateAlm4_THDUaSC_Msk   	0x0001  //A相短路报警
#define StateAlm4_THDUaEA_Msk   	0x0002  //A相电弧报警
#define StateAlm4_THDUaVC_Msk   	0x0004  //A相虚接打火报警

#define StateAlm4_THDUbSC_Msk   	0x0008  //B相短路报警
#define StateAlm4_THDUbEA_Msk  	 	0x0010  //B相电弧报警
#define StateAlm4_THDUbVC_Msk   	0x0020  //B相虚接打火报警

#define StateAlm4_THDUcSC_Msk   	0x0040  //C相短路报警
#define StateAlm4_THDUcEA_Msk   	0x0080  //C相电弧报警
#define StateAlm4_THDUcVC_Msk   	0x0100  //C相虚接打火报警

#define StateAlm4_SC_MSK         (StateAlm4_THDUaSC_Msk | StateAlm4_THDUbSC_Msk | StateAlm4_THDUcSC_Msk)
#define StateAlm4_EA_MSK         (StateAlm4_THDUaEA_Msk | StateAlm4_THDUbEA_Msk | StateAlm4_THDUcEA_Msk)
#define StateAlm4_VC_MSK         (StateAlm4_THDUaVC_Msk | StateAlm4_THDUbVC_Msk | StateAlm4_THDUcVC_Msk)
/* ---------------------------------------------------------------------
    N电流比例系数:
    N = 1200*Vi/Ib

    Vi=0.33V, Ib=5A
    N = 1200*0.33/5 = 79.2

    Vi=0.33V, Ib=100A
    N = 1200*0.33/100 = 3.96

    Vi=0.33V, Ib=400A
    N = 1200*0.33/400 = 0.99

    Vi=0.33V, Ib=600A
    N = 1200*0.33/600 = 0.66
    
    Vi=0.33V, Ib=200A
    N = 1200*0.33/200 = 1.98
    
    Vi=0.33V, Ib=1000A
    N = 1200*0.33/1000 = 0.396    
   --------------------------------------------------------------------- */
#define N_Ib5A_Vi330mV             (79.2)
#define N_Ib100A_Vi330mV           (3.96)
#define N_Ib400A_Vi330mV           (0.99)
#define N_Ib600A_Vi330mV           (0.66)
#define N_Ib200A_Vi330mV           (1.98)
#define N_Ib1000A_Vi330mV          (0.396)
/* ---------------------------------------------------------------------
    EC电表脉冲常数:

    Ib=5A:
    EC = 6400imp/kWh

    Ib=100A:
    EC = 400imp/kWh

    Ib=400A:
    EC = 100imp/kWh

    Ib=600A:
    EC = 60imp/kWh
   --------------------------------------------------------------------- */
#define EC_Ib5A                    (6400)
#define EC_Ib100A                  (400)
#define EC_Ib400A                  (100)
#define EC_Ib600A                  (60)
#define EC_Ib200A                  (300)
#define EC_Ib1000A                 (40)
/* ---------------------------------------------------------------------
    HFconst高频脉冲常数:
    G=1.163, Vu=0.264V, Vi=0.33V, Un=220V
    Femu = 921.6kHz:  HFconst = INT[2.592*10^10*G*G*Vu*Vi/(EC*Un*Ib)]
                              = INT[2.592*10^10*1.163*1.163*Vu*Vi/(EC*Un*Ib)]
                              = INT[35058588480*Vu*Vi/(EC*Un*Ib)]

    Femu = 1.8432MHz: HFconst = INT[5.184*10^10*G*G*Vu*Vi/(EC*Un*Ib)]
                              = INT[5.184*10^10*1.163*1.163*Vu*Vi/(EC*Un*Ib)]
                              = INT[70117176960*Vu*Vi/(EC*Un*Ib)]

    EC=6400imp/kWh, Ib=5A:
    Femu = 921.6kHz:  HFconst = INT[35058588480*0.264*0.33/(6400*220*5)]  = INT[433.85] = 0x01B1
    Femu = 1.8432MHz: HFconst = INT[70117176960*0.264*0.33/(6400*220*5)]  = INT[867.70] = 0x0363

    EC=400imp/kWh,  Ib=100A:
    Femu = 921.6kHz:  HFconst = INT[35058588480*0.264*0.33/(400*220*100)] = INT[347.08] = 0x015B
    Femu = 1.8432MHz: HFconst = INT[70117176960*0.264*0.33/(400*220*100)] = INT[694.16] = 0x02B6

    EC=100imp/kWh,  Ib=400A:
    Femu = 921.6kHz:  HFconst = INT[35058588480*0.264*0.33/(100*220*400)] = INT[347.08] = 0x015B
    Femu = 1.8432MHz: HFconst = INT[70117176960*0.264*0.33/(100*220*400)] = INT[694.16] = 0x02B6

    EC=60imp/kWh,   Ib=600A:
    Femu = 921.6kHz:  HFconst = INT[35058588480*0.264*0.33/(60*220*600)]  = INT[385.64] = 0x0181
    Femu = 1.8432MHz: HFconst = INT[70117176960*0.264*0.33/(60*220*600)]  = INT[771.29] = 0x0303
    
    EC=300imp/kWh,   Ib=200A:
    Femu = 921.6kHz:  HFconst = INT[35058588480*0.264*0.33/(300*220*200)]  = INT[231.38] = 0x00E7
    Femu = 1.8432MHz: HFconst = INT[70117176960*0.264*0.33/(300*220*200)]  = INT[462.77] = 0x01CE
    
    EC=40imp/kWh,  Ib=1000A:
    Femu = 921.6kHz:  HFconst = INT[35058588480*0.264*0.33/(40*220*1000)] = INT[347.08] = 0x015B
    Femu = 1.8432MHz: HFconst = INT[70117176960*0.264*0.33/(40*220*1000)] = INT[694.16] = 0x02B6
   --------------------------------------------------------------------- */
#if EMU_FREQ == 0
#define HFconst_EC6400_Ib5A       (0x01B1)
#define HFconst_EC400_Ib100A      (0x015B)
#define HFconst_EC100_Ib400A      (0x015B)
#define HFconst_EC60_Ib600A       (0x0181)
#define HFconst_EC300_Ib200A      (0x00E7)
#else
#define HFconst_EC6400_Ib5A       (0x0363)
#define HFconst_EC400_Ib100A      (0x02B6)
#define HFconst_EC100_Ib400A      (0x02B6)
#define HFconst_EC60_Ib600A       (0x0303)
#define HFconst_EC300_Ib200A      (0x01CE)
#define HFconst_EC40_Ib1000A      (0x02B6)
#endif

/* ---------------------------------------------------------------------
    K功率参数系数:
    Femu = 921.6kHz:  K = 2.592*10^10/(HFconst*EC*2^23)
    Femu = 1.8432MHz: K = 5.184*10^10/(HFconst*EC*2^23)

    EC=6400imp/kWh, HFconst=433:
    Femu = 921.6kHz:  K = 2.592*10^10/(433*6400*2^23) = 0.00112
    Femu = 1.8432MHz: K = 5.184*10^10/(867*6400*2^23) = 0.00111

    EC=400imp/kWh,  HFconst=347:
    Femu = 921.6kHz:  K = 2.592*10^10/(347*400*2^23)  = 0.02226
    Femu = 1.8432MHz: K = 5.184*10^10/(694*400*2^23)  = 0.02226

    EC=100imp/kWh,  HFconst=347:
    Femu = 921.6kHz:  K = 2.592*10^10/(347*100*2^23)  = 0.08905
    Femu = 1.8432MHz: K = 5.184*10^10/(694*100*2^23)  = 0.08906

    EC=60imp/kWh,   HFconst=385:
    Femu = 921.6kHz:  K = 2.592*10^10/(385*60*2^23)   = 0.13376
    Femu = 1.8432MHz: K = 5.184*10^10/(771*60*2^23)   = 0.13361
    
    EC=300imp/kWh,   HFconst=231:
    Femu = 921.6kHz:  K = 2.592*10^10/(231*200*2^23)  = 0.06688
    Femu = 1.8432MHz: K = 5.184*10^10/(462*200*2^23)  = 0.06688
    
    EC=40imp/kWh,  HFconst=347:
    Femu = 921.6kHz:  K = 2.592*10^10/(347*40*2^23)  = 0.2226
    Femu = 1.8432MHz: K = 5.184*10^10/(694*40*2^23)  = 0.2226
   --------------------------------------------------------------------- */
#if EMU_FREQ == 0
#define K_EC6400_Ib5A              (0.00112)
#define K_EC400_Ib100A             (0.02226)
#define K_EC100_Ib400A             (0.08905)
#define K_EC60_Ib600A              (0.13376)
#else
#define K_EC6400_Ib5A              (0.00111)
#define K_EC400_Ib100A             (0.02226)
#define K_EC100_Ib400A             (0.08906)
#define K_EC60_Ib600A              (0.13361)
#define K_EC300_Ib200A             (0.06688)
#define K_EC40_Ib1000A             (0.2226)
#endif


/* ---------------------------------------------------------------------
    Pstartup起动功率:
    Femu = 921.6kHz:  Pstartup = INT[0.6*Ub*Ib*HFconst*EC*k%*2^23/(2.592*10^10)]
    Femu = 1.8432MHz: Pstartup = INT[0.6*Ub*Ib*HFconst*EC*k%*2^23/(5.184*10^10)]

    EC=6400imp/kWh, k%=0.001:
    Femu = 921.6kHz:  Pstartup = INT[0.6*220*5*433*6400*k%*2^23/(2.592*10^10)]  = INT[591925.036*k%] = INT[591.93] = 0x024F
    Femu = 1.8432MHz: Pstartup = INT[0.6*220*5*867*6400*k%*2^23/(5.184*10^10)]  = INT[592608.552*k%] = INT[592.61] = 0x0250

    EC=400imp/kWh,  k%=0.001:
    Femu = 921.6kHz:  Pstartup = INT[0.6*220*100*347*400*k%*2^23/(2.592*10^10)] = INT[592950.310*k%] = INT[592.95] = 0x0250
    Femu = 1.8432MHz: Pstartup = INT[0.6*220*100*694*400*k%*2^23/(5.184*10^10)] = INT[592950.310*k%] = INT[592.95] = 0x0250

    EC=100imp/kWh,  k%=0.001:
    Femu = 921.6kHz:  Pstartup = INT[0.6*220*400*347*100*k%*2^23/(2.592*10^10)] = INT[592950.310*k%] = INT[592.95] = 0x0250
    Femu = 1.8432MHz: Pstartup = INT[0.6*220*400*694*100*k%*2^23/(5.184*10^10)] = INT[592950.310*k%] = INT[592.95] = 0x0250

    EC=60imp/kWh,   k%=0.001:
    Femu = 921.6kHz:  Pstartup = INT[0.6*220*600*385*60*k%*2^23/(2.592*10^10)]  = INT[592095.915*k%] = INT[592.10] = 0x0250
    Femu = 1.8432MHz: Pstartup = INT[0.6*220*600*771*60*k%*2^23/(5.184*10^10)]  = INT[592864.870*k%] = INT[592.86] = 0x0250
    
    EC=300imp/kWh,   k%=0.001:
    Femu = 921.6kHz:  Pstartup = INT[0.6*220*300*231*200*k%*2^23/(2.592*10^10)]  = INT[592095.915*k%] = INT[592.09] = 0x0250
    Femu = 1.8432MHz: Pstartup = INT[0.6*220*300*462*200*k%*2^23/(5.184*10^10)]  = INT[592095.915*k%] = INT[592.09] = 0x0250
   --------------------------------------------------------------------- */
#if EMU_FREQ == 0
#define REG_Pstartup_Ib5A         (0x024F)
#define REG_Pstartup_Ib100A       (0x0250)
#define REG_Pstartup_Ib400A       (0x0250)
#define REG_Pstartup_Ib600A       (0x0250)
#else
#define REG_Pstartup_Ib5A         (0x0250)
#define REG_Pstartup_Ib100A       (0x0250)
#define REG_Pstartup_Ib400A       (0x0250)
#define REG_Pstartup_Ib600A       (0x0250)
#define REG_Pstartup_Ib200A       (0x0250)
#define REG_Pstartup_Ib1000A       (0x0250)
#endif


/* ---------------------------------------------------------------------
    起动:
   --------------------------------------------------------------------- */
#define ESE_MINIMUM_CUR_Ib5A      (0.005)      //Ib=5A, 0.1%Ib
#define ESE_MINIMUM_CUR_Ib100A    (0.100)      //
#define ESE_MINIMUM_CUR_Ib400A    (0.400)      //
#define ESE_MINIMUM_CUR_Ib600A    (0.600)      //
#define ESE_MINIMUM_CUR_Ib200A    (0.200)      //
#define ESE_MINIMUM_CUR_Ib1000A   (1.000)      //

#define ESE_MINIMUM_POW_Ib5A      (1.100)      //Ib=5A, 0.1%Ib*Ub
#define ESE_MINIMUM_POW_Ib100A    (22.00)      //
#define ESE_MINIMUM_POW_Ib400A    (88.00)      //
#define ESE_MINIMUM_POW_Ib600A    (132.0)      //
#define ESE_MINIMUM_POW_Ib200A    (44.0)       //
#define ESE_MINIMUM_POW_Ib1000A   (220.00)      //

#if PROD_TYPE == PROD_ESP
#define ESE_STARTUP_VOL           (0.0)        //ESP启动电压
#else
#define ESE_STARTUP_VOL           (22.0)       //10%Un
#endif

/* ---------------------------------------------------------------------
    默认校准:
   --------------------------------------------------------------------- */
#define ESE_CALI_CUR_Ib5A         (5.0)        //默认校准电流A
#define ESE_CALI_CUR_Ib100A       (100.0)      //默认校准电流A
#define ESE_CALI_CUR_Ib400A       (400.0)      //默认校准电流A
#define ESE_CALI_CUR_Ib600A       (600.0)      //默认校准电流A
#define ESE_CALI_CUR_Ib200A       (200.0)      //默认校准电流A
#define ESE_CALI_CUR_Ib1000A      (1000.0)      //默认校准电流A

#define ESE_CALI_POW_Ib5A         (500.0)      //默认校准功率W
#define ESE_CALI_POW_Ib100A       (500.0)      //默认校准功率W
#define ESE_CALI_POW_Ib400A       (500.0)      //默认校准功率W
#define ESE_CALI_POW_Ib600A       (500.0)      //默认校准功率W
#define ESE_CALI_POW_Ib200A       (500.0)      //默认校准功率W
#define ESE_CALI_POW_Ib1000A      (500.0)      //默认校准功率W

#define ESE_CALI_VOL              (220.0)      //默认校准电压V

#define ESE_CALI_PF               (0.5)        //默认校准功率因数




/* Exported types ------------------------------------------------------------*/
/* SPD monitoring terminal */

typedef union
{
    struct
    {
        uint16_t  DO[2];      //  01 06
        uint16_t  DI[4];      //  01 08
        uint16_t  Tmp[4];     //  01 0C

    } st;
} ESE_Elem_st;  //占byte




/* Exported constants --------------------------------------------------------*/
extern ESE_Elem_st gESE_Elem;



/* Private functions ---------------------------------------------------------*/
void osThreadNew_ESE_Task(void);






#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB || PROD_TYPE == PROD_ESA */


