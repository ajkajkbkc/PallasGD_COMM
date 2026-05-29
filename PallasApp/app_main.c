
/* Private includes ----------------------------------------------------------*/
#include "main.h"

#include "app_main.h"
#include "app_log.h"
#include "app_uart.h"
#include "app_led.h"
#include "app_key.h"
#include "app_ntc.h"
#include "app_parameter.h"
#include "internet.h"
#include "app_att7022eu.h"
#include "app_collect.h"
#include "app_oled.h"
#include "kalyke_4G_task.h"
#include "plc_netcfg.h"
#include "plc_task.h"
#include "kalyke_monitor_task.h"
#include "app_tool.h"
#include "module_ESE.h"
//#include "kalyke_uart_task.h"
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
#define Read_DI_1()    HAL_GPIO_ReadPin(DI1_GPIO_Port, DI1_Pin)
#define Read_DI_2()    HAL_GPIO_ReadPin(DI2_GPIO_Port, DI2_Pin)
#define Read_DI_3()    HAL_GPIO_ReadPin(DI3_GPIO_Port, DI3_Pin)
#define Read_DI_4()    HAL_GPIO_ReadPin(DI4_GPIO_Port, DI4_Pin)

#define Read_DO_1()    HAL_GPIO_ReadPin(DO1_GPIO_Port, DO1_Pin)
#define Read_DO_2()    HAL_GPIO_ReadPin(DO2_GPIO_Port, DO2_Pin)

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  外部中断回传函数
  * @param  GPIO_Pin GPIO引脚
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    //LOGW("app_main", "!!!!!");   //better not printf log in interrupt
    switch (GPIO_Pin)
    {
#if PROD_TYPE == PROD_FSS
    case W5500_INT_Pin://GPIO_PIN_4

        break;

    case GPIO_PIN_6://PC6
        gPeCount++;
        PeTimer_Reset_FromISR();
        break;

    case GPIO_PIN_10://PC10
        gL1Count++;
        L1Timer_Reset_FromISR();
        break;

    case GPIO_PIN_11://PC11
        gL2Count++;
        L2Timer_Reset_FromISR();
        break;

    case FSS_COUNT_Pin://GPIO_PIN_13
        extiTrig_SSPD_LtTimes_FromISR();
        break;

#elif PROD_TYPE == PROD_FL
    case LT_P_EXTI2_Pin://GPIO_PIN_2
        extiTrig_Thunder_FromISR(LT_POLARITY_P);
        break;

    case LT_N_EXTI3_Pin://GPIO_PIN_3
        extiTrig_Thunder_FromISR(LT_POLARITY_N);
        break;

#elif PROD_TYPE == PROD_FS
    case FS_COUNT_Pin://GPIO_PIN_2
        extiTrig_FS_LtTimes_FromISR();
        break;

#endif

    default:
        break;
    }
}

/**
  * @brief  输入转换
  * @param  None
  * @retval None
  */
static void input_convert(void)
{
    gESE_Elem.st.DI[0] = P_GET_PIN(gFlashParam.st.DI_THUp[0], Read_DI_1());
    gESE_Elem.st.DI[1] = P_GET_PIN(gFlashParam.st.DI_THUp[1], Read_DI_2());
    gESE_Elem.st.DI[2] = P_GET_PIN(gFlashParam.st.DI_THUp[2], Read_DI_3());
    gESE_Elem.st.DI[3] = P_GET_PIN(gFlashParam.st.DI_THUp[3], Read_DI_4());
    
    gESE_Elem.st.DO[0] = Read_DO_1();
    gESE_Elem.st.DO[1] = Read_DO_2();

    if(gESE_Elem.st.DI[0])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI1_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm3], StateAlm3_DI1_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI1_Msk);
        }
    }

    if(gESE_Elem.st.DI[1])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI2_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm3], StateAlm3_DI2_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI2_Msk);
        }
    }
    
    if(gESE_Elem.st.DI[2])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI3_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm3], StateAlm3_DI3_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI3_Msk);
        }
    }
    
    if(gESE_Elem.st.DI[3])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI4_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm3], StateAlm3_DI4_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI4_Msk);
        }
    }
    
    if(gESE_Elem.st.DI[0] || gESE_Elem.st.DI[1] || gESE_Elem.st.DI[2] || gESE_Elem.st.DI[3])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DI_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_DI_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DI_Msk);
        }
    }
    
    if(gESE_Elem.st.DO[0])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO1_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm3], StateAlm3_DO1_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO1_Msk);
        }
    }

    if(gESE_Elem.st.DO[1])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO2_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm3], StateAlm3_DO2_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO2_Msk);
        }
    }
    
    if(gESE_Elem.st.DO[0] || gESE_Elem.st.DO[1])
    {
        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DO_Msk);
    }
    else
    {
        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_DO_Msk))
        {
            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DO_Msk);
        }
    }
}

