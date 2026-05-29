
///* Private includes ----------------------------------------------------------*/
//#include "app_tm1650.h"
//#include "app_parameter.h"

//#include "cmsis_os.h"

///* Private define ------------------------------------------------------------*/
//#define TM1650_SCL_Set()    HAL_GPIO_WritePin(TM1650_SCL_GPIO_Port, TM1650_SCL_Pin, GPIO_PIN_SET)
//#define TM1650_SCL_Reset()  HAL_GPIO_WritePin(TM1650_SCL_GPIO_Port, TM1650_SCL_Pin, GPIO_PIN_RESET)

//#define TM1650_SDA_Set()    HAL_GPIO_WritePin(TM1650_SDA_GPIO_Port, TM1650_SDA_Pin, GPIO_PIN_SET)
//#define TM1650_SDA_Reset()  HAL_GPIO_WritePin(TM1650_SDA_GPIO_Port, TM1650_SDA_Pin, GPIO_PIN_RESET)

//#define TM1650_READ_SDA     HAL_GPIO_ReadPin(TM1650_SDA_GPIO_Port, TM1650_SDA_Pin)

//typedef enum
//{
//    NUM_0 = 0,
//    NUM_1,
//    NUM_2,
//    NUM_3,
//    NUM_4,
//    NUM_5,
//    NUM_6,
//    NUM_7,
//    NUM_8,
//    NUM_9,
//    NUM_A,
//    NUM_B,
//    NUM_C,
//    NUM_D,
//    NUM_E,
//    NUM_F,
//    NUM_NULL,
//    NUM_MAX
//} hex_number;

///* Private variables ---------------------------------------------------------*/
///* -------------------------------------- 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F     NULL */
//const uint8_t g_cyDisplayCode[NUM_MAX] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x00};
//static const uint8_t g_Animation[16] =
//{
//    /* 数码管动画 */
//    0x01, 0x00,
//    0x00, 0x01,
//    0x00, 0x02,
//    0x00, 0x04,
//    0x00, 0x08,
//    0x08, 0x00,
//    0x10, 0x00,
//    0x20, 0x00,
//};

///* Private function prototypes -----------------------------------------------*/



///* Private user code ---------------------------------------------------------*/
///*
// * counts = 0, 6 MCU tick, 0.083333us
// * counts = 1, 9 MCU tick, 0.125000us
// * counts = 2, 14        , 0.194444us
// * counts = 3, 20        , 0.277778us
// * counts = 4, 26        , 0.361111us
// 故根据counts计算微秒的公式为：
//  [(counts - 2) * 6 + 14]/72   (counts > 2)
//*/
//void TM1650_easy_delay(uint32_t counts)
//{
//    while(counts--);
//}

//static inline void TM1650_SDAPinOutput_Init(void)
//{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};

//    GPIO_InitStruct.Pin = TM1650_SDA_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(TM1650_SDA_GPIO_Port, &GPIO_InitStruct);
//}

//static inline void TM1650_SDAPinInput_Init(void)
//{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};

//    GPIO_InitStruct.Pin = TM1650_SDA_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(TM1650_SDA_GPIO_Port, &GPIO_InitStruct);
//}

//static void TM1650LED_Stop(void)
//{
//    //TM1650_SDAPinOutput_Init();
//    TM1650_SCL_Reset();
//    TM1650_SDA_Reset();
//    TM1650_easy_delay(45);
//    TM1650_SCL_Set();
//    TM1650_SDA_Set();
//    TM1650_easy_delay(45);
//}

//static void TM1650LED_Start(void)
//{
//    //TM1650_SDAPinOutput_Init();
//    TM1650_SDA_Set();
//    TM1650_SCL_Set();
//    TM1650_easy_delay(45);
//    TM1650_SDA_Reset();
//    TM1650_easy_delay(45);
//    TM1650_SCL_Reset();
//}

//static void TM1650LED_Wait(void)
//{
//    uint16_t wErrTime;

//    TM1650_SDAPinInput_Init();
//    TM1650_SDA_Set();
//    TM1650_easy_delay(10);
//    TM1650_SCL_Set();
//    TM1650_easy_delay(10);

//    wErrTime = 0;

