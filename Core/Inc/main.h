/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "app_opts.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;
extern IWDG_HandleTypeDef hiwdg;
extern RTC_HandleTypeDef hrtc;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
#define  collectTaskPriority     osPriorityNormal2
#define  mbtcpTaskPriority       osPriorityNormal3
#define  netbiosTaskPriority     osPriorityNormal4
#define  httpTaskPriority        osPriorityNormal5
#define  mqttTaskPriority        osPriorityNormal6
#define  hzudpTaskPriority       osPriorityNormal7
#define  lltudpTaskPriority      osPriorityNormal7
#define  keyTaskPriority         osPriorityAboveNormal
#define  keyTaskOLEDPriority     osPriorityAboveNormal
#define  ledTaskPriority         osPriorityAboveNormal1
#define  L3_inputTaskPriority    osPriorityAboveNormal1
#define  uartTaskPriority        osPriorityAboveNormal2
#define  internetTaskPriority    osPriorityAboveNormal3
#define  ntcTaskPriority         osPriorityAboveNormal4
#define  att7022TaskPriority     osPriorityAboveNormal4
#define  rn8209dTaskPriority     osPriorityAboveNormal4
#define  sspd_inputTaskPriority  osPriorityAboveNormal5
#define  inputTaskPriority       osPriorityAboveNormal5
#define  ds18b20TaskPriority     osPriorityAboveNormal6
#define  R2900TaskPriority       osPriorityAboveNormal7
#define  thunderTaskPriority     osPriorityHigh1
#define  AFDTaskPriority         osPriorityHigh2

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/



