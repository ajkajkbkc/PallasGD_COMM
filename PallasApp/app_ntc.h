
#ifndef __APP_NTC_H
#define __APP_NTC_H

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"


/* Exported types ------------------------------------------------------------*/
typedef enum
{
    USER_NTC_1 = 0,
    USER_NTC_2,
    USER_NTC_3,
    USER_NTC_4,
    USER_NTC_MAX
} user_ntc_t;


/* Exported constants --------------------------------------------------------*/



/* Private defines -----------------------------------------------------------*/



/* Private functions ---------------------------------------------------------*/
void osThreadNew_ntcTask(void);

extern osThreadId_t ntcTaskHandle;
#endif /* __APP_NTC_H */