//    while (TM1650_READ_SDA)
//    {
//        wErrTime++;

//        if (2500 < wErrTime)
//        {
//            TM1650LED_Stop();
//            //return 1;
//            break;
//        }
//    }

//    TM1650_SCL_Reset();
//    // return 0;
//}

//static void TM1650LED_SendByte(uint8_t cyData)
//{
//    uint8_t i;
//    //TM1650_SDAPinOutput_Init();
//    TM1650_SCL_Reset();

//    for (i = 0; i < 8; i++)
//    {
//        if (0x80 == (cyData & 0x80) )
//        {
//            TM1650_SDA_Set();
//        }
//        else
//        {
//            TM1650_SDA_Reset();
//        }
//        cyData <<= 1;
//        TM1650_easy_delay(20);
//        TM1650_SCL_Set();
//        TM1650_easy_delay(20);
//        TM1650_SCL_Reset();
//        TM1650_easy_delay(20);
//    }
//}

//void TM1650LED_Display(uint8_t cyLevel, uint8_t *pCyDispBuffer)
//{
//    uint8_t i;
//    uint8_t cyAddr;

//    TM1650_SDAPinOutput_Init();
//    TM1650LED_Start();
//    TM1650LED_SendByte(0x48); //显示命令
//    TM1650LED_Wait();
//    TM1650_SDAPinOutput_Init();
//    TM1650LED_SendByte( ((cyLevel << 4) & 0xF7) | 0x01);  //8段显示
//    TM1650LED_Wait();
//    TM1650_SDAPinOutput_Init();
//    TM1650LED_Stop();
//    osDelay(3);
//    cyAddr = 0x68; //显存地址 0x68 0x6A 0x6C 0x6E
//    for (i = 0; i <  4; i++)
//    {
//        TM1650_SDAPinOutput_Init();
//        TM1650LED_Start();
//        TM1650LED_SendByte(cyAddr);
//        TM1650LED_Wait();
//        TM1650_SDAPinOutput_Init();
//        TM1650LED_SendByte(*(pCyDispBuffer + i) );
//        TM1650LED_Wait();
//        TM1650_SDAPinOutput_Init();
//        TM1650LED_Stop();
//        cyAddr += 2;
//        osDelay(3);
//    }
//}

///**
//  * @brief  显示数(例:num=999,则显示999, num=12345,则显示9999)
//  * @param  数值(uint16_t Num)
//  * @retval None
//  */
//void _Dispaly_num(uint16_t Num)
//{
//    uint16_t wTemp;
//    uint8_t cyBuf[4];

//    if (0 == Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[3] = g_cyDisplayCode[NUM_0];
//    }
//    else if (10 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[3] = g_cyDisplayCode[Num];
//    }
//    else if (100 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[Num / 10];
//        cyBuf[3] = g_cyDisplayCode[Num % 10];
//    }
//    else if (1000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else if (10000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[wTemp / 1000];
//        wTemp = wTemp % 1000;
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_9];
//        cyBuf[1] = g_cyDisplayCode[NUM_9];
//        cyBuf[2] = g_cyDisplayCode[NUM_9];
//        cyBuf[3] = g_cyDisplayCode[NUM_9];
//    }

//    TM1650LED_Display(1, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  显示放大10倍的数(例:num=999,则显示99.9, num=12345,则显示1234)
//  * @param  数值(uint16_t Num)
//  * @retval None
//  */
//void _Dispaly_10x_num(uint16_t Num)
//{
//    uint16_t wTemp;
//    uint8_t cyBuf[4];

//    if (0 == Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[3] = g_cyDisplayCode[NUM_0];
//    }
//    else if (10 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[0] | 0x80;
//        cyBuf[3] = g_cyDisplayCode[Num];
//    }
//    else if (100 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[Num / 10] | 0x80;
//        cyBuf[3] = g_cyDisplayCode[Num % 10];
//    }
//    else if (1000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10] | 0x80;
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else if (10000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[wTemp / 1000];
//        wTemp = wTemp % 1000;
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10] | 0x80;
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else if (65535 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[wTemp / 10000];
//        wTemp = wTemp % 10000;
//        cyBuf[1] = g_cyDisplayCode[wTemp / 1000];
//        wTemp = wTemp % 1000;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 100];
//        wTemp = wTemp % 100;
//        cyBuf[3] = g_cyDisplayCode[wTemp / 10];
//    }
//    else
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_9];
//        cyBuf[1] = g_cyDisplayCode[NUM_9];
//        cyBuf[2] = g_cyDisplayCode[NUM_9];
//        cyBuf[3] = g_cyDisplayCode[NUM_9];
//    }

