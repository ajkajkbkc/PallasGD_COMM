
/* Private includes ----------------------------------------------------------*/
#include "mb_maptable.h"

#include "app_log.h"
#include "app_parameter.h"
#include "FreeRTOS.h"
/* Private define ------------------------------------------------------------*/


static const mb_element_map_table_info stv_ModbusMapTable[] =
{
    /*AddrType          ElementType              Broadcast        StartAddr   EndAddr   StartElement  MaxNum*/

    /* 自定义类型 compatible old protocol ------------------------------------- */
    {MB_WORD_ELEMENT,   MB_WORD_IDINFO,          MB_BROADCAST,    0x0001,     0x0001,   0,            11      },
    {MB_WORD_ELEMENT,   MB_WORD_MQTTUSERNAME,    MB_BROADCAST,    0x0002,     0x0002,   0,            10      },
    {MB_WORD_ELEMENT,   MB_WORD_MQTTPASSWORD,    MB_BROADCAST,    0x0003,     0x0003,   0,            10      },
    {MB_WORD_ELEMENT,   MB_WORD_MQTTPUB,         MB_BROADCAST,    0x0004,     0x0004,   0,            50      },
    {MB_WORD_ELEMENT,   MB_WORD_MQTTSUB,         MB_BROADCAST,    0x0005,     0x0005,   0,            50      },
    {MB_WORD_ELEMENT,   MB_WORD_MQTTALARMPUB,    MB_BROADCAST,    0x0006,     0x0006,   0,            50      },
    {MB_WORD_ELEMENT,   MB_WORD_GATEWAYIP,       MB_BROADCAST,    0x0010,     0x0010,   0,            2       },
    {MB_WORD_ELEMENT,   MB_WORD_LOCALIP,         MB_BROADCAST,    0x0011,     0x0011,   0,            2       },
    {MB_WORD_ELEMENT,   MB_WORD_S0TARGETIP,      MB_BROADCAST,    0x0013,     0x0013,   0,            2       },
    {MB_WORD_ELEMENT,   MB_WORD_S0LOCALPORT,     MB_BROADCAST,    0x0014,     0x0014,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_S0TARGETPORT,    MB_BROADCAST,    0x0015,     0x0015,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_S1TARGETIP,      MB_BROADCAST,    0x0016,     0x0016,   0,            2       },
    {MB_WORD_ELEMENT,   MB_WORD_S1LOCALPORT,     MB_BROADCAST,    0x0017,     0x0017,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_S1TARGETPORT,    MB_BROADCAST,    0x0018,     0x0018,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_NODECHECKINTER,  MB_BROADCAST,    0x0019,     0x0019,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_MQTTPUBINTER,    MB_BROADCAST,    0x001A,     0x001A,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_MACADDR,         MB_BROADCAST,    0x001B,     0x001B,   0,            3       },
    {MB_WORD_ELEMENT,   MB_WORD_MASKIP,          MB_BROADCAST,    0x001C,     0x001C,   0,            2       },
    {MB_WORD_ELEMENT,   MB_WORD_RTCTIME,         MB_BROADCAST,    0x0081,     0x0083,   0,            3       },
    {MB_WORD_ELEMENT,   MB_WORD_DESXLJCLEAR,     MB_BROADCAST,    0x0100,     0x0100,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_IDNUMBER,        MB_BROADCAST,    0x0101,     0x0101,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_L1AICLEAR,       MB_BROADCAST,    0x0103,     0x0103,   0,            1       },
    {MB_WORD_ELEMENT,   MB_WORD_G1A3INTERVAL,    MB_NOBROADCAST,  0x0104,     0x0104,   0,            2       },
    {MB_WORD_ELEMENT,   MB_WORD_G1A3GETNUM,      MB_NOBROADCAST,  0x0105,     0x0105,   0,            2       },
    {MB_WORD_ELEMENT,   MB_WORD_VALUE,           MB_NOBROADCAST,  0x0106,     0x0181,   0,            124      },
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB
    {MB_WORD_ELEMENT,   MB_WORD_POWER_QUALITY,   MB_NOBROADCAST,  0x0200,     0x0245,   0,            70      },  /* 0x0200~0x0245 = 70个地址，与MaxNum=70一致 */
    {MB_WORD_ELEMENT,   MB_WORD_HARMONIC,        MB_NOBROADCAST,  0x0246,     0x02FF,   0,            186     },
#endif
    
    /* new -------------------------------------------------------------------- */
    {MB_WORD_ELEMENT,   MB_WORD_SD,              MB_BROADCAST,    0x0401,     0x0464,   0,            50 * 2 },
    {MB_WORD_ELEMENT,   MB_HALF_WORD_SD,         MB_BROADCAST,    0x0465,     0x052C,   100,          200     },
    {MB_WORD_ELEMENT,   MB_BYTE_SD,              MB_BROADCAST,    0x052D,     0x0600,   300,          212     },

    {MB_WORD_ELEMENT,   MB_WORD_PARAM,           MB_NOBROADCAST,  0x0801,     0x081E,   0,            15 * 2  },
    {MB_WORD_ELEMENT,   MB_HALF_WORD_PARAM,      MB_NOBROADCAST,  0x081F,     0x0904,   30,           230     },
    {MB_WORD_ELEMENT,   MB_BYTE_PARAM,           MB_NOBROADCAST,  0x0905,     0x092C,   60,           40      },

//    {MB_WORD_ELEMENT,   MB_HALF_WORD_LTLIST,     MB_NOBROADCAST,  0x092D,     0x0D2C,   0,            LT_LSIT_SIZE},

#if  PROD_TYPE == PROD_FSS
    {MB_WORD_ELEMENT,   MB_WORD_METER,           MB_BROADCAST,    0x1000,     0x10FF,   0,            256     },
    {MB_WORD_ELEMENT,   MB_WORD_CALICMD,         MB_BROADCAST,    0x1100,     0x1100,   0,            1       },
#elif PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
    {MB_WORD_ELEMENT,   MB_WORD_METER,           MB_BROADCAST,    0x1000,     0x10FF,   0,            255     },
    {MB_WORD_ELEMENT,   MB_WORD_CALICMD,         MB_BROADCAST,    0x1100,     0x1100,   0,            1       },
#endif

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB
    {MB_WORD_ELEMENT,   MB_WORD_SYNC_SAMPLED,    MB_BROADCAST,    0x2000,     0x23FD,   0,            7 * 146 },  //1022 * 16bit
#endif
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
    {MB_WORD_ELEMENT,   MB_HALF_WORD_TOU,        MB_BROADCAST,    0x3000,     0x31F3,   0,            500     },  /* TOU分时计费配置与电能 128个half-word */
#endif    
    
    {MB_ELEMENT_MAX,    0,                       0,               0,          0,        0,            0       }
};


