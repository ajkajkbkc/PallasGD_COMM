

#include "cmsis_os.h"
#include "app_opts.h"
#include "stdint.h"

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA

#ifndef __APP_ATT7022EU_H
#define __APP_ATT7022EU_H

/* Private defines -----------------------------------------------------------*/
#define ATT7022_READ_REG       0x00
#define ATT7022_WRITE_REG      0x80
//#define ESE_ELEMENT_SIZE         54

//计量参数寄存器定义------------------------------------------------------------
#define	r_DeviceID     0x00         //Device ID
#define	r_Pa           0x01         //A相有功功率
#define	r_Pb           0x02         //B相有功功率
#define	r_Pc           0x03         //C相有功功率
#define	r_Pt           0x04         //合相有功功率
#define	r_Qa           0x05         //A相无功功率
#define	r_Qb           0x06         //B相无功功率
#define	r_Qc           0x07         //C相无功功率
#define	r_Qt           0x08         //合相无功功率
#define	r_Sa           0x09         //A相视在功率
#define	r_Sb           0x0A         //B相视在功率
#define	r_Sc           0x0B         //C相视在功率
#define	r_St           0x0C         //合相视在功率
#define	r_UaRms        0x0D         //A相电压有效值
#define	r_UbRms        0x0E         //B相电压有效值
#define	r_UcRms        0x0F         //C相电压有效值
#define	r_IaRms        0x10         //A相电流有效值
#define	r_IbRms        0x11         //B相电流有效值
#define	r_IcRms        0x12         //C相电流有效值
#define	r_ItRms        0x13         //合相电流有效值(矢量和)
#define	r_Pfa          0x14         //A相功率因素
#define	r_Pfb          0x15         //B相功率因素
#define	r_Pfc          0x16         //C相功率因素
#define	r_Pft          0x17         //合相功率因素
#define	r_Pga          0x18         //A相电流与电压夹角
#define	r_Pgb          0x19         //B相电流与电压夹角
#define	r_Pgc          0x1A         //C相电流与电压夹角
#define	r_IFlag        0x1B         //中断标志（读后清零）**
#define	r_Freq         0x1C         //线频率
#define	r_EFlag        0x1D         //电能寄存器状态（读后清零） *
#define	r_Epa          0x1E         //A相有功电能（可配置读后清零）
#define	r_Epb          0x1F         //B相有功电能（可配置读后清零）
#define	r_Epc          0x20         //C相有功电能（可配置读后清零）
#define	r_Ept          0x21         //合相有功电能（可配置读后清零）
#define	r_Eqa          0x22         //A相无功电能（可配置读后清零）
#define	r_Eqb          0x23         //B相无功电能（可配置读后清零）
#define	r_Eqc          0x24         //C相无功电能（可配置读后清零）
#define	r_Eqt          0x25         //合相无功电能（可配置读后清零）
#define	r_YUaUb        0x26         //Ua与Ub的电压夹角
#define	r_YUaUc        0x27         //Ua与Uc的电压夹角
#define	r_YUbUc        0x28         //Ub与Uc的电压夹角
#define	r_InRms        0x29         //零线电流有效值
#define	r_TPSD         0x2A         //温度传感器的输出
#define	r_UtRms        0x2B         //合相电压有效值(矢量和)
#define	r_SFlag        0x2C         //断相,相序,SIG等标志状态 **
#define	r_ComReg       0x2D         //通讯数据备份寄存器 *
#define	r_ComSum       0x2E         //通讯校验和寄存器 *
#define	r_SampleIA     0x2F         //A相电流通道ADC采样数据
#define	r_SampleIB     0x30         //B相电流通道ADC采样数据
#define	r_SampleIC     0x31         //C相电流通道ADC采样数据
#define	r_SampleUA     0x32         //A相电压通道ADC采样数据
#define	r_SampleUB     0x33         //B相电压通道ADC采样数据
#define	r_SampleUC     0x34         //C相电压通道ADC采样数据
#define	r_Esa          0x35         //A相视在电能（可配置读后清零）
#define	r_Esb          0x36         //B相视在电能（可配置读后清零）
#define	r_Esc          0x37         //C相视在电能（可配置读后清零）
#define	r_Est          0x38         //合相视在电能（可配置读后清零）
#define	r_FstCntA      0x39         //A相快速脉冲计数
#define	r_FstCntB      0x3A         //B相快速脉冲计数
#define	r_FstCntC      0x3B         //C相快速脉冲计数
#define	r_FstCntT      0x3C         //合相快速脉冲计数
#define	r_PFlag        0x3D         //功率方向（0正1负） **
#define	r_ChkSum0      0x3E         //校表数据校验和（0x01~0x39）（三线0x01E0CD四线0x01D4CD）*
#define	r_InstADC7     0x3F         //第7路ADC采样数据输出

