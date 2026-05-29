
#ifndef __LLTUDP_H
#define __LLTUDP_H

/* Includes ------------------------------------------------------------------*/




/* Private defines -----------------------------------------------------------*/
#define LLTUDP_VERSION    0x03   //路路通协议版本

//路路通采集器定义
#define LLTUDP_TYPE_JCII  0x0B   //四要素SPD监测
#define LLTUDP_TYPE_ZJCII 0x0C   //多要素SPD监测
#define LLTUDP_TYPE_JDII  0x0D   //接地电阻


/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
void osThreadNew_lltudpTask(void);




#endif /* __LLTUDP_H */
