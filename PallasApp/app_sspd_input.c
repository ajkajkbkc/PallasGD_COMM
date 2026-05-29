
///* Private includes ----------------------------------------------------------*/
//#include "main.h"
//#include "cmsis_os.h"
//#include "timers.h"


//#include "app_log.h"
//#include "app_parameter.h"
//#include "app_tm1650.h"
//#include "app_key.h"
//#include "app_tool.h"
//#include "app_oled.h"
//#include "app_main.h"
///* Private define ------------------------------------------------------------*/
////#define Read_L1()   HAL_GPIO_ReadPin(MCU_S_L1_GPIO_Port, MCU_S_L1)
////#define Read_L2()   HAL_GPIO_ReadPin(MCU_S_L2_GPIO_Port, MCU_S_L2)
//#define Read_L3()   HAL_GPIO_ReadPin(MCU_S_L3_GPIO_Port, MCU_S_L3)
//#define Read_PE()   HAL_GPIO_ReadPin(MCU_S_PE_GPIO_Port, MCU_S_PE)
//#define Read_SW1()  HAL_GPIO_ReadPin(FSS_SW1_GPIO_Port, FSS_SW1_Pin)
//#define Read_SW2()  HAL_GPIO_ReadPin(FSS_SW2_GPIO_Port, FSS_SW2_Pin)
//#define Read_SW3()  HAL_GPIO_ReadPin(FSS_SW3_GPIO_Port, FSS_SW3_Pin)
//#define Read_SW4()  HAL_GPIO_ReadPin(FSS_SW4_GPIO_Port, FSS_SW4_Pin)
//#define Read_D_ID1()  HAL_GPIO_ReadPin(FSS_D_ID1_GPIO_Port, FSS_D_ID1_Pin)
//#define Read_D_ID2()  HAL_GPIO_ReadPin(FSS_D_ID2_GPIO_Port, FSS_D_ID2_Pin)
//#define Read_C_ID1()  HAL_GPIO_ReadPin(FSS_C_ID1_GPIO_Port, FSS_C_ID1_Pin)
//#define Read_C_ID2()  HAL_GPIO_ReadPin(FSS_C_ID2_GPIO_Port, FSS_C_ID2_Pin)
//#define Read_S_ID1()  HAL_GPIO_ReadPin(FSS_S_ID1_GPIO_Port, FSS_S_ID1_Pin)
//#define Read_S_ID2()  HAL_GPIO_ReadPin(FSS_S_ID2_GPIO_Port, FSS_S_ID2_Pin)

//#define GET_ID(D1, D2, C1, C2, S1, S2) (D1 << 5 | D2 << 4 | C1 << 3 | C2 << 2 | S1 << 1 | S2)
//#define GET_PIN(LOGIC_LEVEL, READ_PIN, Val_Pin)\
//(\
//    LOGIC_LEVEL == 0 ? \
//    (Val_Pin = READ_PIN) : \
//    (Val_Pin = !READ_PIN) \
//)\

//#define IO_MAX_S_NUM         8
//#define L1_FILTER_HIGH       100
//#define L2_FILTER_HIGH       100
//#define L3_FILTER_HIGH       3
//#define PE_FILTER_HIGH       100
//#define L1_FILTER_TIMEOUT    1000
//#define L2_FILTER_TIMEOUT    1000
//#define L3_FILTER_TIMEOUT    1000
//#define PE_FILTER_TIMEOUT    1000

///* Private variables ---------------------------------------------------------*/
//TimerHandle_t SSPD_LtTimesTimer = NULL;

///* Definitions for inputTask */
//osThreadId_t sspd_inputTaskHandle;
//osThreadId_t L3_inputTaskHandle;

//const osThreadAttr_t sspd_inputTask_attributes =
//{
//    .name = "sspd_inputTask",
//    .priority = (osPriority_t) sspd_inputTaskPriority,
//    .stack_size = 1024
//};

//const osThreadAttr_t L3_inputTask_attributes =
//{
//    .name = "L3_inputTask",
//    .priority = (osPriority_t) L3_inputTaskPriority,
//    .stack_size = 1024
//};

//static TimerHandle_t gPeTimer = NULL;
//volatile uint16_t gPeCount;

