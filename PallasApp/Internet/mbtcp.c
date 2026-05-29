
/* Private includes ----------------------------------------------------------*/
#include "internet.h"
#include "mbtcp.h"
#include "mb_crc.h"
#include "mb.h"
#include "socket.h"
#include "app_log.h"
#include "app_tool.h"
#include "app_parameter.h"

#include "cmsis_os.h"
#include <string.h>


/* Private define ------------------------------------------------------------*/
/*
 *
 * <------------------------ MODBUS TCP/IP ADU(1) ------------------------->
 *              <----------- MODBUS PDU (1') ---------------->
 *  +-----------+---------------+------------------------------------------+
 *  | TID | PID | Length | UID  |Code | Data                               |
 *  +-----------+---------------+------------------------------------------+
 *  |     |     |        |      |
 * (2)   (3)   (4)      (5)    (6)
 *
 * (2)  ... MB_TCP_TID          = 0 (Transaction Identifier - 2 Byte)
 * (3)  ... MB_TCP_PID          = 2 (Protocol Identifier - 2 Byte)
 * (4)  ... MB_TCP_LEN          = 4 (Number of bytes - 2 Byte)
 * (5)  ... MB_TCP_UID          = 6 (Unit Identifier - 1 Byte)
 * (6)  ... MB_TCP_FUNC         = 7 (Modbus Function Code)
 *
 * (1)  ... Modbus TCP/IP Application Data Unit
 * (1') ... Modbus Protocol Data Unit
 */
#define MB_TCP_BUF_SIZE     ( 256 + 7 ) /* Must hold a complete Modbus TCP frame. */

#define MB_TCP_TID1         0
#define MB_TCP_TID2         1
#define MB_TCP_PID          2
#define MB_TCP_LEN          4
#define MB_TCP_UID          6
#define MB_TCP_FUNC         7

#define MB_TCP_PROTOCOL_ID  0   /* 0 = Modbus Protocol */

#define MB_TCP_PORT         502


/* Private variables ---------------------------------------------------------*/
/* Definitions for mbTcpTask */
osThreadId_t mbTcpTaskHandle;
const osThreadAttr_t mbTcpTask_attributes =
{
    .name = "mbTcpTask",
    .priority = (osPriority_t) mbtcpTaskPriority,
    .stack_size = 1024
};

static uint8_t       aucTCPBuf[MB_TCP_BUF_SIZE];

volatile uint8_t     *pucRTUBufferCur;
volatile uint16_t    usRTUBufferPos;

uint8_t mbTCPtid1, mbTCPtid2;

uint8_t gMbTcpErrorTimes = 0;

/* Private function prototypes -----------------------------------------------*/
void MbTcpTask(void *argument);


/* Private user code ---------------------------------------------------------*/
/**
  * @brief  mb tcp send data
  * @param  *pBuff 数据起始地址
  * @param  len 数据长度
  * @param  clientID
  * @retval None
  */
void mbTcpSend(unsigned char *pBuff, unsigned short len, unsigned char clientID)
{
    LOGV("mbtcp", "Enter %s(), will malloc size %d", __func__, len + 12);

    uint8_t *modbustcpBuf = (uint8_t *)aucTCPBuf;
    LOGI("mbtcp", "modbustcpBuf = 0x%08X", (uint32_t)aucTCPBuf);
    memset(modbustcpBuf, 0, len + 12);
    modbustcpBuf[0] = mbTCPtid1;
    modbustcpBuf[1] = mbTCPtid2;

    modbustcpBuf[2] = 0;
    modbustcpBuf[3] = 0;

    modbustcpBuf[4] = ( len ) >> 8U;
    modbustcpBuf[5] = ( len ) & 0xFF;

    memcpy(modbustcpBuf + 6, pBuff, len);
    uint16_t sendLen = len + 6;
//    hexdump(modbustcpBuf, sendLen);  //打log

    switch(getSn_SR(APP_SOCKET_MODBUS))
    {
#if 0
    case SOCK_UDP :
        sendto(SOCK_DATA, (uint8_t *)pucTCPBufferCur, usTCPBufferPos, net->remote_ip, net->remote_port);
        break;
#endif
    case SOCK_ESTABLISHED:
    case SOCK_CLOSE_WAIT:
        send(APP_SOCKET_MODBUS, (uint8_t *)modbustcpBuf, sendLen);
        break;
    default:
        break;
    }
}