#define	r_LinePa       0x40         //A相基波有功功率
#define	r_LinePb       0x41         //B相基波有功功率
#define	r_LinePc       0x42         //C相基波有功功率
#define	r_LinePt       0x43         //合相基波有功功率
#define	r_LineEPa      0x44         //A相基波有功电能（可配置读后清零）
#define	r_LineEPb      0x45         //B相基波有功电能（可配置读后清零）
#define	r_LineEPc      0x46         //C相基波有功电能（可配置读后清零）
#define	r_LineEPt      0x47         //合相基波有功电能（可配置读后清零）
#define	r_LineUaRms    0x48         //A相基波电压有效值
#define	r_LineUbRms    0x49         //B相基波电压有效值
#define	r_LineUcRms    0x4A         //C相基波电压有效值
#define	r_LineIaRms    0x4B         //A相基波电流有效值
#define	r_LineIbRms    0x4C         //B相基波电流有效值
#define	r_LineIcRms    0x4D         //C相基波电流有效值
#define	r_LEFlag       0x4E         //基波电能寄存器工作状态（读后清零） *
#define	r_SAGFlag      0x4F         //SAG标志寄存器  **
#define	r_PeakUa       0x50         //A相电压最大值
#define	r_PeakUb       0x51         //B相电压最大值
#define	r_PeakUc       0x52         //C相电压最大值
#define	r_LineQa       0x57         //A相基波无功功率
#define	r_LineQb       0x58         //B相基波无功功率
#define	r_LineQc       0x59         //C相基波无功功率
#define	r_LineQt       0x5A         //合相基波无功功率
#define	r_Vrefgain     0x5C         //Vref自动补偿系数 *
#define	r_ChipID       0x5D         //芯片版本指示 *
#define	r_ChkSum1      0x5E         //校表数据校验和（0x60~0x71） *
#define	r_PtrWaveBuff  0x7E         //缓冲数据指针
#define	r_WaveBuff     0x7F         //缓冲数据寄存器


//校表参数寄存器定义------------------------------------------------------------
#define	w_StartSig     0x00         //校表参数寄存器起始标志
#define	w_ModeCfg      0x01         //模式相关控制器
#define	w_PGACtrl      0x02         //ADC增益选择
#define	w_EMUCfg       0x03         //EMU模块配置寄存器
#define	w_PgainA       0x04         //A相有功功率增益
#define	w_PgainB       0x05         //B相有功功率增益
#define	w_PgainC       0x06         //C相有功功率增益
#define	w_QgainA       0x07         //A相无功功率增益
#define	w_QgainB       0x08         //B相无功功率增益
#define	w_QgainC       0x09         //C相无功功率增益
#define	w_SgainA       0x0A         //A相视在功率增益
#define	w_SgainB       0x0B         //B相视在功率增益
#define	w_SgainC       0x0C         //C相视在功率增益
#define	w_PhSregApq0   0x0D         //A相相位校正0
#define	w_PhSregBpq0   0x0E         //B相相位校正0
#define	w_PhSregCpq0   0x0F         //C相相位校正0
#define	w_PhSregApq1   0x10         //A相相位校正1
#define	w_PhSregBpq1   0x11         //B相相位校正1
#define	w_PhSregCpq1   0x12         //C相相位校正1
#define	w_PoffsetA     0x13         //A相有功功率offset校正
#define	w_PoffsetB     0x14         //B相有功功率offset校正
#define	w_PoffsetC     0x15         //C相有功功率offset校正
#define	w_QPhscal      0x16         //无功相位校正
#define	w_UgainA       0x17         //A相电压增益
#define	w_UgainB       0x18         //B相电压增益
#define	w_UgainC       0x19         //C相电压增益
#define	w_IgainA       0x1A         //A相电流增益
#define	w_IgainB       0x1B         //B相电流增益
#define	w_IgainC       0x1C         //C相电流增益
#define	w_Istartup     0x1D         //启动电流阈值设置
#define	w_HFConst      0x1E         //高频脉冲输出设置
#define	w_FailVolt     0x1F         //失压阈值设置
#define	w_GainADC7     0x20         //第七路ADC输入信号增益
#define	w_QoffsetA     0x21         //A相无功功率offset校正
#define	w_QoffsetB     0x22         //B相无功功率offset校正
#define	w_QoffsetC     0x23         //C相无功功率offset校正
#define	w_URmsoffsetA  0x24         //A相电压有效值offset校正
#define	w_URmsoffsetB  0x25         //B相电压有效值offset校正
#define	w_URmsoffsetC  0x26         //C相电压有效值offset校正
#define	w_IRmsoffsetA  0x27         //A相电流有效值offset校正
#define	w_IRmsoffsetB  0x28         //B相电流有效值offset校正
#define	w_IRmsoffsetC  0x29         //C相电流有效值offset校正
#define	w_UoffsetA     0x2A         //A相电压通道ADC_offset校正
#define	w_UoffsetB     0x2B         //B相电压通道ADC_offset校正
#define	w_UoffsetC     0x2C         //C相电压通道ADC_offset校正
#define	w_IoffsetA     0x2D         //A相电流通道ADC_offset校正
#define	w_IoffsetB     0x2E         //B相电流通道ADC_offset校正
#define	w_IoffsetC     0x2F         //C相电流通道ADC_offset校正
#define	w_EMUIE        0x30         //中断使能
#define	w_ModuleCfg    0x31         //电路模块配置寄存器
#define	w_AllGain      0x32         //全通道增益
#define	w_HFDouble     0x33         //脉冲常数加倍选择
#define	w_LineGain     0x34         //基波增益校正
#define	w_PinCtrl      0x35         //数字pin上下拉电阻选择控制
#define	w_Pstartup     0x36         //启动功率阈值设置
#define	w_Iregion0     0x37         //相位补偿区域设置寄存器0
#define	w_Cyclength    0x38         //SAG数据长度设置寄存器
#define	w_SAGLevel     0x39         //SAG检测阈值设置寄存器