//static TimerHandle_t gL1Timer = NULL;
//volatile uint16_t gL1Count;

//static TimerHandle_t gL2Timer = NULL;
//volatile uint16_t gL2Count;

//static TimerHandle_t gL3Timer = NULL;
//volatile uint16_t gL3Count;

///* Private function prototypes -----------------------------------------------*/
//void SSPD_InputTask(void *argument);
//void L3_InputTask(void *argument);

///* Private user code ---------------------------------------------------------*/
///**
//  * @brief  获取硬件ID
//  * @param  None
//  * @retval None
//  */
//static void SSPD_GET_HW_ID(void)
//{
//    gParam.st.HW_ID = GET_ID(Read_D_ID1(), Read_D_ID2(), Read_C_ID1(), Read_C_ID2(), Read_S_ID1(), Read_S_ID2());
//}

///**
//  * @brief  输入检测判断
//  * @param  None
//  * @retval None
//  */
//static void SSPD_Input_Check(void)
//{

//    P_GET_PIN(gFlashParam.st.SPD_SW1_Up, Read_SW1(), gFSS_Elem.st.SW[0]);
//    if(gFSS_Elem.st.SW[0])
//    {
//     SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT1_Msk);   
//    }
//    else
//    {
//     CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT1_Msk);   
//    }
//    
//    P_GET_PIN(gFlashParam.st.SPD_SW2_Up, Read_SW2(), gFSS_Elem.st.SW[1]);
// 
//    if( ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEM4) || ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEA4) )  //三相
//    {
//        if(gFSS_Elem.st.SW[1])
//        {
//         SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT2_Msk);   
//        }
//        else
//        {
//         CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT2_Msk);   
//        }
//        P_GET_PIN(gFlashParam.st.SPD_SW3_Up, Read_SW3(), gFSS_Elem.st.SW[2]);
//        if(gFSS_Elem.st.SW[2])
//        {
//            SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT3_Msk);   
//        }
//        else
//        {
//            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT3_Msk);   
//        }
//        P_GET_PIN(gFlashParam.st.SPD_SW4_Up, Read_SW4(), gFSS_Elem.st.SW[3]);
//        if(gFSS_Elem.st.SW[3])
//        {
//            SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT_Msk);   
//        }
//        else
//        {
//            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT_Msk);   
//        }
//    }
//    else
//    {
//        if(gFSS_Elem.st.SW[1])
//        {
//         SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT_Msk);   
//        }
//        else
//        {
//         CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_NT_Msk);   
//        }        
//        gFSS_Elem.st.SW[2] = FSS_USELESS_PARAM_DEFAULT;
//        gFSS_Elem.st.SW[3] = FSS_USELESS_PARAM_DEFAULT;
//        gFSS_Elem.st.L2 = FSS_USELESS_PARAM_DEFAULT;
//        gFSS_Elem.st.L3 = FSS_USELESS_PARAM_DEFAULT;
//    }
//}


///**
//  * @brief  关闭中断
//  * @param  None
//  * @retval None
//  */
//static void SSPD_LtTimes_DisableIRQ(void)
//{
//    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn); // 关闭中断
//}

///**
//  * @brief  开启中断
//  * @param  None
//  * @retval None
//  */
//static void SSPD_LtTimes_EnableIRQ(void)
//{
//    LOGW("app_input", "open EXTI Enable");
//    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);  //需要先清除标志位，避免开启后会立刻进入中断
//    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
//}

///**
//  * @brief  enable exti
//  * @param  None
//  * @retval None
//  */
//static void enable_SSPD_LtTimes_exti(void *arg)
//{
//    SSPD_LtTimes_EnableIRQ();
//}

///**
//  * @brief  uart2 software timer init
//  * @param  None
//  * @retval None
//  */
//static void SSPD_LtTimes_timer_init(void)
//{
//    SSPD_LtTimesTimer = xTimerCreate((const char *)"SSPD_LtTimesTimer",
//                                     (TickType_t  )gFlashParam.st.LtTimesFilter,
//                                     (UBaseType_t )pdFALSE,
//                                     (void *      )4,
//                                     (TimerCallbackFunction_t)enable_SSPD_LtTimes_exti);
//}

