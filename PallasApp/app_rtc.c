
/* Private includes ----------------------------------------------------------*/
#include "app_rtc.h"
#include "app_log.h"


/* Private define ------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/




/* Private function prototypes -----------------------------------------------*/



/* Private user code ---------------------------------------------------------*/
static uint32_t ConvertDatetimeToSeconds(const rtc_datetime_t *datetime)
{
    /* Number of days from begin of the non Leap-year*/
    uint16_t monthDays[] = {0U, 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};
    uint32_t seconds;

    /* Compute number of days from 1970 till given year*/
    seconds = (datetime->year - 1970U) * DAYS_IN_A_YEAR;
    /* Add leap year days */
    seconds += ((datetime->year / 4) - (1970U / 4));
    /* Add number of days till given month*/
    seconds += monthDays[datetime->month];
    /* Add days in given month. We subtract the current day as it is
     * represented in the hours, minutes and seconds field*/
    seconds += (datetime->day - 1);
    /* For leap year if month less than or equal to Febraury, decrement day counter*/
    if ((!(datetime->year & 3U)) && (datetime->month <= 2U))
    {
        seconds--;
    }

    seconds = (seconds * SECONDS_IN_A_DAY) + (datetime->hour * SECONDS_IN_A_HOUR) +
              (datetime->minute * SECONDS_IN_A_MINUTE) + datetime->second;

    return seconds;
}


