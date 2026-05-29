/*
 * Copyright (c) 2006-2018, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-28     Arrbow       first implementation
 */

/* Private includes ----------------------------------------------------------*/
#include "module_ESE.h"

#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "timers.h"

#include "app_uart.h"
#include "app_log.h"
#include "app_led.h"
#include "app_oled.h"
#include "app_att7022eu.h"
#include "app_tool.h"
#include "app_parameter.h"
#include "mb.h"


/* Private define ------------------------------------------------------------*/
//#define Read_DI_1()    HAL_GPIO_ReadPin(DI1_GPIO_Port, DI1_Pin)
//#define Read_DI_2()    HAL_GPIO_ReadPin(DI2_GPIO_Port, DI2_Pin)


///* Private variables ---------------------------------------------------------*/
///* Definitions for Task */
//osThreadId_t ESE_TaskHandle;
//const osThreadAttr_t ESE_Task_attributes =
//{
//    .name = "ESE_Task",
//    .priority = (osPriority_t) osPriorityNormal2,
//    .stack_size = 256
//};
//static const char *TAG = "ESE";

//static TimerHandle_t gLed1Timer = NULL;

ESE_Elem_st gESE_Elem;

///* Private function prototypes -----------------------------------------------*/
//void ESE_Task(void *argument);



///* Private user code ---------------------------------------------------------*/
///**
//  * @brief  Open some task thread.
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_Task(void)
//{
//    osThreadNew_uartTask();
//    osThreadNew_ledTask();
//    osThreadNew_keyOled();

//    osThreadNew_att7022Task();
//    osThreadNew_ESE_Task();
//}

///**
//  * @brief  Module parameter init
//  * @param  None
//  * @retval None
//  */
//void module_param_init(void)
//{
//    memset(&gESE_Elem, 0, sizeof(gESE_Elem));
//}

///**
//  * @brief  定时器回调函数
//  * @param  None
//  * @retval None
//  */
//static void Led1Timer_callback_func(TimerHandle_t hTimer)
//{
//    LED_1_G();
//}

///**
//  * @brief  定时器初始化
//  * @param  None
//  * @retval None
//  */
//static void Led1Timer_Init(void)
//{
//    gLed1Timer = xTimerCreate((const char *)"Led1Timer",
//                              (TickType_t  )50,
//                              (UBaseType_t )pdFALSE,
//                              (void *      )1,
//                              (TimerCallbackFunction_t)Led1Timer_callback_func);
//    configASSERT(gLed1Timer != NULL);
//}

///**
//  * @brief  开启定时器
//  * @param  None
//  * @retval None
//  */
//static void Led1Timer_Reset_FromISR(void)
//{
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//    if( xTimerResetFromISR( gLed1Timer, &xHigherPriorityTaskWoken ) != pdPASS )
//    {
//        LOGE(TAG, "Reset gPeTimer was not executed successfully");
//        // The start command was not executed successfully.  Take appropriate
//        // action here.
//    }
//    if( xHigherPriorityTaskWoken != pdFALSE )
//    {
//        // Call the interrupt safe yield function here (actual function
//        // depends on the FreeRTOS port being used).
//    }
//}

///**
//  * @brief  LED 1 loop
//  * @param  None
//  * @retval None
//  */
//void module_led_1_loop(void)
//{
//#if PROD_TYPE == PROD_ESB
//    (READ_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_IMB_Msk)) ? (LED_1_R()) : (LED_1_G());
//#elif PROD_TYPE == PROD_ESP
//    if(gMeterParam.Volt[0]*10 > gFlashParam.st.fVol_PNUp)
//    {
//        LED_1_R();
//    }
//    else
//    {
//        LED_1_G();
//    }
//#else
//    vTaskDelete(NULL); //删除任务
//#endif
//    osDelay(500);
//}

///**
//  * @brief  LED 2 loop
//  * @param  None
//  * @retval None
//  */
//void module_led_2_loop(void)
//{
//    uint8_t alm;

//    alm = READ_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DI_Msk);
//    (READ_BIT(alm, gFlashParam.st.DI_Led_relate)) ? (LED_2_R()) : (LED_2_G());
//    osDelay(500);
//}

///**
//  * @brief  LED 3 loop
//  * @param  None
//  * @retval None
//  */
////void module_led_3_loop(void)
////{
////    (gParam.st.AlmOutput == BOOL_Alarm) ? (LED_3_R()) : (LED_3_G());
////    osDelay(500);
////}

///**
//  * @brief  uart handle
//  * @param  None
//  * @retval None
//  */
//void module_handle_uart_data(uint8_t uartport, uint8_t *pbuff, uint16_t len)
//{
//    if(uartport == UART_PORT1)
//    {
//        if(is_mb_protocol(pbuff, len))
//        {
//            md_slave_msg_pack smsg = {0,};

//            smsg.mcv_IsBroadcastInfo = pbuff[0] == 0 ? 1 : 0;
//            smsg.mcp_ReceiveBuff = pbuff;
//            smsg.msv_ReceiveLen = len;
//            smsg.mcv_Sender = MB_SENDER_UART1;
//            smsg.uart_resp_func = bsp_uart1_send_buffer;
//            smsg.mcp_RespBuff = (unsigned char *)pvPortMalloc(2048);
//            LOGI("uart", "smsg.mcp_RespBuff = 0x%08X, Free Heap Size = %d", (uint32_t)smsg.mcp_RespBuff, xPortGetFreeHeapSize());
//            mb_slave_msg_handler(&smsg);
//            vPortFree(smsg.mcp_RespBuff);
//        }
//    }
//}

