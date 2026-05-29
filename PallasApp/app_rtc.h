
#ifndef __APP_RTC_H
#define __APP_RTC_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private defines -----------------------------------------------------------*/
#define SECONDS_IN_A_DAY    (86400U)
#define SECONDS_IN_A_HOUR   (3600U)
#define SECONDS_IN_A_MINUTE (60U)
#define DAYS_IN_A_YEAR      (365U)
#define YEAR_RANGE_START    (1970U)
#define YEAR_RANGE_END      (2099U)


/* Exported types ------------------------------------------------------------*/
typedef struct _rtc_datetime
{
    uint16_t year;  /*!< Range from 1970 to 2099.*/
    uint8_t month;  /*!< Range from 1 to 12.*/
    uint8_t day;    /*!< Range from 1 to 31 (depending on month).*/
    uint8_t hour;   /*!< Range from 0 to 23.*/
    uint8_t minute; /*!< Range from 0 to 59.*/
    uint8_t second; /*!< Range from 0 to 59.*/
    uint16_t msec;  /* 0 to 999 */
} rtc_datetime_t;


/* Exported constants --------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
HAL_StatusTypeDef RTC_SetDatetimeBySecond(uint32_t seconds);
void RTC_GetDatetime(rtc_datetime_t *datetime);
HAL_StatusTypeDef RTC_SetDatetime(rtc_datetime_t *datetime);
void ConvertSecondsToDatetime(uint32_t seconds, rtc_datetime_t *datetime, uint8_t flag);
uint32_t RTC_ReadDivider(RTC_HandleTypeDef *hrtc);


#endif /* __APP_RTC_H */