void ConvertSecondsToDatetime(uint32_t seconds, rtc_datetime_t *datetime, uint8_t flag)
{
    LOGW("naiad_rtc", "Enter %s(), seconds = %u", __func__, seconds);
    uint32_t x;
    uint32_t secondsRemaining, days;
    uint16_t daysInYear;
    /* Table of days in a month for a non leap year. First entry in the table is not used,
     * valid months start from 1
     */
    uint8_t daysPerMonth[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

    /* Start with the seconds value that is passed in to be converted to date time format */
    secondsRemaining = seconds;

    /* Calcuate the number of days, we add 1 for the current day which is represented in the
     * hours and seconds field
     */
    days = secondsRemaining / SECONDS_IN_A_DAY + 1;

    /* Update seconds left*/
    secondsRemaining = secondsRemaining % SECONDS_IN_A_DAY;

    /* Calculate the datetime hour, minute and second fields */
    datetime->hour   = secondsRemaining / SECONDS_IN_A_HOUR;
    secondsRemaining = secondsRemaining % SECONDS_IN_A_HOUR;
    datetime->minute = secondsRemaining / 60U;
    datetime->second = secondsRemaining % SECONDS_IN_A_MINUTE;
    if (flag == 1)
    {
        return;
    }
    /* Calculate year */
    daysInYear     = DAYS_IN_A_YEAR;
    datetime->year = YEAR_RANGE_START;
    while (days > daysInYear)
    {
        /* Decrease day count by a year and increment year by 1 */
        days -= daysInYear;
        datetime->year++;

        /* Adjust the number of days for a leap year */
        if (datetime->year & 3U)
        {
            daysInYear = DAYS_IN_A_YEAR;
        }
        else
        {
            daysInYear = DAYS_IN_A_YEAR + 1;
        }
    }

    /* Adjust the days in February for a leap year */
    if (!(datetime->year & 3U))
    {
        daysPerMonth[2] = 29U;
    }

    for (x = 1U; x <= 12U; x++)
    {
        if (days <= daysPerMonth[x])
        {
            datetime->month = x;
            break;
        }
        else
        {
            days -= daysPerMonth[x];
        }
    }

    datetime->day = days;
}


/*!
 * brief Gets the RTC time and stores it in the given time structure.
 *
 * param datetime Pointer to the structure where the date and time details are stored.
 */
void RTC_GetDatetime(rtc_datetime_t *datetime)
{
    ConvertSecondsToDatetime(RTC_ReadTimeCounter(&hrtc), datetime, 0);
}


/* Copy from HAL_RTC_SetTime
*
*/
HAL_StatusTypeDef RTC_SetDatetimeBySecond(uint32_t seconds)
{
    //uint32_t counter_time = 0U, counter_alarm = 0U;
    uint32_t counter_time = 0U;

    /* Process Locked */
    __HAL_LOCK(&hrtc);

    hrtc.State = HAL_RTC_STATE_BUSY;

    counter_time = seconds;

    /* Write time counter in RTC registers */
    if (RTC_WriteTimeCounter(&hrtc, counter_time) != HAL_OK)
    {
        /* Set RTC state */
        hrtc.State = HAL_RTC_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(&hrtc);

        return HAL_ERROR;
    }
    else
    {
        /* Clear Second and overflow flags */
        CLEAR_BIT(hrtc.Instance->CRL, (RTC_FLAG_SEC | RTC_FLAG_OW));
#if 0
        /* Read current Alarm counter in RTC registers */
        counter_alarm = RTC_ReadAlarmCounter(hrtc);

        /* Set again alarm to match with new time if enabled */
        if (counter_alarm != RTC_ALARM_RESETVALUE)
        {
            if (counter_alarm < counter_time)
            {
                /* Add 1 day to alarm counter*/
                counter_alarm += (uint32_t)(24U * 3600U);

                /* Write new Alarm counter in RTC registers */
                if (RTC_WriteAlarmCounter(hrtc, counter_alarm) != HAL_OK)
                {
                    /* Set RTC state */
                    hrtc->State = HAL_RTC_STATE_ERROR;

                    /* Process Unlocked */
                    __HAL_UNLOCK(hrtc);

                    return HAL_ERROR;
                }
            }
        }
#endif
        hrtc.State = HAL_RTC_STATE_READY;

        __HAL_UNLOCK(&hrtc);

        return HAL_OK;
    }
}


HAL_StatusTypeDef RTC_SetDatetime(rtc_datetime_t *datetime)
{
    //uint32_t counter_time = 0U, counter_alarm = 0U;
    uint32_t counter_time = 0U;

    /* Process Locked */
    __HAL_LOCK(&hrtc);

    hrtc.State = HAL_RTC_STATE_BUSY;

    counter_time = ConvertDatetimeToSeconds(datetime);

    /* Write time counter in RTC registers */
    if (RTC_WriteTimeCounter(&hrtc, counter_time) != HAL_OK)
    {
        /* Set RTC state */
        hrtc.State = HAL_RTC_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(&hrtc);

        return HAL_ERROR;
    }
    else
    {
        /* Clear Second and overflow flags */
        CLEAR_BIT(hrtc.Instance->CRL, (RTC_FLAG_SEC | RTC_FLAG_OW));
#if 0
        /* Read current Alarm counter in RTC registers */
        counter_alarm = RTC_ReadAlarmCounter(hrtc);

        /* Set again alarm to match with new time if enabled */
        if (counter_alarm != RTC_ALARM_RESETVALUE)
        {
            if (counter_alarm < counter_time)
            {
                /* Add 1 day to alarm counter*/
                counter_alarm += (uint32_t)(24U * 3600U);

                /* Write new Alarm counter in RTC registers */
                if (RTC_WriteAlarmCounter(hrtc, counter_alarm) != HAL_OK)
                {
                    /* Set RTC state */
                    hrtc->State = HAL_RTC_STATE_ERROR;

                    /* Process Unlocked */
                    __HAL_UNLOCK(hrtc);

                    return HAL_ERROR;
                }
            }
        }
#endif
        hrtc.State = HAL_RTC_STATE_READY;

        __HAL_UNLOCK(&hrtc);

        return HAL_OK;
    }
}


/* µÃµ½ºÁÃëÊý */
uint32_t RTC_ReadDivider(RTC_HandleTypeDef *hrtc)
{
    uint16_t high1 = 0U, low = 0U;

    high1 = READ_REG(hrtc->Instance->DIVH & RTC_DIVH_RTC_DIV);
    low   = READ_REG(hrtc->Instance->DIVL & RTC_DIVL_RTC_DIV);

    uint32_t divVal = (((uint32_t) high1 << 16U) | low);
    LOGD("naiad_rtc", "divVal = %u", divVal);
    return (32767 - divVal) * 1000 / 32767;
}