#define	w_Iregion1     0x60         //相位补偿区域设置寄存器1
#define	w_PhSregApq2   0x61         //A相相位校正2
#define	w_PhSregBpq2   0x62         //B相相位校正2
#define	w_PhSregCpq2   0x63         //C相相位校正2
#define	w_PoffsetAL    0x64         //A相有功功率offset校正低字节
#define	w_PoffsetBL    0x65         //B相有功功率offset校正低字节
#define	w_PoffsetCL    0x66         //C相有功功率offset校正低字节
#define	w_QoffsetAL    0x67         //A相无功功率offset校正低字节
#define	w_QoffsetBL    0x68         //B相无功功率offset校正低字节
#define	w_QoffsetCL    0x69         //C相无功功率offset校正低字节
#define	w_IRmsoffsetT  0x6A         //电流矢量和offset校正寄存器
#define	w_TPSoffset    0x6B         //TPS初值校正寄存器
#define	w_TPSgain      0x6C         //TPS斜率校正寄存器
#define	w_TCoffA       0x6D         //Vrefgain的二次系数
#define	w_TCoffB       0x6E         //Vrefgain的一次系数
#define	w_TCoffC       0x6F         //Vrefgain的常数项
#define	w_EMCfg        0x70         //新增算法控制寄存器
#define	w_OILevel      0x71         //过流阈值设置寄存器


//特殊命令定义------------------------------------------------------------------
#define	w_BuffStr      0xC0         //采样数据缓冲启动命令
#define	w_BuffSet      0xC1         //缓冲数据读指针设置
#define	w_ClrCali      0xC3         //清校表数据
#define	w_SyncSet      0xC4         //同步数据系数设置
#define	w_SyncStr      0xC5         //同步数据启动命令
#define	w_EnRdCali     0xC6         //校表数据读写使能
#define	w_EnWrCali     0xC9         //校表数据写使能
#define	w_SoftRst      0xD3         //软复位


