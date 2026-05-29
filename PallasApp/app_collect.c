
/* Private includes ----------------------------------------------------------*/
#include "cmsis_os.h"
#include "task.h"
#include "semphr.h"

#include "app_collect.h"
#include "mb.h"
#include "app_log.h"
#include "app_parameter.h"
#include "app_tm1650.h"
#include "app_uart.h"
#include "app_tool.h"
#include "app_key.h"
#include "mb_crc.h"

#include <string.h>
#include "plc_netcfg.h"
/* Private define ------------------------------------------------------------*/
#define  MB_RTU_ADDR_MAX    0x50

/* Private variables ---------------------------------------------------------*/
/* Definitions for collectTask */
osThreadId_t collectTaskHandle;
const osThreadAttr_t collectTask_attributes =
{
    .name = "collectTask",
    .priority = (osPriority_t) collectTaskPriority,
    .stack_size = 512
};

SemaphoreHandle_t gGetOneOK_BinSem = NULL;

osMutexDef( gCtrlPMutexDef );
osMutexId gCtrlPMutexId;



/* Different node total control point ----------------------------
   ----------------------------------------------- ESA  ESB  ESC  ESE  G1A3  M1AI  L2AI  LDA1  DEM2  DEM4  DEP4*/
const uint8_t CtrlPNum_Table[MAX_PRODEUCT_TYPE] = {4,    4,    9,    13,   5,    6,    4,    1,    9,    17,   5};


rtu_info_packet_t  gRtu_InfoPacket;   //要存入flash
rtu_workstate_t    gRtu_WorkState[MAX_RTU_NUM];
ctrlp_alarmlimit_t gCtrlP_AlmLim[MAX_CTRLP_NUM];//要存入flash
ctrlp_info_t       gCtrlP_Info[MAX_CTRLP_NUM];


uint8_t        gRtu_Num;     //已接的终端数量
__IO uint16_t  gRtu_Idx;     //终端查询计数Index
uint16_t       gCtrlP_Num;   //已有的控制点数量
__IO uint16_t  gCtrlP_Idx;   //控制点查询计数Index

uint8_t gGetAtLeastOne = 0; //至少获取过一次数据标志



/* Private function prototypes -----------------------------------------------*/
void CollectTask(void *argument);


/* Private user code ---------------------------------------------------------*/



/**
  * @brief  判断是否是mbClient返回的协议
  * @param  idnum 校验站号
  * @param  mbfuncCode 校验功能码
  * @param  *pData 数据
  * @param  len 数据长度
  * @retval true/false
  */
bool checkRecv_mbClientRtu(uint8_t idnum, uint8_t mbfuncCode, uint8_t *pData, uint16_t len)
{
    if(len < 2)
    {
        return false;
    }
    if( (idnum != *(pData + 0)) || (0 == *(pData + 0)) )
    {
        return false;
    }
    if(mbfuncCode != *(pData + 1))
    {
        return false;
    }
    if(checkRev_crc16(pData, len) != true)
    {
        return false;
    }
    return true;
}

/**
  * @brief  控制点报警阈值初始化
  * @param  None
  * @retval None
  */
void gCtrlP_AlmLim_init(void)
{
    uint16_t i, j, k;  //Rtu num, one CtrlP, all CtrlP

    k = 0;
    for(i = 0; i < gRtu_InfoPacket.RtuNum; i++)
    {
        for(j = 0; j < CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[i].RtuType]; j++)
        {
            if(gRtu_InfoPacket.Rtu_Info[i].RtuType == TYPE_DES3 || gRtu_InfoPacket.Rtu_Info[i].RtuType == TYPE_DES4 || \
                    gRtu_InfoPacket.Rtu_Info[i].RtuType == TYPE_DES9 || gRtu_InfoPacket.Rtu_Info[i].RtuType == TYPE_DESA || gRtu_InfoPacket.Rtu_Info[i].RtuType == TYPE_DEP4 )
            {
                if(j < 3)  //遥信 空开 接地
                {
                    PAR_SET_BIT(gCtrlP_AlmLim[k + j].ValidAlmType, ALARM_1_VALID);
                    gCtrlP_AlmLim[k + j].Alm1_Up = 1;
                }
            }
            else if(gRtu_InfoPacket.Rtu_Info[i].RtuType == TYPE_DEM2 || gRtu_InfoPacket.Rtu_Info[i].RtuType == TYPE_DEM4)
            {
                if(j < 4)  //电压状态1 接地 脱扣1 脱扣2
                {
                    PAR_SET_BIT(gCtrlP_AlmLim[k + j].ValidAlmType, ALARM_1_VALID);
                    gCtrlP_AlmLim[k + j].Alm1_Up = 1;
                }
                else if(j < 9)  //计数 电流1 温度1 温度2 寿命
                {
                    /**/
                }
                else if(j < 13)  //电压状态2 电压状态3 脱扣3 脱扣4
                {
                    PAR_SET_BIT(gCtrlP_AlmLim[k + j].ValidAlmType, ALARM_1_VALID);
                    gCtrlP_AlmLim[k + j].Alm1_Up = 1;
                }
            }
        }
        k += CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[i].RtuType];
    }
}

