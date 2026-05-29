
///* Private includes ----------------------------------------------------------*/
//#include "app_thunder.h"
//#include "app_tool.h"
//#include "app_log.h"
//#include "app_main.h"
//#include "app_parameter.h"
//#include "app_opts.h"
//#include "app_rtc.h"
//#include "app_tm1650.h"
//#include "app_led.h"

//#include "cmsis_os.h"
//#include "timers.h"
//#include <string.h>

///* Private define ------------------------------------------------------------*/
//#define READ_LT_P_Pin()   HAL_GPIO_ReadPin(LT_P_EXTI2_GPIO_Port, LT_P_EXTI2_Pin)
//#define READ_LT_N_Pin()   HAL_GPIO_ReadPin(LT_N_EXTI3_GPIO_Port, LT_N_EXTI3_Pin)

//#define TAKE_START_NUMBER               26   //从第x个数开始取数
//#define TAKE_NUMBERS_TO_AVERAGE         2    //取x个数计算平均数


///* Private variables ---------------------------------------------------------*/
///* Definitions for thunderTask */
//osThreadId_t thunderTaskHandle;
//osThreadId_t thunderAlmTaskHandle;
//const osThreadAttr_t thunderTask_attributes =
//{
//    .name = "thunderTask",
//    .priority = (osPriority_t) thunderTaskPriority,
//    .stack_size = 2048
//};
//const osThreadAttr_t thunderAlmTask_attributes =
//{
//    .name = "thunderAlmTask",
//    .priority = (osPriority_t) osPriorityNormal4,
//    .stack_size = 2048
//};

//osEventFlagsId_t gpAppEvent;

//TimerHandle_t gEnbleExtiTimer =  NULL;

///* Private function prototypes -----------------------------------------------*/
//static void EnbleExti_timer_callback_func(TimerHandle_t ltv_TimeHandle);

//void ThunderTask(void *argument);


///* Private user code ---------------------------------------------------------*/
//#if 0
///**
//  * @brief  输入极性，得到该路的adc值
//  * @param  雷击极性
//  * @retval ADC值
//  */
//uint16_t Adc_GetVal(uint8_t Polarity)
//{

//}
//#endif

///**
//  * @brief  两路adc值，判断出极性值和峰值
//  * @param  *pPolarity 根据峰值判断出的极性 0:+, 1:-
//  * @retval 较大一路ADC的值
//  */
//uint16_t Handle_Adc_Val(uint16_t *pPolarity)
//{
//    uint16_t usBuf10[KEEP_ADC_SIZE],  usBuf11[KEEP_ADC_SIZE];

//    for(uint8_t i = 0; i < KEEP_ADC_SIZE; i++)
//    {
//        usBuf11[i] = gParam.st.aADCCh11Buf[i];
//        usBuf10[i] = gParam.st.aADCCh10Buf[i];
//    }
//    gParam.st.usADCCh11Average = SMA_Compare((uint16_t *)usBuf11, KEEP_ADC_SIZE, TAKE_START_NUMBER, TAKE_NUMBERS_TO_AVERAGE);
//    LOGI("app_thunder", "Positive adc average value = %d", gParam.st.usADCCh11Average);

//    gParam.st.usADCCh10Average = SMA_Compare((uint16_t *)usBuf10, KEEP_ADC_SIZE, TAKE_START_NUMBER, TAKE_NUMBERS_TO_AVERAGE);
//    LOGI("app_thunder", "Negative adc average value = %d", gParam.st.usADCCh10Average);

//    if(gParam.st.usADCCh11Average > gParam.st.usADCCh10Average)
//    {
//        *pPolarity = 0x0000;
//        return gParam.st.usADCCh11Average;
//    }
//    else
//    {
//        *pPolarity = 0x0001;
//        return gParam.st.usADCCh10Average;
//    }
//}

///**
//  * @brief  关闭中断
//  * @param  None
//  * @retval None
//  */
//void Lt_DisableIRQ(void)
//{
//    HAL_NVIC_DisableIRQ(LT_P_EXTI2_EXTI_IRQn); // 关闭中断
//    HAL_NVIC_DisableIRQ(LT_N_EXTI3_EXTI_IRQn);
//}

///**
//  * @brief  开启中断
//  * @param  None
//  * @retval None
//  */
//void Lt_EnableIRQ(void)
//{
//    LOGW("lightning", "open EXTI Enable");
//    __HAL_GPIO_EXTI_CLEAR_IT(LT_P_EXTI2_Pin);  //需要先清除标志位，避免开启后会立刻进入中断
//    __HAL_GPIO_EXTI_CLEAR_IT(LT_N_EXTI3_Pin);
//    HAL_NVIC_EnableIRQ(LT_P_EXTI2_EXTI_IRQn);
//    HAL_NVIC_EnableIRQ(LT_N_EXTI3_EXTI_IRQn);
//}

