
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "app_log.h"
#include "app_parameter.h"
#include "internet.h"

#include "httputil.h"

#include "cmsis_os.h"



/* Private define ------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/
/* Definitions for httpsTask */
osThreadId_t httpsTaskHandle;
const osThreadAttr_t httpsTask_attributes =
{
    .name = "httpsTask",
    .priority = (osPriority_t) httpTaskPriority,
    .stack_size = 512
};



/* Private function prototypes -----------------------------------------------*/
void HttpsTask(void *argument);


/* Private user code ---------------------------------------------------------*/






/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_httpsTask(void)
{
    httpsTaskHandle = osThreadNew(HttpsTask, NULL, &httpsTask_attributes);
}

/**
  * @brief  Function implementing the HttpsTask thread.
  * @param  argument: Not used
  * @retval None
  */
void HttpsTask(void *argument)
{
    LOGD("https", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

    for(;;)
    {
        osDelay(100);
        if((gParam.st.NetLink_State & NET_STATE_OK) != 0)
        {
            continue ;
        }

        W5500MutexLock();
        do_https();
        W5500MutexUnlock();
    }
}





