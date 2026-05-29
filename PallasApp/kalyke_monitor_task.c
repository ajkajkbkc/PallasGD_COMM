/**
  ******************************************************************************
  * @file    kalyke_monitor_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-05
  * @brief   通过调试串口打印出RTOS的剩余HEAP的字节数
  ******************************************************************************
  */
#include <stdio.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "kalyke_event.h"

#include "kalyke_monitor_task.h"
//#include "fsl_tempmon.h"
//#include "fsl_snvs_hp.h"
//#include "fsl_debug_console.h"
//#include "bsp.h"
//#include "bsp_iwdg.h"
//#include "bsp_led.h"
//#include "bsp_flash.h"
#include "plc_element.h"
//#include "kalyke_version.h"
//#include "kalyke_tool.h"
#include "kalyke_internet_task.h"
//#include "kalyke_ota.h"
//#include "bsp_dct.h"
//#include "plc_sysinit.h"
#include "kalyke_4G_task.h"
//#include "kalyke_4G_TCP_task.h"
//#include "bsp_gpio.h"
//#include "kalyke_DS1302.h"
//#include "kalyke_sntp_task.h"
//#include "kalyke_ping_task.h" 
//#include "kalyke_http_task.h"

#include "queue.h"
//#include "plc_netcfg.h"
#include "app_log.h"
#include "app_tool.h"
#include "app_rtc.h"
#include "app_parameter.h"
#include "cmsis_os.h"
#include "mqtt.h"
#include "mb.h"

#include "app_att7022eu.h"
#include "app_oled.h"
#include "app_payload.h"
#include "module_ESE.h"
#include "NTP.h"
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
osThreadId_t MonitorTaskHandle;
const osThreadAttr_t monitor_attributes =
{
    .name = "kalyke_monitor_task",
    .priority = (osPriority_t) osPriorityAboveNormal5,
    .stack_size = 1024
};
/*******************************************************************************
 * Variables
 ******************************************************************************/
TaskHandle_t gKalykeMonitorTaskHandle;
TaskHandle_t gKalykeSecondTaskHandle;
QueueHandle_t gPublishQueue;

volatile uint32_t gKalykeSecondTick;//系统从第一次启动开始，总的运行秒数
volatile uint32_t gKalykeSecondTickCurrent = 0; //此次系统开机运行的秒数
volatile uint32_t gWorldSecondTick = 0; //从SNTP得到的秒数
volatile uint8_t aaa = 0;

volatile uint8_t UAHarmonici = 0;
volatile uint8_t UAHarmonicj = 0;
volatile uint8_t UBHarmonici = 0;
volatile uint8_t UBHarmonicj = 0;
volatile uint8_t UCHarmonici = 0;
volatile uint8_t UCHarmonicj = 0;

volatile uint8_t IAHarmonici = 0;
volatile uint8_t IAHarmonicj = 0;
volatile uint8_t IBHarmonici = 0;
volatile uint8_t IBHarmonicj = 0;
volatile uint8_t ICHarmonici = 0;
volatile uint8_t ICHarmonicj = 0;

volatile uint8_t masterTOalarm = 0;
extern uint32_t SystemCoreClock;

mqtt_config_array_hanyu_st* gpCycles;

#define MONITOR_TASK_STACK_SIZE         512
#define MONITOR_TASK_PRIO               3
/*******************************************************************************
 * Code
 ******************************************************************************/

#if (LOG_OPEN == 1)
static void testSomething(void)
{
    uint8_t buf[32] = {0};
    ip4_addr_t pingTarget;
    IP4_ADDR(&pingTarget, 192U, 168U, 0U, 83U);

    kalyke_start_ping(buf, pingTarget, 0);
}
#endif

typedef struct test
{
    char x1;
    short x2;
    float x3;
    char x4;
}test_t;

typedef struct _test2
{
    char x1;
    short x2;
    float x3;
    char x4;
}__attribute__((aligned(8))) test2_t;

void testAligned(void)
{
    //test_t tt;
    LOGE("monitor", "sizeof(test_t) = %u", sizeof(test_t));
    LOGE("monitor", "sizeof(test2_t) = %u", sizeof(test2_t));
}

//static void testDouble(void)
//{
//#if (PRINT_LOG_OPEN == 1)
//    double a = 9000.896914;
//    double b = 9000.896913;
//    LOGW("DOUBLE", "a = %f, b = %f", a, b);
//    if (a > b)
//    {
//        LOGE("DOUBLE", "a > b");
//    }
//    else if (a == b)
//    {
//        LOGE("DOUBLE", "a == b");
//    }
//    else
//    {
//        LOGE("DOUBLE", "a < b");
//    }
//#endif
//}

//static void tempInit(void)
//{
//    tempmon_config_t config;
//    //TEMPMON_GetDefaultConfig(&config);
//    /*
//     * This bits determines how many RTC clocks to wait before automatically repeating a temperature
//     * measurement. The pause time before remeasuring is the field value multiplied by the RTC period
//     */
//    config.frequency = 0x0001;
//    /* Default high alarm temperature */
//    config.highAlarmTemp = 80U;
//    /* Default panic alarm temperature */
//    config.panicAlarmTemp = 90U;
//    /* Default low alarm temperature */
//    config.lowAlarmTemp = 20U;

//    TEMPMON_Init(TEMPMON, &config);
//    //TEMPMON_StartMeasure(TEMPMON);
//}

//float KalykeGetTemp(void)
//{
//    float temp;

//    temp = TEMPMON_GetCurrentTemperature(TEMPMON);
//    //TEMPMON_Deinit(TEMPMON);
//    return temp;
//}

//static void showDeviceIDString(void)
//{
//    switch(gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId)
//    {
//        case FX_08R06AI08_C1:
//            printf("Yes, I am FX_08R06AI08_C1\r\n");
//            break;
//        case FX_08R06AI08_C2:
//            printf("Yes, I am FX_08R06AI08_C2\r\n");
//            break;

//        case FX_16R14_C1:
//            printf("Yes, I am FX_16R14_C1\r\n");
//            break;
//        case FX_16R14_C2:
//            printf("Yes, I am FX_16R14_C2\r\n");
//            break;

//        case FX_08R06_C1:
//            printf("Yes, I am FX_08R06_C1\r\n");
//            break;
//        case FX_08R06_C2:
//            printf("Yes, I am FX_08R06_C2\r\n");
//            break;

//        case ESX_C2:
//            printf("Yes, I am ESX_C2\r\n");
//            break;

//        case FW_C1:
//            printf("Yes, I am FW_C1\r\n");
//            break;
//        case FW28E_C1:
//            printf("Yes, I am FW28E_C1\r\n");
//            break;
//        case FW_C2:
//            printf("Yes, I am FW_C2\r\n");
//            break;
//        case CC_100:
//            printf("Yes, I am CC_100\r\n");
//            break;
//        case CC_120:
//            printf("Yes, I am CC_120\r\n");
//            break;
//        case CC_130:
//            printf("Yes, I am CC_130\r\n");
//            break;
//        default:
//            printf("OMG, Who Am I ?\r\n");
//            break;
//    }
//}

//void Kalyke_PrintRunFrequency(int32_t run_freq_only)
//{
//    printf("\r\n");
//    printf("***********************************************************\r\n");
//    printf("CPU:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));// 600MHz
//    printf("AHB:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));// 600MHz
//    printf("SEMC:            %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));// 75MHz
//    printf("IPG:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_IpgClk));// 150MHz
//    printf("PER:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_PerClk));// 75MHz


//    printf("OSC:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_OscClk));// 24MHz
//    printf("RTC:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_RtcClk));// 32768Hz

//    printf("ARMPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_ArmPllClk));// 1200MHz
//    if (!run_freq_only)
//    {
//        printf("USB1PLL:         %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllClk));// 480MHz
//        printf("USB1PLLPFD0:     %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk));// 720MHz
//        printf("USB1PLLPFD1:     %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd1Clk));// 246,857,130Hz
//        printf("USB1PLLPFD2:     %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd2Clk));// 332,307,684Hz
//        printf("USB1PLLPFD3:     %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd3Clk));// 576Mhz
//        printf("USB2PLL:         %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb2PllClk));// 24MHz

//        printf("SYSPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));// 528MHz
//        printf("SYSPLLPFD0:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));// 527,999,994Hz
//        printf("SYSPLLPFD1:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));// 594MHz
//        printf("SYSPLLPFD2:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));// 396MHz
//        printf("SYSPLLPFD3:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));// 594MHz

//        printf("ENETPLL0:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_EnetPll0Clk));// 50MHz
//        printf("ENETPLL1:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_EnetPll1Clk));// 25MHz
//        printf("ENETPLL2:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_EnetPll2Clk));// 25MHz

//        printf("AUDIOPLL:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AudioPllClk));// 24MHz
//        printf("VIDEOPLL:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_VideoPllClk));// 24MHz
//    }
//    printf("***********************************************************\r\n");
//    printf("\r\n");
//}

//void printSomeGPIOPriority(void)
//{
//    LOGV("Kalyke_monitor", "Enter %s()", __func__);
//    uint32_t pri = NVIC_GetPriority(X0_IRQ);
//    LOGD("Kalyke_monitor", "X0_IRQ = %u", pri);
//    pri = NVIC_GetPriority(X1_IRQ);
//    LOGD("Kalyke_monitor", "X1_IRQ = %u", pri);
//    pri = NVIC_GetPriority(X2_IRQ);
//    LOGD("Kalyke_monitor", "X2_IRQ = %u", pri);
//    pri = NVIC_GetPriority(X3_IRQ);
//    LOGD("Kalyke_monitor", "X3_IRQ = %u", pri);
//    pri = NVIC_GetPriority(X4_IRQ);
//    LOGD("Kalyke_monitor", "X4_IRQ = %u", pri);

//    pri = NVIC_GetPriority(SysTick_IRQn);
//    LOGD("Kalyke_monitor", "SysTick_IRQn = %u", pri);

//    LOGD("Kalyke_monitor", "__NVIC_PRIO_BITS = %u", __NVIC_PRIO_BITS);
//}

//void printSomeOCOTP(void)
//{
//    LOGV("Kalyke_monitor", "Enter %s()", __func__);
//    uint32_t misc = OCOTP->MISC_CONF1;
//    LOGW("Kalyke_monitor", "OCOTP->MISC_CONF1 = 0x%08X\r\n", misc);
//    uint32_t lock = OCOTP->LOCK;
//    LOGW("Kalyke_monitor", "OCOTP->LOCK = 0x%08X\r\n", lock);

//    /* BOOT_CFG1 ~ BOOT_CFG4 */
//    uint32_t val = OCOTP->CFG4;
//    LOGW("Kalyke_monitor", "offset: 0x450, OCOTP->CFG4 = 0x%08X\r\n", val);

//    val = OCOTP->CFG5;
//    LOGW("Kalyke_monitor", "offset: 0x460, OCOTP->CFG5 = 0x%08X\r\n", val);
//    val = OCOTP->CFG6;
//    LOGW("Kalyke_monitor", "offset: 0x470, OCOTP->CFG6 = 0x%08X\r\n", val);
//    val = OCOTP->MAC0;
//    LOGW("Kalyke_monitor", "offset: 0x620, OCOTP->MAC0 = 0x%08X\r\n", val);
//    val = OCOTP->MAC1;
//    LOGW("Kalyke_monitor", "offset: 0x630, OCOTP->MAC1 = 0x%08X\r\n", val);
//    val = OCOTP->GP1;
//    LOGW("Kalyke_monitor", "offset: 0x660, OCOTP->GP1 = 0x%08X\r\n", val);
//    val = OCOTP->GP2;
//    LOGW("Kalyke_monitor", "offset: 0x670, OCOTP->GP2 = 0x%08X\r\n", val);
//    val = OCOTP->SW_GP1;
//    LOGW("Kalyke_monitor", "offset: 0x680, OCOTP->SW_GP1 = 0x%08X\r\n", val);
//    val = OCOTP->SW_GP20;
//    LOGW("Kalyke_monitor", "offset: 0x690, OCOTP->SW_GP20 = 0x%08X\r\n", val);
//    val = OCOTP->MISC_CONF0;
//    LOGI("Kalyke_monitor", "offset: 0x6D0, OCOTP->MISC_CONF0 = 0x%08X\r\n", val);
//    val = OCOTP->GP30;
//    LOGI("Kalyke_monitor", "offset: 0x880, OCOTP->GP30 = 0x%08X\r\n", val);
//    val = OCOTP->GP40;
//    LOGI("Kalyke_monitor", "offset: 0x8C0, OCOTP->GP40 = 0x%08X\r\n", val);

//    val = SRC->SBMR1;
//    LOGI("Kalyke_monitor", "SRC->SBMR1 = 0x%08X\r\n", val);
//    val = SRC->SBMR2;
//    LOGI("Kalyke_monitor", "SRC->SBMR2 = 0x%08X\r\n", val);
//    val = SRC->SRSR;
//    LOGI("Kalyke_monitor", "SRC->SRSR = 0x%08X\r\n", val);
//}

//void print_vTaskList(const char *func, int line)
//{
//    LOGW("Monitor", "Enter %s(), func = %s, line = %d", __func__, func, line);
//    char *buffers = pvPortMalloc(2048);
//    if (buffers)
//    {
//        buffers[0] = '\r';
//        buffers[1] = '\n';
//        vTaskList(buffers + 2);
//        printf("%s", buffers);
//        vPortFree(buffers);
//    }
//}

uint8_t get_WeekDayNum(uint32_t nYear, uint8_t nMonth, uint8_t nDay)
{
  uint32_t year = 0U, weekday = 0U;

  //year = 2000U + nYear;
  year = nYear;

  if (nMonth < 3U)
  {
    /*D = { [(23 x month)/9] + day + 4 + year + [(year-1)/4] - [(year-1)/100] + [(year-1)/400] } mod 7*/
    weekday = (((23U * nMonth) / 9U) + nDay + 4U + year + ((year - 1U) / 4U) - ((year - 1U) / 100U) + ((year - 1U) / 400U)) % 7U;
  }
  else
  {
    /*D = { [(23 x month)/9] + day + 4 + year + [year/4] - [year/100] + [year/400] - 2 } mod 7*/
    weekday = (((23U * nMonth) / 9U) + nDay + 4U + year + (year / 4U) - (year / 100U) + (year / 400U) - 2U) % 7U;
  }

  return (uint8_t)weekday;
}

//void kalyke_monitor_init(void)
//{
//    snvs_hp_rtc_datetime_t rtcDate;
//    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
//    SET_SD_ELEMENT_VALUE(100, rtcDate.year);
//    SET_SD_ELEMENT_VALUE(101, rtcDate.month);
//    SET_SD_ELEMENT_VALUE(102, rtcDate.day);
//    SET_SD_ELEMENT_VALUE(103, rtcDate.hour);
//    SET_SD_ELEMENT_VALUE(104, rtcDate.minute);
//    SET_SD_ELEMENT_VALUE(105, rtcDate.second);
//    SET_SD_ELEMENT_VALUE(106, get_WeekDayNum(rtcDate.year, rtcDate.month, rtcDate.day));
//}