///**
//  * @brief  start LtTiems timer
//  * @param  None
//  * @retval None
//  */
//static void SSPD_LtTimes_timer_start_FromISR(void)
//{
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//    if( xTimerStartFromISR( SSPD_LtTimesTimer, &xHigherPriorityTaskWoken ) != pdPASS )
//    {
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
//  * @brief  雷击触发（在中断里调用）
//  * @param  None
//  * @retval None
//  */
//void extiTrig_SSPD_LtTimes_FromISR(void)
//{
//    gFSS_Elem.st.LtNum++;

//    SSPD_LtTimes_DisableIRQ();
//    SSPD_LtTimes_timer_start_FromISR();
//}

///**
//  * @brief  定时器回调函数
//  * @param  None
//  * @retval None
//  */
//static void PeTimer_callback_func(TimerHandle_t hTimer)
//{
//    gPeCount = 0;
//}

//static void L1Timer_callback_func(TimerHandle_t hTimer)
//{
//    gL1Count = 0;
//}

//static void L2Timer_callback_func(TimerHandle_t hTimer)
//{
//    gL2Count = 0;
//}

//static void L3Timer_callback_func(TimerHandle_t hTimer)
//{
//    gL3Count = 0;
//}
///**
//  * @brief  定时器初始化
//  * @param  None
//  * @retval None
//  */
//static void PeTimer_Init(void)
//{
//    gPeTimer = xTimerCreate((const char *)"PeTimer",
//                            (TickType_t  )PE_FILTER_TIMEOUT,
//                            (UBaseType_t )pdFALSE,
//                            (void *      )1,
//                            (TimerCallbackFunction_t)PeTimer_callback_func);
//    configASSERT(gPeTimer != NULL);
//}

//static void L1Timer_Init(void)
//{
//    gL1Timer = xTimerCreate((const char *)"L1Timer",
//                            (TickType_t  )L1_FILTER_TIMEOUT,
//                            (UBaseType_t )pdFALSE,
//                            (void *      )1,
//                            (TimerCallbackFunction_t)L1Timer_callback_func);
//    configASSERT(gL1Timer != NULL);
//}

//static void L2Timer_Init(void)
//{
//    gL2Timer = xTimerCreate((const char *)"L2Timer",
//                            (TickType_t  )L2_FILTER_TIMEOUT,
//                            (UBaseType_t )pdFALSE,
//                            (void *      )1,
//                            (TimerCallbackFunction_t)L2Timer_callback_func);
//    configASSERT(gL2Timer != NULL);
//}

//static void L3Timer_Init(void)
//{
//    gL3Timer = xTimerCreate((const char *)"L3Timer",
//                            (TickType_t  )L3_FILTER_TIMEOUT,
//                            (UBaseType_t )pdFALSE,
//                            (void *      )1,
//                            (TimerCallbackFunction_t)L3Timer_callback_func);
//     configASSERT(gL3Timer != NULL);
//}

///**
//  * @brief  开启定时器
//  * @param  None
//  * @retval None
//  */
//void PeTimer_Reset_FromISR(void)
//{
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//    if( xTimerResetFromISR( gPeTimer, &xHigherPriorityTaskWoken ) != pdPASS )
//    {
//        // LOGE(TAG, "Reset gPeTimer was not executed successfully");
//        // The start command was not executed successfully.  Take appropriate
//        // action here.
//    }
//    if( xHigherPriorityTaskWoken != pdFALSE )
//    {
//        // Call the interrupt safe yield function here (actual function
//        // depends on the FreeRTOS port being used).
//    }
//}

//void L1Timer_Reset_FromISR(void)
//{
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//    if( xTimerResetFromISR( gL1Timer, &xHigherPriorityTaskWoken ) != pdPASS )
//    {
//        // LOGE(TAG, "Reset gLTimer was not executed successfully");
//        // The start command was not executed successfully.  Take appropriate
//        // action here.
//    }
//    if( xHigherPriorityTaskWoken != pdFALSE )
//    {
//        // Call the interrupt safe yield function here (actual function
//        // depends on the FreeRTOS port being used).
//    }
//}

