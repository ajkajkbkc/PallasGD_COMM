
#ifndef __APP_MAIN_H
#define __APP_MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"


/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Private defines -----------------------------------------------------------*/
/*
    0: normal   1: error
    SYS_STATE_ATT7022       //ATT7022≥ı ºªØ

*/
/*!< NetLink_State registers */
/*******************  Bit definition for NetLink_State register  *******************/
//#define SYS_STATE_ATT7022_Pos                  (0U)
//#define SYS_STATE_ATT7022_Msk                  (0x1UL << SYS_STATE_ATT7022_Pos)
//#define SYS_STATE_ATT7022                      SYS_STATE_ATT7022_Msk

/* Private functions ---------------------------------------------------------*/
void osThreadNew_Task(void);
void TheDefaultTask(void);


#endif /* __APP_MAIN_H */