/**
  * @brief  网关数据初始化
  * @param  None
  * @retval None
  */
void gatewayparam_init(void)
{
    gRtu_Idx = 0;
    gCtrlP_Idx = 0;

    memset(&gRtu_WorkState, 0, sizeof(gRtu_WorkState));
    memset(&gCtrlP_Info, 0, sizeof(gCtrlP_Info));

//    Parameter_FlashRead(PAR_RTU_INFO_SAVE_ADDR, &gRtu_InfoPacket, sizeof(gRtu_InfoPacket));
//    Parameter_FlashRead(PAR_CTRL_INFO_SAVE_ADDR, &gCtrlP_AlmLim, sizeof(gCtrlP_AlmLim));

#if PROD_TYPE == PROD_FG
    if(gRtu_InfoPacket.RtuNum == 0xFFFFFFFF)
    {
        LOGW("collect", "Rtu InfoPacket and control point alarm limit Packet initialise.");
        memset(&gRtu_InfoPacket, 0, sizeof(gRtu_InfoPacket));
        memset(&gCtrlP_AlmLim, 0, sizeof(gCtrlP_AlmLim));
    }

    gRtu_Num = gRtu_InfoPacket.RtuNum;
    for(uint8_t i = 0; i < gRtu_Num; i++)
    {
        gCtrlP_Num += CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[i].RtuType];
    }
    LOGI("collect", "Rtu number is %d, Total control point is %d", gRtu_Num, gCtrlP_Num);
#else
    if( 0 != memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo, &gFlashParam.st.idInfo, 22) )  //序列号变化
    {
        LOGW("collect", "Rtu InfoPacket and control point alarm limit Packet initialise.");
        memset(&gRtu_InfoPacket, 0, sizeof(gRtu_InfoPacket));
        memset(&gCtrlP_AlmLim, 0, sizeof(gCtrlP_AlmLim));

        gRtu_InfoPacket.RtuNum = 1;
        gRtu_InfoPacket.Rtu_Info[0].RtuIdNum = gFlashParam.st.idNum;

        memcpy(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo, &gFlashParam.st.idInfo, 22);
        if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "DES4", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_DES4;
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "DES9", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_DES9;
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "DESA", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_DESA;
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "G1A3", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_G1A3;
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "M1AI", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_M1AI;
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "L2AI", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_L2AI;
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "LDA1", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_LDA1;
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "DEM2", 4) || 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DEA2", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_DEM2;
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[0].RtuIdInfo[18], "DEM4", 4) || 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DEA4", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_DEM4;
        }
        else
        {
            gRtu_InfoPacket.Rtu_Info[0].RtuType = TYPE_DES4;
        }

        gCtrlP_AlmLim_init();
//        Parameter_FlashWrite(PAR_RTU_INFO_SAVE_ADDR, &gRtu_InfoPacket, sizeof(gRtu_InfoPacket));
//        Parameter_FlashWrite(PAR_CTRL_INFO_SAVE_ADDR, &gCtrlP_AlmLim, sizeof(gCtrlP_AlmLim));
    }

    gRtu_Num = gRtu_InfoPacket.RtuNum;
    gCtrlP_Num = CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[0].RtuType];
    gRtu_WorkState[0].RtuWorkState = RTU_STATE_NORMAL;

    LOGI("collect", "Rtu number is %d, Total control point is %d", gRtu_Num, gCtrlP_Num);
#endif
}

/**
  * @brief  处理寻找Fexlink RTU时返回的数据
  * @param  pBuf 数据起始地址
  * @param  len 数据长度
  * @retval None
  */
void Handle_FindFlkRtu_RecvData(uint8_t *pBuf, uint16_t len)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

    if(checkRecv_mbClientRtu(gRtu_Idx, MB_READ_HOLDING_REGISTER, pBuf, len) &&
            (gRtu_Num < MAX_RTU_NUM) )
    {
        gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdNum = gRtu_Idx;
        memcpy(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo, &pBuf[3], 22);
        LOGI("collect", "FEXLINK node ID is %s", gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo);

        if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DES4", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_DES4;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );//给出二值信号量
            LOGI("collect", "TYPE_DES4");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DES9", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_DES9;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_DES9");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DESA", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_DESA;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_DESA");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "G1A3", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_G1A3;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_G1A3");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "M1AI", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_M1AI;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_M1AI");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "L2AI", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_L2AI;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_L2AI");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "LDA1", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_LDA1;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_LDA1");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DEM2", 4) || 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DEA2", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_DEM2;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_DEM2");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DEM4", 4) || 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DEA4", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_DEM4;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_DEM4");
        }
        else if( 0 == memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo[18], "DEP4", 4) )
        {
            gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_DEP4;
            xReturn = xSemaphoreGive( gGetOneOK_BinSem );
            LOGI("collect", "TYPE_DEP4");
        }

        if( xReturn == pdTRUE )
        {
            LOGD("collect", "GetOneOK_BSem_Handle Give success !\r\n");
        }
        else
        {
            LOGE("collect", "GetOneOK_BSem_Handle Give fail !\r\n");
        }
    }
}