/* USER CODE BEGIN Private defines */
/* LED ---------------------------------------- */
#if PROD_TYPE == PROD_FSS
#define LED_1_Pin GPIO_PIN_15
#define LED_1_GPIO_Port GPIOA
#define LED_2_Pin GPIO_PIN_3
#define LED_2_GPIO_Port GPIOB
#define LED_3_Pin GPIO_PIN_5
#define LED_3_GPIO_Port GPIOB
#elif PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
#define LED_1_Pin GPIO_PIN_8
#define LED_1_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_9
#define LED_2_GPIO_Port GPIOB
#elif PROD_TYPE == PROD_FL
#define LED_1_Pin GPIO_PIN_12
#define LED_1_GPIO_Port GPIOC
#define LED_2_Pin GPIO_PIN_2
#define LED_2_GPIO_Port GPIOD
#define LED_3_Pin GPIO_PIN_5
#define LED_3_GPIO_Port GPIOB
#else
#define LED_1_Pin GPIO_PIN_4
#define LED_1_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_5
#define LED_2_GPIO_Port GPIOB
#define LED_3_Pin GPIO_PIN_8
#define LED_3_GPIO_Port GPIOB
#endif
/* KEY ---------------------------------------- */
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
#define KEY_1_Pin GPIO_PIN_15
#define KEY_1_GPIO_Port GPIOA
#define KEY_2_Pin GPIO_PIN_3
#define KEY_2_GPIO_Port GPIOB
#define KEY_3_Pin GPIO_PIN_5
#define KEY_3_GPIO_Port GPIOB
#define KEY_4_Pin GPIO_PIN_4
#define KEY_4_GPIO_Port GPIOB
#define KEY_5_Pin GPIO_PIN_12
#define KEY_5_GPIO_Port GPIOC
#else
#if PROD_TYPE == PROD_FSS
#define KEY_1_Pin GPIO_PIN_12
#define KEY_1_GPIO_Port GPIOC
#elif PROD_TYPE == PROD_FL
#define KEY_1_Pin GPIO_PIN_3
#define KEY_1_GPIO_Port GPIOB
#else
#define KEY_1_Pin GPIO_PIN_6
#define KEY_1_GPIO_Port GPIOB
#endif
#define KEY_2_Pin GPIO_PIN_2
#define KEY_2_GPIO_Port GPIOD
#endif
/* TM1650 ------------------------------------- */
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
#define TM1650_SCL_Pin GPIO_PIN_6
#define TM1650_SCL_GPIO_Port GPIOB
#define TM1650_SDA_Pin GPIO_PIN_7
#define TM1650_SDA_GPIO_Port GPIOB
#else
#define TM1650_SDA_Pin GPIO_PIN_11
#define TM1650_SDA_GPIO_Port GPIOA
#define TM1650_SCL_Pin GPIO_PIN_12
#define TM1650_SCL_GPIO_Port GPIOA
#endif
/* W5500 -------------------------------------- */
#define W5500_CS_Pin GPIO_PIN_4
#define W5500_CS_GPIO_Port GPIOA
#define W5500_SCK_Pin GPIO_PIN_5
#define W5500_SCK_GPIO_Port GPIOA
#define W5500_MISO_Pin GPIO_PIN_6
#define W5500_MISO_GPIO_Port GPIOA
#define W5500_MOSI_Pin GPIO_PIN_7
#define W5500_MOSI_GPIO_Port GPIOA
#define W5500_INT_Pin GPIO_PIN_6
#define W5500_INT_GPIO_Port GPIOC
#define W5500_INT_EXTI_IRQn EXTI4_IRQn
#define W5500_RST_Pin GPIO_PIN_7
#define W5500_RST_GPIO_Port GPIOC
/* ZIGBEE ------------------------------------- */
#define ZIGBEE_RST_Pin GPIO_PIN_0
#define ZIGBEE_RST_GPIO_Port GPIOA
#define ZIGBEE_DEF_Pin GPIO_PIN_1
#define ZIGBEE_DEF_GPIO_Port GPIOA
/* ATT7022 ------------------------------------ */

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
#define ATT7022_NSS_Pin GPIO_PIN_12
#define ATT7022_NSS_GPIO_Port GPIOB
#define ATT7022_SCK_Pin GPIO_PIN_13
#define ATT7022_SCK_GPIO_Port GPIOB
#define ATT7022_MISO_Pin GPIO_PIN_14
#define ATT7022_MISO_GPIO_Port GPIOB
#define ATT7022_MOSI_Pin GPIO_PIN_15
#define ATT7022_MOSI_GPIO_Port GPIOB
#else
#define ATT7022_MISO_Pin GPIO_PIN_12
#define ATT7022_MISO_GPIO_Port GPIOB
#define ATT7022_MOSI_Pin GPIO_PIN_13
#define ATT7022_MOSI_GPIO_Port GPIOB
#define ATT7022_SCK_Pin GPIO_PIN_14
#define ATT7022_SCK_GPIO_Port GPIOB
#define ATT7022_CS_Pin GPIO_PIN_15
#define ATT7022_CS_GPIO_Port GPIOB
#endif
/* FSS ---------------------------------------- */
#define FSS_COUNT_Pin GPIO_PIN_13
#define FSS_COUNT_GPIO_Port GPIOC
#define FSS_COUNT_EXTI_IRQn EXTI15_10_IRQn
#define MCU_S_PE GPIO_PIN_6
#define MCU_S_PE_GPIO_Port GPIOC
//#define MCU_S_L1 GPIO_PIN_10
//#define MCU_S_L1_GPIO_Port GPIOC
//#define MCU_S_L2 GPIO_PIN_11
//#define MCU_S_L2_GPIO_Port GPIOC
#define MCU_S_L3 GPIO_PIN_1
#define MCU_S_L3_GPIO_Port GPIOD
#define FSS_SW1_Pin GPIO_PIN_9
#define FSS_SW1_GPIO_Port GPIOC
#define FSS_SW2_Pin GPIO_PIN_8
#define FSS_SW2_GPIO_Port GPIOA
#define FSS_SW3_Pin GPIO_PIN_11
#define FSS_SW3_GPIO_Port GPIOA
#define FSS_SW4_Pin GPIO_PIN_12
#define FSS_SW4_GPIO_Port GPIOA
#define FSS_TEMP1_Pin GPIO_PIN_0
#define FSS_TEMP1_GPIO_Port GPIOC
#define FSS_TEMP2_Pin GPIO_PIN_1
#define FSS_TEMP2_GPIO_Port GPIOC
#define FSS_TEMP3_Pin GPIO_PIN_2
#define FSS_TEMP3_GPIO_Port GPIOC
#define FSS_TEMP4_Pin GPIO_PIN_3
#define FSS_TEMP4_GPIO_Port GPIOC
#define FSS_C_ID1_Pin GPIO_PIN_0
#define FSS_C_ID1_GPIO_Port GPIOB
#define FSS_C_ID2_Pin GPIO_PIN_1
#define FSS_C_ID2_GPIO_Port GPIOB
#define FSS_S_ID1_Pin GPIO_PIN_7
#define FSS_S_ID1_GPIO_Port GPIOC
#define FSS_S_ID2_Pin GPIO_PIN_8
#define FSS_S_ID2_GPIO_Port GPIOC
#define FSS_D_ID1_Pin GPIO_PIN_8
#define FSS_D_ID1_GPIO_Port GPIOB
#define FSS_D_ID2_Pin GPIO_PIN_9
#define FSS_D_ID2_GPIO_Port GPIOB
/* FL ---------------------------------------- */
#define LT_N_ADC_Pin GPIO_PIN_0
#define LT_N_ADC_GPIO_Port GPIOC
#define LT_P_ADC_Pin GPIO_PIN_1
#define LT_P_ADC_GPIO_Port GPIOC
#define LT_P_EXTI2_Pin GPIO_PIN_2
#define LT_P_EXTI2_GPIO_Port GPIOC
#define LT_P_EXTI2_EXTI_IRQn EXTI2_IRQn
#define LT_N_EXTI3_Pin GPIO_PIN_3
#define LT_N_EXTI3_GPIO_Port GPIOC
#define LT_N_EXTI3_EXTI_IRQn EXTI3_IRQn
#define LT_P_DISCHARGE_Pin GPIO_PIN_0
#define LT_P_DISCHARGE_Port GPIOB
#define LT_N_DISCHARGE_Pin GPIO_PIN_1
#define LT_N_DISCHARGE_Port GPIOB
/* FS ---------------------------------------- */
#define FS_YL_Pin GPIO_PIN_0
#define FS_YL_GPIO_Port GPIOA
#define FS_KL_Pin GPIO_PIN_1
#define FS_KL_GPIO_Port GPIOA
#define FS_PE_Pin GPIO_PIN_8
#define FS_PE_GPIO_Port GPIOA
#define FS_TEMP1_Pin GPIO_PIN_1
#define FS_TEMP1_GPIO_Port GPIOB
#define FS_TEMP2_Pin GPIO_PIN_10
#define FS_TEMP2_GPIO_Port GPIOB
#define FS_TEMP3_Pin GPIO_PIN_11
#define FS_TEMP3_GPIO_Port GPIOB
#define FS_COUNT_Pin GPIO_PIN_2
#define FS_COUNT_GPIO_Port GPIOC
#define FS_COUNT_EXTI_IRQn EXTI2_IRQn