/**
  * @brief  mb tcp get data获取到完整数据
  * @param  **ppucMBTCPFrame 数据起始地址
  * @param  *usTCPLength 数据长度
  * @retval None
  */
static bool mbTCPGet( uint8_t **ppucMBTCPFrame, uint16_t *usTCPLength )
{
#if 0
    struct __network_info *net = (struct __network_info *)get_S2E_Packet_pointer()->network_info;

    uint8_t  peerip[4];
    uint16_t peerport;
#endif

    uint16_t len;
    uint16_t usTCPBufPos;

    len = getSn_RX_RSR(APP_SOCKET_MODBUS);

    if( len > 0 )
    {
        switch(getSn_SR(APP_SOCKET_MODBUS))
        {
#if 0
        case SOCK_UDP:

            usTCPBufPos = recvfrom(SOCK_DATA, aucTCPBuf, len, peerip, &peerport);

            if(memcmp(peerip, net->remote_ip, sizeof(peerip)) || net->remote_port != peerport)
            {
                net->remote_ip[0] = peerip[0];
                net->remote_ip[1] = peerip[1];
                net->remote_ip[2] = peerip[2];
                net->remote_ip[3] = peerip[3];
                net->remote_port = peerport;
            }
            break;
#endif
        case SOCK_ESTABLISHED:
        case SOCK_CLOSE_WAIT:
            usTCPBufPos = recv(APP_SOCKET_MODBUS, aucTCPBuf, len);
            LOGV("mbtcp", "recv return %u(bytes)", usTCPBufPos);
//            hexdump(aucTCPBuf, len);
            break;
        default:
            break;
        }
        *ppucMBTCPFrame = &aucTCPBuf[0];
        *usTCPLength = usTCPBufPos;
        return true;
    }
    return false;
}

/**
  * @brief  mb tcp packet数据包初步解析
  * @param  *pucRcvAddress 解析后的modbus站号
  * @param  **ppucFrame 数据起始地址
  * @param  *pusLength 数据长度
  * @retval None
  */
static bool mbTCPPackage( uint8_t *pucRcvAddress, uint8_t **ppucFrame, uint16_t *pusLength )
{
    uint8_t		*pucMBTCPFrame;
    uint16_t	usLength;
    uint16_t	usPID;

    if( mbTCPGet( &pucMBTCPFrame, &usLength ) != false )
    {
        usPID = pucMBTCPFrame[MB_TCP_PID] << 8U;
        usPID |= pucMBTCPFrame[MB_TCP_PID + 1];

        if( usPID == MB_TCP_PROTOCOL_ID )
        {
            /* Modbus TCP does not use any addresses. Fake the source address such
            * that the processing part deals with this frame.
            */
            *pucRcvAddress = pucMBTCPFrame[MB_TCP_UID];
            mbTCPtid1 = pucMBTCPFrame[MB_TCP_TID1];
            mbTCPtid2 = pucMBTCPFrame[MB_TCP_TID2];

            *ppucFrame = &pucMBTCPFrame[MB_TCP_FUNC];
            *pusLength = usLength - MB_TCP_FUNC;
            return true;
        }
    }
    return false;
}

/**
  * @brief  tcp to rtu frame，出来后的pucRTUBufferCur是标准的modbus协议数据
  * @param  None
  * @retval None
  */
bool MBtcp2rtuFrame(void)
{
    uint8_t pucRcvAddress;
    uint16_t pusLength;
    uint8_t *ppucFrame;
    uint16_t usCRC16;

    if(mbTCPPackage( &pucRcvAddress, &ppucFrame, &pusLength ) != false)
    {
        pucRTUBufferCur = ppucFrame - 1;
        pucRTUBufferCur[0] = ( uint8_t )pucRcvAddress;
        usRTUBufferPos = pusLength + 1;
        usCRC16 = calc_crc16( ( uint8_t * ) pucRTUBufferCur, usRTUBufferPos );
        pucRTUBufferCur[usRTUBufferPos++] = ( uint8_t )( usCRC16 & 0xFF );
        pucRTUBufferCur[usRTUBufferPos++] = ( uint8_t )( usCRC16 >> 8 );
        return true;
    }

    return false;
}