/**
  * @brief  处理寻找HaiLeiKe RTU时返回的数据
  * @param  pBuf 数据起始地址
  * @param  len 数据长度
  * @retval None
  */
void Handle_FindHlkRtu_RecvData(uint8_t *pBuf, uint16_t len)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
    uint8_t nodeID[23];
    uint8_t tempID[7] = "210815";

    if(checkRecv_mbClientRtu(gRtu_Idx, MB_READ_HOLDING_REGISTER, pBuf, len) &&
            (gRtu_Num < MAX_RTU_NUM) )
    {
        gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdNum = gRtu_Idx;
        memcpy(tempID, &gFlashParam.st.idInfo[7], 6);  //获取网关ID序列号的第7位到第12位，KSDC120“190801”001,/COMM
        sprintf((char *)nodeID, "KSDDES3%s%03u,/DES4", tempID, gRtu_Idx);
        memcpy(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo, &nodeID[0], 22);

        LOGI("collect", "HaiLeiKe node ID is %s", gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo);
        gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType = TYPE_DES3;
        xReturn = xSemaphoreGive( gGetOneOK_BinSem );
        LOGI("collect", "TYPE_DES3");

        if( xReturn == pdTRUE )
        {
            LOGD("collect", "GetOneOK_BSem_Handle Give success !\r\n");
        }
        else
        {
            LOGE("collect", "GetOneOK_BSem_Handle Give fail !\r\n");
        }
    }
}

#if PROD_TYPE == PROD_SFC
/**
  * @brief  自动寻找终端
  * @param  None
  * @retval None
  */
void AutoFindRtu(void)
{
    LOGV("collect", "Enter %s(), MAX_CTRL_POINT_NUM = %u", __func__, MAX_CTRLP_NUM);

    uint8_t ucSendBuf[32];

    gRtu_Num = 0;
    gCtrlP_Num = 0;
    memset(&gRtu_InfoPacket, 0, sizeof(gRtu_InfoPacket));
    memset(gCtrlP_AlmLim, 0, sizeof(gCtrlP_AlmLim));  //报警门限清除
    for (uint16_t i = 0; i < MAX_CTRLP_NUM; i++)  //报警状态初始化
    {
        gCtrlP_Info[i].NewValue = 0;
        gCtrlP_Info[i].OldValue = 0;
        gCtrlP_Info[i].AlmState[ALARM_LEVEL_1] = STATE_NORMAL;
        gCtrlP_Info[i].AlmState[ALARM_LEVEL_2] = STATE_NORMAL;
    }

    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1)
    {
        gtv_UartPortStatus[UART_PORT1].mcv_Mode = UART_MODE_FINDFLKRTU;
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)
    {
        gtv_UartPortStatus[UART_PORT2].mcv_Mode = UART_MODE_FINDFLKRTU;
    }

    osDelay(100);

    LOGI("collect", "Begin search Fexlink 485 node......");
    for(gRtu_Idx = 1; gRtu_Idx <= MB_RTU_ADDR_MAX; gRtu_Idx++)
    {
        if(gRtu_Num >= MAX_RTU_NUM)
        {
            break;
        }

        LOGI("collect", "Fexlink Try ID : %u", gRtu_Idx);
        //_Display_GoRound(gRtu_Num);

        ucSendBuf[0] = gRtu_Idx;
        ucSendBuf[1] = MB_READ_HOLDING_REGISTER;
        ucSendBuf[2] = 0x00;
        ucSendBuf[3] = 0x01;
        ucSendBuf[4] = 0x00;
        ucSendBuf[5] = 0x0B;
        if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1)
        {
            uart_addCRC_send_buffer(&huart1, ucSendBuf, 6);
        }
        else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)
        {
            uart_addCRC_send_buffer(&huart2, ucSendBuf, 6);
        }

        //获取二值信号量 xSemaphore,没获取到则等待
        if(xSemaphoreTake(gGetOneOK_BinSem, gFlashParam.st.findRtuInterval))
        {
            LOGD("collect", "GetOneOK_BSem_Handle Take success!\r\n");
            gRtu_WorkState[gRtu_Num].RtuWorkState = RTU_STATE_NORMAL;
            gCtrlP_Num += CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType];
            gRtu_Num++;

            osDelay(gFlashParam.st.findRtuInterval);
        }
        else
        {
            LOGE("collect", "GetOneOK_BSem_Handle Take fail !\r\n");
        }
    }
    LOGV("collect", "Collect Fexlink Rtu Number = %u", gRtu_Num);

    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1)
    {
        gtv_UartPortStatus[UART_PORT1].mcv_Mode = UART_MODE_FINDHLKRTU;
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)
    {
        gtv_UartPortStatus[UART_PORT2].mcv_Mode = UART_MODE_FINDHLKRTU;
    }
    osDelay(100);

    LOGI("collect", "Begin search HaiLeiKe 485 node......");
    for(gRtu_Idx = 1; gRtu_Idx <= MB_RTU_ADDR_MAX; gRtu_Idx++)
    {
        if(gRtu_Num >= MAX_RTU_NUM)
        {
            break;
        }

        LOGI("collect", "HaiLeiKe Try ID : %u", gRtu_Idx);
        //_Display_GoRound(gRtu_Num);

        ucSendBuf[0] = gRtu_Idx;
        ucSendBuf[1] = MB_READ_HOLDING_REGISTER;
        ucSendBuf[2] = 0x07;
        ucSendBuf[3] = 0xD1;
        ucSendBuf[4] = 0x00;
        ucSendBuf[5] = 0x01;

        if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1)
        {
            uart_addCRC_send_buffer(&huart1, ucSendBuf, 6);
        }
        else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)
        {
            uart_addCRC_send_buffer(&huart2, ucSendBuf, 6);
        }

        //获取二值信号量 xSemaphore,没获取到则等待
        if(xSemaphoreTake(gGetOneOK_BinSem, gFlashParam.st.findRtuInterval))
        {
            LOGD("collect", "GetOneOK_BSem_Handle Take success!\r\n");
            gRtu_WorkState[gRtu_Num].RtuWorkState = RTU_STATE_NORMAL;
            gCtrlP_Num += CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuType];
            gRtu_Num++;

            osDelay(gFlashParam.st.findRtuInterval);
        }
        else
        {
            LOGE("collect", "GetOneOK_BSem_Handle Take fail !\r\n");
        }
    }
    LOGV("collect", "Collect Fexlink and HaiLeiKe Rtu All Number = %u", gRtu_Num);

    if (0 != gRtu_Num)
    {
        gRtu_InfoPacket.RtuNum = gRtu_Num;
        gCtrlP_AlmLim_init();
//        Parameter_FlashWrite(PAR_RTU_INFO_SAVE_ADDR, &gRtu_InfoPacket, sizeof(gRtu_InfoPacket));
//        Parameter_FlashWrite(PAR_CTRL_INFO_SAVE_ADDR, &gCtrlP_AlmLim, sizeof(gCtrlP_AlmLim));
    }
    //Gateway_Dispaly_num(gRtu_InfoPacket.RtuNum);

    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1)
    {
        gtv_UartPortStatus[UART_PORT1].mcv_Mode = UART_MODE_DEFAULT;
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)
    {
        gtv_UartPortStatus[UART_PORT2].mcv_Mode = UART_MODE_DEFAULT;
    }

    LOGV("collect", "Leave %s(), gRtuNum = %u, gCtrlPNum = %u", __func__, gRtu_InfoPacket.RtuNum, gCtrlP_Num);
}

