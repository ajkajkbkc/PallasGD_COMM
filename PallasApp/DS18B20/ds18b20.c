
///* Private includes ----------------------------------------------------------*/
//#include "main.h"
//#include "cmsis_os.h"

//#include "onewire.h"
//#include "app_log.h"
//#include "ds18b20.h"
//#include "app_parameter.h"


///* Private define ------------------------------------------------------------*/



///* Private variables ---------------------------------------------------------*/
///* Definitions for ds18b20Task */
//osThreadId_t ds18b20TaskHandle;
//const osThreadAttr_t ds18b20Task_attributes =
//{
//    .name = "ds18b20Task",
//    .priority = (osPriority_t) ds18b20TaskPriority,
//    .stack_size = 1024
//};

//OneWire_t gOneWire1; //T1-->PB.01 PIN27 外壳上的B8
//OneWire_t gOneWire2; //T2-->PB.10 PIN29 外壳上的B7
//OneWire_t gOneWire3; //T3-->PB.11 PIN30


///* Private function prototypes -----------------------------------------------*/
//void DS18B20Task(void *argument);


///* Private user code ---------------------------------------------------------*/

//static OneWire_t *GetOneWire(uint8_t temp_num)
//{
//    switch(temp_num)
//    {
//    case DS18B20_NUM_1:
//        return (&gOneWire1);

//    case DS18B20_NUM_2:
//        return (&gOneWire2);

//    case DS18B20_NUM_3:
//        return (&gOneWire3);

//    default:
//        return NULL;
//    }
//}

///**
//  * @brief  关闭告警功能
//  * @param  onewire
//  * @retval 是否成功
//  */
//static bool DS18B20_DisableAlarmTemperature(OneWire_t *OneWire)
//{
//    LOGD("DS18B20_neptune", "Enter %s()", __func__);
//    uint8_t tl, th, conf;

//    /* Reset line */
//    uint8_t ret = OneWire_Reset(OneWire);
//    /* Select ROM number */
//    //OneWire_SelectWithPointer(OneWire, ROM);
//    /* Read scratchpad command by onewire protocol */
//    //OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
//    if (ret == 1)
//    {
//        LOGE("DS18B20_neptune", "initialize DS18B20(%u) ERROR!", OneWire->appNum);
//        return false;
//    }
//    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);
//    OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);

//    /* Ignore first 2 bytes */
//    OneWire_ReadByte(OneWire);
//    OneWire_ReadByte(OneWire);

//    th = OneWire_ReadByte(OneWire);
//    tl = OneWire_ReadByte(OneWire);
//    conf = OneWire_ReadByte(OneWire);
//    LOGV("DS18B20_neptune", "th=0x%X, tl=0x%X, conf=0x%X", th, tl, conf);
//    if (th == 125)
//    {
//        return true;
//    }
//    th = 125;
//    tl = (uint8_t) - 55;

//    /* Reset line */
//    OneWire_Reset(OneWire);
//    /* Select ROM number */
//    //OneWire_SelectWithPointer(OneWire, ROM);

//    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);
//    /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
//    OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);

//    /* Write bytes */
//    OneWire_WriteByte(OneWire, th);
//    OneWire_WriteByte(OneWire, tl);
//    OneWire_WriteByte(OneWire, conf);

//    /* Reset line */
//    OneWire_Reset(OneWire);
//    /* Select ROM number */
//    //OneWire_SelectWithPointer(OneWire, ROM);

//    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);
//    /* Copy scratchpad to EEPROM of DS18B20 */
//    OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);

//    return true;
//}

///**
//  * @brief  转换温度
//  * @param  onewire
//  * @retval 是否成功
//  */
//static bool DS18B20_convert_temperature(OneWire_t *onewire)
//{
//    /* Reset line */
//    uint8_t ret = OneWire_Reset(onewire);
//    //LOGI("DS18B20_neptune", "reset ret = %u", ret);
//    if (ret == 1)
//    {
//        return false;
//    }
//    OneWire_WriteByte(onewire, ONEWIRE_CMD_SKIPROM);
//    OneWire_WriteByte(onewire, DS18B20_CMD_CONVERTTEMP);
//    return true;
//}

///**
//  * @brief  读取温度(对于分辨率12位来讲，一次温度的转换是750ms，所以小于750ms的读温度的间隔是没有意义的)
//  * @param  temp_num：第几路温度
//  * @param  pVal：温度值
//  * @param  pFlag：温度正负,返回'+', '-'
//  * @retval 是否成功
//  */
//bool DS18B20_ReadTemp(uint8_t temp_num, uint16_t *pVal, uint8_t *pFlag)
//{

//    uint8_t lcv_buff[9];
//    OneWire_t *onewire = GetOneWire(temp_num);
//    if (onewire == NULL)
//    {
//        return false;
//    }

//    uint8_t ret = OneWire_Reset(onewire);
//    //LOGV("ds18b20", "reset ret = %u", ret);
//    if (ret == 1)
//    {
//        //LOGE("ds18b20", "initialize DS18B20(%u) ERROR!", cyNum);
//        return false;
//    }
//    OneWire_WriteByte(onewire, ONEWIRE_CMD_SKIPROM);
//    OneWire_WriteByte(onewire, ONEWIRE_CMD_RSCRATCHPAD);

//    /* Get data */
//    for (uint8_t i = 0; i < 9; i++)
//    {
//        /* Read byte by byte */
//        lcv_buff[i] = OneWire_ReadByte(onewire);
//    }
//    //hexdump(cyBuf, 9);
//    /* Calculate CRC */
//    uint8_t crc = OneWire_CRC8(lcv_buff, 8);
//    //LOGI("ds18b20", "crc = 0x%X, cyBuf[8] = 0x%X", crc, cyBuf[8]);
//    /* Check if CRC is ok */
//    if (crc != lcv_buff[8])
//    {
//        /* CRC invalid */
//        LOGE("ds18b20", "CRC invalid");
//        return false;
//    }