/**
  * @brief  报警判断
  * @param  None
  * @retval None
  */
void alarm_process(void)
{
    static uint16_t cnt[6];
    uint16_t alm_s;
    uint8_t i;

    /* 过压 欠压 过流 欠流 过功率 欠功率 */
    for(i = 0; i < 3; i++)
    {
        //过压
        gMeterParam.Volt[i] > gFlashParam.st.fVol_THUp ?
        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Up_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Up_Msk << i);

        //欠压
        gMeterParam.Volt[i] < gFlashParam.st.fVol_THDown ?
        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Down_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Down_Msk << i);

        //过流
        gMeterParam.Curr[i] > gFlashParam.st.fCur_THUp ?
        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Up_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Up_Msk << i);

        //欠流
        gMeterParam.Curr[i] < gFlashParam.st.fCur_THDown ?
        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Down_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Down_Msk << i);
#if PROD_TYPE == PROD_SFA || PROD_TYPE == PROD_SFB
        //过功率
        gMeterParam.PowP[i] > gFlashParam.st.fPow_THUp ?
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW1_Up_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW1_Up_Msk << i);

        //欠功率
        gMeterParam.PowP[i] < gFlashParam.st.fPow_THDown ?
        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW1_Down_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW1_Down_Msk << i);
#endif
    }
    for(i = 0; i < 6; i++)
    {
        if(i == 0)
        {
            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL_Up_Msk);
        }
        else if(i == 1)
        {
            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL_Down_Msk);
        }
        else if(i == 2)
        {
            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Up_Msk);
        }
        else if(i == 3)
        {
            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Down_Msk);
        }
#if PROD_TYPE == PROD_SFA || PROD_TYPE == PROD_SFB
        else if(i == 4)
        {
            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW_Up_Msk);
        }
        else if(i == 5)
        {
            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW_Down_Msk);
        }
#endif
        if(alm_s)
        {
            if(cnt[i] >= gFlashParam.st.Alm_Delay[i])
            {
                SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_VOL_Up_Msk << i);
                cnt[i] = gFlashParam.st.Alm_Delay[i];
            }
            else
            {
                cnt[i]++;
            }
        }
        else
        {
            if(cnt[i] <= 0)
            {
                if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_VOL_Up_Msk << i))
                {
                    CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_VOL_Up_Msk << i);
                }
                cnt[i] = 0;
            }
            else
            {
                cnt[i]--;
            }
        }
    }
}

/**
  * @brief  Open some task thread.
  * @param  None
  * @retval None
  */
void osThreadNew_Task(void)
{
    osThreadNew_uartTask();
    osThreadNew_ledTask();
    osThreadNew_ntcTask();
    osThreadNew_att7022euTask();
    
    osThreadNew_PLCTask();
    osThreadNew_MonitorTask();
    osThreadNew_keyOled();
    
    
//    osThreadNew_ESE_Task();
    
//    if(gFlashParam.st.Prod_Protocol & PROTOCOL_FLKMQTT)
//    {
//        osThreadNew_collectTask();
//        
//    }
//    if(gFlashParam.st.Prod_Protocol > PROTOCOL_MB)
//    {
//        osThreadNew_internetTask();
//        osThreadNew_4GTask();
//    }
}

/**
  * @brief  Function implementing the TheDefaultTask thread.
  * @param  None
  * @retval None
  */
