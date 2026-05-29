/*
 * Copyright (c) 2006-2018, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-23     Arrbow       first implementation
 */

#ifndef __APP_OLED_H
#define __APP_OLED_H

/* Includes ------------------------------------------------------------------*/
#include "app_opts.h"
#include "cmsis_os.h"
#include "main.h"


/* Private defines -----------------------------------------------------------*/

/**
  @brief 参数断线时的值
  */
#define PARAM_SIGNED_DISCONNECT_VALUE            (0x7FFF)
#define PARAM_UNSIGNED_DISCONNECT_VALUE          (0xFFFF)

/**
  @brief 时间
  */
#define TIME_OUT_TO_DISPLAY_START_GUI            5000   //显示开始界面时间（ms）
#define TIME_REFRESH_TO_DISPLAY_GUI              1000   //页面刷新时间（ms）
#define TIME_REFRESH_TO_LOOP_DISPLAY_GUI         10     //轮询切换界面时间等于10*TIME_REFRESH_TO_DISPLAY_GUI（ms）

/**
  @brief 通用标题 （值小于20）
  */
#define GUI_TITLE_MAIN_MENU                             0   //主菜单
#define GUI_TITLE_LOOKUP_PARAM                          1   //查询参数
#define GUI_TITLE_ELECTRIC_PARAM                        2   //电气参数
#define GUI_TITLE_COMMON_CONFIG                         3   //通用设置
#define GUI_TITLE_MODULE_CONFIG                         4   //模组设置
#define GUI_TITLE_ALARM_INFO                            5   //报警信息
#define GUI_TITLE_COM_INFO                              6   //网关信息

#define GUI_TITLE_COMMON_CONFIG_MODBUS_ADDR             7   //设置地址
#define GUI_TITLE_COMMON_CONFIG_SERIAL_NUMBER           8   //序列号
#define GUI_TITLE_COMMON_CONFIG_FMW_VERSION             9   //固件版本
#define GUI_TITLE_COMMON_CONFIG_FACTORY_RESET           10  //恢复出厂设置

/**
  @brief 各界面的索引值 （值小于8）
  */
#if PROD_TYPE == PROD_SFA
typedef enum __OLED_GUI_INDEX_E
{
    GUI_START                               =   0,   /* 开始页面 */

    GUI_MAIN_MENU_POINT_LOOP_PARAM          =   1,   /* 主菜单：轮询显示 */
    GUI_MAIN_MENU_POINT_LOOKUP_PARAM        =   2,   /* 主菜单：查询显示 */
    GUI_MAIN_MENU_POINT_COMMON_CONFIG       =   3,   /* 主菜单：通用设置 */
    GUI_MAIN_MENU_POINT_MODULE_CONFIG       =   4,   /* 主菜单：模组设置 */
    GUI_MAIN_MENU_POINT_ALARM_INFO          =   5,   /* 主菜单：报警信息 */
    GUI_MAIN_MENU_POINT_COM_INFO            =   6,   /* 主菜单：网关信息 */
    GUI_MAIN_MENU_POINT_ENERGY_PARAM        =   7,   /* 主菜单：电能数据 */

    GUI_COMMON_CONFIG_POINT_MODBUS_ADDR     =   8,   /* 通用设置：Modbus地址 */
    GUI_COMMON_CONFIG_POINT_SERIAL_NUMBER   =   9,   /* 通用设置：序列号 */
    GUI_COMMON_CONFIG_POINT_FMW_VERSION     =   10,   /* 通用设置：固件版本 */
    GUI_COMMON_CONFIG_POINT_FACTORY_RESET   =   11,   /* 通用设置：恢复出厂设置 */
    GUI_COMMON_CONFIG_POINT_RESET           =   12,  /* 通用设置：重启 */

    GUI_CONFIG_MODBUS_ADDR                  =   13,  /* Modbus地址 */
    GUI_CONFIG_SERIAL_NUMBER                =   14,  /* 序列号 */
    GUI_CONFIG_FMW_VERSION                  =   15,  /* 固件版本 */
    GUI_CONFIG_FACTORY_RESET_POINT_NO       =   16,  /* 恢复出厂设置否 */
    GUI_CONFIG_FACTORY_RESET_POINT_YES      =   17,  /* 恢复出厂设置是 */

} oled_gui_index_e;
#endif
#if PROD_TYPE == PROD_SFB
typedef enum __OLED_GUI_INDEX_E
{
    GUI_START                               =   0,   /* 开始页面 */

    GUI_MAIN_MENU_POINT_LOOP_PARAM          =   1,   /* 主菜单：轮询显示 */
    GUI_MAIN_MENU_POINT_LOOKUP_PARAM        =   2,   /* 主菜单：查询显示 */
    GUI_MAIN_MENU_POINT_COMMON_CONFIG       =   3,   /* 主菜单：通用设置 */
    GUI_MAIN_MENU_POINT_MODULE_CONFIG       =   4,   /* 主菜单：模组设置 */
    GUI_MAIN_MENU_POINT_ALARM_INFO          =   5,   /* 主菜单：报警信息 */
    GUI_MAIN_MENU_POINT_COM_INFO            =   6,   /* 主菜单：网关信息 */
    GUI_MAIN_MENU_POINT_ENERGY_PARAM        =   7,   /* 主菜单：电能数据 */
    GUI_MAIN_MENU_POINT_HARMONIC_PARAM      =   8,   /* 主菜单：相位数据 */

    GUI_COMMON_CONFIG_POINT_MODBUS_ADDR     =   9,   /* 通用设置：Modbus地址 */
    GUI_COMMON_CONFIG_POINT_SERIAL_NUMBER   =   10,  /* 通用设置：序列号 */
    GUI_COMMON_CONFIG_POINT_FMW_VERSION     =   11,  /* 通用设置：固件版本 */
    GUI_COMMON_CONFIG_POINT_FACTORY_RESET   =   12,  /* 通用设置：恢复出厂设置 */
    GUI_COMMON_CONFIG_POINT_RESET           =   13,  /* 通用设置：重启 */

    GUI_CONFIG_MODBUS_ADDR                  =   14,  /* Modbus地址 */
    GUI_CONFIG_SERIAL_NUMBER                =   15,  /* 序列号 */
    GUI_CONFIG_FMW_VERSION                  =   16,  /* 固件版本 */
    GUI_CONFIG_FACTORY_RESET_POINT_NO       =   17,  /* 恢复出厂设置否 */
    GUI_CONFIG_FACTORY_RESET_POINT_YES      =   18,  /* 恢复出厂设置是 */

} oled_gui_index_e;
#endif

