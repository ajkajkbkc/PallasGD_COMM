/**
  ******************************************************************************
  * @file    kalyke_monitor_task.h
  * @author  lixianyu
  * @version V0.0.2
  * @date    2019-05-02
  * @brief   通过调试串口打印出RTOS的剩余HEAP的字节数
             一些监控信息，比如温度等
  ******************************************************************************
  */
#ifndef __KALYKE_MONITOR_TASK_H
#define __KALYKE_MONITOR_TASK_H
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

extern volatile uint8_t aaa;

extern volatile uint8_t UAHarmonici;
extern volatile uint8_t UAHarmonicj;
extern volatile uint8_t UBHarmonici;
extern volatile uint8_t UBHarmonicj;
extern volatile uint8_t UCHarmonici;
extern volatile uint8_t UCHarmonicj;

extern volatile uint8_t IAHarmonici;
extern volatile uint8_t IAHarmonicj;
extern volatile uint8_t IBHarmonici;
extern volatile uint8_t IBHarmonicj;
extern volatile uint8_t ICHarmonici;
extern volatile uint8_t ICHarmonicj;

extern volatile uint8_t masterTOalarm;

void kalyke_cycle_post_master_ELECTRIC(void);
void kalyke_cycle_post_master_Imbalance(void);
void kalyke_cycle_post_master_Uharmonic2_11(void);
void kalyke_cycle_post_master_Uharmonic12_21(void);
void kalyke_cycle_post_master_Uharmonic22_31(void);
void kalyke_cycle_post_master_Iharmonic2_11(void);
void kalyke_cycle_post_master_Iharmonic12_21(void);
void kalyke_cycle_post_master_Iharmonic22_31(void);
void kalyke_cycle_post_master_DIDOTEMP(void);

void kalyke_alarm_post_master(void);
void kalyke_alarm_post_master_SC_EA_VC(void);

extern osThreadId_t MonitorTaskHandle;
void osThreadNew_MonitorTask(void);
extern volatile uint32_t gKalykeSecondTick;
extern volatile uint32_t gKalykeSecondTickCurrent;
extern volatile uint32_t gWorldSecondTick;
extern void kalyke_monitor_task(void *p_arg);

extern TaskHandle_t gKalykeMonitorTaskHandle;
extern TaskHandle_t gKalykeSecondTaskHandle;
extern void Kalyke_PrintRunFrequency(int32_t run_freq_only);
extern void init_SD_SW_version(void);
extern void kalyke_monitor_init(void);
extern void monitor_publish_now(void);
extern void print_vTaskList(void);
#endif /* __KALYKE_MONITOR_TASK_H */

