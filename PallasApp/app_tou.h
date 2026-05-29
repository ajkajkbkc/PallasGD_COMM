/**
 * @file    app_tou.h
 * @brief   分时计费(Time-of-Use)功能头文件
 *          支持尖/峰/平/谷/深谷五种费率，每天最多12个时段配置
 *          支持月份/季节分时规则、阶梯电价
 * @date    2026-05-14
 *
 * @note    功能概述：
 *          1. 日时段表 — 定义每天的尖/峰/平/谷/深谷时段划分
 *          2. 月份规则 — 不同季节使用不同的日时段表（夏季尖峰，春秋季常规）
 *          3. 阶梯电价 — 根据月累计用电量分档，不同档位不同电价系数
 *          4. 费率匹配逻辑 — 按月份→日期→日时段表→时段顺序匹配当前费率
 */

#ifndef __APP_TOU_H
#define __APP_TOU_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "app_rtc.h"

/* ======================== 费率与时段配置 ======================== */

/** @defgroup TOU_Rate_Index 费率索引定义
  * @brief 四种费率类型，索引号与数组下标对应
  * @{
  */
#define TOU_RATE_TOP_IDX    0   /**< 尖费率 (最高电价)   */
#define TOU_RATE_PEAK_IDX   1   /**< 峰费率 (较高电价)   */
#define TOU_RATE_FLAT_IDX   2   /**< 平费率 (标准电价)   */
#define TOU_RATE_VALLEY_IDX 3   /**< 谷费率 (较低电价)   */
#define TOU_RATE_DEEP_IDX   4   /**< 深谷费率 (最低电价)   */
#define TOU_RATE_COUNT      5   /**< 费率总数            */
/** @} */

/** @defgroup TOU_Config_Limit 配置限制
  * @{
  */
#define TOU_MAX_DAILY_RULES     12   /**< 日时段表最大数量        */
#define TOU_MAX_MONTH_RULES     8    /**< 月份/季节规则最大数量   */
#define TOU_MAX_TIERS           3    /**< 阶梯电价最大档位        */
/** @} */

/* ======================== 数据结构定义 ======================== */
/** @defgroup TOU_RateFactor_Default 费率系数默认值
  * @{
  */
typedef struct
{
    float basePrice;                /**< 基准电价 (元/kWh)       */
    float topFactor;                /**< 尖时段电价系数          */
    float peakFactor;               /**< 峰时段电价系数          */
    float flatFactor;               /**< 平时段电价系数          */
    float valleyFactor;             /**< 谷时段电价系数          */
    float deepFactor;               /**< 深谷时段电价系数        */
} tou_RateFactor_t;

/**
  * @brief  日时段规则结构体
  *         定义一天的某个时间段属于哪种费率。
  *         支持跨天时段（如 22:00-06:00 谷时段），此时 endHour < startHour。
  */
typedef struct
{
    uint16_t startHour;       /**< 起始小时 (0-23)     */
    uint16_t endHour;         /**< 结束小时 (0-23)     */
    uint16_t rateIdx;         /**< 费率索引 (0=尖,1=峰,2=平,3=谷，4=深谷) */
    uint16_t dailyTableIdx;   /**< 保留字段 */
} tou_day_rule_t;/* 每时段4字节 */
/**
  * @brief  月份规则结构体
  *         定义某段日期区间（如夏季7/1~9/30）及其对应的日时段表。
  *         每个月份规则内嵌一套完整的日时段表（最多12个时段），
  *         不同规则可配置不同数量的时段，通过 slotCount 指定实际使用的时段数。
  *         例如：夏季规则含9个时段（尖/峰/平交错），春秋季规则仅6个时段（无尖峰）。
  */
typedef struct
{
    uint16_t startMonth;     /**< 起始月份 (1-12)                          */
    uint16_t startDay;       /**< 起始日期 (1-31)                          */
    uint16_t endMonth;       /**< 结束月份 (1-12)                          */
    uint16_t endDay;         /**< 结束日期 (1-31)                          */
    uint16_t slotCount;      /**< 该规则实际配置的时段数量 (0~12)           */
    uint16_t reserved;       /**< 保留对齐，确保结构体8字节对齐              */
    tou_day_rule_t dailyRules[TOU_MAX_DAILY_RULES]; /**< 日时段表，最多12个时段 */
} tou_month_rule_t;         /* 每规则 12+96=108 字节 */