/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/
/**
  * @brief  Modbus 协议地址到物理元件地址转换
  * @param  lcv_AddrType 元件寻址方式
  * @param  lsv_MbAddr modbus协议地址
  * @param  lsv_ElementCnt 读取或设置元件长度
  * @param  lcp_ElementType 元件类型
  * @param  lsp_ElementAddr 开始元件编号
  * @param  lcp_CanBroadcast 是否能用广播模式通信
  * @retval None
  */
bool mb_slave_convert_element_info(unsigned char lcv_AddrType, unsigned short lsv_MbAddr, unsigned short lsv_ElementCnt, \
                                   unsigned char *lcp_ElementType, unsigned short *lsp_ElementAddr, unsigned char *lcp_CanBroadcast)
{
    unsigned char i = 0;

    while(stv_ModbusMapTable[i].mcv_AddrType != MB_ELEMENT_MAX)
    {
        if(stv_ModbusMapTable[i].mcv_AddrType != lcv_AddrType) //find lcv_AddrType
        {
            i++;
            //LOGE("mb_map","stv_ModbusMapTable[i].mcv_AddrType != lcv_AddrType");
            continue;
        }

        if((lsv_MbAddr >= stv_ModbusMapTable[i].msv_StartAddr) &&
                (lsv_MbAddr <= stv_ModbusMapTable[i].msv_EndAddr))  //find modbus address
        {
            *lcp_ElementType = stv_ModbusMapTable[i].mcv_ElementType;
            *lsp_ElementAddr = lsv_MbAddr - stv_ModbusMapTable[i].msv_StartAddr + stv_ModbusMapTable[i].msv_StartElement;
            *lcp_CanBroadcast = stv_ModbusMapTable[i].mcv_Broadcast;

            if(*lsp_ElementAddr >= stv_ModbusMapTable[i].msv_StartElement + stv_ModbusMapTable[i].msv_MaxNum)
            {
                LOGE("mb_map", "mb addr illegal, addr = 0x%04X, allow max addr = 0x%04X", lsv_MbAddr, stv_ModbusMapTable[i].msv_StartAddr + stv_ModbusMapTable[i].msv_MaxNum - 1);
                return false;
            }
            if(lsv_ElementCnt > stv_ModbusMapTable[i].msv_StartAddr + stv_ModbusMapTable[i].msv_MaxNum - lsv_MbAddr)
            {
                LOGE("mb_map", "read or write len illegal, len = %d, allow max len = %d", lsv_ElementCnt, stv_ModbusMapTable[i].msv_StartAddr + stv_ModbusMapTable[i].msv_MaxNum - lsv_MbAddr);
                return false;
            }

            return true;
        }
        i++;
    }
    LOGE("mb_map", "mb addr 0x%04X search fail !!!!", lsv_MbAddr);
    return false;
}