void TheDefaultTask(void)
{
    //LOGW("app_main", "SystemCoreClock = %d", SystemCoreClock);

    for(;;)
    {
        osDelay(100);
        HAL_IWDG_Refresh(&hiwdg);

        gParam.st.SecCnt++;
        gFlashParam.st.SecCntAll++;
        if(gParam.st.SecCnt % 3 == 0)  //save SecCntAll in BKP
        {
            LOGE("app_main", "%d(%d)Free heap size is %d bytes", gParam.st.SecCnt, gFlashParam.st.SecCntAll, xPortGetFreeHeapSize());

            HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR41, gFlashParam.st.SecCntAll);
            HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR42, (gFlashParam.st.SecCntAll >> 16));
        }                
         
        if(aaa >= g_plc_netcfg.mqtt.configLength)
        {        
            if(gWANor4G == 1)
            {
                aaa = 0;
                kalyke_cycle_post_master_ELECTRIC();
                kalyke_cycle_post_master_DIDOTEMP();
#if PROD_TYPE == PROD_SFB
                kalyke_cycle_post_master_Imbalance();
#elif PROD_TYPE == PROD_SFE
                kalyke_cycle_post_master_Imbalance();
                kalyke_cycle_post_master_Uharmonic2_11();
								kalyke_cycle_post_master_Uharmonic12_21();
								kalyke_cycle_post_master_Uharmonic22_31();
								kalyke_cycle_post_master_Iharmonic2_11();
								kalyke_cycle_post_master_Iharmonic12_21();
								kalyke_cycle_post_master_Iharmonic22_31();
#endif
                masterTOalarm = 1;            
                LOGE("Kalyke_monitor", "aaa = %d", aaa);
            }
            else if(gWANor4G == 2)
            {
                aaa = 0;   
                osDelay(500);
                kalyke_cycle_post_master_ELECTRIC();
                osDelay(500);
                kalyke_cycle_post_master_DIDOTEMP();
                osDelay(500);
#if PROD_TYPE == PROD_SFB
                kalyke_cycle_post_master_Imbalance();
#elif PROD_TYPE == PROD_SFE
                kalyke_cycle_post_master_Imbalance();
                osDelay(500);
                kalyke_cycle_post_master_Uharmonic2_11();
								osDelay(500);
								kalyke_cycle_post_master_Uharmonic12_21();
								osDelay(500);
								kalyke_cycle_post_master_Uharmonic22_31();
								osDelay(500);
								kalyke_cycle_post_master_Iharmonic2_11();
								osDelay(500);
								kalyke_cycle_post_master_Iharmonic12_21();
                osDelay(500);
								kalyke_cycle_post_master_Iharmonic22_31();
                osDelay(500);
#endif
                masterTOalarm = 1;            
                LOGE("Kalyke_monitor", "aaa = %d", aaa);
            }
        }

        input_convert();
        alarm_process();        

        if(!READ_BIT(gFlashParam.st.AlmOutput_SourceLogic0, Output_OnlyCmd_Msk))  //不用指令控制
        {
            if(READ_BIT(gFlashParam.st.AlmOutput_SourceLogic0, Output_AndAlmS_Msk))  //与
            {
                if(READ_BIT(gParam.st.State_Alarm[0], gFlashParam.st.AlmOutput_Source[0]) == gFlashParam.st.AlmOutput_Source[0])
                {
                    gParam.st.AlmOutput0 = BOOL_Alarm;
                }
                else
                {
                    gParam.st.AlmOutput0 = BOOL_Normal;
                }
            }
            else  //或
            {
                if(READ_BIT(gParam.st.State_Alarm[0], gFlashParam.st.AlmOutput_Source[0]))
                {
                    gParam.st.AlmOutput0 = BOOL_Alarm;
                }
                else
                {
                    gParam.st.AlmOutput0 = BOOL_Normal;
                }
            }
        }

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, (GPIO_PinState)N_GET_PIN(gFlashParam.st.AlmOutput_THUp0, gParam.st.AlmOutput0));
        osDelay(100);
        
        if(!READ_BIT(gFlashParam.st.AlmOutput_SourceLogic1, Output_OnlyCmd_Msk))  //不用指令控制
        {
            if(READ_BIT(gFlashParam.st.AlmOutput_SourceLogic1, Output_AndAlmS_Msk))  //与
            {
                if(READ_BIT(gParam.st.State_Alarm[0], gFlashParam.st.AlmOutput_Source[1]) == gFlashParam.st.AlmOutput_Source[1])
                {
                    gParam.st.AlmOutput1 = BOOL_Alarm;
                }
                else
                {
                    gParam.st.AlmOutput1 = BOOL_Normal;
                }
            }
            else  //或
            {
                if(READ_BIT(gParam.st.State_Alarm[0], gFlashParam.st.AlmOutput_Source[1]))
                {
                    gParam.st.AlmOutput1 = BOOL_Alarm;
                }
                else
                {
                    gParam.st.AlmOutput1 = BOOL_Normal;
                }
            }
        }
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, (GPIO_PinState)N_GET_PIN(gFlashParam.st.AlmOutput_THUp1, gParam.st.AlmOutput1));
        osDelay(100);
    }
    
}
