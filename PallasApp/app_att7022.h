
//#include "app_opts.h"

//#if PROD_TYPE != PROD_FSS 
//#ifndef __APP_ATT7022_H
//#define __APP_ATT7022_H

///* Includes ------------------------------------------------------------------*/



///* Private defines -----------------------------------------------------------*/
//#define ATT7022_READ_REG    0x00
//#define ATT7022_WRITE_REG   0x80

///* 计量参数寄存器 */
//#define ATT7022_DEV_ID      0x00

//#define ATT7022_PA          0x01
//#define ATT7022_PB          0x02
//#define ATT7022_PC          0x03
//#define ATT7022_PT          0x04

//#define ATT7022_QA          0x05
//#define ATT7022_QB          0x06
//#define ATT7022_QC          0x07
//#define ATT7022_QT          0x08

//#define ATT7022_SA          0x09
//#define ATT7022_SB          0x0A
//#define ATT7022_SC          0x0B
//#define ATT7022_ST          0x0C

//#define ATT7022_UARMS       0x0D
//#define ATT7022_UBRMS       0x0E
//#define ATT7022_UCRMS       0x0F
//#define ATT7022_IARMS       0x10
//#define ATT7022_IBRMS       0x11
//#define ATT7022_ICRMS       0x12
//#define ATT7022_ITRMS       0x13

//#define ATT7022_PFA         0x14
//#define ATT7022_PFB         0x15
//#define ATT7022_PFC         0x16
//#define ATT7022_PFT         0x17

//#define ATT7022_PGA         0x18
//#define ATT7022_PGB         0x19
//#define ATT7022_PGC         0x1A

//#define ATT7022_INTFLAG     0x1B
//#define ATT7022_FREQ        0x1C
//#define ATT7022_EFLAG       0x1D

//#define ATT7022_EPA         0x1E
//#define ATT7022_EPB         0x1F
//#define ATT7022_EPC         0x20
//#define ATT7022_EPT         0x21

//#define ATT7022_EQA         0x22
//#define ATT7022_EQB         0x23
//#define ATT7022_EQC         0x24
//#define ATT7022_EQT         0x25

//#define ATT7022_YUAUB       0x26
//#define ATT7022_YUAUC       0x27
//#define ATT7022_YUBUC       0x28

//#define ATT7022_I0RMS       0x29
//#define ATT7022_TPSD0       0x2A
//#define ATT7022_UTRMS       0x2B

//#define ATT7022_SFLAG       0x2C
//#define ATT7022_BCKREG      0x2D
//#define ATT7022_CMDCHKSUM   0x2E

//#define ATT7022_SAMPLE_IA   0x2F
//#define ATT7022_SAMPLE_IB   0x30
//#define ATT7022_SAMPLE_IC   0x31
//#define ATT7022_SAMPLE_UA   0x32
//#define ATT7022_SAMPLE_UB   0x33
//#define ATT7022_SAMPLE_UC   0x34

//#define ATT7022_ESA         0x35
//#define ATT7022_ESB         0x36
//#define ATT7022_ESC         0x37
//#define ATT7022_EST         0x38

//#define ATT7022_FSTCNTA     0x39
//#define ATT7022_FSTCNTB     0x3A
//#define ATT7022_FSTCNTC     0x3B
//#define ATT7022_FSTCNTT     0x3C

//#define ATT7022_PFLAG       0x3D
//#define ATT7022_CHKSUM      0x3E
//#define ATT7022_SAMPLE_I0   0x3F

//#define ATT7022_LINEPA      0x40
//#define ATT7022_LINEPB      0x41
//#define ATT7022_LINEPC      0x42
//#define ATT7022_LINEPT      0x43

//#define ATT7022_LINEEPA     0x44
//#define ATT7022_LINEEPB     0x45
//#define ATT7022_LINEEPC     0x46
//#define ATT7022_LINEEPT     0x47

//#define ATT7022_LINEUARRMS  0x48
//#define ATT7022_LINEUBRRMS  0x49
//#define ATT7022_LINEUCRRMS  0x4A
//#define ATT7022_LINEIARRMS  0x4B
//#define ATT7022_LINEIBRRMS  0x4C
//#define ATT7022_LINEICRRMS  0x4D
//#define ATT7022_LEFLAG      0x4E
//#define ATT7022_PTRWAVEBUFF 0x7E
//#define ATT7022_WAVEBUFF    0x7F

///* 特殊命令 */
//#define SAMPLE_BUFF       0xC0 //采样数据缓冲启动
//#define BUFF_READ_P       0xC1 //缓冲数据读指针设置
//#define CLEAR_TABLE       0xC3 //清校表数据
//#define SYNC_FACTOR       0xC4 //同步数据系数设置
//#define SYNC_START        0xC5 //同步数据启动
//#define CHECK_DATA_WRITE  0xC6 //校表数据读出
//#define CHECK_DATA_READ   0xC9 //校表数据写使能
//#define ATT7022_RESET     0xD3 //软件复位

///* 校表参数寄存器 */
//#define ATT7022_RESERVED01     0x00 //校表参数寄存器起始地址，reset：0xAAAA

