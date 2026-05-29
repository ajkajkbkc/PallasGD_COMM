



///* Private includes ----------------------------------------------------------*/
//#include "main.h"
//#include "cmsis_os.h"
//#include <stdbool.h>

//#include "app_log.h"
//#include "app_att7022.h"
//#include "app_parameter.h"
//#include "app_main.h"
//#include "app_tool.h"

//#include "app_key.h"
//#include "app_tm1650.h"

//#if PROD_TYPE != PROD_FSS && PROD_TYPE != PROD_SFC
///* Private define ------------------------------------------------------------*/
//#define ATT7022_SPI   0  //0:gpio simulation  1:SPI peripheral

//#define ATT7022_CSSet()    HAL_GPIO_WritePin(ATT7022_CS_GPIO_Port, ATT7022_CS_Pin, GPIO_PIN_SET)
//#define ATT7022_CSClr()    HAL_GPIO_WritePin(ATT7022_CS_GPIO_Port, ATT7022_CS_Pin, GPIO_PIN_RESET)

//#if (ATT7022_SPI == 0)
//#define ATT7022_CLKSet()   HAL_GPIO_WritePin(ATT7022_SCK_GPIO_Port, ATT7022_SCK_Pin, GPIO_PIN_SET)
//#define ATT7022_CLKClr()   HAL_GPIO_WritePin(ATT7022_SCK_GPIO_Port, ATT7022_SCK_Pin, GPIO_PIN_RESET)

///* ATT7022的输入 */
//#define ATT7022_DINSet()   HAL_GPIO_WritePin(ATT7022_MOSI_GPIO_Port, ATT7022_MOSI_Pin, GPIO_PIN_SET)
//#define ATT7022_DINClr()   HAL_GPIO_WritePin(ATT7022_MOSI_GPIO_Port, ATT7022_MOSI_Pin, GPIO_PIN_RESET)
///* ATT7022的输出 */
//#define ATT7022_DOUTPin()  HAL_GPIO_ReadPin(ATT7022_MISO_GPIO_Port, ATT7022_MISO_Pin)
//#endif

//#define  _10_bit    0x400
//#define  _15_bit    0x8000
//#define  _16_bit    0x10000
//#define  _20_bit    0x100000
//#define  _23_bit    0x800000
//#define  _24_bit    0x1000000

//#define     KI          10.00
//#define     KU          10.00
//#define     KP          0.004235 //6400 0.00847 /3200   KP = 2.592 * 10^10 /(114(HFconst) * 6400(EC) * 2 ^23)

//#define ATT7022_BYTE_LEN    8     //字节长度
//#define ATT7022_REG_LEN     24    //寄存器长度

//#define ATT7022_PHS_A   0
//#define ATT7022_PHS_B   1
//#define ATT7022_PHS_C   2
//#define ATT7022_PHS_ALL 3

//#define ACTIVE_POWER        0x01  //有功功率
//#define RECTIVE_POWER       0x05  //无功功率
//#define APPARENT_POWER      0x09  //视在功率


///* Private variables ---------------------------------------------------------*/
///* Definitions for att7022Task */
//osThreadId_t att7022TaskHandle;
//const osThreadAttr_t att7022Task_attributes =
//{
//    .name = "att7022Task",
//    .priority = (osPriority_t) att7022TaskPriority,
//    .stack_size = 1024
//};

//static uint16_t gVolBuff[ATT7022_PHS_ALL];
//static uint16_t gCurBuff[ATT7022_PHS_ALL];


///* Private function prototypes -----------------------------------------------*/
//void Att7022Task(void *argument);


///* Private user code ---------------------------------------------------------*/

//#if (ATT7022_SPI == 0)
///* 根据观察:
// * wTime = 0, 0.152778(us)
// * wTime = 1, 2.222222(us)
// * wTime = 2, 4.291666(us)
// * wTime = 3, 6.361111(us)
// * wTime = 10,20.847221(us)
// */
//static void ATT7022_Delay(uint16_t time)
//{
//    uint16_t i;
//    uint8_t j;

//    for (i = 0; i < time; i++)
//    {
//        for (j = 0; j < 20; j++) ;
//    }
//}

///**
//  * @brief  ATT7022端口初始化
//  * @param  None
//  * @retval None
//  */
//void att7022_hw_init(void)
//{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};

