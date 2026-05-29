
#ifndef __APP_LED_H
#define __APP_LED_H

/* Includes ------------------------------------------------------------------*/
#include "app_opts.h"

#include "cmsis_os.h"

/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/



/* Private defines -----------------------------------------------------------*/
#define  LED_1   0
#define  LED_2   1
#define  LED_3   2

#define  EVENT_LED1_FLASH  0x00000001UL
#define  EVENT_LED2_FLASH  0x00000002UL
#define  EVENT_LED3_FLASH  0x00000004UL







/* -------------------------------- -end- ----------------------------------- */
//extern volatile uint8_t WANCOMPLETE;

/* Private functions ---------------------------------------------------------*/
void LED_1_FlashOnce(void);
void LED_2_FlashOnce(void);
void LED_3_FlashOnce(void);

void LED_UART1_TX_FLASH(void);
void LED_UART1_RX_FLASH(void);
void LED_UART2_TX_FLASH(void);
void LED_UART2_RX_FLASH(void);

void osThreadNew_ledTask(void);

#endif /* __APP_LED_H */