//void L2Timer_Reset_FromISR(void)
//{
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//    if( xTimerResetFromISR( gL2Timer, &xHigherPriorityTaskWoken ) != pdPASS )
//    {
//        // LOGE(TAG, "Reset gLTimer was not executed successfully");
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
//  * @brief  接地输入监测
//  * @param  None
//  * @retval None
//  */
//void pe_input_monitor(void)
//{
//    if(gPeCount >= PE_FILTER_HIGH)
//    {
//        gPeCount = PE_FILTER_HIGH;
//        gFSS_Elem.st.PE = 0;
//        CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_PE_Msk);
//    }
//    else
//    {
//        gFSS_Elem.st.PE = 1;
//        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_PE_Msk);
//    }
//    //LOGI("PE", "PE=%d",gFSS_Elem.st.PE);
//}

//void L1_input_monitor(void)
//{
//    if(gL1Count >= L1_FILTER_HIGH)
//    {
//        gL1Count = L1_FILTER_HIGH;
//        gFSS_Elem.st.L1 = 0;
//        CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_L1_Msk);
//    }
//    else
//    {
//        gFSS_Elem.st.L1 = 1;
//        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_L1_Msk);
//    }
//    //LOGI("L1COU", "L1=%d",gFSS_Elem.st.L1);
//}

//void L2_input_monitor(void)
//{
//    if(gL2Count >= L2_FILTER_HIGH)
//    {
//        gL2Count = L2_FILTER_HIGH;
//        gFSS_Elem.st.L2 = 0;
//        CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_L2_Msk);
//    }
//    else
//    {
//        gFSS_Elem.st.L2 = 1;
//        SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_L2_Msk);
//    }
//}

///**
//  * @brief  新建L3检测（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_L3_inputTask(void)
//{
//    L3_inputTaskHandle = osThreadNew(L3_InputTask, NULL, &L3_inputTask_attributes);
//}

///**
//  * @brief  Function implementing the inputTask thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void L3_InputTask(void *argument)
//{
//    L3Timer_Init();
//    for(;;)
//    {
//        if(!Read_L3())
//        {
//            gL3Count++;
//            xTimerStart(gL3Timer,0); 
//        }
//        if(gL3Count >= L3_FILTER_HIGH)
//        {
//            gL3Count = L3_FILTER_HIGH;
//            gFSS_Elem.st.L3 = 0;
//            CLEAR_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_L3_Msk);
//        }
//        else
//        {
//            gFSS_Elem.st.L3 = 1;
//            SET_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_L3_Msk);
//        }
//        osDelay(1);
//    }
//}

///**
//  * @brief  新建线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_sspd_inputTask(void)
//{
//    sspd_inputTaskHandle = osThreadNew(SSPD_InputTask, NULL, &sspd_inputTask_attributes);
//}

///**
//  * @brief  Function implementing the inputTask thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void SSPD_InputTask(void *argument)
//{
//    SSPD_GET_HW_ID();
//    LOGD("input", "%s RUN. Hardware ID is: 0x%02x. Free heap size is %d bytes", __func__, gParam.st.HW_ID, xPortGetFreeHeapSize());

//    SSPD_LtTimes_timer_init();
//    L1Timer_Init();
//    L2Timer_Init();
//    PeTimer_Init();
//    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
//    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);  //中断内用了定时器函数，如果在定时器初始化前就进入中断，会死机

//    for(;;)
//    {
//        SSPD_Input_Check();
//        L1_input_monitor();
//        pe_input_monitor();
//        
//        if( ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEM4) || ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEA4) )
//        {
//            L2_input_monitor();
//        }

//        if(gKey1Mode == KEY_MODE_DEFAULT)
//        {
//            if(gFSS_Elem.st.LtNum != gFlashParam.st.LtTimes)
//            {
//                if(gFSS_Elem.st.LtNum > 9999)
//                {
//                    gFSS_Elem.st.LtNum = 1;
//                }
//                gFlashParam.st.LtTimes = gFSS_Elem.st.LtNum;
//                Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//                if(gFlashParam.st.TUBEEnable)
//                {
//                _Dispaly_num(gFlashParam.st.LtTimes);
//                }
//            }
//        }

//        osDelay(1);
//    }
//}