//    /* GPIO Ports Clock Enable */
//    __HAL_RCC_GPIOB_CLK_ENABLE();

//    /*Configure GPIO pin Output Level */
//    HAL_GPIO_WritePin(GPIOB, ATT7022_MOSI_Pin | ATT7022_SCK_Pin, GPIO_PIN_RESET);

//    /*Configure GPIO pin Output Level */
//    HAL_GPIO_WritePin(ATT7022_CS_GPIO_Port, ATT7022_CS_Pin, GPIO_PIN_SET);

//    /*Configure GPIO pins : ATT7022_MISO_Pin */
//    GPIO_InitStruct.Pin = ATT7022_MISO_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//    /*Configure GPIO pins : ATT7022_MOSI_Pin ATT7022_SCK_Pin */
//    GPIO_InitStruct.Pin = ATT7022_MOSI_Pin | ATT7022_SCK_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//    /*Configure GPIO pin : ATT7022_CS_Pin */
//    GPIO_InitStruct.Pin = ATT7022_CS_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
//    HAL_GPIO_Init(ATT7022_CS_GPIO_Port, &GPIO_InitStruct);
//}

///**
//  * @brief  读寄存器
//  * @param  地址(cyAddr)
//  * @retval 寄存器值(dwRegVal)
//  */
//static uint32_t ATT7022_ReadReg(uint8_t cyAddr)//cyAddr Bit7：0 表示读命令 1 表示写入特殊命令字
//{
//    uint8_t i;
//    uint32_t dwRegVal;

//    ATT7022_CSClr();
//    ATT7022_Delay(1);

//    //写8个bits的地址
//    for(i = 0; i < ATT7022_BYTE_LEN; i++)
//    {
//        ATT7022_CLKSet();
//        if(0 == (cyAddr & 0x80))
//        {
//            ATT7022_DINClr();
//        }
//        else
//        {
//            ATT7022_DINSet();
//        }
//        cyAddr <<= 1;
//        ATT7022_Delay(2);

//        ATT7022_CLKClr();
//        ATT7022_Delay(2);
//    }

//    //读24个bits的数据
//    ATT7022_Delay(10);

//    dwRegVal = 0;
//    for(i = 0; i < ATT7022_REG_LEN; i++)
//    {
//        dwRegVal <<= 1;

//        ATT7022_CLKSet();
//        ATT7022_Delay(2);

//        if(0 != ATT7022_DOUTPin() )
//        {
//            dwRegVal += 1;
//        }

//        ATT7022_CLKClr();
//        ATT7022_Delay(1);
//        ATT7022_Delay(1);
//    }

//    ATT7022_CSSet();

//    return (dwRegVal);
//}


///**
//  * @brief  写寄存器
//  * @param  地址(cyAddr)
//  * @retval 寄存器值(dwRegVal)
//  */
//static void ATT7022_WriteReg(uint8_t cyAddr, uint32_t dwRegVal)//cyAddr Bit7：0 表示读命令 1 表示写入特殊命令字
//{
//    cyAddr |= ATT7022_WRITE_REG;

//    ATT7022_CSClr();
//    ATT7022_Delay(1);

//    //写入8bit也就是1byte的地址
//    for(uint8_t i = 0; i < ATT7022_BYTE_LEN; i++)
//    {
//        ATT7022_CLKSet();
//        if(0 == (cyAddr & 0x80))
//        {
//            ATT7022_DINClr();
//        }
//        else
//        {
//            ATT7022_DINSet();
//        }
//        cyAddr <<= 1;
//        ATT7022_Delay(2);

//        ATT7022_CLKClr();
//        ATT7022_Delay(2);
//    }

//    //写24bits的数据
//    ATT7022_Delay(3);

//    for(uint8_t i = 0; i < ATT7022_REG_LEN; i++)
//    {
//        ATT7022_CLKSet();
//        if(0 == (dwRegVal & 0x800000))
//        {
//            ATT7022_DINClr();
//        }
//        else
//        {
//            ATT7022_DINSet();
//        }
//        dwRegVal <<= 1;
//        ATT7022_Delay(2);
//        ATT7022_CLKClr();
//        ATT7022_Delay(2);
//    }

//    ATT7022_CSSet();
//}
//#else
//static uint32_t ATT7022_ReadReg(uint8_t cyAddr)
//{
//    ATT7022_CSClr();
//    //ATT7022_Delay(1);