/**
  * @brief  根据mbclient的mb地址和类型发送查询指令
  * @param  RtuId MB地址
  * @param  RtuType 产品类型(DES4、G1A3....)
  * @retval None
  */
void Handle_RtuType_ToSend(uint8_t RtuId, uint8_t RtuType)
{
    uint8_t sendBuf[6];
    uint8_t readRegNum;

    if(RtuType == TYPE_DES3)
    {
        sendBuf[0] = RtuId;
        sendBuf[1] = 0x03;
        sendBuf[2] = 0x07;
        sendBuf[3] = 0xD1;
        sendBuf[4] = 0x00;
        sendBuf[5] = 0x03;
    }
    else
    {
        switch(RtuType)
        {
        case TYPE_G1A3:
            readRegNum = 0x05;
            break;

        case TYPE_DES4:
            readRegNum = 0x04;
            break;

        case TYPE_DES9:
        case TYPE_LDA1:
            readRegNum = 0x0E;
            break;

        case TYPE_DESA:
            readRegNum = 0x0F;
            break;

        case TYPE_M1AI:
        case TYPE_L2AI:
            readRegNum = 0x0B;
            break;

        case TYPE_DEM2:
        case TYPE_DEM4:
            readRegNum = 0x11;
            break;
        
        case TYPE_DEP4:
            readRegNum = 0x05;
            break;
        }
        sendBuf[0] = RtuId;
        sendBuf[1] = 0x03;
        sendBuf[2] = 0x01;
        sendBuf[3] = 0x06;
        sendBuf[4] = 0x00;
        sendBuf[5] = readRegNum;
    }
    LOGV("collect", "Enter %s()， start to send read %d (0x%02x) value command.", __func__, RtuId, RtuId);

    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM1)
    {
        uart_addCRC_send_buffer(&huart1, sendBuf, 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)
    {
        uart_addCRC_send_buffer(&huart2, sendBuf, 6);
    }
}
#else
void AutoFindRtu(void)
{
}

void Handle_RtuType_ToSend(uint8_t RtuId, uint8_t RtuType)
{
}
#endif

