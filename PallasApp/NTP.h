/*******************************************************************************

 *******************************************************************************/

#ifndef NTP_H_
#define NTP_H_

#include "stm32f1xx_hal.h"

#define NTP_TIMESTAMP_DELTA 2208988800ull //number of seconds between 1900 and 1970
#define SEC_TIME_ZONE              + (8*60*60)   //Beijing,GMT+8


extern volatile struct tm * Net_time;
void NTPUpdateFunTask(void);
void NTP_DataHandle(uint8_t *buf);
void RTC_Update(void);
void RTC_Show(void);


#endif /* NTP_H_ */