//    TM1650LED_Display(1, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  显示放大100倍的数(例:num=999,则显示9.99, num=12345,则显示123.4)
//  * @param  数值(uint16_t Num)
//  * @retval None
//  */
//void _Dispaly_100x_num(uint16_t Num)
//{
//    uint16_t wTemp;
//    uint8_t cyBuf[4];

//    if (0 == Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[3] = g_cyDisplayCode[NUM_0];
//    }
//    else if (10 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[0] | 0x80;
//        cyBuf[2] = g_cyDisplayCode[0];
//        cyBuf[3] = g_cyDisplayCode[Num];
//    }
//    else if (100 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[0] | 0x80;
//        cyBuf[2] = g_cyDisplayCode[Num / 10];
//        cyBuf[3] = g_cyDisplayCode[Num % 10];
//    }
//    else if (1000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100] | 0x80;
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else if (10000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[wTemp / 1000];
//        wTemp = wTemp % 1000;
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100] | 0x80;
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else if (65535 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[wTemp / 10000];
//        wTemp = wTemp % 10000;
//        cyBuf[1] = g_cyDisplayCode[wTemp / 1000];
//        wTemp = wTemp % 1000;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 100] | 0x80;
//        wTemp = wTemp % 100;
//        cyBuf[3] = g_cyDisplayCode[wTemp / 10];
//    }
//    else
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_6];
//        cyBuf[1] = g_cyDisplayCode[NUM_5];
//        cyBuf[2] = g_cyDisplayCode[NUM_5] | 0x80;
//        cyBuf[3] = g_cyDisplayCode[NUM_3];
//    }

//    TM1650LED_Display(1, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  数码管动画,在循环内调用此函数,数码管前两位显示num，后两位会一直转圈，转圈速度取决于循环时间
//  * @param  None
//  * @retval None
//  */
//void _Display_GoRound(uint8_t num)
//{
//    static uint8_t wTemp = 0;
//    uint8_t cyBuf[4];

//    cyBuf[0] = g_Animation[wTemp++];
//    cyBuf[1] = g_Animation[wTemp++];
//    cyBuf[2] = g_cyDisplayCode[num / 10];
//    cyBuf[3] = g_cyDisplayCode[num % 10];

//    if (wTemp >= 16)
//    {
//        wTemp = 0;
//    }

//    TM1650LED_Display(4, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  数码管显示L1AI信息，左边两位显示次数，右边两位显示峰值
//  * @param  雷击次数(INT16U DisplayLJNum)
//  * @param  雷击峰值(INT32U DisplayLJFz)
//  * @retval None
//  */
//void Thunder_Dispaly(uint16_t LtNum, uint32_t LtPeak)
//{
//    uint8_t lcv_buff[4];
//    uint32_t utemp = LtPeak;

//    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_L2AI)
//    {
//        if(LtPeak < 1000) //100.0A=0.1kA  原本单位是A显示为kA
//        {
//            utemp = 1;
//        }
//        else if(LtPeak < 10000)
//        {
//            utemp /= 1000;
//            if(LtPeak % 1000 >= 500) //四舍五入
//            {
//                utemp += 1;
//            }
//        }
//        else
//        {
//            utemp /= 1000;
//            if(LtPeak % 10000 >= 5000) //四舍五入
//            {
//                utemp += 1;
//            }
//        }
//    }

//    if(LtNum == 0)
//    {
//        utemp = 0;
//    }