//    HAL_SPI_Transmit(&hspi2, &cyAddr, 1, 1000);
//    //ATT7022_Delay(2);
//    uint32_t dwRegVal;
//    uint8_t rVal[4] = {0};
//    HAL_SPI_Receive(&hspi2, (uint8_t *)&rVal[0], 3, 1000);
//    ATT7022_CSSet();
//    //hexdump(rVal, 4);

//    dwRegVal = rVal[0] << 16 | rVal[1] << 8 | (rVal[2] & 0xFF);
//    return dwRegVal;
//}

//static void ATT7022_WriteReg(uint8_t cyAddr, uint32_t dwRegVal)
//{

//    cyAddr |= ATT7022_WRITE_REG;
//    //uint32_t wVal = (cyAddr << 24) | dwRegVal;
//    //hexdump(&wVal, 4);
//    ATT7022_CSClr();
//    //ATT7022_Delay(1);

//    uint8_t sVal[4];
//    sVal[0] = cyAddr;
//    sVal[1] = dwRegVal >> 16;
//    sVal[2] = dwRegVal >> 8;
//    sVal[3] = dwRegVal & 0xFFU;
//    HAL_SPI_Transmit(&hspi2, (uint8_t *)&sVal[0], 4, 1000);

//    ATT7022_CSSet();
//}
//#endif

///**
//  * @brief  读电压有效值
//  * @param  相编号(cyPhsNum)
//  * @retval 电压值(wUVal)
//  */
//uint16_t ATT7022_ReadU(uint8_t cyPhsNum)
//{
//    uint16_t wUVal;
//    uint32_t dwTemp;
//    float fTemp;

//    dwTemp = ATT7022_ReadReg(cyPhsNum + 0x0D);
//    //dwTemp = ATT7022_ReadReg(cyPhsNum);

//    if(dwTemp <= _23_bit)	//收到24bits的最高位为0
//    {
//        fTemp = (( (float)dwTemp * _10_bit) / _23_bit) * KU;
//    }
//    else	//收到24bits的最高位为1
//    {
//        fTemp = (( (float)_24_bit - dwTemp) * _10_bit) / _23_bit * KU;
//    }

//    wUVal = (uint16_t) (fTemp);

//    return (wUVal);
//}

///**
//  * @brief  读电流
//  * @param  相编号(cyPhsNum)
//  * @retval 电流值(wIVal)
//  */
//uint16_t ATT7022_ReadI(uint8_t cyPhsNum)
//{
//    uint16_t wIVal;
//    uint32_t dwTemp;
//    float fTemp;

//    dwTemp = ATT7022_ReadReg(cyPhsNum + 0x10);

//    if (dwTemp <= _23_bit)
//    {
//        fTemp = (( (float)dwTemp * _10_bit) / _23_bit) * KI;
//    }
//    else
//    {
//        fTemp = (( (float)_24_bit - dwTemp) * _10_bit) / _23_bit * KI;
//    }

//    wIVal = (uint16_t)(fTemp);

//    return (wIVal);
//}

///**
//  * @brief  读电流与电压相位角
//  * @param  相编号(cyPhsNum)
//  * @retval 电流与电压相位角(wPgVal)
//  */
//uint16_t ATT7022_ReadPG(uint8_t cyPhsNum)
//{
//    uint16_t wPgVal;
//    uint32_t dwTemp;
//    float  fTemp;

//    dwTemp = ATT7022_ReadReg(cyPhsNum + 0x18);

//    if (dwTemp < _20_bit)
//    {
//        fTemp = dwTemp;
//        fTemp /= _20_bit;
//        fTemp *= 180;
//    }
//    else
//    {
//        fTemp = (_24_bit - dwTemp);
//        fTemp /= _20_bit;
//        fTemp *= 180;
//        fTemp = 360 - fTemp;
//    }

//    //fTemp /= _20_bit;
//    //fTemp *= 360;
//    wPgVal = (uint16_t) (fTemp);

//    return (wPgVal);
//}

///**
//  * @brief  读功率
//  * @param  相编号(cyPhsNum)
//  * @param  功率类型(cyPowerType)
//  * @retval 功率(dwPower)
//  */
//uint16_t ATT7022_ReadPower(uint8_t cyPhsNum, uint8_t cyPowerType)
//{
//    uint16_t wPower;
//    uint32_t dwTemp;
//    float  fTemp;