///**
//  * @brief  软件定时器回调函数
//  * @param  None
//  * @retval None
//  */
//static void EnbleExti_timer_callback_func(TimerHandle_t ltv_TimeHandle)
//{
//    Lt_EnableIRQ();
//}

///*
// * counts = 0, 6 MCU tick, 0.083333us
// * counts = 1, 9 MCU tick, 0.125000us
// * counts = 2, 14        , 0.194444us
// * counts = 3, 20        , 0.277778us
// * counts = 4, 26        , 0.361111us
// 故根据counts计算微秒的公式为：
//  [(counts - 2) * 6 + 14]/72   (counts > 2)
//*/
//void Thunder_easy_delay(uint32_t counts)
//{
//    while(counts--);
//}

///**
//  * @brief  雷击触发（在中断里调用）
//  * @param  雷击极性
//  * @retval None
//  */
//void extiTrig_Thunder_FromISR(uint8_t Polarity)
//{
//    //BaseType_t pxHigherPriorityTaskWoken;
//    uint32_t ulReturn;

//    ulReturn = taskENTER_CRITICAL_FROM_ISR(); /* 进入临界段，临界段可以嵌套 */

//    Lt_DisableIRQ(); //正负极中断都关闭
//    //xTimerStartFromISR(gEnbleExtiTimer, &pxHigherPriorityTaskWoken);
//    //Thunder_easy_delay(18);

//    for(uint16_t i = 0; i < KEEP_ADC_SIZE; i++)
//    {
//        gParam.st.aADCCh11Buf[i] = gParam.st.aADC1Val[0]; //正峰值
//        gParam.st.aADCCh10Buf[i] = gParam.st.aADC1Val[1];
//        Thunder_easy_delay(28);  //~~2.3us
//    }

//    switch(Polarity)
//    {
//    case LT_POLARITY_P:
//        osEventFlagsSet(gpAppEvent, EVENT_THUNDER_P_HAPPENED);
//        break;

//    case LT_POLARITY_N:
//        osEventFlagsSet(gpAppEvent, EVENT_THUNDER_N_HAPPENED);
//        break;

//    default:
//        break;
//    }

//    //portYIELD_FROM_ISR(pxHigherPriorityTaskWoken); //如果需要的话进行一次任务切换
//    taskEXIT_CRITICAL_FROM_ISR( ulReturn ); /* 退出临界段 */
//}

///**
//  * @brief  检查是否报警线程
//  * @param  None
//  * @retval None
//  */
//void Check_ThunderAlm_Thread(void *argument)
//{
//    uint8_t Lt_AlmFlag;
//    uint16_t Lt_i;
//    LtElem_st LtElem = {0};

//    for(;;)
//    {
//        Lt_AlmFlag = 0;
//        for(Lt_i = 1; Lt_i <= gLtList.st.length; Lt_i++)  //查询雷击信息
//        {
//            LtListFlashRead(Lt_i, &LtElem);
//            if(LtElem.Peak >= gFlashParam.st.SPD_Cur_Up)  //超过阈值
//            {
//                Lt_AlmFlag = 1;
//            }
//            osDelay(10);
//        }
//        if(Lt_AlmFlag != 0)
//        {
//            gParam.st.AlmState = 1;
//        }
//        else
//        {
//            gParam.st.AlmState = 0;
//        }

//        osDelay(800);
//    }
//}

///**
//  * @brief  新建雷电流线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_thunderTask(void)
//{
//    thunderTaskHandle = osThreadNew(ThunderTask, NULL, &thunderTask_attributes);
//    thunderAlmTaskHandle = osThreadNew(Check_ThunderAlm_Thread, NULL, &thunderAlmTask_attributes);
//}

///**
//  * @brief  Function implementing the uartTask thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void ThunderTask(void *argument)
//{
//    LOGD("app_thunder", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

//    LtElem_st LtElem = {0};
//    double lfv, lfb;
//    uint16_t lsv;
//    rtc_datetime_t datetime;

//    gpAppEvent = osEventFlagsNew(NULL);
//    LOGD("app_thunder", "gpAppEvent = 0x%08X", (uint32_t)gpAppEvent);
//    uint32_t thunderEvent = EVENT_THUNDER_P_HAPPENED | EVENT_THUNDER_N_HAPPENED;

//    gEnbleExtiTimer = xTimerCreate((const char *)"gEnbleExtiTimer",
//                                   (TickType_t  )TIME_OUT_TO_ENBLE_IRQ,
//                                   (UBaseType_t )pdFALSE,
//                                   (void *      )1,
//                                   (TimerCallbackFunction_t)EnbleExti_timer_callback_func);

//    /* Run the ADC calibration */
//    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
//    {
//        /* Calibration Error */
//        Error_Handler();
//    }

//    /* Start ADC conversion on regular group with transfer by DMA */
//    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)gParam.st.aADC1Val, ADC_CONVERT_SIZE) != HAL_OK)
//    {
//        /* Start Error */
//        Error_Handler();
//    }