#if PROD_TYPE == PROD_SFE
typedef enum __OLED_GUI_INDEX_E
{
    GUI_START                               =   0,   /* 开始页面 */

    GUI_MAIN_MENU_POINT_LOOP_PARAM          =   1,   /* 主菜单：轮询显示 */
    GUI_MAIN_MENU_POINT_LOOKUP_PARAM        =   2,   /* 主菜单：查询显示 */
    GUI_MAIN_MENU_POINT_COMMON_CONFIG       =   3,   /* 主菜单：通用设置 */
    GUI_MAIN_MENU_POINT_MODULE_CONFIG       =   4,   /* 主菜单：模组设置 */
    GUI_MAIN_MENU_POINT_ALARM_INFO          =   5,   /* 主菜单：报警信息 */
    GUI_MAIN_MENU_POINT_COM_INFO            =   6,   /* 主菜单：网关信息 */
    GUI_MAIN_MENU_POINT_ENERGY_PARAM        =   7,  /* 主菜单：电能数据 */
    GUI_MAIN_MENU_POINT_HARMONIC_PARAM      =   8,  /* 主菜单：相位数据 */
    GUI_MAIN_MENU_POINT_HARMONICUI_PARAM    =   9,  /* 主菜单：谐波数据 */

    GUI_COMMON_CONFIG_POINT_MODBUS_ADDR     =   10,   /* 通用设置：Modbus地址 */
    GUI_COMMON_CONFIG_POINT_SERIAL_NUMBER   =   11,   /* 通用设置：序列号 */
    GUI_COMMON_CONFIG_POINT_FMW_VERSION     =   12,   /* 通用设置：固件版本 */
    GUI_COMMON_CONFIG_POINT_FACTORY_RESET   =   13,   /* 通用设置：恢复出厂设置 */
    GUI_COMMON_CONFIG_POINT_RESET           =   14,  /* 通用设置：重启 */

    GUI_CONFIG_MODBUS_ADDR                  =   15,  /* Modbus地址 */
    GUI_CONFIG_SERIAL_NUMBER                =   16,  /* 序列号 */
    GUI_CONFIG_FMW_VERSION                  =   17,  /* 固件版本 */
    GUI_CONFIG_FACTORY_RESET_POINT_NO       =   18,  /* 恢复出厂设置否 */
    GUI_CONFIG_FACTORY_RESET_POINT_YES      =   19,  /* 恢复出厂设置是 */

} oled_gui_index_e;
#endif

/**
  @brief 左右箭头
  */
#define GUI_TITLE_HAVE_NONE          0   //没有箭头
#define GUI_TITLE_HAVE_LEFT          1   //有左箭头
#define GUI_TITLE_HAVE_RIGHT         2   //有右箭头
#define GUI_TITLE_HAVE_LEFT_RIGHT    3   //有左右箭头

/**
  @brief 清除保持
  */
#define OLED_DISPLAY_CLEAR           0   //清屏
#define OLED_DISPLAY_KEEP            1   //保持

/**
  @brief 显示位
  */
#define OLED_DISPLAY_POS_BIT_COLOUR(cur_bit, set_bit)  ((set_bit == cur_bit) ? OLED_BLACK_ON_WHITE : OLED_WHITE_ON_BLACK)

/**
  @brief 标志位定义
  */
#define OLED_FLAG_LOOP_PARAM     0x01

/**
  @brief 显示单位定义
  */
#define OLED_DISPLAY_UNIT_A      0x01
#define OLED_DISPLAY_UNIT_mA     0x02
#define OLED_DISPLAY_UNIT_uA     0x03

/**
  @brief 标题 （值从20开始）
  */
//#define GUI_TITLE_MODULE_CONFIG_OUTPUT_CONTROL          20   //输出控制
//#define GUI_TITLE_MODULE_CONFIG_CURRENT_THUP            21   //电流上限
//#define GUI_TITLE_MODULE_CONFIG_TEMP_THUP               22   //温度上限

