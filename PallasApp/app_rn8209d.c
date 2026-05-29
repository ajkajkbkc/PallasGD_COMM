///*
// * Copyright (c) 2006-2018, Fexlink Development Team
// *
// * SPDX-License-Identifier: Apache-2.0
// *
// * Change Logs:
// * Date           Author       Notes
// * 2022-07-02     Arrbow       first implementation
// */

///* Private includes ----------------------------------------------------------*/
//#include "app_rn8209d.h"

//#if  PROD_TYPE == PROD_FSS
//#include <math.h>
//#include "FreeRTOS.h"
//#include "task.h"
//#include "semphr.h"
//#include "app_log.h"
//#include "app_parameter.h"
//#include "app_tool.h"
//#include "cmsis_os.h"

///* Private define ------------------------------------------------------------*/
//#define RN8209D_CSSet()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
//#define RN8209D_CSClr()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

//#define  SYS_STATE_01_METER_Msk       0x0001


///* Private variables ---------------------------------------------------------*/
///* Definitions for rn8209dTask */
//osThreadId_t rn8209dTaskHandle;
//const osThreadAttr_t rn8209dTask_attributes =
//{
//    .name = "rn8209dTask",
//    .priority = (osPriority_t) rn8209dTaskPriority,
//    .stack_size = 2048
//};
//SemaphoreHandle_t gRn8209dMutex;

//rn_meter_run_info_t rn_gMeterRunInfo;



///* Private function prototypes -----------------------------------------------*/
//void Rn8209dTask(void *argument);

//void rn_meter_cali_param_init(void);


///* Private user code ---------------------------------------------------------*/


//HAL_StatusTypeDef rn_spi_write_and_read(uint8_t *data, uint16_t bytes_number)
//{
//    HAL_StatusTypeDef ret;

//    if(gRn8209dMutex == NULL)
//    {
//        RN8209D_CSClr();
//        ret = HAL_SPI_TransmitReceive( &hspi2, data, data, bytes_number, HAL_MAX_DELAY );
//        RN8209D_CSSet();
//    }
//    else
//    {
//        xSemaphoreTake(gRn8209dMutex, portMAX_DELAY);
//        RN8209D_CSClr();
//        ret = HAL_SPI_TransmitReceive( &hspi2, data, data, bytes_number, HAL_MAX_DELAY );
//        RN8209D_CSSet();
//        xSemaphoreGive(gRn8209dMutex);
//    }

//    return ret;
//}

///**
//  * @brief  Read 8209 register
//  * @param  addr: register address
//  * @retval value
//  */
//static uint32_t rn8209_read_register(uint8_t addr)
//{
//    uint32_t regVal;
//    uint8_t data[5] = {0};

//    memset(data, 0, sizeof(data));
//    data[0] = addr;
//    switch(addr)
//    {
//    case rw_PhsA:
//    case rw_PhsB:
//    case rw_IE:
//    case r_IF:
//    case r_RIF:
//    case r_SysStatus:
//        rn_spi_write_and_read(data, 2);
//        regVal = data[1];
//        break;
//    case rw_SYSCON:
//    case rw_EMUCON:
//    case rw_HFConst:
//    case rw_PStart:
//    case rw_QStart:
//    case rw_GPQA:
//    case rw_GPQB:
//    case rw_QPhsCal:
//    case rw_APOSA:
//    case rw_APOSB:
//    case rw_RPOSA:
//    case rw_RPOSB:
//    case rw_IARMSOS:
//    case rw_IBRMSOS:
//    case rw_IBGain:
//    //case rw_D2FPL:
//    //case rw_D2FPH:
//    //case rw_DCIAH:
//    //case rw_DCIBH:
//    //case rw_DCUH:
//    //case rw_DCL:
//    //case rw_EMUCON2:
//    case rw_PFCnt:
//    case rw_QFCnt:
//    case r_UFreq:
//    case r_LastWData:
//        rn_spi_write_and_read(data, 3);
//        regVal = data[1] << 8 | data[2];
//        break;
//    case r_IARMS:
//    case r_IBRMS:
//    case r_URMS:
//    case r_EnergyP:
//    case r_EnergyP2:
//    case r_EnergyQ:
//    case r_EnergyQ2:
//    case r_EMUStatus:
//    //case r_SPL_IA:
//    //case r_SPL_IB:
//    //case r_SPL_U:
//    //case r_UFREQ2:
//    case r_DeciveID:
//        rn_spi_write_and_read(data, 4);
//        regVal = data[1] << 16 | data[2] << 8 | data[3];
//        break;
//    case r_PowerPA:
//    case r_PowerPB:
//    case r_PowerQ:
//    case r_LastRData:
//        rn_spi_write_and_read(data, 5);
//        regVal = data[1] << 24 | data[2] << 16 | data[3] << 8 | data[4];
//        break;
//    default :
//        break;
//    }

//    return regVal;
//}

