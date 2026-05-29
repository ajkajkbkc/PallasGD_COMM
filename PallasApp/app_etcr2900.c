
///* Private includes ----------------------------------------------------------*/
//#include "main.h"
//#include "cmsis_os.h"

//#include <string.h>
//#include <stdlib.h>

//#include "app_log.h"
//#include "app_uart.h"
//#include "app_etcr2900.h"
//#include "mb.h"
//#include "app_parameter.h"
//#include "app_tm1650.h"
//#include "app_key.h"


///* Private define ------------------------------------------------------------*/



///* Private variables ---------------------------------------------------------*/
///* Definitions for R2900Task */
//osThreadId_t R2900TaskHandle;
//const osThreadAttr_t R2900Task_attributes =
//{
//    .name = "R2900Task",
//    .priority = (osPriority_t) R2900TaskPriority,
//    .stack_size = 1024
//};

//const uint8_t lcv_GetValCmd[6] = {0x01, 0x03, 0x00, 0x0d, 0x00, 0x02};    //同时读取电阻值和小数位命令
//const uint8_t lcv_StartTestCmd[6] = {0x01, 0x06, 0x00, 0x11, 0x00, 0x01}; //开始测试命令

//__IO uint8_t R2900_State = R2900_INIT;

//uint32_t gR2900ItvTime;

///* Private function prototypes -----------------------------------------------*/
//void R2900Task(void *argument);


///* Private user code ---------------------------------------------------------*/

///**
//  * @brief  (uart3)RS485发送数据给下位机(ETCR2900)
//  * @param  数据缓存(*pBuff)
//  * @param  数据长度(len)
//  * @retval None
//  */
//static void R2900_SendData(uint8_t *pBuff, uint16_t len)
//{
//    uart_addCRC_send_buffer(&huart3, pBuff, len);
//}

///**
//  * @brief  发送启动电阻测试指令
//  * @param  None
//  * @retval None
//  */
//void R2900_StartTest(void)
//{
//    uint8_t tempBuf[6];

//    memcpy(tempBuf, lcv_StartTestCmd, sizeof(lcv_StartTestCmd));
//    R2900_SendData(tempBuf, sizeof(lcv_StartTestCmd));
//}

///**
//  * @brief  发送读接地电阻值指令
//  * @param  None
//  * @retval None
//  */
//void R2900_GetVal(void)
//{
//    uint8_t tempBuf[6];

//    memcpy(tempBuf, lcv_GetValCmd, sizeof(lcv_GetValCmd));
//    R2900_SendData(tempBuf, sizeof(lcv_GetValCmd));
//}

///**
//  * @brief  检验接收数据包
//  * @param  站号(IdNum)
//  * @param  功能码(Cmd)
//  * @param  数据缓存(*pBuff)
//  * @param  数据长度(len)
//  * @retval (TRUE , FALSE)
//  */
//static bool R2900_CheckRecvPacket(uint8_t IdNum, uint8_t Cmd, uint8_t *pBuff, uint16_t len)
//{
//    if(IdNum != pBuff[0])
//    {
//        return false;
//    }
//    if(Cmd != pBuff[1])
//    {
//        return false;
//    }
//    if(checkRev_crc16(pBuff, len) != true)
//    {
//        return false;
//    }

//    return true;
//}

///**
//  * @brief  阻值改变
//  * @param  阻值
//  * @retval 改变后的阻值(放大100倍)
//  */
//static uint16_t change_GRes(float start_GRes)
//{
//    uint16_t end_GRes;
//    int32_t ltemp;

//    float fk = gFlashParam.st.GRes_K == 0 ?	1 : (float)gFlashParam.st.GRes_K / 100.0 ;
//    int16_t wb = gFlashParam.st.GRes_B;

//    ltemp = start_GRes * 100;
//    ltemp *= fk;
//    ltemp += wb;
//    if(ltemp > 0xFFFF)
//    {
//        ltemp = 0xFFFF;
//    }
//    else if(ltemp < 0)
//    {
//        ltemp = 0;
//    }
//    end_GRes = (uint16_t)ltemp;

//    if(gFlashParam.st.changeGResFlag)
//    {
//        if( (end_GRes <= gFlashParam.st.changeGResMax) && (end_GRes >= gFlashParam.st.changeGResMin) )
//        {
//            srand((unsigned)HAL_GetTick());  //随机数种子更新
//            ltemp = gFlashParam.st.changeGRes + rand() % gFlashParam.st.changeGResWidth;
//            if(ltemp > 0xFFFF)
//            {
//                ltemp = 0xFFFF;
//            }
//            else if(ltemp < 0)
//            {
//                ltemp = 0;
//            }
//            end_GRes = (uint16_t)ltemp;
//        }
//    }

