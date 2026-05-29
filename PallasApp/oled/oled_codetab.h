/*
 * Copyright (c) 2006-2018, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-23     Arrbow       first implementation
 */

#ifndef __OLED_I2C_CODETAB_H
#define __OLED_I2C_CODETAB_H

/* Includes ------------------------------------------------------------------*/



/* Private defines -----------------------------------------------------------*/

/**
  @brief 中文字符
  */
typedef enum __OLED_CHAR_CN_E
{
    OLED_CHAR_dian        =   0,   /* 电 */
    OLED_CHAR_qi          =   1,   /* 气 */
    OLED_CHAR_can         =   2,   /* 参 */
    OLED_CHAR_shu         =   3,   /* 数 */
    OLED_CHAR_wen         =   4,   /* 温 */
    OLED_CHAR_du          =   5,   /* 度 */
    OLED_CHAR_sheshidu    =   6,   /* ℃ */
    OLED_CHAR_liu         =   7,   /* 流 */
    OLED_CHAR_di          =   8,   /* 地 */
    OLED_CHAR_zhi         =   9,   /* 址 */
    OLED_CHAR_mo          =   10,  /* 模 */
    OLED_CHAR_zu          =   11,  /* 组 */
    OLED_CHAR_xin         =   12,  /* 信 */
    OLED_CHAR_xi          =   13,  /* 息 */
    OLED_CHAR_cha         =   14,  /* 查 */
    OLED_CHAR_lun         =   15,  /* 轮 */
    OLED_CHAR_xun         =   16,  /* 询 */
    OLED_CHAR_xian        =   17,  /* 显 */
    OLED_CHAR_shi         =   18,  /* 示 */
    OLED_CHAR_tong        =   19,  /* 通 */
    OLED_CHAR_yong        =   20,  /* 用 */
    OLED_CHAR_bao         =   21,  /* 报 */
    OLED_CHAR_jing        =   22,  /* 警 */
    OLED_CHAR_she         =   23,  /* 设 */
    OLED_CHAR_zhi1        =   24,  /* 置 */
    OLED_CHAR_zuojiantou  =   25,  /* 实心左三角 */
    OLED_CHAR_youjiantou  =   26,  /* 实心右三角 */
    OLED_CHAR_zuojiantou0 =   27,  /* 空心左三角 */
    OLED_CHAR_youjiantou0 =   28,  /* 空心右三角 */
    OLED_CHAR_zhu         =   29,  /* 主 */
    OLED_CHAR_cai         =   30,  /* 菜 */
    OLED_CHAR_dan         =   31,  /* 单 */
    OLED_CHAR_wei         =   32,  /* 未 */
    OLED_CHAR_lian        =   33,  /* 连 */
    OLED_CHAR_jie         =   34,  /* 接 */
    OLED_CHAR_yi          =   35,  /* 异 */
    OLED_CHAR_chang       =   36,  /* 常 */
    OLED_CHAR_wu          =   37,  /* 无 */
    OLED_CHAR_zheng       =   38,  /* 正 */
    OLED_CHAR_ji          =   39,  /* 计 */
    OLED_CHAR_shou        =   40,  /* 寿 */
    OLED_CHAR_ming        =   41,  /* 命 */
    OLED_CHAR_ya          =   42,  /* 压 */
    OLED_CHAR_ci          =   43,  /* 次 */
    OLED_CHAR_xu          =   44,  /* 序 */
    OLED_CHAR_lie         =   45,  /* 列 */
    OLED_CHAR_hao         =   46,  /* 号 */
    OLED_CHAR_gu          =   47,  /* 固 */
    OLED_CHAR_jian1       =   48,  /* 件 */
    OLED_CHAR_ban         =   49,  /* 版 */
    OLED_CHAR_ben         =   50,  /* 本 */
    OLED_CHAR_hui         =   51,  /* 恢 */
    OLED_CHAR_fu          =   52,  /* 复 */
    OLED_CHAR_chang1      =   53,  /* 厂 */
    OLED_CHAR_shi2        =   54,  /* 是 */
    OLED_CHAR_fou         =   55,  /* 否 */
    OLED_CHAR_ZHI         =   56,  /* 智 */
    OLED_CHAR_NENG        =   57,  /* 能 */
    OLED_CHAR_XING        =   58,  /* 型 */
    OLED_CHAR_YONG        =   59,  /* 涌 */
    OLED_CHAR_BAO         =   60,  /* 保 */
    OLED_CHAR_HU          =   61,  /* 护 */
    OLED_CHAR_QI          =   62,  /* 器 */
    OLED_CHAR_XIANG       =   63,  /* 相 */
    OLED_CHAR_chu         =   64,  /* 出 */
    OLED_CHAR_zhuang      =   65,  /* 状 */
    OLED_CHAR_tai         =   66,  /* 态 */
    OLED_CHAR_san         =   67,  /* 三 */
    OLED_CHAR_tuo         =   68,  /* 脱 */
    OLED_CHAR_kou         =   69,  /* 扣 */
    OLED_CHAR_lou         =   70,  /* 漏 */
    OLED_CHAR_zkh         =   71,  /* ( */    
    OLED_CHAR_ykh         =   72,  /* ) */
    OLED_CHAR_WEI         =   73,  /* 位 */
    OLED_CHAR_YI          =   74,  /* 异 */
    OLED_CHAR_CHANG       =   75,  /* 常 */
    OLED_CHAR_huo         =   76,  /* 火 */
    OLED_CHAR_xlan        =   77,  /* 线 */
    OLED_CHAR_ling        =   78,  /* 零 */
    OLED_CHAR_nai         =   79,  /* 耐 */
    OLED_CHAR_jiu         =   80,  /* 久 */
    OLED_CHAR_bei         =   81,  /* 备 */
    OLED_CHAR_duan        =   82,  /* 断 */
    OLED_CHAR_chong       =   83,  /* 重 */
    OLED_CHAR_qi2         =   84,  /* 启 */
    OLED_CHAR_shu1        =   85,  /* 输 */
    OLED_CHAR_ru          =   86,  /* 入 */
    OLED_CHAR_zai         =   87,  /* 灾 */
    OLED_CHAR_jian        =   88,  /* 监 */
    OLED_CHAR_ce          =   89,  /* 测 */
    OLED_CHAR_kong1       =   90,  /* 控 */
    OLED_CHAR_zhi2        =   91,  /* 制 */
    OLED_CHAR_shang       =   92,  /* 上 */
    OLED_CHAR_xian2       =   93,  /* 限 */
    OLED_CHAR_qu          =   94,  /* 取 */
    OLED_CHAR_xiao        =   95,  /* 消 */
    OLED_CHAR_sheng       =   96,  /* 剩 */
    OLED_CHAR_yu          =   97,  /* 余 */
    OLED_CHAR_wang        =   98,  /* 网 */
    OLED_CHAR_guan        =   99,  /* 关 */
    OLED_CHAR_fang        =   100, /* 方 */
    OLED_CHAR_shl         =   101, /* 式 */
    OLED_CHAR_yl          =   102, /* 一 */
    OLED_CHAR_baN         =   103, /* 般 */
    OLED_CHAR_jl          =   104, /* 极 */
    OLED_CHAR_cHa         =   105, /* 差 */
    OLED_CHAR_biao        =   106, /* 表 */
    OLED_CHAR_pin         =   107, /* 频 */
    OLED_CHAR_lv          =   108, /* 率 */
    OLED_CHAR_you         =   109, /* 有 */
    OLED_CHAR_gong        =   110, /* 功 */
    OLED_CHAR_zong        =   111, /* 总 */
    OLED_CHAR_shi1        =   112, /* 视 */
    OLED_CHAR_zai1        =   113, /* 在 */
    OLED_CHAR_yin         =   114, /* 因 */
    OLED_CHAR_liang       =   115, /* 两 */
    OLED_CHAR_lu          =   116, /* 路 */
    OLED_CHAR_neng        =   117, /* 能 */
    OLED_CHAR_ju          =   118, /* 据 */
    OLED_CHAR_wel         =   119, /* 位 */
    OLED_CHAR_xie         =   120, /* 谐 */
    OLED_CHAR_bo          =   121, /* 波 */
    OLED_CHAR_jiao        =   122, /* 角 */
    OLED_CHAR_pian        =   123, /* 偏 */
    OLED_CHAR_cha1        =   124, /* 差 */
    OLED_CHAR_bu          =   125, /* 不 */
    OLED_CHAR_ping        =   126, /* 平 */
    OLED_CHAR_heng        =   127, /* 衡 */
    OLED_CHAR_ji1         =   128, /* 畸 */
    OLED_CHAR_bian        =   129, /* 变 */
    OLED_CHAR_xiang1      =   130, /* 向 */
    OLED_CHAR_fan         =   131, /* 反 */
    OLED_CHAR_zhi3        =   132, /* 质 */
    OLED_CHAR_l1ang       =   133, /* 量 */
    OLED_CHAR_pei         =   134, /* 配 */
    OLED_CHAR_guo         =   135, /* 过 */
    OLED_CHAR_qian        =   136, /* 欠 */
    OLED_CHAR_e           =   137, /* 恶 */
    OLED_CHAR_x           =   138, /* 性 */
    OLED_CHAR_f           =   139, /* 负 */
    OLED_CHAR_z           =   140, /* 载 */
    OLED_CHAR_si          =   141, /* 四 */
    
} oled_char_cn_e;


/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/
extern unsigned char F16x16[];
extern const unsigned char F6x8[][6];
extern const unsigned char F8X16[];
extern unsigned char BMP_8x8_Sigma[];
extern unsigned char BMP_8x16_Epsilon[];
extern unsigned char BMP_8x16_Delta[];
extern unsigned char BMP_8x16_Angle[];
extern unsigned char BMP_8x16_Degree[];

/* Private functions ---------------------------------------------------------*/



#endif /* __OLED_I2C_CODETAB_H */