typedef enum
{
    offset_StartSig = 0,
    offset_ModeCfg = 1,             //模式相关控制器
    offset_PGACtrl = 2,             //ADC增益选择
    offset_EMUCfg,                  //EMU模块配置寄存器
    offset_PgainA,                  //A相有功功率增益
    offset_PgainB,                  //B相有功功率增益
    offset_PgainC,                  //C相有功功率增益
    offset_QgainA,                  //A相无功功率增益
    offset_QgainB,                  //B相无功功率增益
    offset_QgainC,                  //C相无功功率增益
    offset_SgainA,                  //A相视在功率增益
    offset_SgainB,                  //B相视在功率增益
    offset_SgainC,                  //C相视在功率增益
    offset_PhSregApq0,              //A相相位校正0
    offset_PhSregBpq0,              //B相相位校正0
    offset_PhSregCpq0,              //C相相位校正0
    offset_PhSregApq1,              //A相相位校正1
    offset_PhSregBpq1,              //B相相位校正1
    offset_PhSregCpq1,              //C相相位校正1
    offset_PoffsetA,                //A相有功功率offset校正
    offset_PoffsetB,                //B相有功功率offset校正
    offset_PoffsetC,                //C相有功功率offset校正
    offset_QPhscal,                 //无功相位校正
    offset_UgainA,                  //A相电压增益
    offset_UgainB,                  //B相电压增益
    offset_UgainC,                  //C相电压增益
    offset_IgainA,                  //A相电流增益
    offset_IgainB,                  //B相电流增益
    offset_IgainC,                  //C相电流增益
    offset_Istartup,                //启动电流阈值设置
    offset_HFConst,                 //高频脉冲输出设置
    offset_FailVolt,                //失压阈值设置
    offset_GainADC7,                //第七路ADC输入信号增益
    offset_QoffsetA,                //A相无功功率offset校正
    offset_QoffsetB,                //B相无功功率offset校正
    offset_QoffsetC,                //C相无功功率offset校正
    offset_URmsoffsetA,             //A相电压有效值offset校正
    offset_URmsoffsetB,             //B相电压有效值offset校正
    offset_URmsoffsetC,             //C相电压有效值offset校正
    offset_IRmsoffsetA,             //A相电流有效值offset校正
    offset_IRmsoffsetB,             //B相电流有效值offset校正
    offset_IRmsoffsetC,             //C相电流有效值offset校正
    offset_UoffsetA,                //A相电压通道ADC offset校正
    offset_UoffsetB,                //B相电压通道ADC offset校正
    offset_UoffsetC,                //C相电压通道ADC offset校正
    offset_IoffsetA,                //A相电流通道ADC offset校正
    offset_IoffsetB,                //B相电流通道ADC offset校正
    offset_IoffsetC,                //C相电流通道ADC offset校正
    offset_EMUIE,                   //中断使能
    offset_ModuleCfg,               //电路模块配置寄存器
    offset_AllGain,                 //全通道增益
    offset_HFDouble,                //脉冲常数加倍选择
    offset_LineGain,                //基波增益校正
    offset_PinCtrl,                 //数字pin上下拉电阻选择控制
    offset_Pstartup,                //启动功率阈值设置
    offset_Iregion0,                //相位补偿区域设置寄存器0
    offset_Cyclength,               //SAG数据长度设置寄存器
    offset_SAGLevel,                //SAG检测阈值设置寄存器

    offset_Iregion1,                //相位补偿区域设置寄存器1
    offset_PhSregApq2,              //A相相位校正2
    offset_PhSregBpq2,              //B相相位校正2
    offset_PhSregCpq2,              //C相相位校正2
    offset_PoffsetAL,               //A相有功功率offset校正低字节
    offset_PoffsetBL,               //B相有功功率offset校正低字节
    offset_PoffsetCL,               //C相有功功率offset校正低字节
    offset_QoffsetAL,               //A相无功功率offset校正低字节
    offset_QoffsetBL,               //B相无功功率offset校正低字节
    offset_QoffsetCL,               //C相无功功率offset校正低字节
    offset_IRmsoffsetT,             //电流矢量和offset校正寄存器
    offset_TPSoffset,               //TPS初值校正寄存器
    offset_TPSgain,                 //TPS斜率校正寄存器
    offset_TCoffA,                  //Vrefgain的二次系数
    offset_TCoffB,                  //Vrefgain的一次系数
    offset_TCoffC,                  //Vrefgain的常数项
    offset_EMCfg,                   //新增算法控制寄存器
    offset_OILevel,                 //过流阈值设置寄存器

} cali_offset_e;

/* Exported types ------------------------------------------------------------*/
#define Special_Offset                  (w_Iregion1 - w_SAGLevel)
#define ParamOffset_to_RegAddr(offset)  (offset > w_SAGLevel ? offset + Special_Offset - 1 : offset)
#define RegAddr_to_ParamOffset(addr)    (addr > w_SAGLevel ? addr - Special_Offset + 1 : addr)