///**
//  * @brief  Write 8209 register
//  * @param  addr: register address
//  * @param  regVal: value
//  * @retval None
//  */
//static void rn8209_write_register(uint8_t addr, uint16_t regVal)
//{
//    uint8_t data[4];

//    //写使能
//    memset(data, 0, sizeof(data));
//    data[0] = w_Cmd;
//    data[1] = w_Cmd_Write_Enable;
//    rn_spi_write_and_read(data, 2);

//    //写数据
//    memset(data, 0, sizeof(data));
//    data[0] = addr | RN8209D_WRITE_REG;
//    switch(addr)
//    {
//    case rw_PhsA:
//    case rw_PhsB:
//    case rw_IE:
//    case w_Cmd:
//        data[1] = regVal & 0xFF;
//        rn_spi_write_and_read(data, 2);
//        break;
//    case rw_SYSCON:
//    case rw_EMUCON:
//    case rw_HFConst:
//    case rw_PStart:
//    case rw_QStart:
//    case rw_GPQA:
//    case rw_GPQB:
//    case rw_QPhsCal:
//    case rw_APOSA:
//    case rw_APOSB:
//    case rw_RPOSA:
//    case rw_RPOSB:
//    case rw_IARMSOS:
//    case rw_IBRMSOS:
//    case rw_IBGain:
//    //case rw_D2FPL:
//    //case rw_D2FPH:
//    //case rw_DCIAH:
//    //case rw_DCIBH:
//    //case rw_DCUH:
//    //case rw_DCL:
//    //case rw_EMUCON2:
//    case rw_PFCnt:
//    case rw_QFCnt:
//        data[1] = regVal >> 8;
//        data[2] = regVal & 0xFF;
//        rn_spi_write_and_read(data, 3);
//        break;
//    default :
//        break;
//    }

//    //写保护
//    memset(data, 0, sizeof(data));
//    data[0] = w_Cmd;
//    data[1] = w_Cmd_Write_Protect;
//    rn_spi_write_and_read(data, 2);
//}

//static bool rn8209d_spi_is_ok(void)
//{
//    uint32_t regVal;
//    uint8_t errtimes;

//    errtimes = 0;
//    while(errtimes++ < 10)
//    {
//        vTaskDelay(10);
//        regVal = rn8209_read_register(r_DeciveID);
//        LOGI("rn8209d", "regVal = 0x%04X", regVal);
//        if (0x820900 == regVal)
//        {
//            return true;
//        }
//    }

//    return false;
//}

///**
//  * @brief  读取计量参数
//  * @param  None
//  * @retval 读取值
//  */
//unsigned int RN8209D_Get_MeterPara(unsigned char addr)
//{
//    uint32_t value = 0;

//    if(PAR_READ_BIT(gParam.st.SysState01, SYS_STATE_01_METER_Msk))
//    {
//        return 0;
//    }

//    value = rn8209_read_register(addr);

//    return value;
//}

///**
//  * @brief  修复计量模块
//  * @param  None
//  * @retval None
//  */
//void rn_Repair_Metering(void)
//{
//    uint16_t param;
//    uint8_t i, addr, first_run;

//    rn8209_write_register(w_Cmd, w_Cmd_Reset);  //软复位
//    vTaskDelay(100);
//    rn8209_read_register(r_SysStatus); //系统状态寄存器读后清零，变为0x04
//    first_run = gFlashParam.st.meter_cali[rn_offset_SYSCON] == 0xFFFF ? 1 : 0; //第一次启动?
//    rn_gMeterRunInfo.CheckSum0 = 0;
//    for(i = 0; i <= rn_offset_IBGain; i++)
//    {
//        if(first_run) //读取计量芯片校表参数
//        {
//            gFlashParam.st.meter_cali[i] = rn8209_read_register(i);
//        }
//        else
//        {
//            addr = i;
//            param = rn8209_read_register(addr);
//            if(param != gFlashParam.st.meter_cali[i])
//            {
//                rn8209_write_register(addr, gFlashParam.st.meter_cali[i]);
//            }
//        }
//        rn_gMeterRunInfo.CheckSum0 += gFlashParam.st.meter_cali[i];
//    }
//    vTaskDelay(1);  //一次检验和计算需要11.2us

//    if(first_run)
//    {
//        LOGV("rn8209d", "first run init gFlashParam.st.meter_cali.");
//        rn_meter_cali_param_init();
//        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//    }
//    else
//    {
//        rn_gMeterRunInfo.EmuWork = rn_EMU_RUNNING;
//    }
//}

///**
//  * @brief  读取电流
//  * @param  phase 0/1 => A/B
//  * @retval None
//  */
//float rn_Read_Current(uint8_t phase)
//{
//    uint32_t itemp;
//    float ftemp;

//    itemp = rn8209_read_register(r_IARMS + phase);  //A,B,C相电流
//    ftemp = (float)itemp;
//    ftemp /= gFlashParam.st.N_Ib_RN8209D;