#define GUI_TITLE_MODULE_CONFIG_OUTPUT_CONTROL          20   //输出控制
#define GUI_TITLE_ENERGY_PARAM_LOOKUP                   21   //电能参数
#define GUI_TITLE_ENERGY_PARAM                          22   //电能数据
#define GUI_TITLE_HARMONIC_PARAM_LOOKUP                 23   //谐波参数
#define GUI_TITLE_HARMONIC_PARAM                        24   //谐波数据
#define GUI_TITLE_XIANG_PARAM_LOOKUP                    25   //相位参数
#define GUI_TITLE_XIANG_PARAM                           26   //相位数据
#define GUI_TITLE_MODULE_CONFIG_CURRENT_THUP            27   //电流上限
#define GUI_TITLE_MODULE_CONFIG_TEMP_THUP               28   //温度上限

/**
  @brief 各界面的索引值 （值从16开始）
  */
#if PROD_TYPE == PROD_SFA
typedef enum __MODULE_OLED_GUI_INDEX_E
{   
    GUI_LOOKUP_PARAM_POINT_U1_U2_U3         =   18,  /* 查询参数：1.相电压 */
    GUI_LOOKUP_PARAM_POINT_U12_U23_U31      =   19,  /* 查询参数：2.线电压 */
    GUI_LOOKUP_PARAM_POINT_F                =   20,  /* 查询参数：3.频率 */
    GUI_LOOKUP_PARAM_POINT_I1_I2_I3         =   21,  /* 查询参数：4.相电流 */
    GUI_LOOKUP_PARAM_POINT_P1_P2_P3         =   22,  /* 查询参数：5.相有功功率 */
    GUI_LOOKUP_PARAM_POINT_Q1_Q2_Q3         =   23,  /* 查询参数：6.相无功功率 */
    GUI_LOOKUP_PARAM_POINT_S1_S2_S3         =   24,  /* 查询参数：7.相视在功率 */
    GUI_LOOKUP_PARAM_POINT_PF1_PF2_PF3      =   25,  /* 查询参数：8.相功率因数 */
    GUI_LOOKUP_PARAM_POINT_P_Q_S            =   26,  /* 查询参数：9.总功率 */
    GUI_LOOKUP_PARAM_POINT_PF               =   27,  /* 查询参数：10.总功率因数 */
    GUI_LOOKUP_PARAM_POINT_DI1_DI2          =   28,  /* 查询参数：11.两路输入 */
    GUI_LOOKUP_PARAM_POINT_DO1_DO2          =   29,  /* 查询参数：12.两路输出 */
    GUI_LOOKUP_PARAM_POINT_CURRENT          =   30,  /* 查询参数：13.剩余电流 */
    GUI_LOOKUP_PARAM_POINT_T1_T2            =   31,  /* 查询参数：14.温度1/2 */
    GUI_LOOKUP_PARAM_POINT_T3_T4            =   32,  /* 查询参数：15.温度3/4 */

    GUI_ELECTRIC_PARAM_POINT_U1_U2_U3       =   33,  /* 电气参数：1.相电压 */
    GUI_ELECTRIC_PARAM_POINT_U12_U23_U31    =   34,  /* 电气参数：2.线电压 */
    GUI_ELECTRIC_PARAM_POINT_F              =   35,  /* 电气参数：3.频率 */
    GUI_ELECTRIC_PARAM_POINT_I1_I2_I3       =   36,  /* 电气参数：4.相电流 */
    GUI_ELECTRIC_PARAM_POINT_P1_P2_P3       =   37,  /* 电气参数：5.相有功功率 */
    GUI_ELECTRIC_PARAM_POINT_Q1_Q2_Q3       =   38,  /* 电气参数：6.相无功功率 */
    GUI_ELECTRIC_PARAM_POINT_S1_S2_S3       =   39,  /* 电气参数：7.相视在功率 */
    GUI_ELECTRIC_PARAM_POINT_PF1_PF2_PF3    =   40,  /* 电气参数：8.相功率因数 */
    GUI_ELECTRIC_PARAM_POINT_P_Q_S          =   41,  /* 电气参数：9.总功率 */
    GUI_ELECTRIC_PARAM_POINT_PF             =   42,  /* 电气参数：10.总功率因数 */
    GUI_ELECTRIC_PARAM_POINT_DI1_DI2        =   43,  /* 电气参数：11.两路输入 */
    GUI_ELECTRIC_PARAM_POINT_DO1_DO2        =   44,  /* 电气参数：12.两路输出 */
    GUI_ELECTRIC_PARAM_POINT_CURRENT        =   45,  /* 电气参数：13.剩余电流 */
    GUI_ELECTRIC_PARAM_POINT_T1_T2          =   46,  /* 电气参数：14.温度1/2 */
    GUI_ELECTRIC_PARAM_POINT_T3_T4          =   47,  /* 电气参数：15.温度3/4 */

    GUI_ALARM_INFO                          =   48,  /* 报警信息 */
    GUI_COM_INFO                            =   49,  /* 报警信息 */

    GUI_MODULE_CONFIG_POINT_OUTPUT_CONTROL  =   50,  /* 模组设置：输出控制 */
    GUI_MODULE_CONFIG_POINT_CURRENT_THUP    =   51,  /* 模组设置：电流上限 */
    GUI_MODULE_CONFIG_POINT_TEMP_THUP       =   52,  /* 模组设置：温度上限 */

    GUI_OUTPUT_CONTROL_POINT_NORMAL         =   53,  /* 输出控制：正常 */
    GUI_OUTPUT_CONTROL_POINT_ALARM          =   54,  /* 输出控制：报警 */
    GUI_OUTPUT_CONTROL_POINT_CANCEL         =   55,  /* 输出控制：取消 */

    GUI_CONFIG_CURRENT_THUP                 =   56,  /* 电流上限 */
    GUI_CONFIG_TEMP_THUP                    =   57,  /* 温度上限 */
    
    GUI_ENERGY_PARAM_LOOKUP_POINT_EPI       =   58,  /* 电能查询：相正向有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EPE       =   59,  /* 电能查询：相反向有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EP        =   60,  /* 电能查询：总有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQL       =   61,  /* 电能查询：相正向无功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQC       =   62,  /* 电能查询：相反向无功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQ        =   63,  /* 电能查询：总无功电能 */

    GUI_ENERGY_PARAM_POINT_EPI              =   64,  /* 电能参数：相正向有功电能 */
    GUI_ENERGY_PARAM_POINT_EPE              =   65,  /* 电能参数：相反向有功电能 */
    GUI_ENERGY_PARAM_POINT_EP               =   66,  /* 电能参数：总有功电能 */
    GUI_ENERGY_PARAM_POINT_EQL              =   67,  /* 电能参数：相正向无功电能 */
    GUI_ENERGY_PARAM_POINT_EQC              =   68,  /* 电能参数：相反向无功电能 */
    GUI_ENERGY_PARAM_POINT_EQ               =   69,  /* 电能参数：总无功电能 */

} module_oled_gui_index_e;
#endif
#if PROD_TYPE == PROD_SFB
typedef enum __MODULE_OLED_GUI_INDEX_E
{   
    GUI_LOOKUP_PARAM_POINT_U1_U2_U3         =   19,  /* 查询参数：1.相电压 */
    GUI_LOOKUP_PARAM_POINT_U12_U23_U31      =   20,  /* 查询参数：2.线电压 */
    GUI_LOOKUP_PARAM_POINT_F                =   21,  /* 查询参数：3.频率 */
    GUI_LOOKUP_PARAM_POINT_I1_I2_I3         =   22,  /* 查询参数：4.相电流 */
    GUI_LOOKUP_PARAM_POINT_P1_P2_P3         =   23,  /* 查询参数：5.相有功功率 */
    GUI_LOOKUP_PARAM_POINT_Q1_Q2_Q3         =   24,  /* 查询参数：6.相无功功率 */
    GUI_LOOKUP_PARAM_POINT_S1_S2_S3         =   25,  /* 查询参数：7.相视在功率 */
    GUI_LOOKUP_PARAM_POINT_PF1_PF2_PF3      =   26,  /* 查询参数：8.相功率因数 */
    GUI_LOOKUP_PARAM_POINT_P_Q_S            =   27,  /* 查询参数：9.总功率 */
    GUI_LOOKUP_PARAM_POINT_PF               =   28,  /* 查询参数：10.总功率因数 */
    GUI_LOOKUP_PARAM_POINT_DI1_DI2          =   29,  /* 查询参数：11.两路输入 */
    GUI_LOOKUP_PARAM_POINT_DO1_DO2          =   30,  /* 查询参数：12.两路输出 */
    GUI_LOOKUP_PARAM_POINT_CURRENT          =   31,  /* 查询参数：13.剩余电流 */
    GUI_LOOKUP_PARAM_POINT_T1_T2            =   32,  /* 查询参数：14.温度1/2 */
    GUI_LOOKUP_PARAM_POINT_T3_T4            =   33,  /* 查询参数：15.温度3/4 */

    GUI_ELECTRIC_PARAM_POINT_U1_U2_U3       =   34,  /* 电气参数：1.相电压 */
    GUI_ELECTRIC_PARAM_POINT_U12_U23_U31    =   35,  /* 电气参数：2.线电压 */
    GUI_ELECTRIC_PARAM_POINT_F              =   36,  /* 电气参数：3.频率 */
    GUI_ELECTRIC_PARAM_POINT_I1_I2_I3       =   37,  /* 电气参数：4.相电流 */
    GUI_ELECTRIC_PARAM_POINT_P1_P2_P3       =   38,  /* 电气参数：5.相有功功率 */
    GUI_ELECTRIC_PARAM_POINT_Q1_Q2_Q3       =   39,  /* 电气参数：6.相无功功率 */
    GUI_ELECTRIC_PARAM_POINT_S1_S2_S3       =   40,  /* 电气参数：7.相视在功率 */
    GUI_ELECTRIC_PARAM_POINT_PF1_PF2_PF3    =   41,  /* 电气参数：8.相功率因数 */
    GUI_ELECTRIC_PARAM_POINT_P_Q_S          =   42,  /* 电气参数：9.总功率 */
    GUI_ELECTRIC_PARAM_POINT_PF             =   43,  /* 电气参数：10.总功率因数 */
    GUI_ELECTRIC_PARAM_POINT_DI1_DI2        =   44,  /* 电气参数：11.两路输入 */
    GUI_ELECTRIC_PARAM_POINT_DO1_DO2        =   45,  /* 电气参数：12.两路输出 */
    GUI_ELECTRIC_PARAM_POINT_CURRENT        =   46,  /* 电气参数：13.剩余电流 */
    GUI_ELECTRIC_PARAM_POINT_T1_T2          =   47,  /* 电气参数：14.温度1/2 */
    GUI_ELECTRIC_PARAM_POINT_T3_T4          =   48,  /* 电气参数：15.温度3/4 */

    GUI_ALARM_INFO                          =   49,  /* 报警信息 */
    GUI_COM_INFO                            =   50,  /* 报警信息 */

    GUI_MODULE_CONFIG_POINT_OUTPUT_CONTROL  =   51,  /* 模组设置：输出控制 */
    GUI_MODULE_CONFIG_POINT_CURRENT_THUP    =   52,  /* 模组设置：电流上限 */
    GUI_MODULE_CONFIG_POINT_TEMP_THUP       =   53,  /* 模组设置：温度上限 */

    GUI_OUTPUT_CONTROL_POINT_NORMAL         =   54,  /* 输出控制：正常 */
    GUI_OUTPUT_CONTROL_POINT_ALARM          =   55,  /* 输出控制：报警 */
    GUI_OUTPUT_CONTROL_POINT_CANCEL         =   56,  /* 输出控制：取消 */

    GUI_CONFIG_CURRENT_THUP                 =   57,  /* 电流上限 */
    GUI_CONFIG_TEMP_THUP                    =   58,  /* 温度上限 */
    
    GUI_ENERGY_PARAM_LOOKUP_POINT_EPI       =   59,  /* 电能查询：相正向有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EPE       =   60,  /* 电能查询：相反向有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EP        =   61,  /* 电能查询：总有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQL       =   62,  /* 电能查询：相正向无功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQC       =   63,  /* 电能查询：相反向无功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQ        =   64,  /* 电能查询：总无功电能 */

    GUI_ENERGY_PARAM_POINT_EPI              =   65,  /* 电能参数：相正向有功电能 */
    GUI_ENERGY_PARAM_POINT_EPE              =   66,  /* 电能参数：相反向有功电能 */
    GUI_ENERGY_PARAM_POINT_EP               =   67,  /* 电能参数：总有功电能 */
    GUI_ENERGY_PARAM_POINT_EQL              =   68,  /* 电能参数：相正向无功电能 */
    GUI_ENERGY_PARAM_POINT_EQC              =   69,  /* 电能参数：相反向无功电能 */
    GUI_ENERGY_PARAM_POINT_EQ               =   70,  /* 电能参数：总无功电能 */

    GUI_HARMONIC_PARAM_LOOKUP_POINT_AgU     =   71,  /* 谐波查询：相电压相角 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_AgI     =   72,  /* 谐波查询：相电流相角 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_DvU     =   73,  /* 谐波查询：相电压偏差 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_DvUL    =   74,  /* 谐波查询：线电压偏差 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_DvF     =   75,  /* 谐波查询：频率偏差 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_ImbU    =   76,  /* 谐波查询：电压不平衡度 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_ImbI    =   77,  /* 谐波查询：电流不平衡度 */

    GUI_HARMONIC_PARAM_POINT_AgU            =   78,  /* 谐波参数：相电压相角 */
    GUI_HARMONIC_PARAM_POINT_AgI            =   79,  /* 谐波参数：相电流相角 */
    GUI_HARMONIC_PARAM_POINT_DvU            =   80,  /* 谐波参数：相电压偏差 */
    GUI_HARMONIC_PARAM_POINT_DvUL           =   81,  /* 谐波参数：线电压偏差 */
    GUI_HARMONIC_PARAM_POINT_DvF            =   82,  /* 谐波参数：频率偏差 */
    GUI_HARMONIC_PARAM_POINT_ImbU           =   83,  /* 谐波参数：电压不平衡度 */
    GUI_HARMONIC_PARAM_POINT_ImbI           =   84,  /* 谐波参数：电流不平衡度 */

} module_oled_gui_index_e;
#endif
#if PROD_TYPE == PROD_SFE
typedef enum __MODULE_OLED_GUI_INDEX_E
{   
    GUI_LOOKUP_PARAM_POINT_U1_U2_U3         =   20,  /* 查询参数：1.相电压 */
    GUI_LOOKUP_PARAM_POINT_U12_U23_U31      =   21,  /* 查询参数：2.线电压 */
    GUI_LOOKUP_PARAM_POINT_F                =   22,  /* 查询参数：3.频率 */
    GUI_LOOKUP_PARAM_POINT_I1_I2_I3         =   23,  /* 查询参数：4.相电流 */
    GUI_LOOKUP_PARAM_POINT_P1_P2_P3         =   24,  /* 查询参数：5.相有功功率 */
    GUI_LOOKUP_PARAM_POINT_Q1_Q2_Q3         =   25,  /* 查询参数：6.相无功功率 */
    GUI_LOOKUP_PARAM_POINT_S1_S2_S3         =   26,  /* 查询参数：7.相视在功率 */
    GUI_LOOKUP_PARAM_POINT_PF1_PF2_PF3      =   27,  /* 查询参数：8.相功率因数 */
    GUI_LOOKUP_PARAM_POINT_P_Q_S            =   28,  /* 查询参数：9.总功率 */
    GUI_LOOKUP_PARAM_POINT_PF               =   29,  /* 查询参数：10.总功率因数 */
    GUI_LOOKUP_PARAM_POINT_DI1_DI2          =   30,  /* 查询参数：11.两路输入 */
    GUI_LOOKUP_PARAM_POINT_DO1_DO2          =   31,  /* 查询参数：12.两路输出 */
    GUI_LOOKUP_PARAM_POINT_CURRENT          =   32,  /* 查询参数：13.剩余电流 */
    GUI_LOOKUP_PARAM_POINT_T1_T2            =   33,  /* 查询参数：14.温度1/2 */
    GUI_LOOKUP_PARAM_POINT_T3_T4            =   34,  /* 查询参数：15.温度3/4 */

    GUI_ELECTRIC_PARAM_POINT_U1_U2_U3       =   35,  /* 电气参数：1.相电压 */
    GUI_ELECTRIC_PARAM_POINT_U12_U23_U31    =   36,  /* 电气参数：2.线电压 */
    GUI_ELECTRIC_PARAM_POINT_F              =   37,  /* 电气参数：3.频率 */
    GUI_ELECTRIC_PARAM_POINT_I1_I2_I3       =   38,  /* 电气参数：4.相电流 */
    GUI_ELECTRIC_PARAM_POINT_P1_P2_P3       =   39,  /* 电气参数：5.相有功功率 */
    GUI_ELECTRIC_PARAM_POINT_Q1_Q2_Q3       =   40,  /* 电气参数：6.相无功功率 */
    GUI_ELECTRIC_PARAM_POINT_S1_S2_S3       =   41,  /* 电气参数：7.相视在功率 */
    GUI_ELECTRIC_PARAM_POINT_PF1_PF2_PF3    =   42,  /* 电气参数：8.相功率因数 */
    GUI_ELECTRIC_PARAM_POINT_P_Q_S          =   43,  /* 电气参数：9.总功率 */
    GUI_ELECTRIC_PARAM_POINT_PF             =   44,  /* 电气参数：10.总功率因数 */
    GUI_ELECTRIC_PARAM_POINT_DI1_DI2        =   45,  /* 电气参数：11.两路输入 */
    GUI_ELECTRIC_PARAM_POINT_DO1_DO2        =   46,  /* 电气参数：12.两路输出 */
    GUI_ELECTRIC_PARAM_POINT_CURRENT        =   47,  /* 电气参数：13.剩余电流 */
    GUI_ELECTRIC_PARAM_POINT_T1_T2          =   48,  /* 电气参数：14.温度1/2 */
    GUI_ELECTRIC_PARAM_POINT_T3_T4          =   49,  /* 电气参数：15.温度3/4 */

    GUI_ALARM_INFO                          =   50,  /* 报警信息 */
    GUI_COM_INFO                            =   51,  /* 报警信息 */

    GUI_MODULE_CONFIG_POINT_OUTPUT_CONTROL  =   52,  /* 模组设置：输出控制 */
    GUI_MODULE_CONFIG_POINT_CURRENT_THUP    =   53,  /* 模组设置：电流上限 */
    GUI_MODULE_CONFIG_POINT_TEMP_THUP       =   54,  /* 模组设置：温度上限 */

    GUI_OUTPUT_CONTROL_POINT_NORMAL         =   55,  /* 输出控制：正常 */
    GUI_OUTPUT_CONTROL_POINT_ALARM          =   56,  /* 输出控制：报警 */
    GUI_OUTPUT_CONTROL_POINT_CANCEL         =   57,  /* 输出控制：取消 */

    GUI_CONFIG_CURRENT_THUP                 =   58,  /* 电流上限 */
    GUI_CONFIG_TEMP_THUP                    =   59,  /* 温度上限 */
    
    GUI_ENERGY_PARAM_LOOKUP_POINT_EPI       =   60,  /* 电能查询：相正向有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EPE       =   61,  /* 电能查询：相反向有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EP        =   62,  /* 电能查询：总有功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQL       =   63,  /* 电能查询：相正向无功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQC       =   64,  /* 电能查询：相反向无功电能 */
    GUI_ENERGY_PARAM_LOOKUP_POINT_EQ        =   65,  /* 电能查询：总无功电能 */

    GUI_ENERGY_PARAM_POINT_EPI              =   66,  /* 电能参数：相正向有功电能 */
    GUI_ENERGY_PARAM_POINT_EPE              =   67,  /* 电能参数：相反向有功电能 */
    GUI_ENERGY_PARAM_POINT_EP               =   68,  /* 电能参数：总有功电能 */
    GUI_ENERGY_PARAM_POINT_EQL              =   69,  /* 电能参数：相正向无功电能 */
    GUI_ENERGY_PARAM_POINT_EQC              =   70,  /* 电能参数：相反向无功电能 */
    GUI_ENERGY_PARAM_POINT_EQ               =   71,  /* 电能参数：总无功电能 */

    GUI_HARMONIC_PARAM_LOOKUP_POINT_AgU     =   72,  /* 谐波查询：相电压相角 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_AgI     =   73,  /* 谐波查询：相电流相角 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_DvU     =   74,  /* 谐波查询：相电压偏差 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_DvUL    =   75,  /* 谐波查询：线电压偏差 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_DvF     =   76,  /* 谐波查询：频率偏差 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_ImbU    =   77,  /* 谐波查询：电压不平衡度 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_ImbI    =   78,  /* 谐波查询：电流不平衡度 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_U       =   79,  /* 谐波查询：U谐波畸变率 */
    GUI_HARMONIC_PARAM_LOOKUP_POINT_I       =   80,  /* 谐波查询：I谐波畸变率 */

    GUI_HARMONIC_PARAM_POINT_AgU            =   81,  /* 谐波参数：相电压相角 */
    GUI_HARMONIC_PARAM_POINT_AgI            =   82,  /* 谐波参数：相电流相角 */
    GUI_HARMONIC_PARAM_POINT_DvU            =   83,  /* 谐波参数：相电压偏差 */
    GUI_HARMONIC_PARAM_POINT_DvUL           =   84,  /* 谐波参数：线电压偏差 */
    GUI_HARMONIC_PARAM_POINT_DvF            =   85,  /* 谐波参数：频率偏差 */
    GUI_HARMONIC_PARAM_POINT_ImbU           =   86,  /* 谐波参数：电压不平衡度 */
    GUI_HARMONIC_PARAM_POINT_ImbI           =   87,  /* 谐波参数：电流不平衡度 */
    GUI_HARMONIC_PARAM_POINT_U              =   88,  /* 谐波参数：U谐波畸变率 */
    GUI_HARMONIC_PARAM_POINT_I              =   89,  /* 谐波参数：I谐波畸变率 */

} module_oled_gui_index_e;
#endif