//    /* First two bytes of scratchpad are temperature values */
//    uint16_t temperature = lcv_buff[0] | (lcv_buff[1] << 8);

//    /* Reset line */
//    //OneWire_Reset(&gOneWire);

//    /* Check if temperature is negative */
//    if (temperature & 0x8000)
//    {
//        /* Two's complement, temperature is negative */
//        temperature = ~temperature + 1;
//        *pFlag = '-';
//    }
//    else
//    {
//        *pFlag = '+';
//    }

//    /* Get sensor resolution */
//    uint8_t resolution = ((lcv_buff[4] & 0x60) >> 5) + 9;
//    //    LOGV("ds18b20", "resolution = %u", resolution);

//    /* Store temperature integer digits and decimal digits */
//    int8_t digit = temperature >> 4;
//    digit |= ((temperature >> 8) & 0x7) << 4;

//    float decimal;
//    /* Store decimal digits */
//    switch (resolution)
//    {
//    case 9:
//        decimal = (temperature >> 3) & 0x01;
//        decimal *= (float)DS18B20_DECIMAL_STEPS_9BIT;
//        break;
//    case 10:
//        decimal = (temperature >> 2) & 0x03;
//        decimal *= (float)DS18B20_DECIMAL_STEPS_10BIT;
//        break;
//    case 11:
//        decimal = (temperature >> 1) & 0x07;
//        decimal *= (float)DS18B20_DECIMAL_STEPS_11BIT;
//        break;
//    case 12:
//        decimal = temperature & 0x0F;
//        decimal *= (float)DS18B20_DECIMAL_STEPS_12BIT;
//        break;
//    default:
//        decimal = 0xFF;
//        digit = 0;
//    }

//    decimal = digit + decimal;
//    decimal *= 10.0;

//    *pVal = decimal;

//    /* 对于分辨率12位来讲，一次温度的转换时间是750ms */
//    DS18B20_convert_temperature(onewire);

//    //LOGV("ds18b20", "Leave %s(), fTemp: %.3f, *pWVal = %u", __func__, decimal, *pWVal);
//    return true;
//}

///**
//  * @brief  初始化DS18B20
//  * @param  None
//  * @retval None
//  */
//void DS18B20_Init(void)
//{
//    OneWire_Init(&gOneWire1, FS_TEMP1_GPIO_Port, FS_TEMP1_Pin);
//    gOneWire1.appNum = DS18B20_NUM_1;
//    OneWire_Init(&gOneWire2, FS_TEMP2_GPIO_Port, FS_TEMP2_Pin);
//    gOneWire1.appNum = DS18B20_NUM_2;
//    //OneWire_Init(&gOneWire3, FS_TEMP3_GPIO_Port, FS_TEMP3_Pin);
//    //gOneWire1.appNum = DS18B20_NUM_3;

//    DS18B20_DisableAlarmTemperature(&gOneWire1);
//    DS18B20_DisableAlarmTemperature(&gOneWire2);
//    //DS18B20_DisableAlarmTemperature(&gOneWire3);

//    DS18B20_convert_temperature(&gOneWire1);
//    DS18B20_convert_temperature(&gOneWire2);
//    //DS18B20_convert_temperature(&gOneWire3);

//    LOGD("ds18b20", "Leave %s()", __func__);
//}


///**
//  * @brief  新建线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_ds18b20Task(void)
//{
//    ds18b20TaskHandle = osThreadNew(DS18B20Task, NULL, &ds18b20Task_attributes);
//}

///**
//  * @brief  Function implementing the DS18B20Task thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void DS18B20Task(void *argument)
//{
//    LOGD("ds18b20", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());
//    uint16_t TempVal[DS18B20_NUM_ALL];
//    uint8_t TempFlag, TryTimes = 0;

//    DS18B20_Init();

//    for(;;)
//    {
//        osDelay(1000);

//        for(uint8_t i = DS18B20_NUM_1; i < DS18B20_NUM_ALL; i++)
//        {
//            TryTimes = 0;
//            while(1)
//            {
//                TryTimes++;
//                if( DS18B20_ReadTemp(i, &TempVal[i], &TempFlag) == true )
//                {
//                    //LOGD("ds18b20", "read[%d] ok", i);
//                    if(TryTimes != 0)
//                    {
//                        TryTimes = 0;
//                    }
//#if 0
//                    if ('-' == TempFlag)
//                    {
//                        TempVal[i] |= 0x8000;
//                    }
//                    else
//                    {
//                        TempVal[i] &= 0xEFFF;
//                    }
//#else
//                    if ('-' == TempFlag)
//                    {
//                        TempVal[i] = ~TempVal[i] + 1;
//                    }
//                    else
//                    {
//                        TempVal[i] &= 0xEFFF;
//                    }
//#endif
//                    break;
//                }
//                if(TryTimes > DS18B20_TRY_TIMES_MAX)
//                {
//                    TempVal[i] = 0xFFFF;
//                    //LOGD("ds18b20", "read[%d] fail", i);
//                    break;
//                }
//                osDelay(100);
//            }
//        }

//        gFS_Elem.st.Tmp1 = TempVal[DS18B20_NUM_1];
//        gFS_Elem.st.Tmp2 = TempVal[DS18B20_NUM_2];
//        //LOGI("ds18b20", "temp1 = %d, temp2 = %d", gFS_Elem.st.Tmp1, gFS_Elem.st.Tmp2);
//    }
//}