//    dwTemp = ATT7022_ReadReg(cyPhsNum + cyPowerType);

//    fTemp = KP;

//    if (dwTemp <= _23_bit)
//    {
//        fTemp *= dwTemp;
//    }
//    else
//    {
//        fTemp *= (_24_bit - dwTemp);
//    }

//    wPower = (uint16_t) (fTemp);

//    return (wPower);
//}

///**
//  * @brief  初始化ATT7022
//  * @param  None
//  * @retval 初始化成功/失败
//  */
//// 220V三相四线表为例，设计参数如下： 脉冲常数 EC为3200imp/kWh，额定电流输入时，电流通道的输入电压Vi为0.22V，参比电压输入时，电压通道的输入电压Vu为0.05V
//bool ATT7022_Init(void)
//{
//    LOGD("att7022", "Enter %s()", __func__);
//    uint32_t dwTemp;
//    uint8_t cyCnt = 0;

//#if (ATT7022_SPI == 0)
//    //att7022_hw_init();   //in <main.c> MX_GPIO_Init

//    ATT7022_CLKClr();
//    ATT7022_CSSet();
//#endif

//    while (cyCnt++ < 10)
//    {
//        ATT7022_WriteReg(ATT7022_MODECFG, 0xB97E);
//        osDelay(10);
//        dwTemp = ATT7022_ReadReg(ATT7022_BCKREG);
//        LOGI("att7022", "dwTemp = 0x%04X", dwTemp);
//        if (0xB97E == dwTemp)
//        {
//            break;
//        }
//    }
//    if(cyCnt >= 10)
//    {
//        LOGE("att7022", "ATT7022_Init error !!");
//        return false;
//    }

//    //写电压通道模式控制寄存器
//    ATT7022_WriteReg(0x80 | 0x01, 0xB97F);

//    //ADC增益配置寄存器
//    ATT7022_WriteReg(0x80 | 0x02, 0x154);

//    //EMU单元配置
//    //ATT7022_WriteReg(0x80 | 0x03, 0xF804);
//    ATT7022_WriteReg(0x80 | 0x03, 0xFC04);

//    //HFConst = INT[2.592*10^10*G*G*Vu*Vi/(EC*Un*Ib)]
//    //        = INT[2.592*10^10*1.163*1.163*0.22*0.05/(3200*220*1.5)]
//    //        = INT[365.193]= 365 = 16DH
//    ATT7022_WriteReg(0x80 | 0x1E, 0x16D);  //3200

//    //写起动电流，设置点为0.4%： Io = 1.5 * 40 * 0.4% = 0.24
//    //Istartup=0.8* Io * 2^13 = 0.8*0.24 * 2^13 = 1572 = 624H
//    ATT7022_WriteReg(0x80 | 0x1D, 0x624);

//    //写A相功率增益： 三相电压输入220伏，仅输入A相电流1.5安培，功率因数为1.0，
//    //标准表的电能误差读数为–0.74%，即err=-0.0074，
//    //则： Pgain=  -err/1+err  =0.0074/(1-0.0074)=0.00745516
//    //     Pgain= Pgain*2^15=0.00745516*2^15=244=0F4H
//    ATT7022_WriteReg(0x80 | 0x04, 0xF4);  //将有功、无功、视在增益寄存器写同样的校正值
//    ATT7022_WriteReg(0x80 | 0x07, 0xF4);  //将有功、无功、视在增益寄存器写同样的校正值
//    ATT7022_WriteReg(0x80 | 0x0A, 0xF4);  //将有功、无功、视在增益寄存器写同样的校正值

//    //写A相相位校正：三相电压输入220伏，仅输入A相电流1.5安培，功率因数为0.5L，
//    //标准表的电能误差读数为0.34%,err=0.0034，
//    //则: θ=-0.0034/1.732= -0.001963 < 0
//    // Phsreg=2^16+θ*2^15=65471=FFBFH
//    ATT7022_WriteReg(0x80 | 0x0D, 0xFFBF);
//    ATT7022_WriteReg(0x80 | 0x10, 0xFFBF); //不分段，将2段的寄存器写同样的校正值