/**
  * @brief  阶梯电价档位结构体
  *         根据月累计用电量分档，不同档位对应不同的电价系数。
  *         例如：第一档0~200度系数1.0，第二档200~400度系数1.1，第三档>400度系数1.3
  */
typedef struct
{
    float threshold;        /**< 月累计用电量阈值(kWh)，超过此值进入下一档 */
    float priceFactor;      /**< 电价系数（相对于基准电价）               */
} tou_tier_t;               /* 每阶梯8字节 */

/**
  * @brief  费率配置结构体（存储于Flash参数中，永久保存）
  *         包含分时计费的全部配置：月份规则、阶梯电价、费率系数等。
  * @note   每个月份规则内嵌独立日时段表，通过 slotCount 指定实际时段数。
  */
typedef struct
{
    /* ---- 基础配置(4字节) ---- */
    uint16_t  enable;                /**< 分时计费总使能: 0=关闭, 1=开启        */
    uint16_t  monthRuleCount;        /**< 月份规则实际数量 (0~7)               */
    uint16_t  tierCount;             /**< 阶梯档位实际数量 (0~3)               */
    uint16_t  reserved0;             /**< 保留字段                            */

    /* ---- 电价参数(24字节) ---- */
    tou_RateFactor_t rateFactors;

    /* ---- 阶梯电价(3档,24字节) ---- */
    tou_tier_t tiers[TOU_MAX_TIERS];

    /* ---- 月份/季节规则(8条,每条约108字节) ---- */
    tou_month_rule_t monthRules[TOU_MAX_MONTH_RULES];

} tou_config_t;
/*
 * 总大小估算:
 *   基础: 4
 *   电价参数:     6*4  = 24
 *   tiers:       3*8  = 24
 *   monthRules:  8*108 = 864
 *   ─────────────────
 *   总计: 916 字节
 */

/**
  * @brief  分时电能累计结构体（存储于Flash，掉电不丢失）
  *         每种费率分别累计正/反向有功电能和正/反向无功电能。
  */
typedef struct
{
    float EPI[TOU_RATE_COUNT];  /**< 5种费率正向有功电能 (kWh) [尖/峰/平/谷/深谷] */
    float EPE[TOU_RATE_COUNT];  /**< 5种费率反向有功电能 (kWh) [尖/峰/平/谷/深谷] */
    float EQL[TOU_RATE_COUNT];  /**< 5种费率正向无功电能 (kvarh) [尖/峰/平/谷/深谷] */
    float EQC[TOU_RATE_COUNT];  /**< 5种费率反向无功电能 (kvarh) [尖/峰/平/谷/深谷] */
} tou_energy_t; /* 总大小 = 5*4*4 = 80字节 */

/* ======================== 函数声明 ======================== */

void TOU_Init(void);
uint8_t TOU_GetCurrentRate(const rtc_datetime_t *dt);
void TOU_AccumulateEnergy(uint8_t rateIdx, float epi, float epe, float eql, float eqc);
void TOU_ClearEnergy(uint8_t rateIdx);
void TOU_SetConfig(const tou_config_t *pConfig);
void TOU_GetConfig(tou_config_t *pConfig);
void TOU_SaveConfig(void);
void TOU_SaveEnergy(void);
void TOU_LoadDefaultConfig(tou_config_t *pConfig);

/**
  * @brief  获取当前阶梯电价档位
  *         根据当月累计用电量，判断属于第几档阶梯。
  * @param  monthEnergy 当月累计正向有功电能 (kWh)
  * @retval 阶梯档位索引 (0~TOU_MAX_TIERS-1)
  */
uint8_t TOU_GetCurrentTier(float monthEnergy);

/**
  * @brief  获取当前时间对应的费率系数（含阶梯）
  *         综合费率系数 = 时段系数 × 阶梯系数
  * @param  dt          当前日期时间指针
  * @param  monthEnergy 当月累计正向有功电能 (kWh)
  * @retval 综合费率系数
  */
float TOU_GetCurrentPriceFactor(const rtc_datetime_t *dt, float monthEnergy);

/* ======================== 全局变量声明 ======================== */
extern tou_energy_t gTouEnergy;     /**< 分时电能累计全局变量 */
extern tou_config_t gTouConfig;     /**< 分时费率配置全局变量 */

#endif /* __APP_TOU_H */