//EMU工作状态定义
typedef enum
{
    EMU_INIT                        = 0,   //初始化
    EMU_RUNNING                     = 1,   //运行中

    EMU_CONFIG_ISTARTUP             = 5,   //设置起动电流百分比

    EMU_CALI_OFFSET_CUR             = 10,  //校准电流偏移
    EMU_CALI_N_CUR                  = 11,  //校准电流比例系数N
    EMU_CALI_GAIN_CUR               = 12,  //校准电流增益
    EMU_CALI_GAIN_CUR_USE_ERR       = 13,  //校准电流增益（使用误差校准）

    EMU_CALI_OFFSET_VOL             = 20,  //校准电压偏移
    EMU_CALI_GAIN_VOL               = 21,  //校准电压增益
    EMU_CALI_GAIN_VOL_USE_ERR       = 22,  //校准电压增益（使用误差校准）

    EMU_CALI_GAIN_POW               = 30,  //校准功率增益
    EMU_CALI_OFFSET_POW             = 31,  //校准功率偏移
    EMU_CALI_GAIN_POW_USE_ERR       = 32,  //校准功率增益（使用误差校准）
    EMU_CALI_GAIN_POW_USE_FORMULA   = 33,  //校准功率增益（已知电压和电流增益下使用公式计算）

    EMU_CALI_GAIN_PHASE             = 40,  //校准相位
    EMU_CALI_GAIN_PHASE_USE_ERR     = 41,  //校准相位（使用误差校准）

    EMU_CALI_U_I_P_PF               = 50,  //校准电压、电流、功率和功率因数（12->21->33->40）

} emu_work_e;

#define C_100Un_220V               (220)        //Un电压:220.0V
#define C_100LUn_380V              (380)        //LUn线电压:380V
#define C_100F_50Hz                (50)         //F频率:50Hz

#define SamplePointN               (128)        //一个波形的采样点数

//#define	C_120Un     2640u        //120%Un电压:264.0V
//#define	C_75Un      1650u        //75%Un电压:165.0V
//#define	C_70Un      1540u        //70%Un电压:143.0V
//#define	C_60Un      1320u        //60%Un电压:132.0V
//#define	C_10Un      220u         //10%Un电压:22V

//#define	C_05Ib      250u         //5%Ib: 250mA
//#define	C_StrIb     20u

//#define	C_StrPow    216u         //启动功率阈值设置 (Un*Ib*HFConst*imp*0.3%*2^23/(2.592*10^10))



/* ---------------------------- 电流采样电路 ----------------------------
     Ib |     ------ Ii ---------------------
       _|_ ___|           _|_           |
      ( | )               | | R         Vi
       -|- ---|           -|-           |
        |     -------------------------------
     ATT7022E芯片Vi范围0-500mV
   --------------------------------------------------------------------- */
/* ---------------------------------------------------------------------
    互感器参数1A/1mA  并联电阻R200  放大电路x1000
    2400uA/2.4uA  =>  2.4uA*200Ω=0.48mV  =>  0.48mV*1000=480mV  => Ib/Vi=2400uA/480mV

    测电流电流参数2400uA/480mV  设Ib=2400mA   则：
    Ib/Vi=2400uA/480mV
    N为比例系数  N = 1.2*Vi/Ib = 1.2*480mV/2400uA = 0.24
   --------------------------------------------------------------------- */
#define N_Ib2400uA_Vi480mV        (0.24)

/* ---------------------------- 电流采样电路 ----------------------------
     Ib |     ------ Ii ---------------------
       _|_ ___|           _|_           |
      ( | )               | | R         Vi
       -|- ---|           -|-           |
        |     -------------------------------
     ATT7022E芯片Vi范围0-500mV
   --------------------------------------------------------------------- */



/* Exported types ------------------------------------------------------------*/
typedef struct
{
    float real;  //实部
    float imag;  //虚部

} compx_t; //复数


typedef struct
{
    unsigned int CheckSum0;    //计量芯片校验和
    unsigned int CheckSum1;    //计量芯片校验和
    unsigned char EmuWork;     //工作状态

} meter_run_info_t;