//    //写B相功率增益： 三相电压输入220伏，仅输入B相电流1.5安培，功率因数为1.0，
//    //标准表的电能误差，标准表的电能误差读数为1.26%,err=0.0126，
//    //则： Pgain=errerr+.1= -0.0126/(1+0.0126)= -0.01244322<0
//    //Pgain=2^16+Pgain*2^15 =2^16 - 0.01244322*2^15=65128=FE68H
//    ATT7022_WriteReg(0x80 | 0x05, 0xFE68);  //将有功、无功、视在增益寄存器写同样的校正值
//    ATT7022_WriteReg(0x80 | 0x08, 0xFE68);  //将有功、无功、视在增益寄存器写同样的校正值
//    ATT7022_WriteReg(0x80 | 0x0B, 0xFE68);  //将有功、无功、视在增益寄存器写同样的校正值

//    //写B相相位校正: 三相电压输入220伏，仅输入B相电流1.5安培，功率因数为0.5L，
//    //标准表的电能误差读数是-0.34%,err=-0.0174，
//    //则: θ=0.0034/1.732=0.001963>0
//    //Phsreg=θ*2^15=64=40H
//    ATT7022_WriteReg(0x80 | 0x0E, 0x40);
//    ATT7022_WriteReg(0x80 | 0x11, 0x40); //不分段，将2段的寄存器写同样的校正值

//    //写C相功率增益 三相电压输入220伏，仅输入C相电流1.5安培，功率因数为1.0，
//    //标准表测得的电能误差是0.5%,ERR=0.005,
//    //则： Pgain=1errerr.+= -0.005/(1+0.005)= -0.00497512<0
//    //Pgain=2^16+Pgain*2^15==2^16 - 0.00497512*2^15=65372=FF5CH
//    ATT7022_WriteReg(0x80 | 0x06, 0xFF5C);  //将有功、无功、视在增益寄存器写同样的校正值
//    ATT7022_WriteReg(0x80 | 0x09, 0xFF5C);  //将有功、无功、视在增益寄存器写同样的校正值
//    ATT7022_WriteReg(0x80 | 0x0C, 0xFF5C);  //将有功、无功、视在增益寄存器写同样的校正值

//    //写C相相位校正 三相电压输入220伏，仅输入C相电流1.5安培，功率因数为0.5L，
//    //标准表的电能误差读数是0.24%,err=0.0024，
//    //则: θ=-0.0024/1.732 = - 0.001385681<0
//    //Phsreg=2^16+θ*2^15=65490=FFD2H
//    ATT7022_WriteReg(0x80 | 0x0F, 0xFFD2);
//    ATT7022_WriteReg(0x80 | 0x12, 0xFFD2); //不分段，将2段的寄存器写同样的校正值

//    //写A、B、C相电压校正: 三相电压输入220伏，三相电流输入1.5安培，功率因数1.0，
//    //电压寄存器的值Vu分别为0x209275=2134645，0x208564=2131300，0x1FF4CC=2094285,
//    //经有效值计算: Urms=Vu*2^10/2^23=Vu/2^13
//    //电能表上的电压读数分别为260.576V、260.168V、255.650V，
//    //电压校正值为: Ugain=Ur/Urms -1 其中Ur为标准表的电压读数。

//    //A相电压校正值Ugain =220/260.576-1=-0.155716<0
//    //             Ugain=INT(2^16+Ugain*2^15)=0xEC11
//    //ATT7022_WriteReg(0x80 | 0x17, 0xEC11);
//    ATT7022_WriteReg(0x80 | 0x17, 0x0000);

//    //B相电压校正值Ugain=220/260.168-1=-0.1543925<0
//    //             Ugain=INT(2^16+Ugain*2^15)=0xEC3C
//    //ATT7022_WriteReg(0x80 | 0x18, 0xEC3C);
//    ATT7022_WriteReg(0x80 | 0x18, 0x0000);

//    //C相电压校正值Ugain= 220/255.650-1=-0.139448 <0
//    //             Ugain=INT(2^16+Ugain*2^15)=0xEE26
//    //ATT7022_WriteReg(0x80 | 0x19, 0xEE26);
//    ATT7022_WriteReg(0x80 | 0x19, 0x0000);


//    //写A、B、C相电流校正: 三相电压输入220伏，三相电流输入1.5安培，功率因数1.0,
//    //读电流寄存器的值Vi分别为0x7C000=507904,0x75CCC=482508，0x77D78=491520，
//    //经有效值计算 : Urms=Vi*2^10/2^23=Vi/2^13/40
//    //电能表上的电压读数分别为1.55A，1.4725A，1.498A,
//    //电流校正值为 Igain=Ir/Irms -1 其中Ir为标准表的电流读数