#if (ONLY_READ_TIME_FROM_DS1302 == 1)
static void kalyke_second_task(void *p_arg)
{
    LOGV("second_task", "kalyke_second_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    kalyke_ds1302_init();
    vTaskDelay(10000);
    uint32_t *pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD200];
    uint32_t *pSD202 = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD202];
    snvs_hp_rtc_datetime_t rtcDate;
    uint16_t second = 0;
    
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    LOGW("second_task", "KALYKE_EVENT_ENET_INIT_DONE_PLC  Happened.");
    //SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
    kalyke_get_ds1302_times(&rtcDate);
    SET_SD_ELEMENT_VALUE(100, rtcDate.year);
    SET_SD_ELEMENT_VALUE(101, rtcDate.month);
    SET_SD_ELEMENT_VALUE(102, rtcDate.day);
    SET_SD_ELEMENT_VALUE(103, rtcDate.hour);
    SET_SD_ELEMENT_VALUE(104, rtcDate.minute);
    SET_SD_ELEMENT_VALUE(105, rtcDate.second);
    SET_SD_ELEMENT_VALUE(106, get_WeekDayNum(rtcDate.year, rtcDate.month, rtcDate.day));
    for (;;)
    {
        vTaskDelay(1000); // 延时1秒钟
        gKalykeSecondTick++;
        *pSD = gKalykeSecondTick;
        *pSD202 = gKalykeSecondTickCurrent++;
        gWorldSecondTick++;

        second = GET_SD_ELEMENT_VALUE(105);
        second++;
        if (second > 59)
        {
            kalyke_get_ds1302_times(&rtcDate);
            SET_SD_ELEMENT_VALUE(100, rtcDate.year);
            SET_SD_ELEMENT_VALUE(101, rtcDate.month);
            SET_SD_ELEMENT_VALUE(102, rtcDate.day);
            SET_SD_ELEMENT_VALUE(103, rtcDate.hour);
            SET_SD_ELEMENT_VALUE(104, rtcDate.minute);
            SET_SD_ELEMENT_VALUE(105, rtcDate.second);
            SET_SD_ELEMENT_VALUE(106, get_WeekDayNum(rtcDate.year, rtcDate.month, rtcDate.day));
            second = rtcDate.second;
        }
        else
        {
            SET_SD_ELEMENT_VALUE(105, second);
        }
    }
}
#else
//static void kalyke_second_task(void *p_arg)
//{
//    //vTaskDelay(10000);
//    vTaskDelay(3000);
//    uint32_t *pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD200];
//    uint32_t *pSD202 = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD202];
//    snvs_hp_rtc_datetime_t rtcDate;
//    uint16_t second = 0;

//    LOGV("Kalyke_monitor", "kalyke_second_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
//    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
//    LOGW("Kalyke_monitor", "KALYKE_EVENT_ENET_INIT_DONE_PLC  Happened.");
//    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
//    SET_SD_ELEMENT_VALUE(100, rtcDate.year);
//    SET_SD_ELEMENT_VALUE(101, rtcDate.month);
//    SET_SD_ELEMENT_VALUE(102, rtcDate.day);
//    SET_SD_ELEMENT_VALUE(103, rtcDate.hour);
//    SET_SD_ELEMENT_VALUE(104, rtcDate.minute);
//    SET_SD_ELEMENT_VALUE(105, rtcDate.second);
//    SET_SD_ELEMENT_VALUE(106, get_WeekDayNum(rtcDate.year, rtcDate.month, rtcDate.day));
//    for(;;)
//    {
//        vTaskDelay(1000);
//        gKalykeSecondTick++;
//        *pSD = gKalykeSecondTick;
//        *pSD202 = gKalykeSecondTickCurrent++;
//        gWorldSecondTick++;
//        second = GET_SD_ELEMENT_VALUE(105);
//        second++;
//        if (second > 59)
//        {
//            SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
//            SET_SD_ELEMENT_VALUE(100, rtcDate.year);
//            SET_SD_ELEMENT_VALUE(101, rtcDate.month);
//            SET_SD_ELEMENT_VALUE(102, rtcDate.day);
//            SET_SD_ELEMENT_VALUE(103, rtcDate.hour);
//            SET_SD_ELEMENT_VALUE(104, rtcDate.minute);
//            SET_SD_ELEMENT_VALUE(105, rtcDate.second);
//            SET_SD_ELEMENT_VALUE(106, get_WeekDayNum(rtcDate.year, rtcDate.month, rtcDate.day));
//            second = rtcDate.second;
//        }
//        else
//        {
//            SET_SD_ELEMENT_VALUE(105, second);
//        }
//    #if 0 // LiXianyu 20220808
//        if (gKalykeSecondTick % 30 == 0)
//        {
//            taskENTER_CRITICAL();
//            bsp_save_KalykeSecondTick(gKalykeSecondTick);
//            taskEXIT_CRITICAL();
//        }
//    #endif
//    }
//}
#endif

//static void start_second_task(void)
//{
//    gKalykeSecondTick = bsp_get_KalykeSecondTick();
//    if (gKalykeSecondTick == 0xFFFFFFFF) //系统第一次运行，初始化为0
//    {
//        bsp_save_KalykeSecondTick(0);
//        gKalykeSecondTick = 0;
//    }
//    srand(gKalykeSecondTick);
//    xTaskCreate((TaskFunction_t)kalyke_second_task,
//                (const char *)"second_task",
//                SECOND_TASK_STACK_SIZE,
//                (void *)NULL,
//                SECOND_TASK_PRIO,
//                &gKalykeSecondTaskHandle);
//}

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
void show_task_list(void)
{
    char *buffers = pvPortMalloc(2048);
    buffers[0] = '\r';
    buffers[1] = '\n';
    vTaskList(buffers + 2);
//    printf("%s", buffers);
    vPortFree(buffers);
}
#endif

//static void printf_int_in_binary(int val)
//{
//    LOGV("monitor", "%s(%d)", __func__, val);
//    unsigned char *p = (unsigned char*)&val + 3; //从低位到高位,低端字节计算机
//    for(int k = 0; k <= 3; k++)
//    {
//        int val2 = *(p - k);
//        for (int i = 7; i >= 0; i--)
//        {
//            if(val2 & (1 << i))
//            {
//                printf("1");
//            }
//            else
//            {
//                printf("0");
//            }
//        }
//        printf(" ");
//    }
//    printf("\r\n");
//}

//static void printf_float_in_binary(float val)
//{
//    LOGV("monitor", "%s(%f)", __func__, val);
//    unsigned char *p = (unsigned char*)&val + 3; //从低位到高位,低端字节计算机
//    for(int k = 0; k <= 3; k++)
//    {
//        int val2 = *(p - k);
//        for (int i = 7; i >= 0; i--)
//        {
//            if(val2 & (1 << i))
//            {
//                printf("1");
//            }
//            else
//            {
//                printf("0");
//            }
//        }
//        printf(" ");
//    }
//    printf("\r\n");
//}

static int16_t getElementValue_int16(mqtt_config_array_st *pCfg)
{
    //LOGD("monitor", "Enter %s(), address = %u", __func__, pCfg->address);
    
    int16_t val = 0;
    if (pCfg->element == ELEM_D) //(memcmp(pCfg->element, "D", 1) == 0)
    {
        val =  GET_D_ELEMENT_VALUE(pCfg->address);
        //LOGE("monitor", "ELEM_D = %d", gtv_PlcElement.msp_DElement[pCfg->address]);
        //LOGE("monitor", "ELEM_D = %d", val);
    }
    else if (pCfg->element == ELEM_R) //(memcmp(pCfg->element, "R", 1) == 0)
    {
        val =  GET_R_ELEMENT_VALUE(pCfg->address);
        LOGE("monitor", "ELEM_R = %d", val);
    }
    else if (pCfg->element == ELEM_C) //(memcmp(pCfg->element, "C", 1) == 0)
    {
        val =  GET_C16_CURRENT_VALUE(pCfg->address);
    }
    else if (pCfg->element == ELEM_SD) //(memcmp(pCfg->element, "SD", 2) == 0)
    {
        val =  GET_SD_ELEMENT_VALUE(pCfg->address);
    }
    return val;
}

static char* getElementValue_string(mqtt_config_array_st *pCfg)
{
    static char sRet[32];
     unsigned short lsv_Temp;
    if (pCfg->element == ELEM_D) //(memcmp(pCfg->element, "D", 1) == 0)
    {
        for (int i = 0;i < 32;i++)
        {
           lsv_Temp =  GET_D_ELEMENT_VALUE(pCfg->address + i);

           sRet[2*i] = lsv_Temp&0xFF;
           if((lsv_Temp&0xFF) == 0x00) 
              break;
           sRet[2*i + 1] = ((lsv_Temp>>8)&0xFF);
           if(((lsv_Temp>>8)&0xFF) == 0x00) 
              break;
        }
    }
    else if (pCfg->element == ELEM_R) //(memcmp(pCfg->element, "R", 1) == 0)
    {
        for (int i = 0;i < 32;i++)
        {
           lsv_Temp =  GET_R_ELEMENT_VALUE(pCfg->address + i);

           sRet[2*i] = lsv_Temp&0xFF;
           if((lsv_Temp&0xFF) == 0x00) 
              break;

           sRet[2*i + 1] = ((lsv_Temp>>8)&0xFF);
           if(((lsv_Temp>>8)&0xFF) == 0x00) 
              break;
        }
    }
    LOGE("Kalyke_monitor", "getElementValue_string = %s\r\n", sRet);
    return sRet;
}

static int32_t getElementValue_int32(mqtt_config_array_st *pCfg)
{
#if 0
    uint16_t low16 = GET_D_ELEMENT_VALUE(pCfg->address);
    uint16_t high16 = GET_D_ELEMENT_VALUE(pCfg->address+1);
    int32_t val = (high16 << 16) | low16;
#else
    int32_t *pval = 0;
    if (pCfg->element == ELEM_D) //(memcmp(pCfg->element, "D", 1) == 0)
    {
        pval =  (int32_t *)&gtv_PlcElement.msp_DElement[pCfg->address];
    }
    else if (pCfg->element == ELEM_C) //(memcmp(pCfg->element, "C", 1) == 0)
    {
        *pval = (int32_t)GET_C32_CURRENT_VALUE(pCfg->address);
        LOGV("monitor", "*pval = %d", *pval);
    }
    else if (pCfg->element == ELEM_R) //(memcmp(pCfg->element, "R", 1) == 0)
    {
        pval = (int32_t *)&gtv_PlcElement.msp_RElement[pCfg->address];
    }
    else if (pCfg->element == ELEM_SD) //(memcmp(pCfg->element, "SD", 2) == 0)
    {
        pval =  (int32_t *)&gtv_PlcElement.msp_SDElement[pCfg->address];
    }
#endif
    return *pval;
}

static uint8_t getElementValue_bool(mqtt_config_array_st *pCfg)
{
    uint8_t val = 0;

    if (pCfg->element == ELEM_M) //(memcmp(pCfg->element, "M", 1) == 0)
    {
        val = plc_get_bit_element_value(M_ELEMENT, pCfg->address);
    }
    else if (pCfg->element == ELEM_X) //(memcmp(pCfg->element, "X", 1) == 0)
    {
        val = plc_get_bit_element_value(X_ELEMENT, pCfg->address);
    }
    else if (pCfg->element == ELEM_Y) //(memcmp(pCfg->element, "Y", 1) == 0)
    {
        val = plc_get_bit_element_value(Y_ELEMENT, pCfg->address);
    }
    else if (pCfg->element == ELEM_S) //(memcmp(pCfg->element, "S", 1) == 0)
    {
        val = plc_get_bit_element_value(S_ELEMENT, pCfg->address);
    }
    else if (pCfg->element == ELEM_SM) //(memcmp(pCfg->element, "SM", 1) == 0)
    {
        val = plc_get_bit_element_value(SM_ELEMENT, pCfg->address);
    }

    return val;
}

static float getElementValue_float(mqtt_config_array_st *pCfg)
{
    //LOGD("monitor", "Enter %s(), address = %u", __func__, pCfg->address);

    uint16_t au[2];
    float *pval = 0;
    if (pCfg->element == ELEM_C) //(memcmp(pCfg->element, "C", 1) == 0)
    {
        *pval = (float)GET_C32_CURRENT_VALUE(pCfg->address);
        goto EXIT_ME;
    }

    if (pCfg->address % 2 == 0)
    {
        if (pCfg->element == ELEM_D) //(memcmp(pCfg->element, "D", 1) == 0)
        {
            //uint16_t low16 = GET_D_ELEMENT_VALUE(pCfg->address);
            //uint16_t high16 = GET_D_ELEMENT_VALUE(pCfg->address+1);
            pval = (float *)&gtv_PlcElement.msp_DElement[pCfg->address];
            //LOGE("monitor", "low16 = %u, high16 = %u, val = %f", low16, high16, *pval);
            //printf_float_in_binary(*pval);
        }
        else if (pCfg->element == ELEM_R) //(memcmp(pCfg->element, "R", 1) == 0)
        {
            pval = (float *)&gtv_PlcElement.msp_RElement[pCfg->address];
        }
        else if (pCfg->element == ELEM_SD) //(memcmp(pCfg->element, "SD", 2) == 0)
        {
            pval =  (float *)&gtv_PlcElement.msp_SDElement[pCfg->address];
        }
    }
    else
    {
        LOGW("monitor", "pCfg->address %% 2 != 0");
        if (pCfg->element == ELEM_D) //(memcmp(pCfg->element, "D", 1) == 0)
        {
            au[0] = GET_D_ELEMENT_VALUE(pCfg->address);
            au[1] = GET_D_ELEMENT_VALUE(pCfg->address + 1);
            pval = (float *)&au[0];
        }
        else if (pCfg->element == ELEM_R) //(memcmp(pCfg->element, "R", 1) == 0)
        {
            au[0] = GET_R_ELEMENT_VALUE(pCfg->address);
            au[1] = GET_R_ELEMENT_VALUE(pCfg->address + 1);
            pval = (float *)&au[0];
        }
        else if (pCfg->element == ELEM_SD) //(memcmp(pCfg->element, "SD", 2) == 0)
        {
            au[0] = GET_SD_ELEMENT_VALUE(pCfg->address);
            au[1] = GET_SD_ELEMENT_VALUE(pCfg->address + 1);
            pval = (float *)&au[0];
        }
        LOGE("monitor", "*pval = %f", *pval);
    }

EXIT_ME:
    return *pval;
}

//void kalyke_post_to_mqtt(rtc_datetime_t *pTime, uint32_t milliSecond, double tempeture, uint32_t freeheap)
//{
//    LOGW("Kalyke_monitor", "Enter %s(), vender = %s", __func__, g_plc_netcfg.mqtt.vender);
//    //float fVal;
//    char *tempStr = pvPortMalloc(1024);
//    char *mqttBuf = pvPortMalloc(4096);

