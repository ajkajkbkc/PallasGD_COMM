
///* Private includes ----------------------------------------------------------*/
//#include "main.h"
//#include "cmsis_os.h"
//#include "timers.h"

//#include "app_log.h"
//#include "app_parameter.h"
//#include "app_tm1650.h"
//#include "app_key.h"
//#include "app_tool.h"


///* Private define ------------------------------------------------------------*/
//#define Read_YL()   HAL_GPIO_ReadPin(FS_YL_GPIO_Port, FS_YL_Pin)
//#define Read_KL()   HAL_GPIO_ReadPin(FS_KL_GPIO_Port, FS_KL_Pin)
//#define Read_PE()   HAL_GPIO_ReadPin(FS_PE_GPIO_Port, FS_PE_Pin)


///* Private variables ---------------------------------------------------------*/
//TimerHandle_t FS_LtTimesTimer = NULL;

///* Definitions for inputTask */
//osThreadId_t fs_inputTaskHandle;
//const osThreadAttr_t fs_inputTask_attributes =
//{
//    .name = "fs_inputTask",
//    .priority = (osPriority_t) sspd_inputTaskPriority,
//    .stack_size = 1024
//};


///* Private function prototypes -----------------------------------------------*/
//void FS_InputTask(void *argument);


///* Private user code ---------------------------------------------------------*/


///**
//  * @brief  输入检测判断
//  * @param  None
//  * @retval None
//  */
//static void FS_Input_Check(void)
//{
//    N_GET_PIN(gFlashParam.st.SPD_YL_Up, Read_YL(), gFS_Elem.st.YL);
//    N_GET_PIN(gFlashParam.st.SPD_KL_Up, Read_KL(), gFS_Elem.st.KL);
//    N_GET_PIN(gFlashParam.st.SPD_PE_Up, Read_PE(), gFS_Elem.st.Pe);

//    //LOGI("app_input", "YL=%d  KL=%d  Pe=%d", gFS_Elem.st.YL, gFS_Elem.st.KL, gFS_Elem.st.Pe);
//}

///**
//  * @brief  关闭中断
//  * @param  None
//  * @retval None
//  */
//static void FS_LtTimes_DisableIRQ(void)
//{
//    HAL_NVIC_DisableIRQ(FS_COUNT_EXTI_IRQn); // 关闭中断
//}

///**
//  * @brief  开启中断
//  * @param  None
//  * @retval None
//  */
//static void FS_LtTimes_EnableIRQ(void)
//{
//    LOGW("app_input", "open EXTI Enable");
//    __HAL_GPIO_EXTI_CLEAR_IT(FS_COUNT_Pin);  //需要先清除标志位，避免开启后会立刻进入中断
//    HAL_NVIC_EnableIRQ(FS_COUNT_EXTI_IRQn);
//}

///**
//  * @brief  enable exti
//  * @param  None
//  * @retval None
//  */
//static void enable_FS_LtTimes_exti(void *arg)
//{
//    FS_LtTimes_EnableIRQ();
//}

///**
//  * @brief  uart2 software timer init
//  * @param  None
//  * @retval None
//  */
//static void FS_LtTimes_timer_init(void)
//{
//    FS_LtTimesTimer = xTimerCreate((const char *)"FS_LtTimesTimer",
//                                   (TickType_t  )gFlashParam.st.LtTimesFilter,
//                                   (UBaseType_t )pdFALSE,
//                                   (void *      )4,
//                                   (TimerCallbackFunction_t)enable_FS_LtTimes_exti);
//}

///**
//  * @brief  start LtTiems timer
//  * @param  None
//  * @retval None
//  */
//static void FS_LtTimes_timer_start_FromISR(void)
//{
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//    if( xTimerStartFromISR( FS_LtTimesTimer, &xHigherPriorityTaskWoken ) != pdPASS )
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
//void extiTrig_FS_LtTimes_FromISR(void)
//{
//    gFS_Elem.st.LtNum++;

//    FS_LtTimes_DisableIRQ();
//    FS_LtTimes_timer_start_FromISR();
//}

///**
//  * @brief  新建线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_fs_inputTask(void)
//{
//    fs_inputTaskHandle = osThreadNew(FS_InputTask, NULL, &fs_inputTask_attributes);
//}

///**
//  * @brief  Function implementing the inputTask thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void FS_InputTask(void *argument)
//{
//    LOGD("input", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

//    FS_LtTimes_timer_init();
//    HAL_NVIC_EnableIRQ(FS_COUNT_EXTI_IRQn);  //中断内用了定时器函数，如果在定时器初始化前就进入中断，会死机

//    for(;;)
//    {
//        FS_Input_Check();

//        if(gKey1Mode == KEY_MODE_DEFAULT)
//        {
//            if(gFS_Elem.st.LtNum != gFlashParam.st.LtTimes)
//            {
//                if(gFS_Elem.st.LtNum > 9999)
//                {
//                    gFS_Elem.st.LtNum = 1;
//                }
//                gFlashParam.st.LtTimes = gFS_Elem.st.LtNum;
//                Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//                _Dispaly_num(gFlashParam.st.LtTimes);
//            }
//        }

//        osDelay(500);
//    }
//}