//    //A相电流校正值Igain=1.5/1.55-1=-0.032258 <0
//    //Igain=INT(2^16+Igain*2^15)=0xFBDE
//    //ATT7022_WriteReg(0x80 | 0x1A, 0xFBDE);
//    ATT7022_WriteReg(0x80 | 0x1A, 0x0000);

//    //B相电流校正值Igain=1.5/1.4725-1=0.01867572 >0
//    //Igain=INT(Igain*2^23)=0x263
//    //ATT7022_WriteReg(0x80 | 0x1B, 0x263);
//    ATT7022_WriteReg(0x80 | 0x1B, 0x0000);

//    //C相电流校正值Igain=1.5/1.498-1=0.0013351
//    //Igain=INT(Igain*2^15)=0x2B
//    //ATT7022_WriteReg(0x80 | 0x1C, 0x2B);
//    ATT7022_WriteReg(0x80 | 0x1C, 0x0000);

//    ATT7022_WriteReg(0x80 | 0x31, 0x3437);
//    ATT7022_WriteReg(0x80 | 0x34, 0x2C52); //基波增益

//    ATT7022_WriteReg(0x80 | 0x6D, 0xFF00);
//    ATT7022_WriteReg(0x80 | 0x6E, 0x0DB8);
//    ATT7022_WriteReg(0x80 | 0x6F, 0xD1DA);

//    return true;
//}

//#if 0
//void ATT7022_Test(void)
//{
//    uint32_t tick0, tick1, tickInterval;
//    double us;
//    uint32_t regVal;

//    tick0 = SysTick->VAL;
//    regVal = ATT7022_ReadReg(ATT7022_DEV_ID);   // Device ID
//    tick1 = SysTick->VAL;
//    tickInterval = tick0 - tick1;
//    us = tickInterval * 0.013888888; // 1 / 72 = 0.0138888
//    LOGE("att7022", "tick0 = %u, tick1 = %u, interval = %u, %f(us)", tick0, tick1, tickInterval, us);
//    LOGI("att7022", "Device ID = 0x%08X", ATT7022_ReadReg(ATT7022_DEV_ID)); //Device ID : 0x7122A0

//    HAL_Delay(99);
//    uint32_t temperature = ATT7022_ReadReg(ATT7022_TPSD0);
//    LOGI("att7022", "temperature = 0x%08X", temperature);
//    //if (temperature == 0)
//    {
//        uint16_t setVal = 0x4527 | (1 << 4);
//        tick0 = SysTick->VAL;
//        ATT7022_WriteReg(ATT7022_MODULE_CFG, setVal);
//        tick1 = SysTick->VAL;
//        if (tick0 >= tick1)
//        {
//            tickInterval = tick0 - tick1;
//        }
//        else
//        {
//            tickInterval = 72000 - tick1 + tick0;
//        }
//        us = tickInterval * 0.013888888;
//        LOGE("att7022", "tick0 = %u, tick1 = %u, interval = %u, %f(us)", tick0, tick1, tickInterval, us);
//        LOGV("att7022", "setVal = 0x%08X", setVal);
//    }
//#if 1
//    uint32_t voltage = ATT7022_ReadReg(ATT7022_UARMS);
//    LOGV("att7022", "UA_RMS = 0x%08X", voltage);
//    voltage = ATT7022_ReadReg(ATT7022_UBRMS);
//    LOGD("att7022", "UB_RMS = 0x%08X", voltage);
//    voltage = ATT7022_ReadReg(ATT7022_UCRMS);
//    LOGI("att7022", "UC_RMS = 0x%08X", voltage);

//    voltage = ATT7022_ReadReg(ATT7022_IARMS);
//    LOGV("att7022", "IARMS = 0x%08X", voltage);
//    voltage = ATT7022_ReadReg(ATT7022_IBRMS);
//    LOGD("att7022", "IBRMS = 0x%08X", voltage);
//    voltage = ATT7022_ReadReg(ATT7022_ICRMS);
//    LOGI("att7022", "ICRMS = 0x%08X", voltage);
//#endif
//}
//#endif