/**
  * @brief  判断控制点报警门限类型
  * @param  Rtu_Type: 产品类型
  * @param  OneRtuCtrlP_Idx: 各产品的第几个数据
  * @retval NO_ALMLIM         没有报警概念的数据，如采集次数
            HAVE_UP_DOWN_LIM  有上下限（如温度电压）
            HAVE_UP_LIM       只有上限（未启用）
            0xFF              未知产品类型
  */
uint8_t Handle_RtuCtrlP_AlmLimType(uint8_t Rtu_Type, uint8_t OneRtuCtrlP_Idx)
{
    uint8_t ret;

    if( (Rtu_Type == TYPE_DES3) || (Rtu_Type == TYPE_DES4) || (Rtu_Type == TYPE_DES9) || (Rtu_Type == TYPE_DESA) || (Rtu_Type == TYPE_DEP4)  )
    {
        if(OneRtuCtrlP_Idx > 3) //计数 电压 电流 温度 寿命
        {
            ret = HAVE_UP_DOWN_ALMLIM;
        }
        else  //遥信 空开 接地
        {
            ret = HAVE_IO_ALMLIM;
        }
    }
    else if(Rtu_Type == TYPE_G1A3)
    {
        if(OneRtuCtrlP_Idx == 2) //主地网阻值
        {
            ret = HAVE_UP_DOWN_ALMLIM;
        }
        else
        {
            ret = NO_ALMLIM;
        }
    }
    else if((Rtu_Type == TYPE_M1AI) || (Rtu_Type == TYPE_L2AI) )
    {
        if(OneRtuCtrlP_Idx == 2) //极性
        {
            ret = HAVE_IO_ALMLIM;
        }
        else  //次数 峰值 持续时间 电荷量 单位能量
        {
            ret = HAVE_UP_DOWN_ALMLIM;
        }
    }
    else if(Rtu_Type == TYPE_G1A3)
    {
        ret = HAVE_UP_DOWN_ALMLIM;
    }
    else if((Rtu_Type == TYPE_DEM2) || (Rtu_Type == TYPE_DEM4) )
    {
        if(OneRtuCtrlP_Idx < 4) //电压状态1 接地 脱扣1 脱扣2
        {
            ret = HAVE_IO_ALMLIM;
        }
        else if(OneRtuCtrlP_Idx < 9) //计数 电流1 温度1 温度2 寿命
        {
            ret = HAVE_UP_DOWN_ALMLIM;
        }
        else if(OneRtuCtrlP_Idx < 13)  //电压状态2 电压状态3 脱扣3 脱扣4
        {
            ret = HAVE_IO_ALMLIM;
        }
        else  //电流2 电流3 温度3 温度4
        {
            ret = HAVE_UP_DOWN_ALMLIM;
        }
    }
    else
    {
        ret = 0xFF;
    }

    return ret;
}


/**
  * @brief  处理获取RTU数据时返回的数据
  * @param  pBuf 数据起始地址
  * @param  len 数据长度
  * @retval None
  */
