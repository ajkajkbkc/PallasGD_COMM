
//#ifndef __APP_THUNDER_H
//#define __APP_THUNDER_H

///* Includes ------------------------------------------------------------------*/
//#include "main.h"


///* Private defines -----------------------------------------------------------*/
///* The event group allows multiple bits for each event */
//#define EVENT_THUNDER_P_HAPPENED            0x00000001UL
//#define EVENT_THUNDER_N_HAPPENED            0x00000002UL

///* channal 11 正极雷 / channal 10 负极雷  */
//#define  LT_POLARITY_P                      0x00000001UL    //正极性雷
//#define  LT_POLARITY_N                      0x00000002UL    //负极性雷

//#define TIME_OUT_TO_ENBLE_IRQ               500             //100ms后在开启中断

///* Exported types ------------------------------------------------------------*/



///* Exported constants --------------------------------------------------------*/




///* Private functions ---------------------------------------------------------*/
//void extiTrig_Thunder_FromISR(uint8_t Polarity);
//void osThreadNew_thunderTask(void);

//uint16_t IPeak_Calc_Q(uint16_t ipeak);
//uint16_t IPeak_Calc_WR(uint16_t ipeak);

//#endif /* __APP_THUNDER_H */