//    memset(mqttBuf, 0, 1024);
//    if (memcmp(g_plc_netcfg.mqtt.vender, "TLINK", 5) == 0)
//    {
//        strcpy(mqttBuf, "{\"sensorDatas\":[");
//        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
//        {
//            if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_F32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "float32") == 0)
//            {
//                sprintf(tempStr, "{\"value\":%.3f},", getElementValue_float(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I16)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int16") == 0)
//            {
//                sprintf(tempStr, "{\"value\":%d},", getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int32") == 0)
//            {
//                sprintf(tempStr, "{\"value\":%d},", getElementValue_int32(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_BOOL)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "bool") == 0)
//            {
//                sprintf(tempStr, "{\"value\":%d},", getElementValue_bool(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_STRING)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "string") == 0)
//            {
//                sprintf(tempStr, "{\"value\":\"%s\"},", getElementValue_string(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_SYS)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "sys") == 0)
//            {
//                if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "second") == 0)
//                {
//                    sprintf(tempStr, "{\"value\":%u},", gKalykeSecondTickCurrent);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "tempeture") == 0)
//                {
//                    sprintf(tempStr, "{\"value\":%.1f},", tempeture);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "bootTime") == 0)
//                {
//                    sprintf(tempStr, "{\"value\":%u},", bsp_get_kalyke_boot_time());
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "SW_VERSION") == 0)
//                {
//                    sprintf(tempStr, "{\"str\":%s},", SW_VERSION);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "secondAll") == 0)
//                {
//                    sprintf(tempStr, "{\"value\":%u},", gKalykeSecondTick);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "free_heap") == 0)
//                {
//                    sprintf(tempStr, "{\"value\":%u},", freeheap);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "ucode_len") == 0)
//                {
//                    sprintf(tempStr, "{\"value\":%u},", gUcodeLen);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "DeviceID") == 0)
//                {
//                    char device_id[32] = {0};
//                    memcpy(device_id, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
//                    sprintf(tempStr, "{\"str\":\"%s\"},", device_id);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "mcu_id") == 0)
//                {
//                    sprintf(tempStr, "{\"str\":\"%016llX\"},", gMcuID);
//                }
//            #if (KALYKE_FEATURE_4G_TASK == 1)
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "gps_state") == 0)
//                {
//                    sprintf(tempStr, "{\"value\":%u},", gGpsValue.gpsState);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "satellite_count") == 0)
//                {
//                    sprintf(tempStr, "{\"value\":%u},", gGpsValue.satelliteCount);
//                }
//            #endif
//            }
//            else // int16 default
//            {
//                sprintf(tempStr, "{\"value\":%d},", getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            strcat(mqttBuf, tempStr);
//        }
//        int len = strlen(mqttBuf);
//        mqttBuf[len - 1] = '\0';
//        strcat(mqttBuf, "]}");
//        //LOGV("Kalyke_monitor", "len = %u, mqttBuf = %s", len, mqttBuf);
//    }
//    else if (memcmp(g_plc_netcfg.mqtt.vender, "ROOTMQTT", 8) == 0)
//    {
//        strcpy(mqttBuf, "{");
//        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
//        {
//            if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_F32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "float32") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%.3f,",
//                    g_plc_netcfg.mqtt.pConfigs[i].name,
//                    getElementValue_float(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I16)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int16") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,",
//                    g_plc_netcfg.mqtt.pConfigs[i].name,
//                    getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int32") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,",
//                    g_plc_netcfg.mqtt.pConfigs[i].name,
//                    getElementValue_int32(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_BOOL)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "bool") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,",
//                    g_plc_netcfg.mqtt.pConfigs[i].name,
//                    getElementValue_bool(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_STRING)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "string") == 0)
//            {
//                sprintf(tempStr, "\"%s\":\"%s\",",
//                    g_plc_netcfg.mqtt.pConfigs[i].name,
//                    getElementValue_string(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else // int16 default
//            {
//                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//                tempStr[0] = '\0';
//            }
//            strcat(mqttBuf, tempStr);
//        }
//        sprintf(tempStr, "\"TIMESTAMP_LOCAL\":\"%04u-%02u-%02u %02u:%02u:%02u.%03u\"}", pTime->year, pTime->month, pTime->day, pTime->hour, pTime->minute, pTime->second, milliSecond);
//        strcat(mqttBuf, tempStr);
//        LOGI("Kalyke_monitor", "mqttBuf(ROOTMQTT) = %s", mqttBuf);
//    }
//    else if (memcmp(g_plc_netcfg.mqtt.vender, "AliMQTT", 7) == 0)
//    {
//        uint32_t worldSec = gWorldSecondTick;
//    #if 0
//        sprintf(mqttBuf, "{\"id\":\"%u\",\"version\":\"1.0\",\"params\":{\"second\":{\"value\":%d,\"time\":%u000},\"secondAll\":{\"value\":%d,\"time\":%u000}},\"method\":\"thing.event.property.post\"}",
//                            gKalykeSecondTick, gKalykeSecondTickCurrent, worldSec, gKalykeSecondTick, worldSec);
//    #endif
//        sprintf(mqttBuf, "{\"id\":\"%u\",\"version\":\"1.0\",\"params\":{", gKalykeSecondTick);
//        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
//        {
//            if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_F32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "float32") == 0)
//            {
//                sprintf(tempStr, "\"%s\":{\"value\":%.3f,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_float(&g_plc_netcfg.mqtt.pConfigs[i]), worldSec);
//                LOGV("monitor", "float32: %s", tempStr);
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I16)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int16") == 0)
//            {
//                sprintf(tempStr, "\"%s\":{\"value\":%d,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]), worldSec);
//                LOGD("monitor", "int16: %s", tempStr);
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int32") == 0)
//            {
//                sprintf(tempStr, "\"%s\":{\"value\":%d,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int32(&g_plc_netcfg.mqtt.pConfigs[i]), worldSec);
//                LOGI("monitor", "int32: %s", tempStr);
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_BOOL)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "bool") == 0)
//            {
//                sprintf(tempStr, "\"%s\":{\"value\":%d,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_bool(&g_plc_netcfg.mqtt.pConfigs[i]), worldSec);
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_STRING)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "string") == 0)
//            {
//                sprintf(tempStr, "\"%s\":{\"value\":\"%s\",\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_string(&g_plc_netcfg.mqtt.pConfigs[i]), worldSec);
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_SYS)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "sys") == 0)
//            {
//                if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "second") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":%d,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, gKalykeSecondTickCurrent, worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "tempeture") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":%.1f,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, tempeture, worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "bootTime") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":%u,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, bsp_get_kalyke_boot_time(), worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "SW_VERSION") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":\"%s\",\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, SW_VERSION, worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "secondAll") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":%u,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, gKalykeSecondTick, worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "free_heap") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":%u,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, freeheap, worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "ucode_len") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":%u,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, gUcodeLen, worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "DeviceID") == 0)
//                {
//                    char device_id[32] = {0};
//                    memcpy(device_id, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
//                    sprintf(tempStr, "\"%s\":{\"value\":\"%s\",\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, device_id, worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "mcu_id") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":\"%016llX\",\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, gMcuID, worldSec);
//                }
//            #if (KALYKE_FEATURE_4G_TASK == 1)
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "gps_state") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":%u,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, gGpsValue.gpsState, worldSec);
//                }
//                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[i].name, "satellite_count") == 0)
//                {
//                    sprintf(tempStr, "\"%s\":{\"value\":%u,\"time\":%u000},", g_plc_netcfg.mqtt.pConfigs[i].name, gGpsValue.satelliteCount, worldSec);
//                }
//            #endif
//            }
//            else // int16 default
//            {
//                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//                tempStr[0] = '\0';
//            }
//            strcat(mqttBuf, tempStr);
//        }
//    #if (KALYKE_FEATURE_4G_TASK == 1)
//        if (bspIsHave4G() && gGpsValue.gpsState)
//        {
//            sprintf(tempStr, "\"GeoLocation\":{\"value\":{\"Longitude\":%f,\"Latitude\":%f,\"Altitude\":%.1f,\"CoordinateSystem\":2},\"time\":%u000},", gGpsValue.longitude, gGpsValue.latitude, gGpsValue.altitude, worldSec);
//            strcat(mqttBuf, tempStr);
//        }
//    #endif
//        strcat(mqttBuf, "\"method\":\"thing.event.property.post\"}}");
//        LOGV("Kalyke_monitor", "mqttBuf = %s", mqttBuf);
//    }
//    else if (memcmp(g_plc_netcfg.mqtt.vender, "HANYU", 5) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0 )
//    {
//        LOGV("Kalyke_monitor", "g_plc_netcfg.mqtt.configLength = %d", g_plc_netcfg.mqtt.configLength);
//        strcpy(mqttBuf, "{");
//        sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"Data\":[", pTime->year, pTime->month, pTime->day, pTime->hour, pTime->minute, pTime->second);
//        strcat(mqttBuf, tempStr);

//        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
//        {
//            if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_F32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "float32") == 0)
//            {
//                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%.3f},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_float(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I16)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int16") == 0)
//            {
//                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int32") == 0)
//            {
//                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int32(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_BOOL)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "bool") == 0)
//            {
//                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_bool(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_STRING)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "string") == 0)
//            {
//                sprintf(tempStr, "{\"name\":\"%s\",\"value\":\"%s\"},", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_string(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else // int16 default
//            {
//                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//                tempStr[0] = '\0';
//            }
//            strcat(mqttBuf, tempStr);
//        }
//        int len = strlen(mqttBuf);
//        mqttBuf[len - 1] = '\0';
//        strcat(mqttBuf, "]}\r\n");
//        LOGV("Kalyke_monitor", "mqttBuf = %s", mqttBuf);
//    }
//    else if (memcmp(g_plc_netcfg.mqtt.vender, "CUHEBMQTT", 9) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "FEXLINK", 7) == 0 )
//    {
//        strcpy(mqttBuf, "{");
//        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
//        {
//            if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_F32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "float32") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%.3f,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_float(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I16)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int16") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int32") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int32(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_BOOL)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "bool") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_bool(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_STRING)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "string") == 0)
//            {
//                sprintf(tempStr, "\"%s\":\"%s\",", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_string(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else // int16 default
//            {
//                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//                tempStr[0] = '\0';
//            }
//            strcat(mqttBuf, tempStr);
//        }
//        int len = strlen(mqttBuf);
//        //LOGD("Kalyke_monitor", "mqttBuf len = %u", len);
//        mqttBuf[len - 1] = '\0';
//        strcat(mqttBuf, "}");
//        LOGV("Kalyke_monitor", "mqttBuf = %s", mqttBuf);
//    }
//    else
//    {
//        strcpy(mqttBuf, "{");
//        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
//        {
//            if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_F32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "float32") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%.3f,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_float(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I16)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int16") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_I32)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "int32") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int32(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_BOOL)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "bool") == 0)
//            {
//                sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_bool(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else if (g_plc_netcfg.mqtt.pConfigs[i].dataType == DTYPE_STRING)  //(strcmp(g_plc_netcfg.mqtt.pConfigs[i].dataType, "string") == 0)
//            {
//                sprintf(tempStr, "\"%s\":\"%s\",", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_string(&g_plc_netcfg.mqtt.pConfigs[i]));
//            }
//            else // int16 default
//            {
//                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
//                tempStr[0] = '\0';
//            }
//            strcat(mqttBuf, tempStr);
//        }
//        char device_id[32] = {0};
//        memcpy(device_id, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
//#if (KALYKE_FEATURE_4G_TASK == 1)
//        sprintf(tempStr, "\"free_heap\":%u, \"tempeture\":%.1f, \"secondAll\":%u, \"second\":%u, \"ucode_len\":%u, \
//\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\", \"bootTime\":%u, \"SW_VERSION\":\"%s\", \"image_id\":%u, \"mcu_id\":\"%016llX\", \"DeviceID\":\"%s\", \
//\"gps_state\":%d, \"satellite_count\":%d, \"latitude\":%f, \"longitude\":%f",
//            freeheap,
//            tempeture, gKalykeSecondTick, gKalykeSecondTickCurrent, gUcodeLen,
//            pTime->year, pTime->month, pTime->day, pTime->hour, pTime->minute, pTime->second,
//            bsp_get_kalyke_boot_time(),
//            SW_VERSION, ota_get_image_id(),
//            gMcuID, device_id, gGpsValue.gpsState, gGpsValue.satelliteCount,
//            gGpsValue.latitude, gGpsValue.longitude);
//#else
//        sprintf(tempStr, "\"free_heap\":%u, \"tempeture\":%.1f, \"secondAll\":%u, \"second\":%u, \"ucode_len\":%u, \
//\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\", \"bootTime\":%u, \"SW_VERSION\":\"%s\", \"image_id\":%u, \"mcu_id\":\"%016llX\", \"DeviceID\":\"%s\", \
//\"gps_state\":%d, \"satellite_count\":%d, \"latitude\":%f, \"longitude\":%f",
//            freeheap,
//            tempeture, gKalykeSecondTick, gKalykeSecondTickCurrent, gUcodeLen,
//            pTime->year, pTime->month, pTime->day, pTime->hour, pTime->minute, pTime->second,
//            bsp_get_kalyke_boot_time(),
//            SW_VERSION, ota_get_image_id(),
//            gMcuID, device_id, 0, 0,
//            0, 0);
//#endif
//        strcat(mqttBuf, tempStr);
//        //int len = strlen(mqttBuf);
//        //LOGD("Kalyke_monitor", "mqttBuf len = %u", len);
//        //mqttBuf[len - 1] = '\0';
//        strcat(mqttBuf, "}");
//        //LOGV("Kalyke_monitor", "mqttBuf = %s", mqttBuf);
//    }
//    vPortFree(tempStr);
//    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuf, 0);
//    vPortFree(mqttBuf);
//}

//void kalyke_cycle_post_all(void)
//{
//    LOGV("monitor", "Enter %s()", __func__);
//    
//}

void kalyke_mqtt_publish(char *topic, char *payload, uint8_t qos)
{
//    LOGD("MQTT", "Enter %s(), gMqttConnected = %d, gWanOK = %d, gLanOK = %d, surfing = %d", __func__, gMqttConnected, gWanOK, gLanOK, g_plc_netcfg.surfing);
//    if (bsp_get_deviceID() == 0xFFFF)
//    {
//        LOGV("MQTT", "Device ID = 0xFFFF, so just return.");
//        return;
//    }
    
    //suspend_task_when_download_ucode();
    if (g_plc_netcfg.surfing == 0)
    {
        RTC_Show();
        int payloadlen = 0;
        payloadlen = strlen(payload);
        Mqtt_Transmit(topic, payload, payloadlen);
        LOGD("Kalyke_monitor", "Leave %s(), mqttBuf = %s", __func__, payload);
        return;
    }
    if (g_plc_netcfg.surfing == 1)
    {
        ec20_mqtt_publish(topic, payload, qos);
        return;
    }
    //resume_task_after_download_ucode();
    
//    if (gMqttConnected == false)
//    {
//        LOGD("MQTT", "Just return...4...MQTT not connected.");
//        return;
//    }
//    if (gWanOK == false)
//    {
//        LOGD("MQTT", "Just return...5...WAN not OK.");
//        return;
//    }
    
//    uint32_t topicLen, payloadLen;

//    mqtt_msg_st *pMqttMsg = pvPortMalloc(sizeof(mqtt_msg_st));
//    pMqttMsg->type = 1;
//    pMqttMsg->qos = qos;
//    topicLen = strlen(topic);
//    payloadLen = strlen(payload);
//    pMqttMsg->topic = pvPortMalloc(topicLen + 2);
//    memcpy(pMqttMsg->topic, topic, topicLen);
//    pMqttMsg->topic[topicLen] = 0;
//    pMqttMsg->topic[topicLen + 1] = 0;
//    pMqttMsg->dataBuff = pvPortMalloc(payloadLen + 2);
//    memcpy(pMqttMsg->dataBuff, payload, payloadLen);
//    pMqttMsg->dataBuff[payloadLen] = 0;
//    pMqttMsg->dataBuff[payloadLen + 1] = 0;
//    pMqttMsg->msgLength = payloadLen;
    
    LOGV("MQTT", "Leave %s()", __func__);
}