#define DI1_Pin GPIO_PIN_8
#define DI1_GPIO_Port GPIOC
#define DI2_Pin GPIO_PIN_9
#define DI2_GPIO_Port GPIOC

#define DI3_Pin GPIO_PIN_10
#define DI3_GPIO_Port GPIOB
#define DI4_Pin GPIO_PIN_11
#define DI4_GPIO_Port GPIOB

#define DO1_Pin GPIO_PIN_12
#define DO1_GPIO_Port GPIOA
#define DO2_Pin GPIO_PIN_11
#define DO2_GPIO_Port GPIOA

/* common defines -----------------------------------------------------------*/
#define LED_1_G()  HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET)
#define LED_1_R() HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET)
#define LED_1_TOGGLE() HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin)
#define LED_2_G()  HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_SET)
#define LED_2_R() HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET)
#define LED_2_TOGGLE() HAL_GPIO_TogglePin(LED_2_GPIO_Port, LED_2_Pin)
#if PROD_TYPE != PROD_SFE && PROD_TYPE != PROD_SFB && PROD_TYPE != PROD_SFA
#define LED_3_G()  HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_SET)
#define LED_3_R() HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_RESET)
#define LED_3_TOGGLE() HAL_GPIO_TogglePin(LED_3_GPIO_Port, LED_3_Pin)
#endif

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
