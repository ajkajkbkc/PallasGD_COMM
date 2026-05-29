
#ifndef __DS18B20_H
#define __DS18B20_H

/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"


/* Private defines -----------------------------------------------------------*/
/* DS18B20 read temperature command */
#define DS18B20_CMD_CONVERTTEMP			0x44 	/* Convert temperature */
#define DS18B20_DECIMAL_STEPS_12BIT		0.0625
#define DS18B20_DECIMAL_STEPS_11BIT		0.125
#define DS18B20_DECIMAL_STEPS_10BIT		0.25
#define DS18B20_DECIMAL_STEPS_9BIT		0.5

#define DS18B20_NUM_1    0
#define DS18B20_NUM_2    1
#define DS18B20_NUM_3    2
#define DS18B20_NUM_ALL  2  //目前只用两个温度传感器

#define DS18B20_TRY_TIMES_MAX    5    //最大尝试次数

/* Exported types ------------------------------------------------------------*/




/* Exported constants --------------------------------------------------------*/



/* Private functions ---------------------------------------------------------*/
void osThreadNew_ds18b20Task(void);


#endif /* __DS18B20_H */