bool is_RtuReturn_protocol(uint8_t *pBuf, uint16_t len)
{
    uint16_t oneP_Val;
    uint16_t i;

    if(checkRecv_mbClientRtu(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuIdNum, MB_READ_HOLDING_REGISTER, pBuf, len) != true)
    {
        return false;
    }
    if( (gCtrlP_Idx + CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType]) > gCtrlP_Num)
    {
        uint32_t temp = gCtrlP_Idx + CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType];
        LOGE("collect", "Error return False,gCtrlPNum = %d,gCtrlPIdx =%d,temp = %d", gCtrlP_Num, gCtrlP_Idx, temp);

        return false;
    }

    //根据终端的控制点数
    for(i = 0; i < CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType]; i++)
    {
        //LOGW("collect", "start to get %d(0x%02x) value and judge alarm ", gCtrlP_Idx + i, gCtrlP_Idx + i);

        /******************************将从终端读的信息进行赋值******************************/
        if(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_DES3)
        {
            if(i == 0)      //遥信
            {
                oneP_Val = pBuf[5] << 8 | pBuf[6];
            }
            else if(i == 1) //空开
            {
                oneP_Val = pBuf[7] << 8 | pBuf[8];
            }
            else if(i == 2) //空
            {
                oneP_Val = 0;
            }
            else if(i == 3) //雷击计数
            {
                oneP_Val = pBuf[3] << 8 | pBuf[4];
            }
        }
        else if(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_LDA1)
        {
            if(i == 0)  //剩余电流
            {
                oneP_Val = pBuf[25] << 8 | pBuf[26];
            }
        }
        else if(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_DEP4)
        {
            if(i == 0)      //遥信
            {
                oneP_Val = pBuf[5] << 8 | pBuf[6];
            }
            else if(i == 1) //空开
            {
                oneP_Val = 0;
            }
            else if(i == 2) //空
            {
                oneP_Val = 0;
            }
            else if(i == 3) //雷击计数
            {
                oneP_Val = pBuf[9] << 8 | pBuf[10];
            }
            else if(i == 4) //雷击计数
            {
                oneP_Val = pBuf[11] << 8 | pBuf[12];
            }
        }
        else if((gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_DES9) || (gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_DESA) )
        {
            if(i < 4)      //遥信 空开 接地 计数
            {
                oneP_Val = pBuf[3 + 2 * i] << 8 | pBuf[4 + 2 * i];
            }
            else if(i < 7)  //漏流
            {
                oneP_Val = pBuf[19 + 2 * (i - 4)] << 8 | pBuf[20 + 2 * (i - 4)];
            }
            else if(i < 9)  //温度
            {
                oneP_Val = pBuf[25 + 2 * (i - 7)] << 8 | pBuf[26 + 2 * (i - 7)];
            }
            else if(i < 12) //电压
            {
                oneP_Val = pBuf[13 + 2 * (i - 9)] << 8 | pBuf[14 + 2 * (i - 9)];
            }
            else  //寿命
            {
                oneP_Val = pBuf[31 + 2 * (i - 12)] << 8 | pBuf[32 + 2 * (i - 12)];
            }
        }
        else if( (gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_M1AI) || (gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_L2AI) )
        {
            if(i < 1) //次数
            {
                oneP_Val = pBuf[3] << 8 | pBuf[4];
            }
            else if(i < 4)   //峰值 极性 持续时间
            {
                oneP_Val = pBuf[11 + 2 * (i - 1)] << 8 | pBuf[12 + 2 * (i - 1)];
            }
            else  //电荷量 单位能量
            {
                oneP_Val = pBuf[19 + 2 * (i - 4)] << 8 | pBuf[20 + 2 * (i - 4)];
            }
        }
        else if( (gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_DEM2) || (gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType == TYPE_DEM4) )
        {
            if(i < 1) //电压状态1
            {
                oneP_Val = pBuf[3] << 8 | pBuf[4];
            }
            else if(i < 4)   //接地 脱扣1 脱扣2
            {
                oneP_Val = pBuf[9 + 2 * (i - 1)] << 8 | pBuf[10 + 2 * (i - 1)];
            }
            else if(i < 6)  //计数 电流1
            {
                oneP_Val = pBuf[19 + 2 * (i - 4)] << 8 | pBuf[20 + 2 * (i - 4)];
            }
            else if(i < 8)  //温度1 温度2
            {
                oneP_Val = pBuf[27 + 2 * (i - 6)] << 8 | pBuf[28 + 2 * (i - 6)];
            }
            else if(i < 9)  //寿命
            {
                oneP_Val = pBuf[35 + 2 * (i - 8)] << 8 | pBuf[36 + 2 * (i - 8)];
            }
            else if(i < 11)  //电压状态2 电压状态3
            {
                oneP_Val = pBuf[5 + 2 * (i - 9)] << 8 | pBuf[6 + 2 * (i - 9)];
            }
            else if(i < 13)  //脱扣3 脱扣4
            {
                oneP_Val = pBuf[15 + 2 * (i - 11)] << 8 | pBuf[16 + 2 * (i - 11)];
            }
            else if(i < 15)  //电流2 电流3
            {
                oneP_Val = pBuf[23 + 2 * (i - 13)] << 8 | pBuf[24 + 2 * (i - 13)];
            }
            else //温度3 温度4
            {
                oneP_Val = pBuf[31 + 2 * (i - 15)] << 8 | pBuf[32 + 2 * (i - 15)];
            }
        }
        else
        {
            oneP_Val = pBuf[3 + 2 * i] << 8 | pBuf[4 + 2 * i];
        }
        gCtrlP_Info[gCtrlP_Idx + i].NewValue = oneP_Val;

        /******************************赋值结束 开始判断处理告警******************************/
        //一级报警有效
        if( PAR_READ_BIT(gCtrlP_AlmLim[gCtrlP_Idx + i].ValidAlmType, ALARM_1_VALID) )
        {
            LOGW("collect", "MB address [%d(0x%02x)] Control point [%d] have valid alarm 01 limit", gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuIdNum, gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuIdNum, i);

            if(Handle_RtuCtrlP_AlmLimType(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType, i) == NO_ALMLIM)
            {
                /* do nothing */
            }
            else if(Handle_RtuCtrlP_AlmLimType(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType, i) == HAVE_IO_ALMLIM)
            {
                if(oneP_Val == gCtrlP_AlmLim[gCtrlP_Idx + i].Alm1_Up)
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] == STATE_NORMAL)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] = STATE_ALARM_START;
                    }
                }
                else //正常
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] == STATE_ALARM_RUNING)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] = STATE_ALARM_RELEASE;
                    }
                }
            }
            else if(Handle_RtuCtrlP_AlmLimType(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType, i) == HAVE_UP_ALMLIM)
            {
                if(oneP_Val > gCtrlP_AlmLim[gCtrlP_Idx + i].Alm1_Up)  //超出门限
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] == STATE_NORMAL)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] = STATE_ALARM_START;
                    }
                }
                else //正常
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] == STATE_ALARM_RUNING)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] = STATE_ALARM_RELEASE;
                    }
                }
            }
            else if(Handle_RtuCtrlP_AlmLimType(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType, i) == HAVE_UP_DOWN_ALMLIM)
            {
                if(oneP_Val > gCtrlP_AlmLim[gCtrlP_Idx + i].Alm1_Up)  //超出上限
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] == STATE_NORMAL)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] = STATE_ALARM_START;
                    }
                }
                else if(oneP_Val < gCtrlP_AlmLim[gCtrlP_Idx + i].Alm1_Down)  //超出下限
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] == STATE_NORMAL)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] = STATE_ALARM_START;
                    }
                }
                else  //正常
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] == STATE_ALARM_RUNING)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] = STATE_ALARM_RELEASE;
                    }
                }
            }
        }
        else //一级报警无效
        {
            if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] == STATE_ALARM_RUNING)
            {
                gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_1] = STATE_ALARM_RELEASE;
            }
        }
        //同上 二级报警有效
        if( PAR_READ_BIT(gCtrlP_AlmLim[gCtrlP_Idx + i].ValidAlmType, ALARM_2_VALID) )
        {
            LOGW("collect", "MB address [%d(0x%02x)] Control point [%d] have valid alarm 02 limit", gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuIdNum, gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuIdNum, i);

            if(Handle_RtuCtrlP_AlmLimType(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType, i) == NO_ALMLIM)
            {
                /* do nothing */
            }
            else if(Handle_RtuCtrlP_AlmLimType(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType, i) == HAVE_IO_ALMLIM)
            {
                if(oneP_Val == gCtrlP_AlmLim[gCtrlP_Idx + i].Alm2_Up)
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] == STATE_NORMAL)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] = STATE_ALARM_START;
                    }
                }
                else //正常
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] == STATE_ALARM_RUNING)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] = STATE_ALARM_RELEASE;
                    }
                }
            }
            else if(Handle_RtuCtrlP_AlmLimType(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType, i) == HAVE_UP_ALMLIM)
            {
                if(oneP_Val > gCtrlP_AlmLim[gCtrlP_Idx + i].Alm2_Up)  //超出门限
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] == STATE_NORMAL)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] = STATE_ALARM_START;
                    }
                }
                else //正常
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] == STATE_ALARM_RUNING)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] = STATE_ALARM_RELEASE;
                    }
                }
            }
            else if(Handle_RtuCtrlP_AlmLimType(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType, i) == HAVE_UP_DOWN_ALMLIM)
            {
                if(oneP_Val > gCtrlP_AlmLim[gCtrlP_Idx + i].Alm2_Up)  //超出上限
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] == STATE_NORMAL)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] = STATE_ALARM_START;
                    }
                }
                else if(oneP_Val < gCtrlP_AlmLim[gCtrlP_Idx + i].Alm2_Down)  //超出下限
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] == STATE_NORMAL)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] = STATE_ALARM_START;
                    }
                }
                else  //正常
                {
                    if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] == STATE_ALARM_RUNING)
                    {
                        gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] = STATE_ALARM_RELEASE;
                    }
                }
            }
        }
        else //二级报警无效
        {
            if(gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] == STATE_ALARM_RUNING)
            {
                gCtrlP_Info[gCtrlP_Idx + i].AlmState[ALARM_LEVEL_2] = STATE_ALARM_RELEASE;
            }
        }
        /*********************************** 处理告警结束 ****************************************/
    } /* end of for(i = 0; i < CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType]; i++) */

    LOGI("collected", "%03u ->%03u-> %.22s", gCtrlP_Num, gCtrlP_Idx, gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuIdInfo);

    xSemaphoreGive( gGetOneOK_BinSem );

    return true;
}