/**
  * @brief  RTU数据解析
  * @param  None
  * @retval None
  */
void mbTCPtoRTU(void)
{
    if(MBtcp2rtuFrame() != false)
    {
#if 0
        while(usRTUBufferPos)
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)pucRTUBufferCur, 1, HAL_MAX_DELAY);
            pucRTUBufferCur++;
            usRTUBufferPos--;
        }
#endif
//        hexdump((void *)pucRTUBufferCur, usRTUBufferPos);
        //此时已经是标准的modbus内容了
        if (is_mb_protocol((uint8_t *)pucRTUBufferCur, usRTUBufferPos))
        {
            md_slave_msg_pack smsg = {0,};

            smsg.mcv_IsBroadcastInfo = pucRTUBufferCur[0] == 0 ? 1 : 0;
            smsg.mcp_ReceiveBuff = (uint8_t *)pucRTUBufferCur;
            smsg.msv_ReceiveLen = usRTUBufferPos;
            smsg.mcv_Sender = MB_SENDER_TCP;
            smsg.tcp_resp_func = mbTcpSend;

            smsg.mcp_RespBuff = (unsigned char *)pvPortMalloc(2048);
            LOGI("mbtcp", "smsg.mcp_RespBuff = 0x%08X, Free Heap Size = %d", (uint32_t)smsg.mcp_RespBuff, xPortGetFreeHeapSize());
            mb_slave_msg_handler(&smsg);
            vPortFree(smsg.mcp_RespBuff);
        }
    }
}

/**
  * @brief  新建线程（任务）
  * @param  None
  * @retval None
  */
void osThreadNew_mbTcpTask(void)
{
    mbTcpTaskHandle = osThreadNew(MbTcpTask, NULL, &mbTcpTask_attributes);
}

/**
  * @brief  Function implementing the uartTask thread.
  * @param  argument: Not used
  * @retval None
  */
void MbTcpTask(void *argument)
{
    LOGD("mbtcp", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

    uint8_t sn = APP_SOCKET_MODBUS;
    uint16_t port = MB_TCP_PORT;
    int32_t ret;

    for(;;)
    {
        osDelay(100);

        if((gParam.st.NetLink_State & NET_STATE_OK) != 0)
        {
            continue ;
        }

        W5500MutexLock();
        switch(getSn_SR(sn))
        {

        case SOCK_ESTABLISHED :
        case SOCK_UDP:
            if(getSn_IR(sn) & Sn_IR_CON)//Sn_IR 位清零的话，主机应该将置‘ 1’
            {
                setSn_IR(sn, Sn_IR_CON);
            }
            if(getSn_RX_RSR(sn)) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
            {
                mbTCPtoRTU();
            }
            break;

        case SOCK_CLOSE_WAIT:
            LOGW("mbtcp", "SOCK_CLOSE_WAIT");
            ret = disconnect(sn);
            if(ret != SOCK_OK)
            {
                LOGE("mbtcp", "ret = %d, %d:Socket Closed fail\r\n", ret, sn);
            }
            LOGW("mbtcp", "%d:Socket Closed\r\n", sn);
            break;

        case SOCK_INIT :
            LOGV("mbtcp", "%d:Listen, TCP modbus server, port [%d]\r\n", sn, port);
            ret = listen(sn);
            if(ret != SOCK_OK)
            {
                LOGE("mbtcp", "ret = %d, %d:Listen fail\r\n", ret, sn);
            }
            break;

        case SOCK_CLOSED:
            LOGD("mbtcp", "%d:TCP server modbus start\r\n", sn);
            if((ret = socket(sn, Sn_MR_TCP, port, 0x00)) != sn)
            {
                LOGE("mbtcp", "ret = %d, %d:TCP server modbus start fail\r\n", ret, sn);
                gMbTcpErrorTimes++;
            }
            else
            {
                if(gMbTcpErrorTimes != 0)
                {
                    gMbTcpErrorTimes = 0;
                }
            }
            LOGI("mbtcp", "%d:Socket opened\r\n", sn);
            break;

        case SOCK_LISTEN:
            //LOGI("mbtcp", "%d:Socket LISTEN", sn);
            //osDelay(1000);  //不能在此堵塞 占用W5500Mutex
            break;

        default:
            break;
        }

        W5500MutexUnlock();
    }
}