//    if (0 == LtNum)
//    {
//        lcv_buff[0] = g_cyDisplayCode[NUM_NULL];
//        lcv_buff[1] = g_cyDisplayCode[NUM_0];
//    }
//    else if (10 > LtNum)
//    {
//        lcv_buff[0] = g_cyDisplayCode[NUM_NULL];
//        lcv_buff[1] = g_cyDisplayCode[LtNum];
//    }
//    else if (100 > LtNum)
//    {
//        lcv_buff[0] = g_cyDisplayCode[LtNum / 10];
//        lcv_buff[1] = g_cyDisplayCode[LtNum % 10];
//    }
//    else
//    {
//        lcv_buff[0] = g_cyDisplayCode[NUM_9];
//        lcv_buff[1] = g_cyDisplayCode[NUM_9];
//    }

//    if (0 == utemp)
//    {
//        lcv_buff[2] = g_cyDisplayCode[NUM_NULL];
//        lcv_buff[3] = g_cyDisplayCode[NUM_0];
//    }
//    else if (10 > utemp)
//    {
//        lcv_buff[2] = g_cyDisplayCode[0] | 0x80;
//        lcv_buff[3] = g_cyDisplayCode[utemp];
//    }
//    else if (100 > utemp)
//    {
//        lcv_buff[2] = g_cyDisplayCode[utemp / 10] | 0x80;
//        lcv_buff[3] = g_cyDisplayCode[utemp % 10];
//    }
//    else if (1000 > utemp)
//    {
//        utemp /= 10;
//        lcv_buff[2] = g_cyDisplayCode[utemp / 10];
//        lcv_buff[3] = g_cyDisplayCode[utemp % 10];
//    }
//    else
//    {
//        lcv_buff[2] = g_cyDisplayCode[NUM_9];
//        lcv_buff[3] = g_cyDisplayCode[NUM_9];
//    }

//    TM1650LED_Display(1, lcv_buff); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  网关显示数(例:num=999,则显示999, num=12345,则显示9999)
//  * @param  数值(uint16_t Num)
//  * @retval None
//  */
//void Gateway_Dispaly_num(uint16_t Num)
//{
//    uint16_t wTemp;
//    uint8_t cyBuf[4];

//    if (0 == Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[NUM_0];
//        cyBuf[3] = g_cyDisplayCode[NUM_0];
//    }
//    else if (10 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[NUM_0];
//        cyBuf[3] = g_cyDisplayCode[Num];
//    }
//    else if (100 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[2] = g_cyDisplayCode[Num / 10];
//        cyBuf[3] = g_cyDisplayCode[Num % 10];
//    }
//    else if (1000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else if (10000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[wTemp / 1000];
//        wTemp = wTemp % 1000;
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_9];
//        cyBuf[1] = g_cyDisplayCode[NUM_9];
//        cyBuf[2] = g_cyDisplayCode[NUM_9];
//        cyBuf[3] = g_cyDisplayCode[NUM_9];
//    }

//    TM1650LED_Display(1, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  显示Hello
//  * @param  None
//  * @retval None
//  */
//void _Display_Hello(void)
//{
//    uint8_t cyBuf[4];

//    cyBuf[0] = 0x76;
//    cyBuf[1] = 0x79;
//    cyBuf[2] = 0x36;
//    cyBuf[3] = 0x3F;

//    TM1650LED_Display(4, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  显示地址，如：A-FF(addr:255)
//  * @param  disp_which 0：都显示  1：不显示个位  2：不显示十位
//  * @retval None
//  */
//void _Dispaly_hex_addrNum(uint16_t Num, uint8_t disp_which)
//{
//    uint8_t cyBuf[4];

//    cyBuf[0] = g_cyDisplayCode[NUM_A];//'A'
//    cyBuf[1] = 0x40;  //'-'

//    if (0 == Num)
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_0];
//        cyBuf[3] = g_cyDisplayCode[NUM_0];
//    }
//    else if (0x10 > Num)
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_0];
//        cyBuf[3] = g_cyDisplayCode[Num];
//    }
//    else if (0x100 > Num)
//    {
//        cyBuf[2] = g_cyDisplayCode[Num / 0x10];
//        cyBuf[3] = g_cyDisplayCode[Num % 0x10];
//    }
//    else
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_F];
//        cyBuf[3] = g_cyDisplayCode[NUM_F];
//    }

//    if(disp_which == 1)
//    {
//        cyBuf[3] = g_cyDisplayCode[NUM_NULL];
//    }
//    else if(disp_which == 2)
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_NULL];
//    }