/**
  * @brief  获取所有终端的数据
  * @param  None
  * @retval None
  */
void GetAllRtuValue(void)
{
    LOGV("collect", "Enter %s()", __func__);

    CtrlPMutexLock();

    LOGI("collect", "Collect data begin...");
#if PROD_TYPE == PROD_FG
    for(gRtu_Idx = 0, gCtrlP_Idx = 0; gRtu_Idx < gRtu_Num; gRtu_Idx++)
    {
        Handle_RtuType_ToSend(gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuIdNum, gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType);

        //获取二值信号量 xSemaphore,没获取到则等待
        if(xSemaphoreTake(gGetOneOK_BinSem, gFlashParam.st.getRtuInterval))
        {
            LOGD("collect", "GetOneOK_BSem_Handle Take success!\r\n");

            if( (gRtu_WorkState[gRtu_Idx].RtuWorkState == RTU_STATE_LOSE) || (gRtu_WorkState[gRtu_Idx].RtuLoseCnt != 0) )
            {
                gRtu_WorkState[gRtu_Idx].RtuWorkState = RTU_STATE_NORMAL;
                gRtu_WorkState[gRtu_Idx].RtuLoseCnt = 0;
            }

            osDelay(gFlashParam.st.getRtuInterval);
        }
        else
        {
            gRtu_WorkState[gRtu_Idx].RtuLoseCnt++;
            if( (gRtu_WorkState[gRtu_Idx].RtuLoseCnt > RTU_LOSE_MAX_CNT) && (gRtu_WorkState[gRtu_Idx].RtuWorkState == RTU_STATE_NORMAL))
            {
                gRtu_WorkState[gRtu_Idx].RtuWorkState = RTU_STATE_LOSE;
            }

            LOGE("collect", "GetOneOK_BSem_Handle Take fail !\r\n");
        }

        gCtrlP_Idx += CtrlPNum_Table[gRtu_InfoPacket.Rtu_Info[gRtu_Idx].RtuType];
        gGetAtLeastOne++;
    }
#else /* no gateway */
    md_slave_msg_pack smsg = {0,};
    static uint8_t sendBuf[8];
    unsigned short lsv_Crc;

    sendBuf[0] = gFlashParam.st.idNum;
    sendBuf[1] = MB_READ_HOLDING_REGISTER;
    sendBuf[2] = 0x01;
    sendBuf[3] = 0x06;
    sendBuf[4] = 0x00;
#if 0
    sendBuf[5] = 0x0F;
#else
    sendBuf[5] = 0x11;
#endif
    lsv_Crc = calc_crc16(sendBuf, 6);
    sendBuf[6] = (unsigned char)lsv_Crc;
    sendBuf[7] = (unsigned char)(lsv_Crc >> 8);

    smsg.mcv_IsBroadcastInfo = 0;
    smsg.mcp_ReceiveBuff = sendBuf;
    smsg.msv_ReceiveLen = 8;
    smsg.mcv_Sender = ADD_CRC_NO_SENDER;
    smsg.mcp_RespBuff = (unsigned char *)pvPortMalloc(64);
    LOGI("uart", "smsg.mcp_RespBuff = 0x%08X, Free Heap Size = %d", (uint32_t)smsg.mcp_RespBuff, xPortGetFreeHeapSize());
    mb_slave_msg_handler(&smsg);
    is_RtuReturn_protocol(smsg.mcp_RespBuff, smsg.msv_RespLen);

    vPortFree(smsg.mcp_RespBuff);
    gGetAtLeastOne++;
#endif

    CtrlPMutexUnlock();

    LOGV("collect", "Leave %s()", __func__);
}