//#define StateAlm0                 0       //gFlashParam.st.State_Alarm[0]
//#define StateAlm0_VOL1_Msk        0x0001  //电压1异常
//#define StateAlm0_VOL2_Msk        0x0002  //电压2异常
//#define StateAlm0_VOL3_Msk        0x0004  //电压3异常
//#define StateAlm0_Iol1_Msk        0x0008  //漏流1异常
//#define StateAlm0_Iol2_Msk        0x0010  //漏流2异常
//#define StateAlm0_Iol3_Msk        0x0020  //漏流3异常
//#define StateAlm0_Cur_Msk         0x0040  //剩余电流异常
//#define StateAlm0_TMP1_Msk        0x0080  //温度1异常
//#define StateAlm0_TMP2_Msk        0x0100  //温度2异常
//#define StateAlm0_TMP3_Msk        0x0200  //温度1异常
//#define StateAlm0_TMP4_Msk        0x0400  //温度2异常
//#define StateAlm0_LIFE_Msk        0x0800  //寿命异常
//#define StateAlm0_VOL_Msk         (StateAlm0_VOL1_Msk | StateAlm0_VOL2_Msk | StateAlm0_VOL3_Msk)
//#define StateAlm0_IOL_Msk         (StateAlm0_Iol1_Msk | StateAlm0_Iol2_Msk | StateAlm0_Iol3_Msk)
//#define StateAlm0_TMP_Msk         (StateAlm0_TMP1_Msk | StateAlm0_TMP2_Msk | StateAlm0_TMP3_Msk | StateAlm0_TMP4_Msk)
//#define StateAlm0_VOL_IOL_Msk     (StateAlm0_VOL_Msk | StateAlm0_IOL_Msk)
//#define StateAlm0_ALL_Msk         (StateAlm0_VOL_IOL_Msk | StateAlm0_TMP_Msk | StateAlm0_LIFE_Msk)

