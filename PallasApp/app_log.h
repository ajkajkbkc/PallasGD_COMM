
#ifndef __APP_LOG_H
#define __APP_LOG_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "stm32f1xx_hal.h"
#include "app_opts.h"


/* Exported types ------------------------------------------------------------*/
#if (PRINT_LOG_OPEN == 1)
typedef enum
{
    ESP_LOG_NONE,       /*!< No log output */
    ESP_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    ESP_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;
#endif

/* Exported constants --------------------------------------------------------*/
#if (PRINT_LOG_OPEN == 1)
extern uint8_t gLogLevel;
#endif

/* Private defines -----------------------------------------------------------*/

#if ( ( (UART1_AS_LOG == 1) || (UART2_AS_LOG == 1) || (UART3_AS_LOG == 1) || (UART4_AS_LOG == 1) || (UART5_AS_LOG == 1) ) && (PRINT_LOG_OPEN == 1) )

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"

#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D       LOG_COLOR(LOG_COLOR_BLACK)
#define LOG_COLOR_V       LOG_BOLD(LOG_COLOR_BLACK)

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%d) %s: " format LOG_RESET_COLOR "\r\n"

#define LOGE( tag, format, ... )  if (gLogLevel >= ESP_LOG_ERROR)   { printf(LOG_FORMAT(E, format), HAL_GetTick(), tag, ##__VA_ARGS__); }
#define LOGW( tag, format, ... )  if (gLogLevel >= ESP_LOG_WARN)   { printf(LOG_FORMAT(W, format), HAL_GetTick(), tag, ##__VA_ARGS__); }
#define LOGI( tag, format, ... )  if (gLogLevel >= ESP_LOG_INFO)   { printf(LOG_FORMAT(I, format), HAL_GetTick(), tag, ##__VA_ARGS__); }
#define LOGD( tag, format, ... )  if (gLogLevel >= ESP_LOG_DEBUG)   { printf(LOG_FORMAT(D, format), HAL_GetTick(), tag, ##__VA_ARGS__); }
#define LOGV( tag, format, ... )  if (gLogLevel >= ESP_LOG_VERBOSE)   { printf(LOG_FORMAT(V, format), HAL_GetTick(), tag, ##__VA_ARGS__); }
#else
#define LOGE(...)
#define LOGW(...)
#define LOGI(...)
#define LOGD(...)
#define LOGV(...)
#endif

/* Private functions ---------------------------------------------------------*/
void uartlog_init(void);


#endif /* __APP_LOG_H */