typedef struct
{
    float Volt[3];      /* A/B/C电压            */
    float VoltL[3];     /* AB/BC/CA线电压       */
    float Freq;         /* 频率                 */
    float Curr[4];      /* A/B/C/矢量和电流     */
    float PowP[4];      /* A/B/C/总有功功率     */
    float PowQ[4];      /* A/B/C/总无功功率     */
    float PowS[4];      /* A/B/C/总视在功率     */
    float Pf[4];        /* A/B/C/总功率因素     */ 
    float  Cur;         // 剩余电流
} meter_param_t;

typedef struct
{
    float EPI[4];       /* A/B/C/总正向有功电能 */
    float EPE[4];       /* A/B/C/总反向有功电能 */
    float EQL[4];       /* A/B/C/总正向无功电能 */
    float EQC[4];       /* A/B/C/总反向无功电能 */

} meter_energy_param_t;

typedef struct
{
    unsigned char FlagUI;      //电压电流标志(Ua,Ub,Uc,Ut,Ia,Ib,Ic,It)

    /**
      * @brief  功率象限寄存器
      *     @arg bit7: 合相无功功率方向
      *     @arg bit6: C相无功功率方向
      *     @arg bit5: B相无功功率方向
      *     @arg bit4: A相无功功率方向
      *     @arg bit3: 合相有功功率方向
      *     @arg bit2: C相有功功率方向
      *     @arg bit1: B相有功功率方向
      *     @arg bit0: A相有功功率方向
      */
    unsigned char PowQuad;     //功率象限寄存器(A有,B有,C有,合有,A无,B无,C无,合无)
    unsigned short SFlag;      //计量芯片标志状态

} meter_flag_param_t;

//typedef struct
//{
//    unsigned char FlagUI;      //电压电流标志(Ua,Ub,Uc,Ut,Ia,Ib,Ic,It)
//    unsigned char PowQuad;     //功率象限寄存器(A有,B有,C有,合有,A无,B无,C无,合无)
//    unsigned short SFlag;      //计量芯片标志状态

//} meter_flag_param_t;

typedef struct
{
    float Volt_Angle[3];      /* A/B/C电压相角   */
    float Curr_Angle[3];      /* A/B/C电流相角   */
    float Volt_Deviation[3];  /* A/B/C电压偏差   */
    float VoltL_Deviation[3]; /* A/B/C线电压偏差 */
    float Freq_Deviation;     /* 频率偏差        */
    float Volt_Imbalance[2];  /* 零/负序电压不平衡度    */
    float Curr_Imbalance[2];  /* 零/负序电流不平衡度    */
    float Volt_E[3];          /* 零/正/负序电压  */
    float Curr_E[3];          /* 零/正/负序电流  */
    float Fundamental_U[3];   /* 基波A/B/C相电压有效值 */
    float Fundamental_I[3];   /* 基波A/B/C相电流有效值 */
    float Fundamental_UA[3];  /* 基波A/B/C相电压相角 */
    float Fundamental_IA[3];  /* 基波A/B/C相电流相角 */

} power_quality_param_t;

typedef struct
{
    short Volt_1[3];      /* A/B/C电压总谐波畸变率 */
    short Curr_1[3];      /* A/B/C电流总谐波畸变率 */
    short Volt_2_31[30][3];  /* A/B/C电压2-31次谐波畸变率 */
    short Curr_2_31[30][3];  /* A/B/C电流2-31次谐波畸变率 */

} harmonic_param_t;
extern harmonic_param_t        gHarmonicParam;

typedef union
{
    short mbD[1022];
    short D[7][146];  //Ua Ub Uc Ia Ib Ic
//    short mbD[210];
//    short D[7][30];  //Ua Ub Uc Ia Ib Ic

} sync_sampleD_n;

/* Exported constants --------------------------------------------------------*/
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
extern meter_param_t           gMeterParam;
extern meter_energy_param_t    gMeterEnergy;
extern meter_flag_param_t      gFlagMeterParam;
#endif

//#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB
extern power_quality_param_t   gPowerQualityParam;
extern sync_sampleD_n Sync_SampleD;  //同步数据缓存
//#endif

//#if PROD_TYPE == PROD_SFE
extern meter_run_info_t    gMeterRunInfo;
//#endif

extern osThreadId_t att7022TaskHandle;

/* Private functions ---------------------------------------------------------*/
extern void osThreadNew_att7022euTask(void);

extern unsigned int ATT7022_Get_MeterPara(unsigned char addr);
extern void meter_clear_energy(unsigned short clr);

#endif

#endif //__APP_ATT7022EU_H