/* Exported types ------------------------------------------------------------*/

typedef struct
{
    uint8_t cur_index;//当前索引项
    uint8_t next;//选择
    uint8_t enter;//确定
    void (*current_operation)(uint8_t, uint8_t); //	当前索引执行的函数(界面)

} key_oled_gui_d;

typedef struct
{
    uint8_t cur_index;//当前索引项
    uint8_t next;//选择
    uint8_t enter;//确定
    void (*current_operation)(uint8_t, uint8_t); //	当前索引执行的函数(界面)

} key_oled_gui_u;

typedef struct
{
    uint8_t cur_index;//当前索引项
    uint8_t next;//选择
    uint8_t enter;//确定
    void (*current_operation)(uint8_t, uint8_t); //	当前索引执行的函数(界面)

} key_oled_gui_r;

typedef struct
{
    uint8_t cur_index;//当前索引项
    uint8_t next;//选择
    uint8_t enter;//确定
    void (*current_operation)(uint8_t, uint8_t); //	当前索引执行的函数(界面)

} key_oled_gui_l;

typedef struct
{
    uint8_t cur_index; //当前页面索引
    uint8_t last_index; //上一页面索引
    uint8_t flag; //轮询显示参数标志
    uint8_t alarm_status; //报警状态

} key_oled_run_info_t;