//    TM1650LED_Display(1, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  显示选择模式序号，如：b-01, c-01, ....
//  * @param  mode       1：显示b  2：显示c
//  * @param  disp_which 0：都显示  1：不显示个位  2：不显示十位 3：不显示千位
//  * @retval None
//  */
//void _Dispaly_modeNum(uint8_t mode, uint16_t Num, uint8_t disp_which)
//{
//    uint8_t cyBuf[4];

//    cyBuf[1] = 0x40;  //'-'

//    if(1 == mode)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_B];//'B'
//    }
//    else if(2 == mode)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_C];//'C'
//    }

//    if (0 == Num)
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_0];
//        cyBuf[3] = g_cyDisplayCode[NUM_0];
//    }
//    else if (10 > Num)
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_0];
//        cyBuf[3] = g_cyDisplayCode[Num];
//    }
//    else if (100 > Num)
//    {
//        cyBuf[2] = g_cyDisplayCode[Num / 10];
//        cyBuf[3] = g_cyDisplayCode[Num % 10];
//    }
//    else
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_9];
//        cyBuf[3] = g_cyDisplayCode[NUM_9];
//    }

//    if(disp_which == 1)
//    {
//        cyBuf[3] = g_cyDisplayCode[NUM_NULL];
//    }
//    else if(disp_which == 2)
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_NULL];
//    }
//    else if(disp_which == 3)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//    }

//    TM1650LED_Display(1, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  显示设置参数，如：00FF(num:255)
//  * @param  num 要显示的数值
//  * @param  disp_which 0：都显示  1：不显示个位  2：不显示十位  3：不显示百位  4：不显示千位
//  * @param  multiple 倍数  0：不变   1：num/10后显示（显示小数点后）  2：num/100显示（显示小数点后）
//  * @retval None
//  */
//void _Dispaly_setNum(uint16_t Num, uint8_t multiple, uint8_t disp_which)
//{
//    uint16_t wTemp;
//    uint8_t cyBuf[4];

//    if (0 == Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_0];
//        if(multiple == 2)
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_0] | 0x80;
//        }
//        else
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_0];
//        }
//        if(multiple == 1)
//        {
//            cyBuf[2] = g_cyDisplayCode[NUM_0] | 0x80;
//        }
//        else
//        {
//            cyBuf[2] = g_cyDisplayCode[NUM_0];
//        }
//        cyBuf[3] = g_cyDisplayCode[NUM_0];
//    }
//    else if (10 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_0];
//        if(multiple == 2)
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_0] | 0x80;
//        }
//        else
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_0];
//        }
//        if(multiple == 1)
//        {
//            cyBuf[2] = g_cyDisplayCode[NUM_0] | 0x80;
//        }
//        else
//        {
//            cyBuf[2] = g_cyDisplayCode[NUM_0];
//        }
//        cyBuf[3] = g_cyDisplayCode[Num];
//    }
//    else if (100 > Num)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_0];
//        if(multiple == 2)
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_0] | 0x80;
//        }
//        else
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_0];
//        }
//        if(multiple == 1)
//        {
//            cyBuf[2] = g_cyDisplayCode[Num / 10] | 0x80;
//        }
//        else
//        {
//            cyBuf[2] = g_cyDisplayCode[Num / 10];
//        }
//        cyBuf[3] = g_cyDisplayCode[Num % 10];
//    }
//    else if (1000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[NUM_0];
//        if(multiple == 2)
//        {
//            cyBuf[1] = g_cyDisplayCode[wTemp / 100] | 0x80;
//        }
//        else
//        {
//            cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        }
//        wTemp = wTemp % 100;
//        if(multiple == 1)
//        {
//            cyBuf[2] = g_cyDisplayCode[wTemp / 10] | 0x80;
//        }
//        else
//        {
//            cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        }
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else if (10000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = g_cyDisplayCode[wTemp / 1000];
//        wTemp = wTemp % 1000;
//        if(multiple == 2)
//        {
//            cyBuf[1] = g_cyDisplayCode[wTemp / 100] | 0x80;
//        }
//        else
//        {
//            cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        }
//        wTemp = wTemp % 100;
//        if(multiple == 1)
//        {
//            cyBuf[2] = g_cyDisplayCode[wTemp / 10] | 0x80;
//        }
//        else
//        {
//            cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        }
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_9];
//        if(multiple == 2)
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_9] | 0x80;
//        }
//        else
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_9];
//        }
//        if(multiple == 1)
//        {
//            cyBuf[2] = g_cyDisplayCode[NUM_9] | 0x80;
//        }
//        else
//        {
//            cyBuf[2] = g_cyDisplayCode[NUM_9];
//        }
//        cyBuf[3] = g_cyDisplayCode[NUM_9];
//    }