///**
//  * @brief  输入转换
//  * @param  None
//  * @retval None
//  */
//static void input_convert(void)
//{
//    gESE_Elem.st.DI[0] = P_GET_PIN(gFlashParam.st.DI_THUp[0], Read_DI_1());
//    gESE_Elem.st.DI[1] = P_GET_PIN(gFlashParam.st.DI_THUp[1], Read_DI_2());

//    if(gESE_Elem.st.DI[0])
//    {
//        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DI1_Msk);
//    }
//    else
//    {
//        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_DI1_Msk))
//        {
//            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DI1_Msk);
//        }
//    }

//    if(gESE_Elem.st.DI[1])
//    {
//        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DI2_Msk);
//    }
//    else
//    {
//        if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_DI2_Msk))
//        {
//            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_DI2_Msk);
//        }
//    }
//}

///**
//  * @brief  报警判断
//  * @param  None
//  * @retval None
//  */
//void alarm_process(void)
//{
//    static uint16_t cnt[6];
//    uint16_t alm_s;
//    uint8_t i;

//    /* 过压 欠压 过流 欠流 过功率 欠功率 */
//    for(i = 0; i < 3; i++)
//    {
//        //过压
//        gMeterParam.Volt[i] > gFlashParam.st.fVol_THUp ?
//        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Up_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Up_Msk << i);

//        //欠压
//        gMeterParam.Volt[i] < gFlashParam.st.fVol_THDown ?
//        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Down_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Down_Msk << i);

//        //过流
//        gMeterParam.Curr[i] > gFlashParam.st.fCur_THUp ?
//        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Up_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Up_Msk << i);

//        //欠流
//        gMeterParam.Curr[i] < gFlashParam.st.fCur_THDown ?
//        SET_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Down_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Down_Msk << i);

//        //过功率
//        gMeterParam.PowP[i] > gFlashParam.st.fPow_THUp ?
//        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW1_Up_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW1_Up_Msk << i);

//        //欠功率
//        gMeterParam.PowP[i] < gFlashParam.st.fPow_THDown ?
//        SET_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW1_Down_Msk << i) : CLEAR_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW1_Down_Msk << i);
//    }
//    for(i = 0; i < 6; i++)
//    {
//        if(i == 0)
//        {
//            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL_Up_Msk);
//        }
//        else if(i == 1)
//        {
//            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL_Down_Msk);
//        }
//        else if(i == 2)
//        {
//            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Up_Msk);
//        }
//        else if(i == 3)
//        {
//            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Down_Msk);
//        }
//        else if(i == 4)
//        {
//            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW_Up_Msk);
//        }
//        else if(i == 5)
//        {
//            alm_s = READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_POW_Down_Msk);
//        }

//        if(alm_s)
//        {
//            if(cnt[i] >= gFlashParam.st.Alm_Delay[i])
//            {
//                SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_VOL_Up_Msk << i);
//                cnt[i] = gFlashParam.st.Alm_Delay[i];
//            }
//            else
//            {
//                cnt[i]++;
//            }
//        }
//        else
//        {
//            if(cnt[i] <= 0)
//            {
//                if(!READ_BIT(gFlashParam.st.AlmState_Keep[StateAlm0], StateAlm0_VOL_Up_Msk << i))
//                {
//                    CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_VOL_Up_Msk << i);
//                }
//                cnt[i] = 0;
//            }
//            else
//            {
//                cnt[i]--;
//            }
//        }
//    }
//}

///**
//  * @brief  外部中断回传函数
//  * @param  GPIO_Pin GPIO引脚
//  * @retval None
//  */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//    static uint32_t mstick = 0;

//    switch (GPIO_Pin)
//    {
//    case METER_CF1_Pin:
//        gParam.st.Meter_CF_Interval_Time[0] = xTaskGetTickCount() - mstick;  //计算CF的脉冲频率
//        mstick = xTaskGetTickCount();
//        LED_1_R();
//        Led1Timer_Reset_FromISR();
//        break;

//    case METER_CF2_Pin:

//        break;

//    default:
//        break;
//    }
//}

///**
//  * @brief  新建串口线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_ESE_Task(void)
//{
//    ESE_TaskHandle = osThreadNew(ESE_Task, NULL, &ESE_Task_attributes);
//}

///**
//  * @brief  Function implementing the Task thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void ESE_Task(void *argument)
//{
//    LOGD(TAG, "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

//#if PROD_TYPE == PROD_ESB
//#else
//    //Led1Timer_Init();
//    //HAL_NVIC_EnableIRQ(METER_CF1_EXTI_IRQn);
//    //HAL_NVIC_EnableIRQ(METER_CF2_EXTI_IRQn);
//#endif

//    for(;;)
//    {
//        input_convert();
//        alarm_process();

//        if(!READ_BIT(gFlashParam.st.AlmOutput_SourceLogic, Output_OnlyCmd_Msk))  //不用指令控制
//        {
//            if(READ_BIT(gFlashParam.st.AlmOutput_SourceLogic, Output_AndAlmS_Msk))  //与
//            {
//                if(READ_BIT(gParam.st.State_Alarm[0], gFlashParam.st.AlmOutput_Source[0]) == gFlashParam.st.AlmOutput_Source[0])
//                {
//                    gParam.st.AlmOutput = BOOL_Alarm;
//                }
//                else
//                {
//                    gParam.st.AlmOutput = BOOL_Normal;
//                }
//            }
//            else  //或
//            {
//                if(READ_BIT(gParam.st.State_Alarm[0], gFlashParam.st.AlmOutput_Source[0]))
//                {
//                    gParam.st.AlmOutput = BOOL_Alarm;
//                }
//                else
//                {
//                    gParam.st.AlmOutput = BOOL_Normal;
//                }
//            }
//        }

//        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, (GPIO_PinState)N_GET_PIN(gFlashParam.st.AlmOutput_THUp, gParam.st.AlmOutput));
//        osDelay(100);
//    }
//}

