
#ifndef __MBTCP_H
#define __MBTCP_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>


/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/
extern uint8_t gMbTcpErrorTimes;

/* Private defines -----------------------------------------------------------*/
#define  MBTCP_ERROR_TIMES   12     //失败多少次即复位W5500

/* Private functions ---------------------------------------------------------*/
void osThreadNew_mbTcpTask(void);


#endif /* __MBTCP_H */