void CtrlPMutexInit(void)
{
    gCtrlPMutexId = osMutexNew( osMutex(gCtrlPMutexDef) );
}

int CtrlPMutexLock(void)
{
    return osMutexWait( gCtrlPMutexId, osWaitForever );
}

int CtrlPMutexUnlock(void)
{
    return osMutexRelease( gCtrlPMutexId );
}

/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_collectTask(void)
{
    collectTaskHandle = osThreadNew(CollectTask, NULL, &collectTask_attributes);
}

/**
  * @brief  Function implementing the CollectTask thread.
  * @param  argument: Not used
  * @retval None
  */
void CollectTask(void *argument)
{
    LOGD("collect", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());
    LOGI("collect", "sizeof(gRtu_InfoPacket) = %d,  sizeof(gCtrlP_AlmLim) = %d ", sizeof(gRtu_InfoPacket), sizeof(gCtrlP_AlmLim));

    for (uint16_t i = 0; i < MAX_CTRLP_NUM; i++)  //报警状态初始化
    {
        gCtrlP_Info[i].NewValue = 0;
        gCtrlP_Info[i].OldValue = 0;
        gCtrlP_Info[i].AlmState[ALARM_LEVEL_1] = STATE_NORMAL;
        gCtrlP_Info[i].AlmState[ALARM_LEVEL_2] = STATE_NORMAL;
    }

    CtrlPMutexInit();

    /* 创建 BinarySem */
    gGetOneOK_BinSem = xSemaphoreCreateBinary();
    if(NULL != gGetOneOK_BinSem)
    {
        LOGD("collect", "GetOneOK_BSem_Handle create success!\r\n");
    }

    for(;;)
    {
        osDelay(100);

#if 1
        //30s内或正在find Rtu时不进行mqtt
        if( (gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_FINDFLKRTU) || (gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_FINDHLKRTU) || (gParam.st.SecCnt < 30) ||
                (gtv_UartPortStatus[UART_PORT2].mcv_Mode == UART_MODE_FINDFLKRTU) || (gtv_UartPortStatus[UART_PORT2].mcv_Mode == UART_MODE_FINDHLKRTU) )
#else  //该代码只用作一体机，不用做网关
        if(gParam.st.SecCnt < 30)
#endif
        //AutoFindRtu();
        {
            continue ;
        }

        GetAllRtuValue();

        osDelay(gFlashParam.st.getRtuCycleTime - 100);
    }
}