///**
//  * @brief  新建线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_att7022Task(void)
//{
//    att7022TaskHandle = osThreadNew(Att7022Task, NULL, &att7022Task_attributes);
//}

///**
//  * @brief  Function implementing the att7022Task thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void Att7022Task(void *argument)
//{
//    LOGD("att7022", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());
//    uint8_t i;
//    float fVol_K, fCur_K;
//    float fVol_B, fCur_B;

//    if(!ATT7022_Init())  //检测不到ATT7022
//    {
//        LOGW("att7022", "att7022 task delate.");
//        PAR_SET_BIT(gParam.st.SysState01, SYS_STATE_ATT7022);
//        //osThreadSuspend(NULL);
//        //osThreadTerminate(att7022TaskHandle);//创建其它任务后,删除自己,参数为待删除任务的任务句柄
//        osThreadTerminate(NULL);//如果任务在任务函数中删除自己,也可以将参数写为"NULL"，就像这个里面一样。
//    }
//    PAR_CLEAR_BIT(gParam.st.SysState01, SYS_STATE_ATT7022);

//    for(;;)
//    {
//        for(i = ATT7022_PHS_A; i < ATT7022_PHS_ALL; i++)
//        {
//            gVolBuff[i] = ATT7022_ReadU(i);
//            gCurBuff[i] = ATT7022_ReadI(i);

//            if (ATT7022_PHS_A == i)
//            {
//                fVol_K  = gFlashParam.st.VolA_K;
//                fVol_B = (float)gFlashParam.st.VolA_B;
//                fCur_K  = gFlashParam.st.CurA_K;
//                fCur_B = (float)gFlashParam.st.CurA_B;
//            }
//            else if (ATT7022_PHS_B == i)
//            {
//                fVol_K  = gFlashParam.st.VolB_K;
//                fVol_B = (float)gFlashParam.st.VolB_B;
//                fCur_K  = gFlashParam.st.CurB_K;
//                fCur_B = (float)gFlashParam.st.CurB_B;
//            }
//            else if (ATT7022_PHS_C == i)
//            {
//                fVol_K  = gFlashParam.st.VolC_K;
//                fVol_B = (float)gFlashParam.st.VolC_B;
//                fCur_K  = gFlashParam.st.CurC_K;
//                fCur_B = (float)gFlashParam.st.CurC_B;
//            }
//            fVol_K /= 1000.0;
//            fCur_K /= 1000.0;
//            //LOGI("att7022", "Num[%d] Vol = %d, Cur = %d", i, gVolBuff[i], gCurBuff[i]);

//            fVol_K *= gVolBuff[i];
//            fCur_K *= gCurBuff[i];

//            fVol_B /= 100.0;
//            fCur_B /= 100.0;

//            fVol_K += fVol_B;
//            if(fVol_K < 50)  //屏蔽小于5.0V
//            {
//                fVol_K = 0;
//            }

//            fCur_K += fCur_B;
//            if(fCur_K < 50)  //屏蔽小于5.0uA
//            {
//                fCur_K = 0;
//            }

//            gVolBuff[i] = (uint16_t)fVol_K;
//            gCurBuff[i] = (uint16_t)fCur_K;
//        }

//        gFS_Elem.st.VolA = gVolBuff[ATT7022_PHS_A];
//        gFS_Elem.st.VolB = gVolBuff[ATT7022_PHS_B];
//        gFS_Elem.st.VolC = gVolBuff[ATT7022_PHS_C];
//        gFS_Elem.st.CurA = gCurBuff[ATT7022_PHS_A];
//        gFS_Elem.st.CurB = gCurBuff[ATT7022_PHS_B];
//        gFS_Elem.st.CurC = gCurBuff[ATT7022_PHS_C];

//        //LOGI("att7022", "VolA = %d, VolB = %d, VolC = %d", gFS_Elem.VolA, gFS_Elem.VolB, gFS_Elem.VolC);
//        //LOGI("att7022", "CurA = %d, CurB = %d, CurC = %d", gFS_Elem.st.CurA, gFS_Elem.st.CurB, gFS_Elem.st.CurC);

//#if PROD_TYPE == PROD_FD
//        if(gKey1Mode == KEY_MODE_DEFAULT)
//        {
//            _Dispaly_10x_num(gFS_Elem.st.CurA);
//        }
//#endif

//        osDelay(1000);
//    }
//}


//#endif