/* Exported constants --------------------------------------------------------*/
extern osThreadId_t keyOledTaskHandle;



/* Private functions ---------------------------------------------------------*/
extern void osThreadNew_keyOled(void);

extern void oled_display_main_menu_gui(uint8_t page_index, uint8_t key_val);
extern void oled_display_main_menu_gui_u(uint8_t page_index, uint8_t key_val);
extern void oled_display_main_menu_gui_r(uint8_t page_index, uint8_t key_val);
extern void oled_display_main_menu_gui_l(uint8_t page_index, uint8_t key_val);

extern void oled_display_common_config_gui(uint8_t page_index, uint8_t key_val);
extern void oled_display_common_config_gui_u(uint8_t page_index, uint8_t key_val);
extern void oled_display_common_config_gui_r(uint8_t page_index, uint8_t key_val);
extern void oled_display_common_config_gui_l(uint8_t page_index, uint8_t key_val);

extern void oled_display_config_modbus_addr_gui(uint8_t page_index, uint8_t key_val);
extern void oled_display_config_serial_number_gui(uint8_t page_index, uint8_t key_val);
extern void oled_display_config_fmw_version_gui(uint8_t page_index, uint8_t key_val);
extern void oled_display_config_factory_reset_gui(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_start_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_lookup_param_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_lookup_param_gui_u(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_lookup_param_gui_r(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_lookup_param_gui_l(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_electric_param_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_electric_param_gui_u(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_alarm_info(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_com_info(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_module_config_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_module_config_gui_u(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_harmonicUI_param_lookup_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_harmonicUI_param_gui(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_set_count_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_output_control_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_output_control_gui_u(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_harmonic_param_lookup_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_harmonic_param_lookup_gui_u(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_harmonic_param_lookup_gui_r(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_harmonic_param_lookup_gui_l(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_harmonic_param_gui(uint8_t page_index, uint8_t key_val);

extern void oled_display_ESE_main_menu_gui(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_energy_param_lookup_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_energy_param_lookup_gui_u(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_energy_param_lookup_gui_r(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_energy_param_lookup_gui_l(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_energy_param_gui(uint8_t page_index, uint8_t key_val);

extern void module_oled_display_config_current_thup_gui(uint8_t page_index, uint8_t key_val);
extern void module_oled_display_config_temp_thup_gui(uint8_t page_index, uint8_t key_val);

extern void iotb_key_init(void);
extern uint8_t Val_To_String(uint8_t Sign, uint32_t Val, uint8_t *pBuf);
extern uint8_t ValDivideBy10_To_String(uint8_t Sign, uint32_t Val, uint8_t *pBuf);

#define  KEY_TURN_ON_LEVEL        1   //按键有效电平
#define  KEY_SCAN_CYCLE           20  //扫描按键频率(ms)

/**
  @brief 标志位定义
  */
//#define StateAlm0                 0       //gFlashParam.st.State_Alarm[0]
//#define StateAlm0_L1_Msk          0x0001  //A相异常
//#define StateAlm0_L2_Msk          0x0002  //B相异常
//#define StateAlm0_L3_Msk          0x0004  //C相异常
//#define StateAlm0_PE_Msk          0x0008  //接地异常
//#define StateAlm0_NT1_Msk         0x0010  //A相脱扣异常
//#define StateAlm0_NT2_Msk         0x0020  //B相脱扣异常
//#define StateAlm0_NT3_Msk         0x0040  //C相脱扣异常
//#define StateAlm0_NT_Msk          0x0080  //N相脱扣异常
//#define StateAlm0_LIFE_Msk        0x0800  //寿命异常
//#define StateAlm0_T1_Msk          0x1000  //A相温度异常
//#define StateAlm0_T2_Msk          0x2000  //B相温度异常
//#define StateAlm0_T3_Msk          0x4000  //C相温度异常
//#define StateAlm0_TN_Msk          0x8000  //N相温度异常
//#define StateAlm0_Phase_Msk       (StateAlm0_L1_Msk | StateAlm0_L2_Msk | StateAlm0_L3_Msk | StateAlm0_PE_Msk)
//#define StateAlm0_ANT_Msk         (StateAlm0_NT1_Msk | StateAlm0_NT2_Msk | StateAlm0_NT3_Msk |StateAlm0_NT_Msk)
#if PROD_TYPE == PROD_FSS
#define StateAlm0_TMP_Msk         (StateAlm0_T1_Msk | StateAlm0_T2_Msk | StateAlm0_T3_Msk | StateAlm0_TN_Msk)
#define StateAlm0_ALL_Msk         (StateAlm0_Phase_Msk | StateAlm0_ANT_Msk | StateAlm0_TMP_Msk | StateAlm0_LIFE_Msk)
typedef enum
{
    USER_BUTTON_1 = 0,
    USER_BUTTON_2,
    USER_BUTTON_MAX
} iotb_user_button_t;
#else
typedef enum
{
    USER_BUTTON_1 = 0,
    USER_BUTTON_2,
    USER_BUTTON_3,
    USER_BUTTON_4,
    USER_BUTTON_5,
    USER_BUTTON_MAX
} iotb_user_button_t;

#endif

#endif /* __APP_OLED_H */