void kalyke_cycle_post_master_ELECTRIC(void)
{
    LOGE("kalyke_cycle_post_master_ELECTRIC", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(1440);
    memset(mqttBuff, 0, 1440);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
                "U1", gMeterParam.Volt[0],  \
                "U2", gMeterParam.Volt[1],  \
                "U3", gMeterParam.Volt[2],  \
                "U12", gMeterParam.VoltL[0],  \
                "U23", gMeterParam.VoltL[1],  \
                "U31", gMeterParam.VoltL[2],  \
                "FREQ", gMeterParam.Freq,  \
                "I1", gMeterParam.Curr[0],  \
                "I2", gMeterParam.Curr[1],  \
                "I3", gMeterParam.Curr[2],  \
                "I0", gMeterParam.Curr[3],  \
                "I", gMeterParam.Cur,  \
                "POWP1", gMeterParam.PowP[0],  \
                "POWP2", gMeterParam.PowP[1],  \
                "POWP3", gMeterParam.PowP[2],  \
                "POWP0", gMeterParam.PowP[3],  \
                "POWQ1", gMeterParam.PowQ[0],  \
                "POWQ2", gMeterParam.PowQ[1],  \
                "POWQ3", gMeterParam.PowQ[2],  \
                "POWQ0", gMeterParam.PowQ[3],  \
                "POWS1", gMeterParam.PowS[0],  \
                "POWS2", gMeterParam.PowS[1],  \
                "POWS3", gMeterParam.PowS[2],  \
                "POWS0", gMeterParam.PowS[3],  \
                "PF1", gMeterParam.Pf[0],  \
                "PF2", gMeterParam.Pf[1],  \
                "PF3", gMeterParam.Pf[2],  \
                "PF0", gMeterParam.Pf[3],  \
                "EPI1", gMeterEnergy.EPI[0],  \
                "EPI2", gMeterEnergy.EPI[1],  \
                "EPI3", gMeterEnergy.EPI[2],  \
                "EPI0", gMeterEnergy.EPI[3],  \
                "EPE1", gMeterEnergy.EPE[0],  \
                "EPE2", gMeterEnergy.EPE[1],  \
                "EPE3", gMeterEnergy.EPE[2],  \
                "EPE0", gMeterEnergy.EPE[3],  \
                "EQL1", gMeterEnergy.EQL[0],  \
                "EQL2", gMeterEnergy.EQL[1],  \
                "EQL3", gMeterEnergy.EQL[2],  \
                "EQL0", gMeterEnergy.EQL[3],  \
                "EQC1", gMeterEnergy.EQC[0],  \
                "EQC2", gMeterEnergy.EQC[1],  \
                "EQC3", gMeterEnergy.EQC[2],  \
                "EQC0", gMeterEnergy.EQC[3]);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_ELECTRIC", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_cycle_post_master_Imbalance(void)
{
    LOGE("kalyke_cycle_post_master_Imbalance", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(1024);
    memset(mqttBuff, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
                "ANGU1", gPowerQualityParam.Volt_Angle[0],  \
                "ANGU2", gPowerQualityParam.Volt_Angle[1],  \
                "ANGU3", gPowerQualityParam.Volt_Angle[2],  \
                "ANGI1", gPowerQualityParam.Curr_Angle[0],  \
                "ANGI2", gPowerQualityParam.Curr_Angle[2],  \
                "ANGI3", gPowerQualityParam.Curr_Angle[1],  \
                "DU1", gPowerQualityParam.Volt_Deviation[0],  \
                "DU2", gPowerQualityParam.Volt_Deviation[1],  \
                "DU3", gPowerQualityParam.Volt_Deviation[2],  \
                "DU12", gPowerQualityParam.VoltL_Deviation[0],  \
                "DU23", gPowerQualityParam.VoltL_Deviation[1],  \
                "DU31", gPowerQualityParam.VoltL_Deviation[2],  \
                "DUF", gPowerQualityParam.Freq_Deviation,  \
                "EU0", gPowerQualityParam.Volt_Imbalance[0],  \
                "EU2", gPowerQualityParam.Volt_Imbalance[1],  \
                "EI0", gPowerQualityParam.Curr_Imbalance[0],  \
                "EI2", gPowerQualityParam.Curr_Imbalance[1],  \
                "EV0", gPowerQualityParam.Volt_E[0],  \
                "EV1", gPowerQualityParam.Volt_E[1],  \
                "EV2", gPowerQualityParam.Volt_E[2],  \
                "EC0", gPowerQualityParam.Curr_E[0],  \
                "EC1", gPowerQualityParam.Curr_E[1],  \
                "EC2", gPowerQualityParam.Curr_E[2]);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_Imbalance", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_cycle_post_master_Uharmonic2_11(void)
{
    LOGE("kalyke_cycle_post_master_Uharmonic", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(1024);
    memset(mqttBuff, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
                "THDUa", gHarmonicParam.Volt_1[0]*0.01,  \
                "THDUb", gHarmonicParam.Volt_1[1]*0.01,  \
                "THDUc", gHarmonicParam.Volt_1[2]*0.01,  \
								"HRUa2", gHarmonicParam.Volt_2_31[0][0]*0.01,  \
                "HRUb2", gHarmonicParam.Volt_2_31[0][1]*0.01,  \
                "HRUc2", gHarmonicParam.Volt_2_31[0][2]*0.01,  \
                "HRUa3", gHarmonicParam.Volt_2_31[1][0]*0.01,  \
                "HRUb3", gHarmonicParam.Volt_2_31[1][1]*0.01,  \
                "HRUc3", gHarmonicParam.Volt_2_31[1][2]*0.01,  \
                "HRUa4", gHarmonicParam.Volt_2_31[2][0]*0.01,  \
                "HRUb4", gHarmonicParam.Volt_2_31[2][1]*0.01,  \
                "HRUc4", gHarmonicParam.Volt_2_31[2][2]*0.01,  \
                "HRUa5", gHarmonicParam.Volt_2_31[3][0]*0.01,  \
                "HRUb5", gHarmonicParam.Volt_2_31[3][1]*0.01,  \
                "HRUc5", gHarmonicParam.Volt_2_31[3][2]*0.01,  \
                "HRUa6", gHarmonicParam.Volt_2_31[4][0]*0.01,  \
                "HRUb6", gHarmonicParam.Volt_2_31[4][1]*0.01,  \
                "HRUc6", gHarmonicParam.Volt_2_31[4][2]*0.01,  \
                "HRUa7", gHarmonicParam.Volt_2_31[5][0]*0.01,  \
                "HRUb7", gHarmonicParam.Volt_2_31[5][1]*0.01,  \
                "HRUc7", gHarmonicParam.Volt_2_31[5][2]*0.01,  \
								"HRUa8", gHarmonicParam.Volt_2_31[6][0]*0.01,  \
                "HRUb8", gHarmonicParam.Volt_2_31[6][1]*0.01,  \
                "HRUc8", gHarmonicParam.Volt_2_31[6][2]*0.01,  \
								"HRUa9", gHarmonicParam.Volt_2_31[7][0]*0.01,  \
                "HRUb9", gHarmonicParam.Volt_2_31[7][1]*0.01,  \
                "HRUc9", gHarmonicParam.Volt_2_31[7][2]*0.01,  \
								"HRUa10", gHarmonicParam.Volt_2_31[8][0]*0.01,  \
                "HRUb10", gHarmonicParam.Volt_2_31[8][1]*0.01,  \
                "HRUc10", gHarmonicParam.Volt_2_31[8][2]*0.01,  \
								"HRUa11", gHarmonicParam.Volt_2_31[9][0]*0.01,  \
                "HRUb11", gHarmonicParam.Volt_2_31[9][1]*0.01,  \
                "HRUc11", gHarmonicParam.Volt_2_31[9][2]*0.01);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_Uharmonic", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_cycle_post_master_Uharmonic12_21(void)
{
    LOGE("kalyke_cycle_post_master_Uharmonic", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(1024);
    memset(mqttBuff, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
								"HRUa12", gHarmonicParam.Volt_2_31[10][0]*0.01,  \
                "HRUb12", gHarmonicParam.Volt_2_31[10][1]*0.01,  \
                "HRUc12", gHarmonicParam.Volt_2_31[10][2]*0.01,  \
								"HRUa13", gHarmonicParam.Volt_2_31[11][0]*0.01,  \
                "HRUb13", gHarmonicParam.Volt_2_31[11][1]*0.01,  \
                "HRUc13", gHarmonicParam.Volt_2_31[11][2]*0.01,  \
								"HRUa14", gHarmonicParam.Volt_2_31[12][0]*0.01,  \
                "HRUb14", gHarmonicParam.Volt_2_31[12][1]*0.01,  \
                "HRUc14", gHarmonicParam.Volt_2_31[12][2]*0.01,  \
								"HRUa15", gHarmonicParam.Volt_2_31[13][0]*0.01,  \
                "HRUb15", gHarmonicParam.Volt_2_31[13][1]*0.01,  \
                "HRUc15", gHarmonicParam.Volt_2_31[13][2]*0.01,  \
								"HRUa16", gHarmonicParam.Volt_2_31[14][0]*0.01,  \
                "HRUb16", gHarmonicParam.Volt_2_31[14][1]*0.01,  \
                "HRUc16", gHarmonicParam.Volt_2_31[14][2]*0.01,  \
								"HRUa17", gHarmonicParam.Volt_2_31[15][0]*0.01,  \
                "HRUb17", gHarmonicParam.Volt_2_31[15][1]*0.01,  \
                "HRUc17", gHarmonicParam.Volt_2_31[15][2]*0.01,  \
								"HRUa18", gHarmonicParam.Volt_2_31[16][0]*0.01,  \
                "HRUb18", gHarmonicParam.Volt_2_31[16][1]*0.01,  \
                "HRUc18", gHarmonicParam.Volt_2_31[16][2]*0.01,  \
								"HRUa19", gHarmonicParam.Volt_2_31[17][0]*0.01,  \
                "HRUb19", gHarmonicParam.Volt_2_31[17][1]*0.01,  \
                "HRUc19", gHarmonicParam.Volt_2_31[17][2]*0.01,  \
								"HRUa20", gHarmonicParam.Volt_2_31[18][0]*0.01,  \
                "HRUb20", gHarmonicParam.Volt_2_31[18][1]*0.01,  \
                "HRUc20", gHarmonicParam.Volt_2_31[18][2]*0.01,  \
                "HRUa21", gHarmonicParam.Volt_2_31[19][0]*0.01,  \
                "HRUb21", gHarmonicParam.Volt_2_31[19][1]*0.01,  \
                "HRUc21", gHarmonicParam.Volt_2_31[19][2]*0.01);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_Uharmonic", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_cycle_post_master_Uharmonic22_31(void)
{
    LOGE("kalyke_cycle_post_master_Uharmonic", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(1024);
    memset(mqttBuff, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
								"HRUa22", gHarmonicParam.Volt_2_31[20][0]*0.01,  \
                "HRUb22", gHarmonicParam.Volt_2_31[20][1]*0.01,  \
                "HRUc22", gHarmonicParam.Volt_2_31[20][2]*0.01,  \
								"HRUa23", gHarmonicParam.Volt_2_31[21][0]*0.01,  \
                "HRUb23", gHarmonicParam.Volt_2_31[21][1]*0.01,  \
                "HRUc23", gHarmonicParam.Volt_2_31[21][2]*0.01,  \
								"HRUa24", gHarmonicParam.Volt_2_31[22][0]*0.01,  \
                "HRUb24", gHarmonicParam.Volt_2_31[22][1]*0.01,  \
                "HRUc24", gHarmonicParam.Volt_2_31[22][2]*0.01,  \
								"HRUa25", gHarmonicParam.Volt_2_31[23][0]*0.01,  \
                "HRUb25", gHarmonicParam.Volt_2_31[23][1]*0.01,  \
                "HRUc26", gHarmonicParam.Volt_2_31[23][2]*0.01,  \
								"HRUa26", gHarmonicParam.Volt_2_31[24][0]*0.01,  \
                "HRUb26", gHarmonicParam.Volt_2_31[24][1]*0.01,  \
                "HRUc26", gHarmonicParam.Volt_2_31[24][2]*0.01,  \
								"HRUa27", gHarmonicParam.Volt_2_31[25][0]*0.01,  \
                "HRUb27", gHarmonicParam.Volt_2_31[25][1]*0.01,  \
                "HRUc27", gHarmonicParam.Volt_2_31[25][2]*0.01,  \
								"HRUa28", gHarmonicParam.Volt_2_31[26][0]*0.01,  \
                "HRUb28", gHarmonicParam.Volt_2_31[26][1]*0.01,  \
                "HRUc28", gHarmonicParam.Volt_2_31[26][2]*0.01,  \
								"HRUa29", gHarmonicParam.Volt_2_31[27][0]*0.01,  \
                "HRUb29", gHarmonicParam.Volt_2_31[27][1]*0.01,  \
                "HRUc29", gHarmonicParam.Volt_2_31[27][2]*0.01,  \
								"HRUa30", gHarmonicParam.Volt_2_31[28][0]*0.01,  \
                "HRUb30", gHarmonicParam.Volt_2_31[28][1]*0.01,  \
                "HRUc30", gHarmonicParam.Volt_2_31[28][2]*0.01,  \
                "HRUa31", gHarmonicParam.Volt_2_31[29][0]*0.01,  \
                "HRUb31", gHarmonicParam.Volt_2_31[29][1]*0.01,  \
                "HRUc31", gHarmonicParam.Volt_2_31[29][2]*0.01);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_Uharmonic", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_cycle_post_master_Iharmonic2_11(void)
{
    LOGE("kalyke_cycle_post_master_Iharmonic", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(1024);
    memset(mqttBuff, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
                "THDIa", gHarmonicParam.Curr_1[0]*0.01,  \
                "THDIb", gHarmonicParam.Curr_1[1]*0.01,  \
                "THDIc", gHarmonicParam.Curr_1[2]*0.01,  \
								"HRIa2", gHarmonicParam.Curr_2_31[0][0]*0.01,  \
                "HRIb2", gHarmonicParam.Curr_2_31[0][1]*0.01,  \
                "HRIc2", gHarmonicParam.Curr_2_31[0][2]*0.01,  \
                "HRIa3", gHarmonicParam.Curr_2_31[1][0]*0.01,  \
                "HRIb3", gHarmonicParam.Curr_2_31[1][1]*0.01,  \
                "HRIc3", gHarmonicParam.Curr_2_31[1][2]*0.01,  \
                "HRIa4", gHarmonicParam.Curr_2_31[2][0]*0.01,  \
                "HRIb4", gHarmonicParam.Curr_2_31[2][1]*0.01,  \
                "HRIc4", gHarmonicParam.Curr_2_31[2][2]*0.01,  \
                "HRIa5", gHarmonicParam.Curr_2_31[3][0]*0.01,  \
                "HRIb5", gHarmonicParam.Curr_2_31[3][1]*0.01,  \
                "HRIc5", gHarmonicParam.Curr_2_31[3][2]*0.01,  \
                "HRIa6", gHarmonicParam.Curr_2_31[4][0]*0.01,  \
                "HRIb6", gHarmonicParam.Curr_2_31[4][1]*0.01,  \
                "HRIc6", gHarmonicParam.Curr_2_31[4][2]*0.01,  \
                "HRIa7", gHarmonicParam.Curr_2_31[5][0]*0.01,  \
                "HRIb7", gHarmonicParam.Curr_2_31[5][1]*0.01,  \
                "HRIc7", gHarmonicParam.Curr_2_31[5][2]*0.01,  \
								"HRIa8", gHarmonicParam.Curr_2_31[6][0]*0.01,  \
                "HRIb8", gHarmonicParam.Curr_2_31[6][1]*0.01,  \
                "HRIc8", gHarmonicParam.Curr_2_31[6][2]*0.01,  \
								"HRIa9", gHarmonicParam.Curr_2_31[7][0]*0.01,  \
                "HRIb9", gHarmonicParam.Curr_2_31[7][1]*0.01,  \
                "HRIc9", gHarmonicParam.Curr_2_31[7][2]*0.01,  \
								"HRIa10", gHarmonicParam.Curr_2_31[8][0]*0.01,  \
                "HRIb10", gHarmonicParam.Curr_2_31[8][1]*0.01,  \
                "HRIc10", gHarmonicParam.Curr_2_31[8][2]*0.01,  \
								"HRIa11", gHarmonicParam.Curr_2_31[9][0]*0.01,  \
                "HRIb11", gHarmonicParam.Curr_2_31[9][1]*0.01,  \
                "HRIc11", gHarmonicParam.Curr_2_31[9][2]*0.01);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_Iharmonic", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_cycle_post_master_Iharmonic12_21(void)
{
    LOGE("kalyke_cycle_post_master_Iharmonic", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(1024);
    memset(mqttBuff, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
								"HRIa12", gHarmonicParam.Curr_2_31[10][0]*0.01,  \
                "HRIb12", gHarmonicParam.Curr_2_31[10][1]*0.01,  \
                "HRIc12", gHarmonicParam.Curr_2_31[10][2]*0.01,  \
								"HRIa13", gHarmonicParam.Curr_2_31[11][0]*0.01,  \
                "HRIb13", gHarmonicParam.Curr_2_31[11][1]*0.01,  \
                "HRIc13", gHarmonicParam.Curr_2_31[11][2]*0.01,  \
								"HRIa14", gHarmonicParam.Curr_2_31[12][0]*0.01,  \
                "HRIb14", gHarmonicParam.Curr_2_31[12][1]*0.01,  \
                "HRIc14", gHarmonicParam.Curr_2_31[12][2]*0.01,  \
								"HRIa15", gHarmonicParam.Curr_2_31[13][0]*0.01,  \
                "HRIb15", gHarmonicParam.Curr_2_31[13][1]*0.01,  \
                "HRIc15", gHarmonicParam.Curr_2_31[13][2]*0.01,  \
								"HRIa16", gHarmonicParam.Curr_2_31[14][0]*0.01,  \
                "HRIb16", gHarmonicParam.Curr_2_31[14][1]*0.01,  \
                "HRIc16", gHarmonicParam.Curr_2_31[14][2]*0.01,  \
								"HRIa17", gHarmonicParam.Curr_2_31[15][0]*0.01,  \
                "HRIb17", gHarmonicParam.Curr_2_31[15][1]*0.01,  \
                "HRIc17", gHarmonicParam.Curr_2_31[15][2]*0.01,  \
								"HRIa18", gHarmonicParam.Curr_2_31[16][0]*0.01,  \
                "HRIb18", gHarmonicParam.Curr_2_31[16][1]*0.01,  \
                "HRIc18", gHarmonicParam.Curr_2_31[16][2]*0.01,  \
								"HRIa19", gHarmonicParam.Curr_2_31[17][0]*0.01,  \
                "HRIb19", gHarmonicParam.Curr_2_31[17][1]*0.01,  \
                "HRIc19", gHarmonicParam.Curr_2_31[17][2]*0.01,  \
								"HRIa20", gHarmonicParam.Curr_2_31[18][0]*0.01,  \
                "HRIb20", gHarmonicParam.Curr_2_31[18][1]*0.01,  \
                "HRIc20", gHarmonicParam.Curr_2_31[18][2]*0.01,  \
                "HRIa21", gHarmonicParam.Curr_2_31[19][0]*0.01,  \
                "HRIb21", gHarmonicParam.Curr_2_31[19][1]*0.01,  \
                "HRIc21", gHarmonicParam.Curr_2_31[19][2]*0.01);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_Iharmonic", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_cycle_post_master_Iharmonic22_31(void)
{
    LOGE("kalyke_cycle_post_master_Iharmonic", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(1024);
    memset(mqttBuff, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\",\
\"%s\":\"%1.2f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
								"HRIa22", gHarmonicParam.Curr_2_31[20][0]*0.01,  \
                "HRIb22", gHarmonicParam.Curr_2_31[20][1]*0.01,  \
                "HRIc22", gHarmonicParam.Curr_2_31[20][2]*0.01,  \
								"HRIa23", gHarmonicParam.Curr_2_31[21][0]*0.01,  \
                "HRIb23", gHarmonicParam.Curr_2_31[21][1]*0.01,  \
                "HRIc23", gHarmonicParam.Curr_2_31[21][2]*0.01,  \
								"HRIa24", gHarmonicParam.Curr_2_31[22][0]*0.01,  \
                "HRIb24", gHarmonicParam.Curr_2_31[22][1]*0.01,  \
                "HRIc24", gHarmonicParam.Curr_2_31[22][2]*0.01,  \
								"HRIa25", gHarmonicParam.Curr_2_31[23][0]*0.01,  \
                "HRIb25", gHarmonicParam.Curr_2_31[23][1]*0.01,  \
                "HRIc25", gHarmonicParam.Curr_2_31[23][2]*0.01,  \
								"HRIa26", gHarmonicParam.Curr_2_31[24][0]*0.01,  \
                "HRIb26", gHarmonicParam.Curr_2_31[24][1]*0.01,  \
                "HRIc26", gHarmonicParam.Curr_2_31[24][2]*0.01,  \
								"HRIa27", gHarmonicParam.Curr_2_31[25][0]*0.01,  \
                "HRIb27", gHarmonicParam.Curr_2_31[25][1]*0.01,  \
                "HRIc27", gHarmonicParam.Curr_2_31[25][2]*0.01,  \
								"HRIa28", gHarmonicParam.Curr_2_31[26][0]*0.01,  \
                "HRIb28", gHarmonicParam.Curr_2_31[26][1]*0.01,  \
                "HRIc28", gHarmonicParam.Curr_2_31[26][2]*0.01,  \
								"HRIa29", gHarmonicParam.Curr_2_31[27][0]*0.01,  \
                "HRIb29", gHarmonicParam.Curr_2_31[27][1]*0.01,  \
                "HRIc29", gHarmonicParam.Curr_2_31[27][2]*0.01,  \
								"HRIa30", gHarmonicParam.Curr_2_31[28][0]*0.01,  \
                "HRIb30", gHarmonicParam.Curr_2_31[28][1]*0.01,  \
                "HRIc30", gHarmonicParam.Curr_2_31[28][2]*0.01,  \
                "HRIa31", gHarmonicParam.Curr_2_31[29][0]*0.01,  \
                "HRIb31", gHarmonicParam.Curr_2_31[29][1]*0.01,  \
                "HRIc31", gHarmonicParam.Curr_2_31[29][2]*0.01);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_Iharmonic", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_cycle_post_master_DIDOTEMP(void)
{
    LOGE("kalyke_cycle_post_master_DIDOTEMP", "Free heap size is %d bytes", xPortGetFreeHeapSize());

    char *mqttBuff = pvPortMalloc(512);
    memset(mqttBuff, 0, 512);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    sprintf(mqttBuff, "{\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",\
\"data\":[{\"%s\":\"%d\",\
\"%s\":\"%d\",\
\"%s\":\"%d\",\
\"%s\":\"%d\",\
\"%s\":\"%d\",\
\"%s\":\"%d\",\
\"%s\":\"%1.1f\",\
\"%s\":\"%1.1f\",\
\"%s\":\"%1.1f\",\
\"%s\":\"%1.1f\"}]}",
                2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds, pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr,  \
                "DI1", gESE_Elem.st.DI[0],  \
                "DI2", gESE_Elem.st.DI[1],  \
                "DI3", gESE_Elem.st.DI[2],  \
                "DI4", gESE_Elem.st.DI[3],  \
                "DO1", gESE_Elem.st.DO[0],  \
                "DO2", gESE_Elem.st.DO[1],  \
                "T1", gESE_Elem.st.Tmp[0]*0.1,  \
                "T2", gESE_Elem.st.Tmp[1]*0.1,  \
                "T3", gESE_Elem.st.Tmp[2]*0.1,  \
                "T4", gESE_Elem.st.Tmp[3]*0.1);
                
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuff, 0);
    vPortFree(mqttBuff);
    
    LOGE("kalyke_cycle_post_master_DIDOTEMP", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

#if PROD_TYPE == PROD_SFA
void kalyke_alarm_post_master(void)
{
    LOGE("kalyke_alarm_post_master", "Free heap size is %d bytes", xPortGetFreeHeapSize());
    
    char *tempStr = pvPortMalloc(512);
    char *mqttBuf = pvPortMalloc(1024);
    memset(tempStr, 0, 512);
    memset(mqttBuf, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    strcpy(mqttBuf, "{");
    sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", 2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
    strcat(mqttBuf, tempStr);
    
    sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",", pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr);
    strcat(mqttBuf, tempStr);
    
    strcat(mqttBuf, "\"data\":[{");            
    
    sprintf(tempStr, "\"%s\":\"0\",", "ERRCODE");
    strcat(mqttBuf, tempStr);

    if(gParam.st.State_Alarm[StateAlm1] != 0 || gParam.st.State_Alarm[StateAlm2] != 0 ||gParam.st.State_Alarm[StateAlm3] != 0)
    {            
        sprintf(tempStr, "\"%s\":\"1\",", "ERRCODE");
        strcat(mqttBuf, tempStr);
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Down_Msk))
        {
            sprintf(tempStr, "\"U1\":\"%1.2f\",", gMeterParam.Volt[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL2_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL2_Down_Msk))
        {
            sprintf(tempStr, "\"U2\":\"%1.2f\",", gMeterParam.Volt[1]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL3_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL3_Down_Msk))
        {
            sprintf(tempStr, "\"U3\":\"%1.2f\",", gMeterParam.Volt[2]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Down_Msk))
        {
            sprintf(tempStr, "\"I1\":\"%1.2f\",", gMeterParam.Curr[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR2_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR2_Down_Msk))
        {
            sprintf(tempStr, "\"I2\":\"%1.2f\",", gMeterParam.Curr[1]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR3_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR3_Down_Msk))
        {
            sprintf(tempStr, "\"I3\":\"%1.2f\",", gMeterParam.Curr[2]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Msk))
        {
            sprintf(tempStr, "\"I\":\"%1.2f\",", gMeterParam.Cur);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI1_Msk))
        {
            sprintf(tempStr, "\"DI1\":\"%d\",", gESE_Elem.st.DI[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI2_Msk))
        {
            sprintf(tempStr, "\"DI2\":\"%d\",", gESE_Elem.st.DI[1]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI3_Msk))
        {
            sprintf(tempStr, "\"DI3\":\"%d\",", gESE_Elem.st.DI[2]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI4_Msk))
        {
            sprintf(tempStr, "\"DI4\":\"%d\",", gESE_Elem.st.DI[3]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO1_Msk))
        {
            sprintf(tempStr, "\"DO1\":\"%d\",", gESE_Elem.st.DO[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO2_Msk))
        {
            sprintf(tempStr, "\"DO2\":\"%d\",", gESE_Elem.st.DO[1]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP1_Msk))
        {
            sprintf(tempStr, "\"T1\":\"%1.1f\",", gESE_Elem.st.Tmp[0]*0.1);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP2_Msk))
        {
            sprintf(tempStr, "\"T2\":\"%1.1f\",", gESE_Elem.st.Tmp[1]*0.1);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP3_Msk))
        {
            sprintf(tempStr, "\"T3\":\"%1.1f\",", gESE_Elem.st.Tmp[2]*0.1);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP4_Msk))
        {
            sprintf(tempStr, "\"T4\":\"%1.1f\",", gESE_Elem.st.Tmp[3]*0.1);
            strcat(mqttBuf, tempStr);
        }
    }
    
    int len = strlen(mqttBuf);
    mqttBuf[len - 1] = '\0';
    strcat(mqttBuf, "}]}\r\n");
                
    vPortFree(tempStr);
    kalyke_mqtt_publish(MQTT_TOPIC_ALARM, mqttBuf, 0);
    vPortFree(mqttBuf);
    
    LOGE("kalyke_alarm_post_master", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}
#elif PROD_TYPE == PROD_SFB
void kalyke_alarm_post_master(void)
{
    LOGE("kalyke_alarm_post_master", "Free heap size is %d bytes", xPortGetFreeHeapSize());
    
    char *tempStr = pvPortMalloc(512);
    char *mqttBuf = pvPortMalloc(1024);
    memset(tempStr, 0, 512);
    memset(mqttBuf, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    strcpy(mqttBuf, "{");
    sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", 2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
    strcat(mqttBuf, tempStr);
    
    sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",", pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr);
    strcat(mqttBuf, tempStr);
    
    strcat(mqttBuf, "\"data\":[{");            
    
    sprintf(tempStr, "\"%s\":\"0\",", "ERRCODE");
    strcat(mqttBuf, tempStr);

    if(gParam.st.State_Alarm[StateAlm1] != 0 || gParam.st.State_Alarm[StateAlm2] != 0 ||gParam.st.State_Alarm[StateAlm3] != 0)
    {            
        sprintf(tempStr, "\"%s\":\"1\",", "ERRCODE");
        strcat(mqttBuf, tempStr);
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Down_Msk))
        {
            sprintf(tempStr, "\"U1\":\"%1.2f\",", gMeterParam.Volt[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL2_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL2_Down_Msk))
        {
            sprintf(tempStr, "\"U2\":\"%1.2f\",", gMeterParam.Volt[1]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL3_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL3_Down_Msk))
        {
            sprintf(tempStr, "\"U3\":\"%1.2f\",", gMeterParam.Volt[2]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Down_Msk))
        {
            sprintf(tempStr, "\"I1\":\"%1.2f\",", gMeterParam.Curr[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR2_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR2_Down_Msk))
        {
            sprintf(tempStr, "\"I2\":\"%1.2f\",", gMeterParam.Curr[1]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR3_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR3_Down_Msk))
        {
            sprintf(tempStr, "\"I3\":\"%1.2f\",", gMeterParam.Curr[2]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Msk))
        {
            sprintf(tempStr, "\"I\":\"%1.2f\",", gMeterParam.Cur);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI1_Msk))
        {
            sprintf(tempStr, "\"DI1\":\"%d\",", gESE_Elem.st.DI[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI2_Msk))
        {
            sprintf(tempStr, "\"DI2\":\"%d\",", gESE_Elem.st.DI[1]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI3_Msk))
        {
            sprintf(tempStr, "\"DI3\":\"%d\",", gESE_Elem.st.DI[2]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI4_Msk))
        {
            sprintf(tempStr, "\"DI4\":\"%d\",", gESE_Elem.st.DI[3]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO1_Msk))
        {
            sprintf(tempStr, "\"DO1\":\"%d\",", gESE_Elem.st.DO[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO2_Msk))
        {
            sprintf(tempStr, "\"DO2\":\"%d\",", gESE_Elem.st.DO[1]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP1_Msk))
        {
            sprintf(tempStr, "\"T1\":\"%1.1f\",", gESE_Elem.st.Tmp[0]*0.1);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP2_Msk))
        {
            sprintf(tempStr, "\"T2\":\"%1.1f\",", gESE_Elem.st.Tmp[1]*0.1);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP3_Msk))
        {
            sprintf(tempStr, "\"T3\":\"%1.1f\",", gESE_Elem.st.Tmp[2]*0.1);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP4_Msk))
        {
            sprintf(tempStr, "\"T4\":\"%1.1f\",", gESE_Elem.st.Tmp[3]*0.1);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOL_IMB_Msk))
        {
            sprintf(tempStr, "\"EU0\":\"%1.2f\",", gPowerQualityParam.Volt_Imbalance[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOL_IMB_Msk))
        {
            sprintf(tempStr, "\"EU2\":\"%1.2f\",", gPowerQualityParam.Volt_Imbalance[1]);
            strcat(mqttBuf, tempStr);
        }   
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CUR_IMB_Msk))
        {
            sprintf(tempStr, "\"EI0\":\"%1.2f\",", gPowerQualityParam.Curr_Imbalance[0]);
            strcat(mqttBuf, tempStr);
        }
        
        if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CUR_IMB_Msk))
        {
            sprintf(tempStr, "\"EI2\":\"%1.2f\",", gPowerQualityParam.Curr_Imbalance[1]);
            strcat(mqttBuf, tempStr);
        }
    }
    
    int len = strlen(mqttBuf);
    mqttBuf[len - 1] = '\0';
    strcat(mqttBuf, "}]}\r\n");
                
    vPortFree(tempStr);
    kalyke_mqtt_publish(MQTT_TOPIC_ALARM, mqttBuf, 0);
    vPortFree(mqttBuf);
    
    LOGE("kalyke_alarm_post_master", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}
#elif PROD_TYPE == PROD_SFE
void kalyke_alarm_post_master(void)
{
    LOGE("kalyke_alarm_post_master", "Free heap size is %d bytes", xPortGetFreeHeapSize());
    
    char *tempStr = pvPortMalloc(512);
    char *mqttBuf = pvPortMalloc(1024);
    memset(tempStr, 0, 512);
    memset(mqttBuf, 0, 1024);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    strcpy(mqttBuf, "{");
    sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", 2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
    strcat(mqttBuf, tempStr);
    
    sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",", pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr);
    strcat(mqttBuf, tempStr);
    
    strcat(mqttBuf, "\"data\":[{");  

		sprintf(tempStr, "\"%s\":\"0\",", "ERRCODE");
    strcat(mqttBuf, tempStr);

    if(gParam.st.State_Alarm[StateAlm1] != 0 || gParam.st.State_Alarm[StateAlm2] != 0 ||gParam.st.State_Alarm[StateAlm3] != 0)
    {            
        sprintf(tempStr, "\"%s\":\"1\",", "ERRCODE");
        strcat(mqttBuf, tempStr);
        
				if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL1_Down_Msk))
				{
						sprintf(tempStr, "\"U1\":\"%1.2f\",", gMeterParam.Volt[0]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL2_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL2_Down_Msk))
				{
						sprintf(tempStr, "\"U2\":\"%1.2f\",", gMeterParam.Volt[1]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL3_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_VOL3_Down_Msk))
				{
						sprintf(tempStr, "\"U3\":\"%1.2f\",", gMeterParam.Volt[2]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR1_Down_Msk))
				{
						sprintf(tempStr, "\"I1\":\"%1.2f\",", gMeterParam.Curr[0]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR2_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR2_Down_Msk))
				{
						sprintf(tempStr, "\"I2\":\"%1.2f\",", gMeterParam.Curr[1]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR3_Up_Msk) || READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR3_Down_Msk))
				{
						sprintf(tempStr, "\"I3\":\"%1.2f\",", gMeterParam.Curr[2]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_CUR_Msk))
				{
						sprintf(tempStr, "\"I\":\"%1.2f\",", gMeterParam.Cur);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm1], StateAlm1_DUF_Msk))
				{
						sprintf(tempStr, "\"DUF\":\"%1.2f\",", gPowerQualityParam.Freq_Deviation);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI1_Msk))
				{
						sprintf(tempStr, "\"DI1\":\"%d\",", gESE_Elem.st.DI[0]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI2_Msk))
				{
						sprintf(tempStr, "\"DI2\":\"%d\",", gESE_Elem.st.DI[1]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI3_Msk))
				{
						sprintf(tempStr, "\"DI3\":\"%d\",", gESE_Elem.st.DI[2]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DI4_Msk))
				{
						sprintf(tempStr, "\"DI4\":\"%d\",", gESE_Elem.st.DI[3]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO1_Msk))
				{
						sprintf(tempStr, "\"DO1\":\"%d\",", gESE_Elem.st.DO[0]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_DO2_Msk))
				{
						sprintf(tempStr, "\"DO2\":\"%d\",", gESE_Elem.st.DO[1]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP1_Msk))
				{
						sprintf(tempStr, "\"T1\":\"%1.1f\",", gESE_Elem.st.Tmp[0]*0.1);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP2_Msk))
				{
						sprintf(tempStr, "\"T2\":\"%1.1f\",", gESE_Elem.st.Tmp[1]*0.1);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP3_Msk))
				{
						sprintf(tempStr, "\"T3\":\"%1.1f\",", gESE_Elem.st.Tmp[2]*0.1);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm3], StateAlm3_TMP4_Msk))
				{
						sprintf(tempStr, "\"T4\":\"%1.1f\",", gESE_Elem.st.Tmp[3]*0.1);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOL_IMB_Msk))
				{
						sprintf(tempStr, "\"EU0\":\"%1.2f\",", gPowerQualityParam.Volt_Imbalance[0]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOL_IMB_Msk))
				{
						sprintf(tempStr, "\"EU2\":\"%1.2f\",", gPowerQualityParam.Volt_Imbalance[1]);
						strcat(mqttBuf, tempStr);
				}   
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CUR_IMB_Msk))
				{
						sprintf(tempStr, "\"EI0\":\"%1.2f\",", gPowerQualityParam.Curr_Imbalance[0]);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CUR_IMB_Msk))
				{
						sprintf(tempStr, "\"EI2\":\"%1.2f\",", gPowerQualityParam.Curr_Imbalance[1]);
						strcat(mqttBuf, tempStr);
				}    
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLA_THD_Msk))
				{
						sprintf(tempStr, "\"THDUa\":\"%1.2f\",", gHarmonicParam.Volt_1[0]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLB_THD_Msk))
				{
						sprintf(tempStr, "\"THDUb\":\"%1.2f\",", gHarmonicParam.Volt_1[1]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLC_THD_Msk))
				{
						sprintf(tempStr, "\"THDUc\":\"%1.2f\",", gHarmonicParam.Volt_1[2]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLAx_THD_Msk))
				{
						sprintf(tempStr, "\"HRUa%d\":\"%1.2f\",", (UAHarmonicj+2), gHarmonicParam.Volt_2_31[UAHarmonicj][UAHarmonici]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLBx_THD_Msk))
				{
						sprintf(tempStr, "\"HRUb%d\":\"%1.2f\",", (UBHarmonicj+2), gHarmonicParam.Volt_2_31[UBHarmonicj][UBHarmonici]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_VOLCx_THD_Msk))
				{
						sprintf(tempStr, "\"HRUc%d\":\"%1.2f\",", (UCHarmonicj+2), gHarmonicParam.Volt_2_31[UCHarmonicj][UCHarmonici]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURA_THD_Msk))
				{
						sprintf(tempStr, "\"THDIa\":\"%1.2f\",", gHarmonicParam.Curr_1[0]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURB_THD_Msk))
				{
						sprintf(tempStr, "\"THDIb\":\"%1.2f\",", gHarmonicParam.Curr_1[1]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURC_THD_Msk))
				{
						sprintf(tempStr, "\"THDIc\":\"%1.2f\",", gHarmonicParam.Curr_1[2]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURAx_THD_Msk))
				{
						sprintf(tempStr, "\"HRIa%d\":\"%1.2f\",", (IAHarmonicj+2), gHarmonicParam.Curr_2_31[IAHarmonicj][IAHarmonici]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURBx_THD_Msk))
				{
						sprintf(tempStr, "\"HRIb%d\":\"%1.2f\",", (IBHarmonicj+2), gHarmonicParam.Curr_2_31[IBHarmonicj][IBHarmonici]*0.01);
						strcat(mqttBuf, tempStr);
				}
				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm2], StateAlm2_CURCx_THD_Msk))
				{
						sprintf(tempStr, "\"HRIc%d\":\"%1.2f\",", (ICHarmonicj+2), gHarmonicParam.Curr_2_31[ICHarmonicj][ICHarmonici]*0.01);
						strcat(mqttBuf, tempStr);
				}
		}
    int len = strlen(mqttBuf);
    mqttBuf[len - 1] = '\0';
    strcat(mqttBuf, "}]}\r\n");
                
    vPortFree(tempStr);
    kalyke_mqtt_publish(MQTT_TOPIC_ALARM, mqttBuf, 0);
    vPortFree(mqttBuf);
    
    LOGE("kalyke_alarm_post_master", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}