//    return end_GRes;
//}

///**
//  * @brief  判断是否是ETCR2900协议
//  * @param  *pBuff 数据
//  * @param  len 数据长度
//  * @retval true/false
//  */
//bool is_R2900_protocol(uint8_t *pBuff, uint16_t len)
//{
//    float fTemp;
//    uint8_t r_K;//电阻整数值高8位,低8位,小数位数
//    uint8_t i, K = 1;

//    if(R2900_CheckRecvPacket(0x01, 0x06, pBuff, len))
//    {
//        for(uint8_t i = 0; i < 6; i++)
//        {
//            if(lcv_StartTestCmd[i] != pBuff[i])
//            {
//                return false;
//            }
//        }
//        R2900_State = R2900_START_SUCESS;
//        LOGD("etcr2900", "收到开始测试响应");

//        return true;
//    }
//    else if(R2900_CheckRecvPacket(0x01, 0x03, pBuff, len))
//    {
//        r_K = pBuff[6]; //小数系数

//        fTemp = pBuff[3] << 8 | pBuff[4];
//        for(i = 0; i < r_K; i++)
//        {
//            K *= 10;
//        }
//        fTemp /= K;
//        LOGW("etcr2900", "Get R2900 Value: %.2f", fTemp);

//        gFlashParam.st.GResVal = change_GRes(fTemp);
//        gFlashParam.st.GResGetTimes++;
//        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));

//        if(gKey1Mode == KEY_MODE_DEFAULT)
//        {
//            _Dispaly_100x_num(gFlashParam.st.GResVal);
//        }
//        R2900_State = R2900_READ_COMPLETE;

//        LOGI("etcr2900", "read gFlashParam.st.GResVal = %d", gFlashParam.st.GResVal);

//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}

///* R2900 Task     |
//                  V___INIT
//                  |
// >>_______________V___IDLE
// |              5s|
// |                |___START
// |             35s|
// |                |___COMPLETE
// |     ItvTime-40s|
// |<<______________V
//*/
///**
//  * @brief  新建线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_R2900Task(void)
//{
//    R2900TaskHandle = osThreadNew(R2900Task, NULL, &R2900Task_attributes);
//}

///**
//  * @brief  Function implementing the R2900Task thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void R2900Task(void *argument)
//{
//    LOGD("etcr2900", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());
//    static uint32_t sTick = 0;

//    for(;;)
//    {

//        switch(R2900_State)
//        {
//        case R2900_INIT:
//            sTick = gParam.st.SecCnt;
//            R2900_State = R2900_IDLE;
//            break;

//        case R2900_IDLE:
//            if(gParam.st.SecCnt - sTick >= 5)
//            {
//                sTick = gParam.st.SecCnt;
//                R2900_StartTest();
//                R2900_State = R2900_START_TEST;
//                LOGD("etcr2900", "发送开始测试命令");
//            }
//            break;

//        case R2900_START_TEST:
//            if(gParam.st.SecCnt - sTick >= 10)
//            {
//                sTick = gParam.st.SecCnt;
//                R2900_State = R2900_IDLE;
//                LOGE("etcr2900", "超过10s未收到开始测试响应");
//            }
//            break;

//        case R2900_START_SUCESS:
//            if(gParam.st.SecCnt - sTick >= 35)
//            {
//                sTick = gParam.st.SecCnt;
//                R2900_GetVal();
//                R2900_State = R2900_READ_VALE;
//                LOGD("etcr2900", "发送读取当前值命令");
//            }
//            break;

//        case R2900_READ_VALE:
//            if(gParam.st.SecCnt - sTick >= 10)
//            {
//                sTick = gParam.st.SecCnt;
//                R2900_State = R2900_IDLE;
//                LOGE("etcr2900", "超过10s未读到当前值");
//            }
//            break;

//        case R2900_READ_COMPLETE:
//            if(gParam.st.SecCnt - sTick >= gR2900ItvTime - 40) //计算出采集花费时间40s
//            {
//                sTick = gParam.st.SecCnt;
//                R2900_State = R2900_IDLE;
//                LOGD("etcr2900", "ETCR状态恢复空闲");
//            }
//            break;
//        }

//        osDelay(1000);
//    }
//}





