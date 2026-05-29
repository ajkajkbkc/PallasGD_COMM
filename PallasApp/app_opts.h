
#ifndef __APP_OPTS_H
#define __APP_OPTS_H

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
/* --------------------- ① 产品系列 --------------------------- */
#define PROD_XX                       0xFF00
#define PROD_FG                       0x0100
#define PROD_FS                       0x0200
#define PROD_FR                       0x0300
#define PROD_FL                       0x0400
#define PROD_FD                       0x0500
#define PROD_FA                       0x0600
#define PROD_FSS                      0x0700
#define PROD_SFC                      0x0800
#define PROD_SFE                      0x0900
#define PROD_SFB                      0x0A00
#define PROD_SFA                      0x0B00

/* --------------------- ② 产品型号 --------------------------- */
#define PARAM_XX                      0x00FF
#define PARAM_SFA1                    0x0001
#define PARAM_SFA2                    0x0002
#define PARAM_SFA3                    0x0003
#define PARAM_SFA4                    0x0004
#define PARAM_SFA5                    0x0005
#define PARAM_SFA6                    0x0006
#define PARAM_SFB1                    0x0007
#define PARAM_SFB2                    0x0008
#define PARAM_SFB3                    0x0009
#define PARAM_SFB4                    0x000A
#define PARAM_SFB5                    0x000B
#define PARAM_SFB6                    0x000C
#define PARAM_SFE1                    0x000D
#define PARAM_SFE2                    0x000E
#define PARAM_SFE3                    0x000F
#define PARAM_SFE4                    0x0010
#define PARAM_SFE5                    0x0011
#define PARAM_SFE6                    0x0012

/* --------------------- ③ 通讯协议 --------------------------- */
#define PROTOCOL_XX                   0xFFFF
#define PROTOCOL_MB                   0x0000
#define PROTOCOL_MBTCP                0x0001  //modbusTCP协议
#define PROTOCOL_FLKMQTT              0x0002  //FexlinkMQTT协议
#define PROTOCOL_HZUDP                0x0004  //华咨UDP协议
#define PROTOCOL_LLTUDP               0x0008  //路路通UDP协议
#if 0
#define PROTOCOL_DHCP                 0x0010
#define PROTOCOL_DNS                  0x0020
#define PROTOCOL_SNTP                 0x0040
#define PROTOCOL_NETBIOS              0x0080
#define PROTOCOL_HTTPS                0x0100
#endif

/** -------------------------------------------------------------------------------------------------------------
    | PARAM_TYPE (0x00FF) | PROD_TYPE (0xFF00) | 产品名称              | 产品参数          | 通讯协议           |
    -------------------------------------------------------------------------------------------------------------
     @PARAM_COM1 (0x0001) | PROD_FG   (0x0100) | 智能网关              | RS485网关         | ...                |
     @PARAM_COM2 (0x0002) |                    |                       | Zigbee网关        | ...                |
    -------------------------------------------------------------------------------------------------------------
     @PARAM_DES4 (0x0003) | PROD_FS   (0x0200) | 防雷器监测模块        | 四要素            | PROTOCOL_MB        |
     @PARAM_DES9 (0x0004) |                    |                       | 九要素            | PROTOCOL_MBTCP     |
     @PARAM_DESA (0x0005) |                    |                       | 多要素            | PROTOCOL_FLKMQTT   |
    ---------------------------------------------------------------------------------------- PROTOCOL_HZUDP     |
     @PARAM_G1A3 (0x0006) | PROD_FR   (0x0300) | 接地电阻监测模块      | ...               | PROTOCOL_LLTUDP    |
    ----------------------------------------------------------------------------------------                    |
     @PARAM_M1AI (0x0007) | PROD_FL   (0x0400) | 雷电流监测模块        | 1kA~120kA、能量   |                    |
     @PARAM_L2AI (0x0008) |                    | 瞬态电流监测模块      | 0.1kA~1kA         |                    |
    ----------------------------------------------------------------------------------------                    |
     @PARAM_LDA1 (0x0009) | PROD_FD   (0x0500) | 剩余电流监测模块      | ...               |                    |
    ----------------------------------------------------------------------------------------                    |
    | PARAM_LDH1 (0x000A) | PROD_FA   (0x0600) | 故障电弧监测模块      | ...               |                    |
    ----------------------------------------------------------------------------------------                    |
     @PARAM_DEM2 (0x000B) | PROD_FSS  (0x0700) | 智能型电涌保护器      | 无漏流、2P        |                    |
     @PARAM_DEM4 (0x000C) |                    |                       | 无漏流、4P        |                    |
     @PARAM_DEA2 (0x000D) |                    |                       | 全电流、2P        |                    |
     @PARAM_DEA4 (0x000E) |                    |                       | 全电流、4P        |                    |
    -------------------------------------------------------------------------------------------------------------
    |                                                                                                           |
    -------------------------------------------------------------------------------------------------------------
    | PARAM_ESM0 (0x0021) | PROD_ESM  (0x2100) | 防雷监测模组          | 四要素            | PROTOCOL_MB        |
    | PARAM_ESM3 (0x0022) |                    |                       | 多要素            |                    |
    ----------------------------------------------------------------------------------------                    |
    | PARAM_E1E2 (0x0023) | PROD_ESE  (0x2200) | 多回路电力监测模组    | 0~5A、2回路       |                    |
    | PARAM_E2E2 (0x0024) |                    |                       | 0~100A、2回路     |                    |
    | PARAM_E3E2 (0x0025) |                    |                       | 0~400A、2回路     |                    |
    | PARAM_E4E2 (0x0026) |                    |                       | 0~600A、2回路     |                    |
    ----------------------------------------------------------------------------------------                    |
    | PARAM_ESF1 (0x0027) | PROD_ESF  (0x2300) | 电气火灾监测模组      | ...               |                    |
    ----------------------------------------------------------------------------------------                    |
    | PARAM_E2C1 (0x0028) | PROD_ESC  (0x2400) | 漏电监测模组          | 0~10mA、1路       |                    |
    | PARAM_E3C1 (0x0029) |                    |                       | 10~3000mA、1路    |                    |
    | PARAM_E2C3 (0x002A) |                    |                       | 0~10mA、3路       |                    |
    | PARAM_E3C3 (0x002B) |                    |                       | 10~3000mA、3路    |                    |
    ----------------------------------------------------------------------------------------                    |
    | PARAM_E1T2 (0x002C) | PROD_EST  (0x2500) | 温度监测模组          | 有线NTC测温       |                    |
    | PARAM_E2T9 (0x002D) |                    |                       | 无线433测温       |                    |
    ----------------------------------------------------------------------------------------                    |
    | PARAM_E1I2 (0x002E) | PROD_ESI  (0x2600) | 输入监测模组          | ...               |                    |
    -------------------------------------------------------------------------------------------------------------
    注：PROD_TYPE同系列产品，修改该系列产品的PARAM_TYPE，可更改产品功能
        不同系列产品的PARAM_TYPE，不可修改！！！！
        仅带 '@' 可在此项目使用
    */