//    return ftemp;
//}

///**
//  * @brief  校准电流偏移
//  * @param  None
//  * @retval None
//  */
//void rn_Cali_Offset_Current(void)
//{
//    uint8_t i;
//    uint16_t times;
//    uint32_t uCur;

//    times = 6;

//    rn8209_write_register(rw_IARMSOS, 0x0000);
//    vTaskDelay(2000);  //清零后需等待

//    rn8209_read_register(r_IARMS);
//    vTaskDelay(100);
//    uCur = 0;
//    for(i = 0; i < times; i++)
//    {
//        uCur += rn8209_read_register(r_IARMS);
//        vTaskDelay(100);
//    }

//    uCur /= times;
//    uCur *= uCur;  //平方
//    uCur = ~uCur;  //取反
//    uCur >>= 8;    //取bit23-8
//    uCur &= 0xFFFF;
//    gFlashParam.st.meter_cali[rn_offset_IARMSOS] = uCur;
//    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

//    rn_gMeterRunInfo.EmuWork = rn_EMU_INIT;
//}

///**
//  * @brief  校准电流比例系数N
//  * @param  fCali_Cur 标准源输出电流值
//  * @retval None
//  */
//void rn_Cali_N_Current(float fCali_Cur)
//{
//    uint8_t i;
//    uint16_t times;
//    uint32_t uCur;

//    times = 6;

//    rn8209_read_register(r_IARMS);
//    vTaskDelay(100);
//    uCur = 0;
//    for(i = 0; i < times; i++)
//    {
//        uCur += rn8209_read_register(r_IARMS);
//        vTaskDelay(100);
//    }
//    uCur /= times;

//    gFlashParam.st.N_Ib_RN8209D = uCur / fCali_Cur;
//    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

//    rn_gMeterRunInfo.EmuWork = rn_EMU_INIT;
//}

///**
//  * @brief  读取计量参数
//  * @param  None
//  * @retval None
//  */
//void rn_Read_MeterPara(void)
//{
//    float ftemp, fstartup;

//    fstartup = (float)FSS_Current_Startup / 10;
//    //读取电流-----------------------------------------------------------
//    ftemp = rn_Read_Current(0);
//    ftemp = ftemp < fstartup ? 0 : ftemp;
//    gFSS_Elem.st.Cur[0] = (uint16_t)(ftemp * 10);
//}

///**
//  * @brief  初始化计量校表参数
//  * @param  None
//  * @retval None
//  */
//void rn_meter_cali_param_init(void)
//{
//    gFlashParam.st.meter_cali[rn_offset_SYSCON] = 0x0001;
//    gFlashParam.st.meter_cali[rn_offset_EMUCON] = 0x0000;

//    gFlashParam.st.meter_cali[rn_offset_IARMSOS] = 0xFF01;
//    gFlashParam.st.meter_cali[rn_offset_IBGain] = 0x00;
//}

///**
//  * @brief  初始化计量任务
//  * @param  None
//  * @retval None
//  */
//void rn_Init_TaskMetering(void)
//{
//    rn_gMeterRunInfo.EmuWork = rn_EMU_INIT;
//    rn_gMeterRunInfo.CheckSum0 = 0;
//}

///**
//  * @brief  新建线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_rn8209dTask(void)
//{
//    rn8209dTaskHandle = osThreadNew(Rn8209dTask, NULL, &rn8209dTask_attributes);
//}

///**
//  * @brief  Function implementing the uartTask thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void Rn8209dTask(void *argument)
//{
//    LOGD("rn8209d", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

//    rn_Init_TaskMetering();

//    rn8209_write_register(w_Cmd, w_Cmd_Reset);  //软复位
//    vTaskDelay(100);
//    if(!rn8209d_spi_is_ok())
//    {
//        LOGE("rn8209d", "Delete Rn8209dTask");
//        PAR_SET_BIT(gParam.st.SysState01, SYS_STATE_01_METER_Msk);
//        vTaskDelete(NULL); //删除任务
//    }
//    PAR_CLEAR_BIT(gParam.st.SysState01, SYS_STATE_01_METER_Msk);

//    for(;;)
//    {
//        switch(rn_gMeterRunInfo.EmuWork)
//        {
//        case rn_EMU_INIT:
//            rn_Repair_Metering();
//            break;

//        case rn_EMU_RUNNING:
//            rn_Read_MeterPara();
//            break;

//        case rn_EMU_CALI_OFFSET_CUR:
//            rn_Cali_Offset_Current();
//            break;

//        case rn_EMU_CALI_N_CUR:
//            rn_Cali_N_Current(gParam.st.fCali_Cur);
//            break;

//        default:
//            rn_gMeterRunInfo.EmuWork = rn_EMU_INIT;
//            break;
//        }

//        osDelay(500);
//    }
//}


//#endif