void kalyke_alarm_post_master_SC_EA_VC(void)
{
    LOGE("kalyke_alarm_post_master_SC_EA_VC", "Free heap size is %d bytes", xPortGetFreeHeapSize());
    
    char *tempStr = pvPortMalloc(512);
    char *mqttBuf = pvPortMalloc(1500);
    memset(tempStr, 0, 512);
    memset(mqttBuf, 0, 1500);
  
    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    payload_string_t pyldStr;
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    memset(&pyldStr, '\0', sizeof(pyldStr));

    memcpy(pyldStr.IdStr, gFlashParam.st.idInfo, 16);
    
    memcpy(pyldStr.TerminalTypeStr, (uint8_t *)&gFlashParam.st.idInfo[18], 4);

    strcpy(mqttBuf, "{");
    sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", 2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
    strcat(mqttBuf, tempStr);
    
    sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":\"%s\",\"slave_name\":\"%s\",", pyldStr.IdStr, "0",(unsigned char *)pyldStr.TerminalTypeStr);
    strcat(mqttBuf, tempStr);
    
    strcat(mqttBuf, "\"data\":[{");            
    
//    sprintf(tempStr, "\"%s\":\"0\",", "ShortCircuit");
//    strcat(mqttBuf, tempStr);

    if(gParam.st.State_Alarm[StateAlm4] != 0)
    {                    				
				if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_SC_MSK))
				{
						sprintf(tempStr, "\"%s\":\"1\",", "ShortCircuit");
						strcat(mqttBuf, tempStr);
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaSC_Msk))
						{	
								sprintf(tempStr, "\"U1\":\"%1.2f\",", gMeterParam.Volt[0]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUa\":\"%1.2f\",", gHarmonicParam.Volt_1[0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I1\":\"%1.2f\",", gMeterParam.Curr[0]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIa\":\"%1.2f\",", gHarmonicParam.Curr_1[0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][0]*0.01);
								strcat(mqttBuf, tempStr);
						}
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbSC_Msk))
						{
								sprintf(tempStr, "\"U2\":\"%1.2f\",", gMeterParam.Volt[1]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUb\":\"%1.2f\",", gHarmonicParam.Volt_1[1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I2\":\"%1.2f\",", gMeterParam.Curr[1]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIb\":\"%1.2f\",", gHarmonicParam.Curr_1[1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][1]*0.01);
								strcat(mqttBuf, tempStr);
						}
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcSC_Msk))
						{
								sprintf(tempStr, "\"U3\":\"%1.2f\",", gMeterParam.Volt[2]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUc\":\"%1.2f\",", gHarmonicParam.Volt_1[2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I3\":\"%1.2f\",", gMeterParam.Curr[2]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIc\":\"%1.2f\",", gHarmonicParam.Curr_1[2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][2]*0.01);
								strcat(mqttBuf, tempStr);
						}
				}
				if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_EA_MSK))
				{
						sprintf(tempStr, "\"%s\":\"1\",", "ElectricArc");
						strcat(mqttBuf, tempStr);
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaEA_Msk))
						{	
								sprintf(tempStr, "\"U1\":\"%1.2f\",", gMeterParam.Volt[0]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUa\":\"%1.2f\",", gHarmonicParam.Volt_1[0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I1\":\"%1.2f\",", gMeterParam.Curr[0]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIa\":\"%1.2f\",", gHarmonicParam.Curr_1[0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][0]*0.01);
								strcat(mqttBuf, tempStr);
						}
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbEA_Msk))
						{
								sprintf(tempStr, "\"U2\":\"%1.2f\",", gMeterParam.Volt[1]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUb\":\"%1.2f\",", gHarmonicParam.Volt_1[1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I2\":\"%1.2f\",", gMeterParam.Curr[1]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIb\":\"%1.2f\",", gHarmonicParam.Curr_1[1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][1]*0.01);
								strcat(mqttBuf, tempStr);
						}
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcEA_Msk))
						{
								sprintf(tempStr, "\"U3\":\"%1.2f\",", gMeterParam.Volt[2]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUc\":\"%1.2f\",", gHarmonicParam.Volt_1[2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I3\":\"%1.2f\",", gMeterParam.Curr[2]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIc\":\"%1.2f\",", gHarmonicParam.Curr_1[2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][2]*0.01);
								strcat(mqttBuf, tempStr);
						}
				}
				if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_VC_MSK))
				{
						sprintf(tempStr, "\"%s\":\"1\",", "VirtualConnect");
						strcat(mqttBuf, tempStr);
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUaVC_Msk))
						{	
								sprintf(tempStr, "\"U1\":\"%1.2f\",", gMeterParam.Volt[0]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUa\":\"%1.2f\",", gHarmonicParam.Volt_1[0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUa7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I1\":\"%1.2f\",", gMeterParam.Curr[0]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIa\":\"%1.2f\",", gHarmonicParam.Curr_1[0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][0]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIa7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][0]*0.01);
								strcat(mqttBuf, tempStr);
						}
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUbVC_Msk))
						{
								sprintf(tempStr, "\"U2\":\"%1.2f\",", gMeterParam.Volt[1]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUb\":\"%1.2f\",", gHarmonicParam.Volt_1[1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUb7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I2\":\"%1.2f\",", gMeterParam.Curr[1]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIb\":\"%1.2f\",", gHarmonicParam.Curr_1[1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][1]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIb7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][1]*0.01);
								strcat(mqttBuf, tempStr);
						}
						if(READ_BIT(gParam.st.State_Alarm[StateAlm4], StateAlm4_THDUcVC_Msk))
						{
								sprintf(tempStr, "\"U3\":\"%1.2f\",", gMeterParam.Volt[2]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDUc\":\"%1.2f\",", gHarmonicParam.Volt_1[2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc2\":\"%1.2f\",", gHarmonicParam.Volt_2_31[0][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc3\":\"%1.2f\",", gHarmonicParam.Volt_2_31[1][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc5\":\"%1.2f\",", gHarmonicParam.Volt_2_31[3][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRUc7\":\"%1.2f\",", gHarmonicParam.Volt_2_31[5][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"I3\":\"%1.2f\",", gMeterParam.Curr[2]);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"THDIc\":\"%1.2f\",", gHarmonicParam.Curr_1[2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc2\":\"%1.2f\",", gHarmonicParam.Curr_2_31[0][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc3\":\"%1.2f\",", gHarmonicParam.Curr_2_31[1][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc5\":\"%1.2f\",", gHarmonicParam.Curr_2_31[3][2]*0.01);
								strcat(mqttBuf, tempStr);
								sprintf(tempStr, "\"HRIc7\":\"%1.2f\",", gHarmonicParam.Curr_2_31[5][2]*0.01);
								strcat(mqttBuf, tempStr);
						}
				}
        
    }
    
    int len = strlen(mqttBuf);
    mqttBuf[len - 1] = '\0';
    strcat(mqttBuf, "}]}\r\n");
                
    vPortFree(tempStr);
    kalyke_mqtt_publish(MQTT_TOPIC_ALARM, mqttBuf, 0);
    vPortFree(mqttBuf);
    
    LOGE("kalyke_alarm_post_master_SC_EA_VC", "LEAVE %d bytes", xPortGetFreeHeapSize());
                
}
#endif

void kalyke_cycle_post(mqtt_config_array_hanyu_st *pCfgs,rtc_datetime_t *pTime, uint32_t milliSecond)
{
    LOGE("kalyke_cycle_post", "Free heap size is %d bytes", xPortGetFreeHeapSize());
    LOGV("Kalyke_monitor", "Enter %s(), report_cycle = %u, reportContentLen = %u", __func__, pCfgs->report_cycle, pCfgs->reportContentLen);
    char *tempStr = pvPortMalloc(1024);
    char *mqttBuf = pvPortMalloc(2048);
    memset(tempStr, 0, 1024);
    memset(mqttBuf, 0, 2048);
    
//    snvs_hp_rtc_datetime_t rtcDate;
//    kalyke_SNVS_HP_RTC_GetDatetime(&rtcDate);
//    rtc_datetime_t rtcDate;
//    RTC_GetDatetime(&rtcDate);
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    LOGE("Kalyke_monitor", "vender: %s", g_plc_netcfg.mqtt.vender);
    
    if (memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0)
    {
        mqtt_config_array_st *pRC = pCfgs->pReportContent;

        for(int i = 0; i < pCfgs->reportContentLen; i++)  //所有数据点都是SYM_CFG 则不需要推送
        {
            if(pRC[i].sym != SYM_CFG)
            {
                break;
            }
            if(i >= pCfgs->reportContentLen - 1) //直到最后一个数据点都是SYM_CFG
            {
                vPortFree(tempStr);
                vPortFree(mqttBuf);
                LOGD("Kalyke_monitor", "Leave %s(), mqttBuf = %s", __func__, mqttBuf);
                return;
            }
        }

        strcpy(mqttBuf, "{");
        //sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
        strcat(mqttBuf, tempStr);

        char device_id[24] = {0};
        //memcpy(device_id, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
        memcpy(device_id, gFlashParam.st.idInfo, 16);
        sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":%d,\"slave_name\":\"%s\",", device_id, pCfgs->slave_id, pCfgs->slave_name);
        strcat(mqttBuf, tempStr);

        strcat(mqttBuf, "\"data\":[");
        for (int i = 0; i < pCfgs->reportContentLen; i++)
        {
            if (pRC[i].sym == SYM_CFG)  //配置信息不需要上传
            {
                continue;
            }

            if (pRC[i].dataType == DTYPE_F32)  //(strcmp(pRC[i].dataType, "float32") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%.1f},", pRC[i].name, getElementValue_float(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_I16)  //(strcmp(pRC[i].dataType, "int16") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", pRC[i].name, getElementValue_int16(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_I32)  //(strcmp(pRC[i].dataType, "int32") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", pRC[i].name, getElementValue_int32(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_BOOL)  //(strcmp(pRC[i].dataType, "bool") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":%d},", pRC[i].name, getElementValue_bool(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_STRING)  //(strcmp(pRC[i].dataType, "string") == 0)
            {
                sprintf(tempStr, "{\"name\":\"%s\",\"value\":\"%s\"},", pRC[i].name, getElementValue_string(&pRC[i]));
            }
            else // int16 default
            {
                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
                tempStr[0] = '\0';
            }
            strcat(mqttBuf, tempStr);
        }
        int len = strlen(mqttBuf);
        if (pCfgs->reportContentLen > 0) 
            mqttBuf[len - 1] = '\0';
        strcat(mqttBuf, "]}\r\n");
    }
    else if (memcmp(g_plc_netcfg.mqtt.vender, "GDMQTT", 6) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "NFYMQTT", 7) == 0)
    {
        mqtt_config_array_st *pRC = pCfgs->pReportContent;

        for(int i = 0; i < pCfgs->reportContentLen; i++)  //所有数据点都是SYM_CFG 则不需要推送
        {
            if(pRC[i].sym != SYM_CFG)
            {
                break;
            }
            if(i >= pCfgs->reportContentLen - 1) //直到最后一个数据点都是SYM_CFG
            {
                vPortFree(tempStr);
                vPortFree(mqttBuf);
                LOGD("Kalyke_monitor", "Leave %s(), mqttBuf = %s", __func__, mqttBuf);
                return;
            }
        }

        strcpy(mqttBuf, "{");
        //sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
        sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", 2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
        strcat(mqttBuf, tempStr);
        
        char device_id[24] = {0};
        //memcpy(device_id, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
        memcpy(device_id, gFlashParam.st.idInfo, 16);
        sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":%d,\"slave_name\":\"%s\",", device_id, pCfgs->slave_id, pCfgs->slave_name);
        strcat(mqttBuf, tempStr);
        
        //memset(tempStr, 0, 512);
        strcat(mqttBuf, "\"data\":[{");
        for (int i = 0; i < pCfgs->reportContentLen; i++)
        {
            if (pRC[i].sym == SYM_CFG)  //配置信息不需要上传
            {
                continue;
            }
            if (memcmp(pRC[i].name, "ERRCODE", 7) == 0)
            {
                continue;
            }
            
            if (pRC[i].dataType == DTYPE_F32)  //(strcmp(pRC[i].dataType, "float32") == 0)
            {
                sprintf(tempStr, "\"%s\":%.1f,", pRC[i].name, getElementValue_float(&pRC[i]));                
            }
            else if (pRC[i].dataType == DTYPE_I16)  //(strcmp(pRC[i].dataType, "int16") == 0)
            {
//                int ret = strcmp(pRC[i].name, "ERRCODE");
//                if(ret == 0)
//                {
//                    //sprintf(tempStr, "\"%s\":0,", pRC[i].name);
//                }
//                else
//                {
                    sprintf(tempStr, "\"%s\":%d,", pRC[i].name, getElementValue_int16(&pRC[i]));
//                }
            }
            else if (pRC[i].dataType == DTYPE_I32)  //(strcmp(pRC[i].dataType, "int32") == 0)
            {
                sprintf(tempStr, "\"%s\":%d,", pRC[i].name, getElementValue_int32(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_BOOL)  //(strcmp(pRC[i].dataType, "bool") == 0)
            {
                sprintf(tempStr, "\"%s\":%d,", pRC[i].name, getElementValue_bool(&pRC[i]));
            }
            else if (pRC[i].dataType == DTYPE_STRING)  //(strcmp(pRC[i].dataType, "string") == 0)
            {
                sprintf(tempStr, "\"%s\":%s,", pRC[i].name, getElementValue_string(&pRC[i]));
            }
            else // int16 default
            {
                //sprintf(tempStr, "\"%s\":%d,", g_plc_netcfg.mqtt.pConfigs[i].name, getElementValue_int16(&g_plc_netcfg.mqtt.pConfigs[i]));
                tempStr[0] = '\0';
            }
            strcat(mqttBuf, tempStr);
        }
        int len = strlen(mqttBuf);
        if (pCfgs->reportContentLen > 0) 
            mqttBuf[len - 1] = '\0';
        strcat(mqttBuf, "}]}\r\n");
    }
    
//    char *topic = (char *)pvPortMalloc(128);
//    int payloadlen = 0;
//    memset(topic, 0, 128);
//    memcpy(topic, gFlashParam.st.mqttPub, 100);
//    payloadlen = strlen(mqttBuf);
//    Mqtt_Transmit(topic, mqttBuf, payloadlen);
//    LOGD("Kalyke_monitor", "Leave %s(), mqttBuf = %s", __func__, mqttBuf);
//    
//    
//    
//    vPortFree(topic);
    vPortFree(tempStr);
    kalyke_mqtt_publish(MQTT_TOPIC_POST, mqttBuf, 0);
    vPortFree(mqttBuf);
    
    aaa++;
    LOGE("Kalyke_monitor", "aaa = %d", aaa);
}

//void kalyke_post_alarm_to_mqtt(mqtt_config_array_st *pCfg)
//{
//    LOGV("alarm_monitor", "Enter %s(), vender = %s", __func__, g_plc_netcfg.mqtt.vender);
//    char *tempStr = pvPortMalloc(1024);
//    char *mqttBuf = pvPortMalloc(4096);

//    memset(mqttBuf, 0, 1024);
//    snvs_hp_rtc_datetime_t rtcDate;

//    kalyke_SNVS_HP_RTC_GetDatetime(&rtcDate);
//// \"\":
//    if (pCfg->alarmType == false)
//    {
//        strcpy(mqttBuf, "{\"AlarmType\":\"AlarmEvent\",");
//        pCfg->alarmType = true;
//    }
//    else
//    {
//        strcpy(mqttBuf, "{\"AlarmType\":\"AlarmRecover\",");
//        pCfg->alarmType = false;
//    }
//    sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
//    strcat(mqttBuf, tempStr);
//    sprintf(tempStr, "\"name\":\"%s\",", pCfg->name);
//    strcat(mqttBuf, tempStr);
//    #if 1
//    sprintf(tempStr, "\"content\":\"%s\",", pCfg->alarmContent);
//    strcat(mqttBuf, tempStr);
//    #endif
//    if (pCfg->dataType == DTYPE_F32)  //(strcmp(pCfg->dataType, "float32") == 0)
//    {
//        sprintf(tempStr, "\"value\":\"%.3f\"}", getElementValue_float(pCfg));
//    }
//    else if (pCfg->dataType == DTYPE_I16)  //(strcmp(pCfg->dataType, "int16") == 0)
//    {
//        sprintf(tempStr, "\"value\":\"%d\"}", getElementValue_int16(pCfg));
//    }
//    else if (pCfg->dataType == DTYPE_I32)  //(strcmp(pCfg->dataType, "int32") == 0)
//    {
//        sprintf(tempStr, "\"value\":\"%d\"}", getElementValue_int32(pCfg));
//    }
//    else if (pCfg->dataType == DTYPE_STRING)  //(strcmp(pCfg->dataType, "string") == 0)
//    {
//        sprintf(tempStr, "\"value\":\"%s\"}", getElementValue_string(pCfg));
//    }
//    strcat(mqttBuf, tempStr);
//    LOGV("alarm_monitor", "mqttBuf = %s", mqttBuf);
//    vPortFree(tempStr);
//    kalyke_mqtt_publish(g_plc_netcfg.mqtt.publish_topic_alarm, mqttBuf, 0);
//    vPortFree(mqttBuf);
//    LOGD("alarm_monitor", "Leave %s()", __func__);
//}

//void init_SD_SW_version(void)
//{
//#if 1
//    int pver[4] = {0, 0, 0, 0};
//    char str[20];
//    char i, j;
// 
//    strcpy(str, SW_VERSION);
//    for(j = 0, i = 0; j < strlen(str); j++)
//    {
//        if(str[j] == '.' || str[j] == '\0')
//        {
//            i++;
//            continue;
//        }
//        pver[i] = (str[j]-'0') + pver[i]*10;
//    }

//    LOGV("monitor", "ver1 = %d, ver2 = %d, ver3 = %d, ver4 = %d", pver[0], pver[1], pver[2], pver[3]);
//    SET_SD_ELEMENT_VALUE(SD209, pver[0]);
//    SET_SD_ELEMENT_VALUE(SD210, pver[1]);
//    SET_SD_ELEMENT_VALUE(SD211, pver[2]);
//    SET_SD_ELEMENT_VALUE(SD212, pver[3]);
//#elif 0
//    int ver1, ver2, ver3, ver4;
//    char str[20];
//  
//    strcpy(str, SW_VERSION);
//    LOGV("monitor", "%s", str);
//    sscanf(str, "%d.%d.%d.%d", &ver1, &ver2, &ver3, &ver4);

//    LOGV("monitor", "ver1 = %d, ver2 = %d, ver3 = %d, ver4 = %d", ver1, ver2, ver3, ver4);
//    SET_SD_ELEMENT_VALUE(SD209, ver1);
//    SET_SD_ELEMENT_VALUE(SD210, ver2);
//    SET_SD_ELEMENT_VALUE(SD211, ver3);
//    SET_SD_ELEMENT_VALUE(SD212, ver4);
//#else
//    int ver1, ver2, ver3, ver4;
//  
//    sscanf(SW_VERSION, "%d.%d.%d.%d", &ver1, &ver2, &ver3, &ver4);

//    LOGV("monitor", "ver1 = %d, ver2 = %d, ver3 = %d, ver4 = %d", ver1, ver2, ver3, ver4);
//    SET_SD_ELEMENT_VALUE(SD209, ver1);
//    SET_SD_ELEMENT_VALUE(SD210, ver2);
//    SET_SD_ELEMENT_VALUE(SD211, ver3);
//    SET_SD_ELEMENT_VALUE(SD212, ver4);
//#endif
//}

static void get_cycles_sorted(mqtt_config_array_hanyu_st *pCycles, uint8_t len)
{
    LOGV("monitor", "Enter %s(), len = %u", __func__, len);
    int i, j, isSorted;
    mqtt_config_array_hanyu_st temp;
    for (i = 0; i < len; i++)
    {
        pCycles[i].report_cycle = g_plc_netcfg.mqtt.pConfigsHANYU[i].report_cycle;
        pCycles[i].slave_id = g_plc_netcfg.mqtt.pConfigsHANYU[i].slave_id;
		strncpy(pCycles[i].slave_name, g_plc_netcfg.mqtt.pConfigsHANYU[i].slave_name,sizeof(pCycles[i].slave_name)-1);
        pCycles[i].reportContentLen = g_plc_netcfg.mqtt.pConfigsHANYU[i].reportContentLen;
        pCycles[i].pReportContent = g_plc_netcfg.mqtt.pConfigsHANYU[i].pReportContent;
    }

    for (i = 0; i < len - 1; i++)
    {
        isSorted = 1;  //假设剩下的元素已经排序好了
        for (j = 0; j < len - 1 - i; j++)
        {
            if(pCycles[j].report_cycle > pCycles[j + 1].report_cycle)
            {
                temp           = pCycles[j];
                pCycles[j]     = pCycles[j + 1];
                pCycles[j + 1] = temp;
                isSorted       = 0;  //一旦需要交换数组元素，就说明剩下的元素没有排序好
            }
        }
        if (isSorted) break; //如果没有发生交换，说明剩下的元素已经排序好了
    }
    //hexdump(pCycles, len * sizeof(mqtt_config_array_hanyu_st));
}

void monitor_publish_now_JIONTECH_reply(char *id)
{
    
}

void monitor_publish_now(void)
{
    LOGV("monitor", "Enter %s()", __func__);
    if (g_plc_netcfg.mqtt.pConfigsHANYU != NULL)
    {
        vTaskDelay(1000);
        while (gPublishQueue == NULL)
        {
            vTaskDelay(1000);
        }
        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
        {
            if (xQueueSend(gPublishQueue, &i, 0) != pdTRUE)
            {
                LOGE("monitor", "xQueueSend to gPublishQueue FAILED, i = %d. line: (%d)", i, __LINE__);
            }
        }
    }
    else
    {
        xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_MQTT_PUBLISH_NOW);
    }
}

static void publish_timer_callback(TimerHandle_t xTimer)
{
    uint32_t timerId = (uint32_t)pvTimerGetTimerID(xTimer);
    LOGV("monitor", "Enter %s(), timerId = %u", __func__, timerId);
    uint8_t msg = timerId;
    if (xQueueSend(gPublishQueue, &msg, 0) != pdTRUE)
    {
        LOGE("monitor", "xQueueSend to gPublishQueue FAILED. line: (%d)", __LINE__);
    }
}

static void mqtt_publish_part_check_task(void *p_arg)
{
    LOGV("Kalyke_monitor", "mqtt_publish_part_check_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    vTaskDelay(3002);
    TimerHandle_t publishTimer[g_plc_netcfg.mqtt.configLength];
    
    gPublishQueue = xQueueCreate(90, 1); //最多30个node，每个node预留3个
    LOGD("Kalyke_monitor", "gPublishQueue = 0x%08X", gPublishQueue);

    gpCycles = pvPortMalloc(sizeof(mqtt_config_array_hanyu_st) * g_plc_netcfg.mqtt.configLength);
    get_cycles_sorted(gpCycles, g_plc_netcfg.mqtt.configLength);
    vTaskDelay(10002);  //保证初次上传正常

    uint8_t recvIdx; //0xFF == publish all
    uint32_t lastPostTime = xTaskGetTickCount();
    uint32_t interval;

    for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
    {
        LOGV("Kalyke_monitor", "report_cycle = %u", gpCycles[i].report_cycle);
        publishTimer[i] = xTimerCreate((const char *)"publish",
                   (TickType_t  )gpCycles[i].report_cycle * 100 / portTICK_PERIOD_MS,
                   (UBaseType_t )pdTRUE,
                   (void *      )i,
                   (TimerCallbackFunction_t)publish_timer_callback);
        LOGI("Kalyke_monitor", "publishTimer[%d] = 0x%08X", i, publishTimer[i]);
        xTimerStart(publishTimer[i], 0);
    }
    uint32_t ms = 0;
    rtc_datetime_t rtcDate;	
    monitor_publish_now();
    while(1)
    {
        LOGI("Kalyke_monitor", "Let's wait in gPublishQueue...");
        if (xQueueReceive(gPublishQueue, &recvIdx, portMAX_DELAY))
        {
            LOGD("Kalyke_monitor", "recvIdx = 0x%X, mqtt.paused = %u", recvIdx, g_plc_netcfg.mqtt.paused);
            if (g_plc_netcfg.mqtt.paused == true)
            {
                continue;
            }
            interval = xTaskGetTickCount() - lastPostTime;
            LOGI("Kalyke_monitor", "interval = %u", interval);
            if (interval < 1000)
            {
                vTaskDelay(1000 - interval);
            }
        #if (KALYKE_FEATURE_4G_TCP_TASK == 1)
            while (g4GTCPBusy == true)
            {
                LOGW("Kalyke_monitor", "g4GTCPBusy == true, wait 2500ms...");
                vTaskDelay(2500);
            }
        #endif
            if (memcmp(g_plc_netcfg.mqtt.vender, "TLINK", 5) != 0)
            {
                ms = RTC_ReadDivider(&hrtc);
                LOGE("Kalyke_monitor", "ms == %d", ms);
            }
            kalyke_cycle_post(&gpCycles[recvIdx], &rtcDate, ms);
            lastPostTime = xTaskGetTickCount();
        }
        else
        {
            LOGE("Kalyke_monitor", "xQueueReceive gPublishQueue ERROR");
        }
    }
}

static void start_mqtt_publish_part_task(void)
{
    LOGV("Kalyke_monitor", "Enter %s()", __func__);
    
    BaseType_t ret = xTaskCreate((TaskFunction_t)mqtt_publish_part_check_task,
                      (const char *)"publish_part",
                      128,
                      (void *)NULL,
                      3,
                      NULL);
    if (ret != pdPASS)
    {
        LOGE("monitor", "Create monitor_TASK ERROR!");
    }
}

#if 1  //KALYKE_NEW_MQTT
static bool isTrigEvent(symbol_e sym, double* pd, double* pdtrig)
{
    bool ret;

    switch (sym)
    {
    case SYM_IDLE:
        ret = false;
        break;

    case SYM_EQUAL:
        ret = (*pd == *pdtrig ? true : false);
        break;

    case SYM_UNEQUAL:
        ret = (*pd != *pdtrig ? true : false);
        break;

    case SYM_CHG:
        if(*pd != *pdtrig)
        {
            ret = true;
            *pdtrig = *pd;
        }
        else
        {
            ret = false;
        }
        break;

    case SYM_GT:
        ret = (*pd > *pdtrig ? true : false);
        break;

    case SYM_LT:
        ret = (*pd < *pdtrig ? true : false);
        break;

    case SYM_GTE:
        ret = (*pd >= *pdtrig ? true : false);
        break;

    case SYM_LTE:
        ret = (*pd >= *pdtrig ? true : false);
        break;

    case SYM_CFG:  //配置信息不会上传
        ret = false;
        break;

    default:
        ret = false;
        LOGE("alarm_monitor", "symbol ERROR.");
        break;
    }

    return ret;
}

static double getElementValue_Str(mqtt_config_array_st *pRC, int i, char* pStr)
{
    double nowVal;

    if (pRC[i].dataType == DTYPE_F32)  //(strcmp(pRC[i].dataType, "float32") == 0)
    {
        nowVal = getElementValue_float(&pRC[i]);
        if(pStr != NULL)
        {
            sprintf(pStr, "%.1f", (float)nowVal);
        }
    }
    else if (pRC[i].dataType == DTYPE_I16)  //(strcmp(pRC[i].dataType, "int16") == 0)
    {
        nowVal = getElementValue_int16(&pRC[i]);
        if(pStr != NULL)
        {
            sprintf(pStr, "%d", (int16_t)nowVal);
        }
    }
    else if (pRC[i].dataType == DTYPE_I32)  //(strcmp(pRC[i].dataType, "int32") == 0)
    {
        nowVal = getElementValue_int32(&pRC[i]);
        if(pStr != NULL)
        {
            sprintf(pStr, "%d", (int32_t)nowVal);
        }
    }
    else if (pRC[i].dataType == DTYPE_BOOL)  //(strcmp(pRC[i].dataType, "bool") == 0)
    {
        nowVal = getElementValue_bool(&pRC[i]);
        if(pStr != NULL)
        {
            sprintf(pStr, "%d", (bool)nowVal);
        }
    }
    else
    {
        nowVal = getElementValue_int16(&pRC[i]);
        if(pStr != NULL)
        {
            sprintf(pStr, "%d", (int16_t)nowVal);
        }
    }

    return nowVal;
}

/*eventRecover  0  TrigEvent
                1  EventRcover */
static void kalyke_alarm_post(mqtt_config_array_hanyu_st *pCfgs, char eventRecover)
{    
    LOGV("Kalyke_monitor", "Enter %s(), reportContentLen = %u", __func__, pCfgs->reportContentLen);
    char *tempStr = pvPortMalloc(512);
    char *mqttBuf = pvPortMalloc(1024);
    memset(tempStr, 0, 512);
    memset(mqttBuf, 0, 1024);
    
    char *valStr = pvPortMalloc(64);
    double nowVal = 0.0;
    memset(valStr, 0, 64);
    
    LOGV("Kalyke_monitor", "Free heap: %u", xPortGetFreeHeapSize());
//    rtc_datetime_t rtcDate;
//    RTC_GetDatetime(&rtcDate);
    
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);

    if (memcmp(g_plc_netcfg.mqtt.vender, "HANYU", 5) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0)
    {
        strcpy(mqttBuf, "{");

        if(eventRecover == 0)
        {
            sprintf(tempStr, "\"alarm_type\":\"TrigEvent\",");
        }
        else
        {
            sprintf(tempStr, "\"alarm_type\":\"EventRecover\",");
        }
        strcat(mqttBuf, tempStr);

//        sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
        strcat(mqttBuf, tempStr);

        if (memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0)
        {
            char device_id[24] = {0};
            //memcpy(device_id, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
            memcpy(device_id, gFlashParam.st.idInfo, 16);
            sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":%d,\"slave_name\":\"%s\",", device_id, pCfgs->slave_id, pCfgs->slave_name);
            strcat(mqttBuf, tempStr);
        }
        strcat(mqttBuf, "\"data\":[");
        mqtt_config_array_st *pRC = pCfgs->pReportContent;
        for (int i = 0; i < pCfgs->reportContentLen; i++)
        {
            if (pRC[i].sym == SYM_CFG)  //配置信息不需要上传
            {
                continue;
            }

            nowVal = getElementValue_Str(pRC, i, valStr);
            if(isTrigEvent(pRC[i].sym, &nowVal, &pRC[i].alarmVal))
            {
                sprintf(tempStr, "{\"%s\":%s,\"cont\":\"%s\"},", pRC[i].name, valStr, pRC[i].alarmContent);  //显示触发content
                pRC[i].alarmType = true;
            }
            else
            {
                sprintf(tempStr, "{\"%s\":%s,\"cont\":\"\"},", pRC[i].name, valStr);  //不显示触发content
                pRC[i].alarmType = false;
            }
            strcat(mqttBuf, tempStr);
        }

        int len = strlen(mqttBuf);
        if (pCfgs->reportContentLen > 0) 
            mqttBuf[len - 1] = '\0';
        strcat(mqttBuf, "]}\r\n");
    }
    else if (memcmp(g_plc_netcfg.mqtt.vender, "GDMQTT", 6) == 0)
    {
        strcpy(mqttBuf, "{");

//        if(eventRecover == 0)
//        {
//            sprintf(tempStr, "\"alarm_type\":\"TrigEvent\",");
//        }
//        else
//        {
//            sprintf(tempStr, "\"alarm_type\":\"EventRecover\",");
//        }
//        strcat(mqttBuf, tempStr);
        
        sprintf(tempStr, "\"time\":\"%04u-%02u-%02u %02u:%02u:%02u\",", 2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date, stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
        strcat(mqttBuf, tempStr);

        char device_id[24] = {0};
        //memcpy(device_id, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
        memcpy(device_id, gFlashParam.st.idInfo, 16);
        sprintf(tempStr, "\"device_id\":\"%s\",\"slave_id\":%d,\"slave_name\":\"%s\",", device_id, pCfgs->slave_id, pCfgs->slave_name);
        strcat(mqttBuf, tempStr);
                    
        strcat(mqttBuf, "\"data\":[{");
        mqtt_config_array_st *pRC = pCfgs->pReportContent;
        for (int i = 0; i < pCfgs->reportContentLen; i++)
        {
            if (pRC[i].sym == SYM_CFG)  //配置信息不需要上传
            {
                continue;
            }
            if (memcmp(pRC[i].name, "ERRCODE", 7) == 0)
            {
                sprintf(tempStr, "\"%s\":0,", "ERRCODE");
            }
            nowVal = getElementValue_Str(pRC, i, valStr);
            if(isTrigEvent(pRC[i].sym, &nowVal, &pRC[i].alarmVal))
            {                
                sprintf(tempStr, "\"%s\":1,", "ERRCODE");
                strcat(mqttBuf, tempStr);
                sprintf(tempStr, "\"%s\":%s,", pRC[i].name, valStr);
                strcat(mqttBuf, tempStr);               
                pRC[i].alarmType = true;
            }
            else
            {
                strcat(mqttBuf, tempStr);              
                pRC[i].alarmType = false;
            }
        }                 
        int len = strlen(mqttBuf);
        if (pCfgs->reportContentLen > 0) 
            mqttBuf[len - 1] = '\0';
        strcat(mqttBuf, "}]}\r\n");
    }

    vPortFree(tempStr);
    vPortFree(valStr);
    kalyke_mqtt_publish(MQTT_TOPIC_ALARM, mqttBuf, 0);
    LOGW("Kalyke_monitor", "Leave %s(), mqttBuf = %s", __func__, mqttBuf);
    vPortFree(mqttBuf);
}

void kalyke_alarm_monitor_task(void *p_arg)
{
    LOGV("alarm_monitor", "kalyke_alarm_monitor_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    vTaskDelay(5001);
    char eventRecover;
    double nowVal = 0.0;
    mqtt_config_array_st *pRC;
    while (1)
    {
        vTaskDelay(5000); // Every 5 seconds checks
        LOGV("alarm_monitor", "Check alarm begin...");
        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
        {
            pRC = g_plc_netcfg.mqtt.pConfigsHANYU[i].pReportContent;
            eventRecover = 0;
            for (int j = 0; j < g_plc_netcfg.mqtt.pConfigsHANYU[i].reportContentLen; j++)
            {
                if (pRC[j].sym == SYM_IDLE)
                {
                    continue;
                }

                nowVal = getElementValue_Str(pRC, j, NULL);
                //LOGV("alarm_monitor", "nowVal = %f, sym = %u, alarmVal = %f, dataType = %s", nowVal, pRC[j].sym, pRC[j].alarmVal, pRC[j].dataType);
                //LOGD("alarm_monitor", "address = %u", pRC[j].address);

                if(pRC[j].alarmType == false)
                {
                    if(isTrigEvent(pRC[j].sym, &nowVal, &pRC[j].alarmVal))//数据点未触发事件时触发
                    {
                        eventRecover = 0;
                        kalyke_alarm_post(&g_plc_netcfg.mqtt.pConfigsHANYU[i], eventRecover);//事件触发后上传                        
                        break;
                    }                    
                }
                else
                {
                    if(!isTrigEvent(pRC[j].sym, &nowVal, &pRC[j].alarmVal))//数据点触发事件时恢复
                    {
                        eventRecover = 1;//事件恢复后上传
                        continue;
                    }
                }
            }
            if(eventRecover)
            {
                kalyke_alarm_post(&g_plc_netcfg.mqtt.pConfigsHANYU[i], eventRecover);
            }
        }
        LOGV("alarm_monitor", "Check alarm finish...");
    }
}
#else
void kalyke_alarm_monitor_task(void *p_arg)
{
    LOGV("alarm_monitor", "kalyke_alarm_monitor_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    vTaskDelay(5001);
    double nowVal = 0.0;
    mqtt_config_array_st *pRC;
    while (1)
    {
        vTaskDelay(5000); // Every 5 seconds checks
        //LOGV("alarm_monitor", "Check alarm begin...");
        for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
        {
            pRC = g_plc_netcfg.mqtt.pConfigsHANYU[i].pReportContent;
            for (int j = 0; j < g_plc_netcfg.mqtt.pConfigsHANYU[i].reportContentLen; j++)
            {
                if (pRC[j].sym == SYM_IDLE)
                {
                    continue;
                }
                if (strcmp(pRC[j].dataType, "float32") == 0)
                {
                    nowVal = getElementValue_float(&pRC[j]);
                }
                else if (strcmp(pRC[j].dataType, "int16") == 0)
                {
                    nowVal = getElementValue_int16(&pRC[j]);
                }
                else if (strcmp(pRC[j].dataType, "int32") == 0)
                {
                    nowVal = getElementValue_int32(&pRC[j]);
                }
                else if (strcmp(pRC[j].dataType, "bool") == 0)
                {
                    nowVal = getElementValue_bool(&pRC[j]);
                }
                //LOGV("alarm_monitor", "nowVal = %f, sym = %u, alarmVal = %f, dataType = %s", nowVal, pRC[j].sym, pRC[j].alarmVal, pRC[j].dataType);
                //LOGD("alarm_monitor", "address = %u", pRC[j].address);
                switch (pRC[j].sym)
                {
                    case SYM_IDLE:
                    case SYM_EQUAL:
                        // Do nothing.
                        break;

                    case SYM_GTE:
                    case SYM_GT:
                        if (nowVal >= pRC[j].alarmVal)
                        {
                            kalyke_post_alarm_to_mqtt(pRC);
                            pRC[j].sym = SYM_LT;
                        }
                        break;

                    case SYM_LTE:
                    case SYM_LT:
                        if (nowVal <= pRC[j].alarmVal)
                        {
                            kalyke_post_alarm_to_mqtt(pRC);
                            pRC[j].sym = SYM_GT;
                        }
                        break;

                    default:
                        LOGE("alarm_monitor", "symbol ERROR.");
                        break;
                }
            }
        }
        //LOGV("alarm_monitor", "Check alarm finish...");
    }
}
#endif

static void start_alarm_monitor_task(void)
{
    LOGV("alarm_monitor", "Enter %s(), pConfigsHANYU = 0x%08X", __func__, g_plc_netcfg.mqtt.pConfigsHANYU);
    if (g_plc_netcfg.mqtt.pConfigsHANYU == NULL)
    {
        return;
    }
    for (int i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
    {
        for (int j = 0; j < g_plc_netcfg.mqtt.pConfigsHANYU[i].reportContentLen; j++)
        {
            if (g_plc_netcfg.mqtt.pConfigsHANYU[i].pReportContent[j].sym != SYM_IDLE)
            {
                LOGI("alarm_monitor", "Find a alarm....%d", i);
                xTaskCreate((TaskFunction_t)kalyke_alarm_monitor_task,
                          (const char *)"Alarm_task",
                          128,
                          (void *)NULL,
                          3,
                          NULL);
                return;
            }
        }
    }
}

void osThreadNew_MonitorTask(void)
{
    MonitorTaskHandle = osThreadNew(kalyke_monitor_task, NULL, &monitor_attributes);
}

void kalyke_monitor_task(void *p_arg)
{
    LOGW("Kalyke_monitor", "monitor_heap_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
        
    //uint32_t *pSD205 = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD205];
    //start_second_task();
    //init_SD_SW_version();

//    rtc_datetime_t rtcDate;

//    tempInit();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
//    printSomeGPIOPriority();
//    bsp_save_boot_time();
//    printSomeOCOTP();
//    Kalyke_PrintRunFrequency(0);
//    testDouble();
    
//    vTaskDelay(10000);
    
    LOGV("monitor", "pConfigsHANYU = 0x%08X, pConfigs = 0x%08X", g_plc_netcfg.mqtt.pConfigsHANYU, g_plc_netcfg.mqtt.pConfigs);
    if (strlen(g_plc_netcfg.mqtt.host) != 0)
    {
        if (g_plc_netcfg.mqtt.pConfigsHANYU != NULL)
        {
            start_mqtt_publish_part_task();
        }
        else if (g_plc_netcfg.mqtt.pConfigs != NULL)
        {
            //start_mqtt_publish_all_task();
        }

        start_alarm_monitor_task();
    }
    else
    {
        LOGW("monitor", "Mqtt host name is null, so do nothing.");    
    }

//#if 1 /* 不需要网口初始化好才运行 */
    vTaskDelay(5000);
//    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_LED);
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC);
//#else
//    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
//#endif

//    vTaskDelay(5001 / portTICK_PERIOD_MS);

//    double temp = 0;
    
    LOGI("Monitor", "Before into monitor loop.");
    while(1)
    {
//        vTaskDelay(2000);
//        bsp_feed_watch_dog();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
//#if (LOG_OPEN == 1)
//        if (gKalykeSecondTickCurrent % 7 == 0)
//        {
//            //testSomething();
//        }
//        if (gKalykeSecondTickCurrent % 31 == 0)
//        {
//            print_vTaskList(__func__, __LINE__);
//        }
//        if (gKalykeSecondTickCurrent % 6 == 0)
//        {
//            LOGI("monitor","mlv_DeviceTypeId = 0x%04X, SW_VERSION = %s, SystemCoreClock=%u(Hz)\r\n", gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId, SW_VERSION,SystemCoreClock);
//            showDeviceIDString();
//        }
//        if (gKalykeSecondTickCurrent % 10 == 0)
//        {
//            SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
//            LOGI("monitor", "RTC: %04u-%02u-%02u  %02u:%02u:%02u", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);

//#if KALYKE_DS1302_FEATURE == 1
//            uint8_t buf[8] = {0};
//            //DS1302_ReadTime2(buf);
//            //LOGE("monitor", "DS1302_ReadTime2: %x-%x-%x  %x:%x:%x, week:%u", buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
//            DS1302_ReadTimeBurst(buf);
//            LOGE("monitor", "DS1302_ReadTimeBurst: %04u-%02u-%02u  %02u:%02u:%02u, week:%u\r\n", buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]&0x7F, buf[7]);
//#endif

//            #if 0
//            uint32_t HP_RTC_MR = SNVS->HPRTCMR;
//            uint32_t HP_RTC_LR = SNVS->HPRTCLR;
//            LOGI("monitor", "HP_RTC_MR = 0x%08X, HP_RTC_LR = 0x%08X", HP_RTC_MR, HP_RTC_LR);
//            #endif
//            //uint32_t lpUart2Priority = NVIC_GetPriority(LPUART2_IRQn);
//            //LOGI("monitor","lpUart2Priority = %d\r\n", lpUart2Priority);
//        }
//        if (gKalykeSecondTickCurrent == 10)
//        {
//            uint8_t *p_m_interrupts_start = (uint8_t *)0x60002000;
//            uint32_t m_interrupts_size = 0x400;
//            //hexdump(p_m_interrupts_start, m_interrupts_size);
//        }
//        if (gKalykeSecondTickCurrent == 11)
//        {
//            uint8_t *p_m_interrupts_ram_start = (uint8_t *)0x20000000;
//            uint32_t m_interrupts_ram_size = 0x400;
//            //hexdump(p_m_interrupts_ram_start, m_interrupts_ram_size + 16);
//        }
//#endif
//        if (gKalykeSecondTickCurrent % 15 == 0)
//        {
//            TEMPMON_StartMeasure(TEMPMON);
//            temp = TEMPMON_GetCurrentTemperature(TEMPMON);
//            TEMPMON_StopMeasure(TEMPMON);
//            SET_SD_ELEMENT_VALUE(SD204, temp * 10);
//            *pSD205 = xPortGetFreeHeapSize();

//            LOGV("monitor", "gX0IntCount = %u. Free heap = %u(bytes), MCU temperature: %.1f", gX0IntCount, *pSD205, temp);
//        }
//        if (gKalykeSecondTickCurrent % 3 == 0)
//        {
//            SET_SD_ELEMENT_VALUE(SD223, getCurrentInternetDevice());
//            LOGV("monitor", "Run time: (%u, %u)", gKalykeSecondTickCurrent, gKalykeSecondTick);
//        }
//#if KALYKE_DS1302_FEATURE == 1
//        if (gKalykeSecondTickCurrent % 86400 == 0)// 24 * 60 * 60 = 86400 , 每隔24小时
//        {
//            LOGE("why", "gKalykeSecondTickCurrent = %d", gKalykeSecondTickCurrent);
//            kalyke_Synch_ds1302_time();
//        }
//#endif
    }
}