/* ---------------------- 选择产品 --------------------------- */
/*configTOTAL_HEAP_SIZE  PARAM_SFA: 46500   PARAM_SFB: 42600   PARAM_SFE: 41500 */
#define PARAM_TYPE                    (PARAM_SFA1)
#define PROTOCOL_TYPE                 (PROTOCOL_MB | PROTOCOL_MBTCP | PROTOCOL_FLKMQTT)  //(PROTOCOL_MB | PROTOCOL_MBTCP | PROTOCOL_FLKMQTT | XX |...)
/* ----------------------------------------------------------- */



/* Default feature opts @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

/* IAP opts -------------------------------------------------------------------------------- */
/* If use this feature, In option [target] set IROM1 Start Address 0x8008000, Size 0x19000 */
#ifndef APP_IAP
#define APP_IAP                 1
#endif


/* Print log opts -------------------------------------------------------------------------- */
#ifndef PRINT_LOG_OPEN
#define PRINT_LOG_OPEN          0 // 1：打开print log功能（只有此为1，才能打印log）
#endif

#ifndef UART1_AS_LOG
#define UART1_AS_LOG            0 // 1: 通过UART1打印log, PA9,PA10
#endif

#ifndef UART2_AS_LOG
#define UART2_AS_LOG            0 // 1: 通过UART2打印log, PA2,PA3
#endif

#ifndef UART3_AS_LOG
#define UART3_AS_LOG            0 // 1: 通过UART3打印log
#endif

/* Production opts -------------------------------------------------------------------------- */
#if   PARAM_TYPE == PARAM_COM1 || PARAM_TYPE == PARAM_COM2
#define PROD_TYPE                     (PROD_FG)
#if   PROTOCOL_TYPE != (PROTOCOL_MB | PROTOCOL_MBTCP | PROTOCOL_FLKMQTT)
#error "Please select correct PROTOCOL_TYPE"
#endif
#elif PARAM_TYPE == PARAM_DES4 || PARAM_TYPE == PARAM_DES9 || PARAM_TYPE == PARAM_DESA
#define PROD_TYPE                     (PROD_FS)
#elif PARAM_TYPE == PARAM_G1A3
#define PROD_TYPE                     (PROD_FR)
#elif PARAM_TYPE == PARAM_M1AI || PARAM_TYPE == PARAM_L2AI
#define PROD_TYPE                     (PROD_FL)
#elif PARAM_TYPE == PARAM_LDH1
#define PROD_TYPE                     (PROD_FA)
#elif PARAM_TYPE == PARAM_LDA1
#define PROD_TYPE                     (PROD_FD)
#elif PARAM_TYPE == PARAM_DEM2 || PARAM_TYPE == PARAM_DEM4 || PARAM_TYPE == PARAM_DEA2 || PARAM_TYPE == PARAM_DEA4
#define PROD_TYPE                     (PROD_FSS)
#elif PARAM_TYPE == PARAM_SFE1 || PARAM_TYPE == PARAM_SFE2 || PARAM_TYPE == PARAM_SFE3 || PARAM_TYPE == PARAM_SFE4 || PARAM_TYPE == PARAM_SFE5 || PARAM_TYPE == PARAM_SFE6
#define PROD_TYPE                     (PROD_SFE)
#elif PARAM_TYPE == PARAM_SFB1 || PARAM_TYPE == PARAM_SFB2 || PARAM_TYPE == PARAM_SFB3 || PARAM_TYPE == PARAM_SFB4 || PARAM_TYPE == PARAM_SFB5 || PARAM_TYPE == PARAM_SFB6
#define PROD_TYPE                     (PROD_SFB)
#elif PARAM_TYPE == PARAM_SFA1 || PARAM_TYPE == PARAM_SFA2 || PARAM_TYPE == PARAM_SFA3 || PARAM_TYPE == PARAM_SFA4 || PARAM_TYPE == PARAM_SFA5 || PARAM_TYPE == PARAM_SFA6
#define PROD_TYPE                     (PROD_SFA)
#else
#error "Please select correct PARAM_TYPE"
#endif





#endif