//#define ATT7022_MODECFG        0x01 //模式相关控制，reset：0x89AA
//#define ATT7022_PGACTRL        0x02 //ADC增益配置
//#define ATT7022_EMUCFG         0x03 //EMU单元配置，reset：0x0804

//#define ATT7022_PGAINA         0x04 //A相有功功率增益
//#define ATT7022_PGAINB         0x05 //B相有功功率增益
//#define ATT7022_PGAINC         0x06 //C相有功功率增益

//#define ATT7022_QGAINA         0x07 //A相无功功率增益
//#define ATT7022_QGAINB         0x08 //B相无功功率增益
//#define ATT7022_QGAINC         0x09 //C相无功功率增益

//#define ATT7022_SGAINA         0x0A //A相视在功率增益
//#define ATT7022_SGAINB         0x0B //B相视在功率增益
//#define ATT7022_SGAINC         0x0C //C相视在功率增益

//#define ATT7022_PH_SREG_APQ0   0x0D //A相相位校正0
//#define ATT7022_PH_SREG_BPQ0   0x0E //B相相位校正0
//#define ATT7022_PH_SREG_CPQ0   0x0F //C相相位校正0
//#define ATT7022_PH_SREG_APQ1   0x10 //A相相位校正1
//#define ATT7022_PH_SREG_BPQ1   0x11 //B相相位校正1
//#define ATT7022_PH_SREG_CPQ1   0x12 //C相相位校正1

//#define ATT7022_POFFSETA       0x13 //A相有功功率offset校正
//#define ATT7022_POFFSETB       0x14 //B相有功功率offset校正
//#define ATT7022_POFFSETC       0x15 //C相有功功率offset校正

//#define ATT7022_QPHSCAL        0x16 //无功相位校正

//#define ATT7022_UGAINA         0x17 //A相电压增益
//#define ATT7022_UGAINB         0x18 //B相电压增益
//#define ATT7022_UGAINC         0x19 //C相电压增益

//#define ATT7022_IGAINA         0x1A //A相电流增益
//#define ATT7022_IGAINB         0x1B //B相电流增益
//#define ATT7022_IGAINC         0x1C //C相电流增益

//#define ATT7022_ISTARUP        0x1D //起动电流阈值设置

//#define ATT7022_HFCONST        0x1E //高频脉冲输出设置

//#define ATT7022_FAIL_VOLTAGE   0x1F //失压阈值设置（reset：0x0600，三相四线；0x1200，三相三线）

//#define ATT7022_GAIN_ADC7      0x20 //第七路ADC输入信号增益

//#define ATT7022_QOFFSETA       0x21 //A相无功功率offset校正
//#define ATT7022_QOFFSETB       0x22 //B相无功功率offset校正
//#define ATT7022_QOFFSETC       0x23 //C相无功功率offset校正

//#define ATT7022_UARMSOFFSET    0x24 //A相电压有效值offset校正
//#define ATT7022_UBRMSOFFSET    0x25 //B相电压有效值offset校正
//#define ATT7022_UCRMSOFFSET    0x26 //C相电压有效值offset校正

//#define ATT7022_IARMSOFFSET    0x27 //A相电流有效值offset校正
//#define ATT7022_IBRMSOFFSET    0x28 //B相电流有效值offset校正
//#define ATT7022_ICRMSOFFSET    0x29 //C相电流有效值offset校正

//#define ATT7022_UOFFSETA       0x2A //A相电压通道ADC offset校正
//#define ATT7022_UOFFSETB       0x2B //B相电压通道ADC offset校正
//#define ATT7022_UOFFSETC       0x2C //C相电压通道ADC offset校正

//#define ATT7022_IOFFSETA       0x2D //A相电流通道ADC offset校正
//#define ATT7022_IOFFSETB       0x2E //B相电流通道ADC offset校正
//#define ATT7022_IOFFSETC       0x2F //C相电流通道ADC offset校正

//#define ATT7022_EMUIE          0x30 //中断使能

//#define ATT7022_MODULE_CFG     0x31 //电路模块配置寄存器，reset：0x4527

//#define ATT7022_ALL_GAIN       0x32 //全通道增益，用于Vref的温度校正

//#define ATT7022_HF_DOUBLE      0x33 //脉冲常数加倍选择

//#define ATT7022_LINE_GAIN      0x34 //基波增益校正，reset：0x2C59

//#define ATT7022_PIN_CTRL       0x35 //数字pin上下拉电阻选择控制，reset：0x000F

//#define ATT7022_PSTART         0x36 //起动功率设置寄存器，reset：0x0030

//#define ATT7022_IREGION        0x37 //相位补偿区域设置寄存器(reset: 0x7FFF)
//#define ATT7022_RESERVED38     0x38 //reserved(reset: 0x1000)
//#define ATT7022_RESERVED39     0x39 //reserved(reset: 0x4500)


///* Exported types ------------------------------------------------------------*/




///* Exported constants --------------------------------------------------------*/



///* Private functions ---------------------------------------------------------*/
//void osThreadNew_att7022Task(void);


//#endif /* __APP_ATT7022_H */
//#endif
