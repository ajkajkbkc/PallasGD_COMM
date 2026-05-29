
/* Private includes ----------------------------------------------------------*/
#include "main.h"


#include "app_log.h"
#include "app_opts.h"
#include "app_led.h"
#include "app_parameter.h"
#include "internet.h"
#include "module_ESE.h"
#include "plc_netcfg.h"
#include "kalyke_monitor_task.h"
/* Private define ------------------------------------------------------------*/
//volatile uint8_t WANCOMPLETE = 0;


/* Private variables ---------------------------------------------------------*/
/* Definitions for ledTask */
osThreadId_t led1_TaskHandle;
const osThreadAttr_t led1_Task_attributes =
{
    .name = "led1_Task",
    .priority = (osPriority_t) ledTaskPriority,
    .stack_size = 256
};

/* Definitions for ledTask */
osThreadId_t led2_TaskHandle;
const osThreadAttr_t led2_Task_attributes =
{
    .name = "led2_Task",
    .priority = (osPriority_t) ledTaskPriority,
    .stack_size = 256
};

volatile uint32_t glv_led_event = 0;

/* Private function prototypes -----------------------------------------------*/
void Led1_Task(void *argument);
void Led2_Task(void *argument);


/* Private user code ---------------------------------------------------------*/
/**
  * @brief  LED灯闪一次
  * @param  uint8_t led_num 第几个LED灯
  * @param  uint16_t freq 闪烁频率(unit: ms)
  * @retval None
  */
//void led_flash_once(uint8_t led_num, uint16_t freq)
//{
//    switch(led_num)
//    {
//    case LED_1:
//        LED_1_G();
//        osDelay(freq);
//        LED_1_R();
//        osDelay(freq);
//        break;

//    case LED_2:
//        LED_2_G();
//        osDelay(freq);
//        LED_2_R();
//        osDelay(freq);
//        break;
//    default:
//        break;
//    }
//}

///**
//  * @brief  LED1闪一下
//  * @param  None
//  * @retval None
//  */
//void LED_1_FlashOnce(void)
//{
////    glv_led_event |= EVENT_LED1_FLASH;
//}

///**
//  * @brief  LED2闪一下
//  * @param  None
//  * @retval None
//  */
//void LED_2_FlashOnce(void)
//{
////    glv_led_event |= EVENT_LED2_FLASH;
//}

//void LED_UART1_TX_FLASH(void)
//{
//#if   PROD_TYPE == PROD_FL || PROD_TYPE == PROD_FA || PROD_TYPE == PROD_FD
//    LED_1_FlashOnce();
//#elif PROD_TYPE == PROD_FG
//    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1)
//    {
//        LED_1_FlashOnce();
//    }
//#endif
//}

//void LED_UART1_RX_FLASH(void)
//{
//#if   PROD_TYPE == PROD_FL || PROD_TYPE == PROD_FA || PROD_TYPE == PROD_FD
//    LED_2_FlashOnce();
//#elif PROD_TYPE == PROD_FG
//    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1)
//    {
//        LED_2_FlashOnce();
//    }
//#endif
//}

//void LED_UART2_TX_FLASH(void)
//{
//#if PROD_TYPE == PROD_FG
//    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)
//    {
//        LED_1_FlashOnce();
//    }
//#endif
//}

//void LED_UART2_RX_FLASH(void)
//{
//#if PROD_TYPE == PROD_FG
//    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)
//    {
//        LED_2_FlashOnce();
//    }
//#endif
//}

/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_ledTask(void)
{
    led1_TaskHandle = osThreadNew(Led1_Task, NULL, &led1_Task_attributes);
    led2_TaskHandle = osThreadNew(Led2_Task, NULL, &led2_Task_attributes);
}

/**
  * @brief  Function implementing the ledTask thread.
  * @param  argument: Not used
  * @retval None
  */
void Led1_Task(void *argument)
{
    LOGD("app_led", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

    LED_1_G();
    vTaskDelay(3000);
    
    for(;;)
    {    
        if(gWANor4G == 0)
        {
            LED_1_R();
        }            
        else if(gWANor4G == 1)
        {
//            if(WANCOMPLETE == 1)
//            {
                LED_1_TOGGLE();
//            }
        }
        else if(gWANor4G == 2)
        {
//            if(WANCOMPLETE == 1)
//            {
                LED_1_TOGGLE();
//            }
        }
        
        osDelay(1000);
    }
}

void Led2_Task(void *argument)
{
    LOGD("app_led", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

    LED_2_G();
    vTaskDelay(3000);
    
    for(;;)
    {    
        if(READ_BIT(gParam.st.State_Alarm[StateAlm0], StateAlm0_ALL_Msk) || 
           READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_ALL_Msk) || 
           READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_ALL_Msk) || 
           READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_ALL_Msk))
        {
            LED_2_G();
        }
        else
        {
            LED_2_R();
        }
        osDelay(500);
    }
}