//    if(disp_which == 1)
//    {
//        cyBuf[3] = g_cyDisplayCode[NUM_NULL];
//    }
//    else if(disp_which == 2)
//    {
//        if(multiple == 1)
//        {
//            cyBuf[2] = g_cyDisplayCode[NUM_NULL] | 0x80;
//        }
//        else
//        {
//            cyBuf[2] = g_cyDisplayCode[NUM_NULL];
//        }
//    }
//    if(disp_which == 3)
//    {
//        if(multiple == 2)
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_NULL] | 0x80;
//        }
//        else
//        {
//            cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//        }
//    }
//    else if(disp_which == 4)
//    {
//        cyBuf[0] = g_cyDisplayCode[NUM_NULL];
//    }

//    TM1650LED_Display(1, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  4个段选显示一致
//  * @param  None
//  * @retval None
//  */
//void _Dispaly_same_Num(uint16_t Num)
//{
//    uint8_t cyBuf[4];

//    cyBuf[0] = g_cyDisplayCode[Num];
//    cyBuf[1] = g_cyDisplayCode[Num];
//    cyBuf[2] = g_cyDisplayCode[Num];
//    cyBuf[3] = g_cyDisplayCode[Num];

//    TM1650LED_Display(4, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

///**
//  * @brief  显示H000~H999
//  * @param  num 要显示的数值
//  * @param  disp_which 0：都显示  1：不显示个位  2：不显示十位  3：不显示百位
//  * @param  multiple 倍数  0：不变   1：num/10后显示（不显示小数点后）
//  * @retval None
//  */
//void _Dispaly_H_Num(uint16_t Num, uint8_t disp_which, uint8_t multiple)
//{
//    uint16_t wTemp;
//    uint8_t cyBuf[4];

//    if(multiple == 1)
//    {
//        Num /= 10;
//    }

//    if (10 > Num)
//    {
//        cyBuf[0] = 0x76;  //'H'
//        cyBuf[1] = g_cyDisplayCode[NUM_0];
//        cyBuf[2] = g_cyDisplayCode[NUM_0];
//        cyBuf[3] = g_cyDisplayCode[Num];
//    }
//    else if (100 > Num)
//    {
//        cyBuf[0] = 0x76;  //'H'
//        cyBuf[1] = g_cyDisplayCode[NUM_0];
//        cyBuf[2] = g_cyDisplayCode[Num / 10];
//        cyBuf[3] = g_cyDisplayCode[Num % 10];
//    }
//    else if (1000 > Num)
//    {
//        wTemp = Num;
//        cyBuf[0] = 0x76;  //'H'
//        cyBuf[1] = g_cyDisplayCode[wTemp / 100];
//        wTemp = wTemp % 100;
//        cyBuf[2] = g_cyDisplayCode[wTemp / 10];
//        cyBuf[3] = g_cyDisplayCode[wTemp % 10];
//    }
//    else
//    {
//        cyBuf[0] = 0x76;  //'H'
//        cyBuf[1] = g_cyDisplayCode[NUM_9];
//        cyBuf[2] = g_cyDisplayCode[NUM_9];
//        cyBuf[3] = g_cyDisplayCode[NUM_9];
//    }

//    if(disp_which == 1)
//    {
//        cyBuf[3] = g_cyDisplayCode[NUM_NULL];
//    }
//    else if(disp_which == 2)
//    {
//        cyBuf[2] = g_cyDisplayCode[NUM_NULL];
//    }
//    if(disp_which == 3)
//    {
//        cyBuf[1] = g_cyDisplayCode[NUM_NULL];
//    }

//    TM1650LED_Display(1, cyBuf); //0为第8级灰度，最亮。1为第1级灰度
//}