//    for(;;)
//    {
//        BaseType_t ev = osEventFlagsWait(gpAppEvent, thunderEvent, osFlagsWaitAny, portMAX_DELAY);
//        //BaseType_t ev = osEventFlagsWait(gpAppEvent, thunderEvent, osFlagsWaitAny, 800);
//        LOGW("app_thunder", "In thunder_interrupt_task loop, happend : 0x%08X", (uint32_t)ev);

//        xTimerStart(gEnbleExtiTimer, 0);  //开启软件定时器，x ms后开启中断
//        if (ev < 0)
//        {
//            continue;
//        }
//        if(ev & thunderEvent)
//        {
//            for(uint16_t i = 0; i < KEEP_ADC_SIZE; i++)
//            {
//                LOGI("app_thunder", "Positive adc value[%d] = %d", i, gParam.st.aADCCh11Buf[i]);
//            }
//            for(uint16_t i = 0; i < KEEP_ADC_SIZE; i++)
//            {
//                LOGI("app_thunder", "Negative adc value[%d] = %d", i, gParam.st.aADCCh10Buf[i]);
//            }

//            //峰值 = K * adc + B
//            lsv = Handle_Adc_Val(&LtElem.Polar);
//            if(LtElem.Polar == 0) //positive thunder
//            {
//                if(lsv < gFlashParam.st.usADCCh11Threshold) //Invaid trig, default adc < 110
//                {
//                    continue;
//                }
//                else if(lsv < gFlashParam.st.Adc_K01_K02) //If adc < gFlashParam.st.Adc_K01_K02,use ADC01_K, else use ADC02_K
//                {
//                    lfv = gFlashParam.st.PAdc01_K;
//                    lfb = gFlashParam.st.PAdc01_B;
//                }
//                else //If adc >= gFlashParam.st.Adc_K01_K02, use ADC02_K
//                {
//                    lfv = gFlashParam.st.PAdc02_K;
//                    lfb = gFlashParam.st.PAdc02_B;
//                }
//            }
//            else //negative thunder
//            {
//                if(lsv < gFlashParam.st.usADCCh10Threshold)
//                {
//                    continue;
//                }
//                else if(lsv < gFlashParam.st.Adc_K01_K02)
//                {
//                    lfv = gFlashParam.st.NAdc01_K;
//                    lfb = gFlashParam.st.NAdc01_B;
//                }
//                else
//                {
//                    lfv = gFlashParam.st.NAdc02_K;
//                    lfb = gFlashParam.st.NAdc02_B;
//                }
//            }
//            lfv /= 1000.0;
//            lfv *= lsv;
//            lfb /= 10.0;
//            lfv += lfb;

//            LtElem.Peak = (uint16_t)lfv;
//            PrintLtElem(LtElem);

//            RTC_GetDatetime(&datetime);
//            LtElem.Year = datetime.year - 2000;
//            LtElem.Month = datetime.month;
//            LtElem.Day = datetime.day;
//            LtElem.Hour = datetime.hour;
//            LtElem.Minute = datetime.minute;
//            LtElem.Second = datetime.second;
//            LtElem.KeepTime = 0;
//            LtElem.reserved1 = 0;
//            LtElem.reserved2 = 0;

//            if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_M1AI)
//            {
//                LtElem.usQ = IPeak_Calc_Q(LtElem.Peak);
//                LtElem.usWR = IPeak_Calc_WR(LtElem.Peak);
//            }
//            else
//            {
//                LtElem.usQ = 0;
//                LtElem.usWR = 0;
//            }
//            osDelay(10); //在不打log情况下，不加此延迟会出现断电数据没保存的情况，原因不明
//            LtListFlashWrite(LtElem);
//        }
//    }
//}

///**
//  * @brief  计算电荷量Q
//  * @param  uint16_t ipeak 雷电流峰值(放大十倍:1.2kA的峰值则输入12)
//  * @retval 电荷量Q (放大十倍:1.2As的电荷量返回12)
//  */
//uint16_t IPeak_Calc_Q(uint16_t ipeak)
//{
//    float ftemp;
//    uint16_t thunder_Q;

//    ftemp = 0.5 * ipeak;
//    thunder_Q = (uint16_t)ftemp;

//    return thunder_Q;
//}

///**
//  * @brief  计算单位能量W/R
//  * @param  uint16_t ipeak 雷电流峰值(放大十倍:1.2kA的峰值则输入12)
//  * @retval 单位能量W/R (放大十倍:1.2kJ/Ω的单位能量返回12)
//  */
//uint16_t IPeak_Calc_WR(uint16_t ipeak)
//{
//    float ftemp;
//    uint32_t ultemp;
//    uint16_t thunder_WR;

//    ultemp = ipeak * ipeak;
//    ftemp = ultemp / 1000.0;
//    ftemp *= 25;
//    thunder_WR = (uint16_t)ftemp;

//    return thunder_WR;
//}

