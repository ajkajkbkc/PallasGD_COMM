/**
 * @file    app_tou.c
 * @brief   分时计费(Time-of-Use)功能实现
 *          支持尖/峰/平/谷四种费率，每天最多8个时段
 *          支持多套日时段表(按月份/季节切换)、阶梯电价、
 *          居民/工商业/大工业用电区分、法定节假日判断
 * @date    2026-05-14
 *
 * @note    费率匹配逻辑：
 *          1. 先判断是否为节假日 → 使用节假日日时段表
 *          2. 否则按月份规则 → 匹配对应日时段表
 *          3. 在日时段表中遍历时段 → 匹配当前时间对应的费率
 */

/* Private includes ----------------------------------------------------------*/
#include "app_tou.h"
#include "app_parameter.h"
#include "app_log.h"
#include <string.h>

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA

/* Private define ------------------------------------------------------------*/
#if PRINT_LOG_OPEN == 1
static const char *TAG = "TOU";
#endif

/* 组合存储结构体：配置 + 电能累计 */
typedef struct
{
    tou_config_t config;    /**< 费率配置（~320字节） */
    tou_energy_t energy;    /**< 分时电能（64字节）   */
} tou_saved_data_t;         /* 总大小约 384 字节 */

/* Private variables ---------------------------------------------------------*/
tou_energy_t gTouEnergy;    /**< 分时电能累计全局变量（运行时使用）*/
tou_config_t gTouConfig;    /**< 分时费率配置全局变量（运行时使用）*/

/** 组合存储缓冲区（用于与Flash交互）*/
static tou_saved_data_t gTouSavedData;

/** Flash写入偏移量（用于Parameter_FlashWrite_InOnePage磨损均衡）*/
static uint32_t gTouFlashOffset = 0xFFFFFFFF;

/** 当月累计正向有功电能（运行时累加，用于阶梯电价判断） */
static float gMonthEPITotal = 0;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  判断给定时间是否在指定的时段范围内
  * @param  hour   当前小时 (0-23)
  * @param  min    当前分钟 (0-59)
  * @param  pSlot  时段定义指针
  * @retval true   在时段内
  * @retval false  不在时段内
  */
static bool TOU_IsInSlot(uint8_t hour, uint8_t min, const tou_day_rule_t *pSlot)
{
    uint16_t now  = (uint16_t)hour * 60 + min;                          // 当前时间(分钟)
    uint16_t start = (uint16_t)pSlot->startHour * 60; // 时段起始(分钟)
    uint16_t end   = (uint16_t)pSlot->endHour * 60;     // 时段结束(分钟)

    if (start == end)
    {
        /* 起始=结束表示全天时段 */
        return true;
    }
    else if (start < end)
    {
        /* 普通时段：不跨天 */
        return (now >= start && now < end);
    }
    else
    {
        /* 跨天时段：如 22:00-06:00 */
        return (now >= start || now < end);
    }
}

/* ======================== 公共函数实现 ======================== */

/**
  * @brief  分时计费初始化
  *         从Flash加载费率配置和分时电能累计值。
  *         如果是首次运行（Flash全0xFF），加载默认的时段配置。
  *         初始化完成后，gTouConfig和gTouEnergy即包含最新数据。
  */
void TOU_Init(void)
{
    uint8_t i;

    /* 清零分时电能累计（初始值） */
    memset(&gTouEnergy, 0, sizeof(gTouEnergy));
    gMonthEPITotal = 0;

    /* 初始化Flash写入偏移（首次写入时由Parameter_FindOffset_InOnePage自动确定）*/
    gTouFlashOffset = 0xFFFFFFFF;

    /* 从Flash读取组合存储数据 */
    Parameter_FlashRead(TOU_FLASH_SAVE_ADDR, &gTouSavedData, sizeof(gTouSavedData));

    /* 检查配置是否有效（使能标志不为0xFF且月份规则数量合法即认为有效） */
    if (gTouSavedData.config.enable == 0xFF ||
        gTouSavedData.config.monthRuleCount == 0 ||
        gTouSavedData.config.monthRuleCount > TOU_MAX_MONTH_RULES)
    {
        /* Flash未初始化或数据损坏，加载默认配置 */
        TOU_LoadDefaultConfig(&gTouConfig);
        LOGI(TAG, "TOU: Using default config (Flash not initialized)");
    }
    else
    {
        /* 从Flash恢复配置到运行时变量 */
        memcpy(&gTouConfig, &gTouSavedData.config, sizeof(gTouConfig));
    }

    /* 从Flash恢复分时电能累计值 */
    memcpy(&gTouEnergy, &gTouSavedData.energy, sizeof(gTouEnergy));

    /* 检查电能数据有效性（若全为0xFF说明未初始化）*/
    {
        uint8_t *p = (uint8_t *)&gTouEnergy;
        uint8_t allFF = 1;
        for (i = 0; i < sizeof(gTouEnergy); i++)
        {
            if (p[i] != 0xFF) { allFF = 0; break; }
        }
        if (allFF)
        {
            memset(&gTouEnergy, 0, sizeof(gTouEnergy));
        }
        else
        {
            /* 计算当月累计正向有功电能（所有费率之和，用于阶梯电价判断） */
            for (i = 0; i < TOU_RATE_COUNT; i++)
            {
                gMonthEPITotal += gTouEnergy.EPI[i];
            }
        }
    }

    LOGI(TAG, "TOU: Init done, enable=%d, monthRules=%d, tiers=%d",
         gTouConfig.enable, gTouConfig.monthRuleCount, gTouConfig.tierCount);
}

/**
  * @brief  根据月份规则匹配当前的日时段表索引
  *         遍历所有月份规则，找到第一个覆盖当前日期的规则。
  *         支持跨年规则（如 10/1 ~ 次年 3/31 冬季规则）。
  * @param  month 当前月份 (1-12)
  * @param  day   当前日期 (1-31)
  * @retval 日时段表索引 (0~3)，未匹配返回0
  */
static uint8_t TOU_FindMonthRule(uint8_t month, uint8_t day)
{
    uint8_t r;
    uint16_t nowDays, startDays, endDays;

    /* 将日期转为年内天数(每月30天近似)，用于比较 */
    nowDays   = (uint16_t)(month - 1) * 30 + day;
    for (r = 0; r < gTouConfig.monthRuleCount; r++)
    {
        startDays = (uint16_t)(gTouConfig.monthRules[r].startMonth - 1) * 30
                    + gTouConfig.monthRules[r].startDay;
        endDays   = (uint16_t)(gTouConfig.monthRules[r].endMonth - 1) * 30
                    + gTouConfig.monthRules[r].endDay;

        if (startDays <= endDays)
        {
            /* 普通规则：不跨年 */
            if (nowDays >= startDays && nowDays <= endDays)
                return r;
        }
        else
        {
            /* 跨年规则：如 10/1 ~ 次年 3/31 */
            if (nowDays >= startDays || nowDays <= endDays)
                return r;
        }
    }
    /* 未匹配，返回第一个日时段表 */
    return 0;
}

/**
  * @brief  获取当前时间对应的费率索引
  *
  *         匹配顺序：
  *         1. 分时计费未使能 → 返回平值(TOU_RATE_FLAT_IDX)
  *         2. 判断节假日 → 如果是节假日，视为谷时段(TOU_RATE_VALLEY_IDX)
  *         3. 按月份规则匹配 → 找到当前日期对应的月份规则
  *         4. 在该规则的日时段表中遍历，找到当前时间所在的时段 → 返回该时段费率
  *         5. 未匹配任何时段 → 返回平值(TOU_RATE_FLAT_IDX)
  * @param  dt 当前日期时间结构体指针（不可为NULL）
  * @retval 费率索引 (TOU_RATE_TOP_IDX ~ TOU_RATE_DEEP_IDX)
  */
uint8_t TOU_GetCurrentRate(const rtc_datetime_t *dt)
{
    uint8_t i;
    uint8_t dailyTableIdx;                /* 当前生效的月份规则索引 */
    uint8_t slotCount;                    /* 当前规则实际配置的时段数量 */

    if (dt == NULL)
    {
        return TOU_RATE_FLAT_IDX;
    }

    /* 1. 分时计费未使能 → 返回平值 */
    if (!gTouConfig.enable)
    {
        return TOU_RATE_FLAT_IDX;
    }

    /* 2. 按月份规则匹配日期 → 获取对应的月份规则索引 */
    if (gTouConfig.monthRuleCount == 0)
    {
        return TOU_RATE_FLAT_IDX;
    }
    dailyTableIdx = TOU_FindMonthRule(dt->month, dt->day);

    /* 3. 在当前月份规则的日时段表中遍历，匹配当前时间 */
    slotCount = gTouConfig.monthRules[dailyTableIdx].slotCount;
    if (slotCount == 0 || slotCount > TOU_MAX_DAILY_RULES)
    {
        /* slotCount 非法，回退为平值 */
        return TOU_RATE_FLAT_IDX;
    }

    for (i = 0; i < slotCount; i++)
    {
        if (TOU_IsInSlot(dt->hour, dt->minute,
            &gTouConfig.monthRules[dailyTableIdx].dailyRules[i]))
        {
            return gTouConfig.monthRules[dailyTableIdx].dailyRules[i].rateIdx;
        }
    }

    /* 4. 未匹配任何时段 → 返回平值 */
    LOGW(TAG, "TOU: No slot matched for %02d:%02d, fallback to flat",
         dt->hour, dt->minute);
    return TOU_RATE_FLAT_IDX;
}

/**
  * @brief  获取当前阶梯电价档位
  *         根据当月累计正向有功电能，判断属于第几档。
  *         第一档(0): monthEnergy ≤ tiers[0].threshold
  *         第二档(1): tiers[0].threshold < monthEnergy ≤ tiers[1].threshold
  *         第三档(2): monthEnergy > tiers[1].threshold
  */
uint8_t TOU_GetCurrentTier(float monthEnergy)
{
    uint8_t tierIdx;
    if (gTouConfig.tierCount <= 1) return 0;
    for (tierIdx = 0; tierIdx < gTouConfig.tierCount; tierIdx++)
    {
        if (monthEnergy <= gTouConfig.tiers[tierIdx].threshold)
            return tierIdx;
    }
    return gTouConfig.tierCount - 1;
}

/**
  * @brief  获取综合费率系数（含时段系数 × 阶梯系数）
  */
float TOU_GetCurrentPriceFactor(const rtc_datetime_t *dt, float monthEnergy)
{
    float factor;
    uint8_t rateIdx = TOU_GetCurrentRate(dt);

    /* 时段费率系数 */
    switch (rateIdx)
    {
        case TOU_RATE_TOP_IDX:    factor = gTouConfig.rateFactors.topFactor;    break;
        case TOU_RATE_PEAK_IDX:   factor = gTouConfig.rateFactors.peakFactor;   break;
        case TOU_RATE_FLAT_IDX:   factor = gTouConfig.rateFactors.flatFactor;   break;
        case TOU_RATE_VALLEY_IDX: factor = gTouConfig.rateFactors.valleyFactor; break;
        case TOU_RATE_DEEP_IDX:   factor = gTouConfig.rateFactors.deepFactor;   break;
        default:                  factor = 1.0f;                    break;
    }

    /* 阶梯电价系数 */
    if (gTouConfig.tierCount > 1)
    {
        uint8_t tier = TOU_GetCurrentTier(monthEnergy);
        factor *= gTouConfig.tiers[tier].priceFactor;
    }

    return factor;
}

/**
  * @brief  累计分时电能
  *         将本周期电能增量按当前费率累加到对应的费率桶中，
  *         同时更新当月累计正向有功电能（用于阶梯电价判断）。
  */
void TOU_AccumulateEnergy(uint8_t rateIdx, float epi, float epe, float eql, float eqc)
{
    if (rateIdx >= TOU_RATE_COUNT) return;

    gTouEnergy.EPI[rateIdx] += epi;
    gTouEnergy.EPE[rateIdx] += epe;
    gTouEnergy.EQL[rateIdx] += eql;
    gTouEnergy.EQC[rateIdx] += eqc;

    /* 更新当月累计正向有功电能（用于阶梯电价判断） */
    gMonthEPITotal += epi;
}

/**
  * @brief  清除指定费率的所有电能累计值
  */
void TOU_ClearEnergy(uint8_t rateIdx)
{
    uint8_t i;

    if (rateIdx == 0xFF)
    {
        /* 清除所有费率 */
        for (i = 0; i < TOU_RATE_COUNT; i++)
        {
            gTouEnergy.EPI[i] = 0;
            gTouEnergy.EPE[i] = 0;
            gTouEnergy.EQL[i] = 0;
            gTouEnergy.EQC[i] = 0;
        }
        LOGW(TAG, "TOU: Clear all rates energy");
    }
    else if (rateIdx < TOU_RATE_COUNT)
    {
        /* 清除指定费率 */
        gTouEnergy.EPI[rateIdx] = 0;
        gTouEnergy.EPE[rateIdx] = 0;
        gTouEnergy.EQL[rateIdx] = 0;
        gTouEnergy.EQC[rateIdx] = 0;
        LOGW(TAG, "TOU: Clear rate[%d] energy", rateIdx);
    }

    /* 立即保存到Flash */
    TOU_SaveEnergy();
}

/**
  * @brief  设置新的费率时段配置
  *         将外部配置复制到运行时变量并保存到Flash。
  * @param  pConfig 新配置指针（不可为NULL）
  */
void TOU_SetConfig(const tou_config_t *pConfig)
{
    if (pConfig == NULL) return;

    memcpy(&gTouConfig, pConfig, sizeof(gTouConfig));

    /* 参数范围约束：防止非法值溢出 */
    if (gTouConfig.monthRuleCount > TOU_MAX_MONTH_RULES) gTouConfig.monthRuleCount = TOU_MAX_MONTH_RULES;
    if (gTouConfig.tierCount      > TOU_MAX_TIERS)       gTouConfig.tierCount      = TOU_MAX_TIERS;

    /* 遍历每个月份规则，约束 slotCount */
    {
        uint8_t r;
        for (r = 0; r < gTouConfig.monthRuleCount; r++)
        {
            if (gTouConfig.monthRules[r].slotCount > TOU_MAX_DAILY_RULES)
            {
                gTouConfig.monthRules[r].slotCount = TOU_MAX_DAILY_RULES;
            }
        }
    }

    TOU_SaveConfig();
    LOGI(TAG, "TOU: Config updated, monthRules=%d, tiers=%d",
         gTouConfig.monthRuleCount, gTouConfig.tierCount);
}

/**
  * @brief  获取当前费率配置
  */
void TOU_GetConfig(tou_config_t *pConfig)
{
    if (pConfig == NULL)
    {
        return;
    }

    memcpy(pConfig, &gTouConfig, sizeof(gTouConfig));
}

/**
  * @brief  将运行时配置和电能数据打包写入Flash（公共存储函数）
  *         使用 Parameter_FlashWrite_InOnePage 单页增量写入，实现磨损均衡。
  *         配置变更时调用 TOU_SaveConfig()，电能变更时调用 TOU_SaveEnergy()，
  *         两者均通过本函数完成实际Flash写入。
  */
static void TOU_SaveToFlash(void)
{
    /* 将当前运行时配置和电能数据同步到组合存储缓冲区 */
    memcpy(&gTouSavedData.config, &gTouConfig, sizeof(gTouConfig));
    memcpy(&gTouSavedData.energy, &gTouEnergy, sizeof(gTouEnergy));

    /* 单页增量写入Flash，写满后自动擦除重写（磨损均衡） */
    Parameter_FlashWrite_InOnePage(TOU_FLASH_SAVE_ADDR, sizeof(gTouSavedData),
                                   &gTouSavedData, &gTouFlashOffset);
}

/**
  * @brief  保存费率配置到Flash（对外接口）
  *         通常在配置变更时（如远程下发新时段表）调用。
  */
void TOU_SaveConfig(void)
{
    TOU_SaveToFlash();
}

/**
  * @brief  保存分时电能到Flash（对外接口）
  *         每约10秒调用一次（与 gMeterEnergy 同步写入），防止掉电丢失。
  */
void TOU_SaveEnergy(void)
{
    TOU_SaveToFlash();
}

/**
  * @brief  加载默认费率时段表（首次运行或Flash数据损坏时使用）
  *
  * @note    默认配置为：居民用电，3条月份规则，3档阶梯电价
  *
  *          【月份规则覆盖】
  *           规则0: 1/1~6/30（春/冬季标准表，6个时段，峰/平/谷，无尖峰）
  *           规则1: 7/1~9/30（夏季尖峰表，9个时段，尖/峰/平/谷）
  *           规则2: 10/1~12/31（秋季标准表，6个时段，峰/平/谷，无尖峰）
  *
  *          【各规则时段详情】
  *           规则0 (标准): 00-08谷, 08-10平, 10-12峰, 12-14平, 14-19峰, 19-24平
  *           规则1 (夏季): 00-08谷, 08-10平, 10-11峰, 11-12尖, 12-14平,
  *                         14-15峰, 15-17尖, 17-19峰, 19-24平
  *           规则2 (秋季): 同规则0
  *
  *          【阶梯电价3档（居民用电标准）】
  *           第一档 0~200度    系数1.0
  *           第二档 200~400度  系数1.1
  *           第三档 >400度     系数1.3
  *
  *          【电价系数】
  *           基准电价 0.5283元/kWh
  *           尖×2.0, 峰×1.5, 平×1.0, 谷×0.5, 深谷×0.3
  *
  *          【法定节假日（由 TOU_IsHoliday 判断，优先级高于月份规则）】
  *           元旦(1/1)、春节(农历~1月底2月初)、清明(4/4~4/6)、
  *           劳动节(5/1~5/3)、端午(农历5月初)、中秋(农历8月15)、
  *           国庆(10/1~10/7) → 节假日当天视为谷时段(TOU_RATE_VALLEY_IDX)
  */
void TOU_LoadDefaultConfig(tou_config_t *pConfig)
{
    if (pConfig == NULL) return;

    memset(pConfig, 0, sizeof(tou_config_t));

    /* ====== 基础配置 ====== */
    pConfig->enable         = 1;    /**< 默认使能分时计费           */
    pConfig->monthRuleCount = 3;    /**< 3条月份规则(春夏秋/冬)     */
    pConfig->tierCount      = 3;    /**< 3档阶梯电价               */
    pConfig->reserved0      = 0;    /**< 3档阶梯电价               */

    /* ====== 电价系数（居民用电基准 0.5283元/kWh） ====== */
    pConfig->rateFactors.basePrice    = 0.5283f;
    pConfig->rateFactors.topFactor    = 0.5283f * 2.0f;  /**< 尖: 基准×2.0 */
    pConfig->rateFactors.peakFactor   = 0.5283f * 1.5f;  /**< 峰: 基准×1.5 */
    pConfig->rateFactors.flatFactor   = 0.5283f * 1.0f;  /**< 平: 基准×1.0 */
    pConfig->rateFactors.valleyFactor = 0.5283f * 0.5f;  /**< 谷: 基准×0.5 */
    pConfig->rateFactors.deepFactor   = 0.5283f * 0.3f;  /**< 深谷:基准×0.3 */

    /* ====== 阶梯电价3档（居民用电标准） ====== */
    pConfig->tiers[0].threshold   = 200.0f;     /**< 第一档: 0~200度    */
    pConfig->tiers[0].priceFactor = 1.0f;       /**< 电价系数 1.0       */
    pConfig->tiers[1].threshold   = 400.0f;     /**< 第二档: 200~400度  */
    pConfig->tiers[1].priceFactor = 1.1f;       /**< 电价系数 1.1       */
    pConfig->tiers[2].threshold   = 999999.0f;  /**< 第三档: >400度     */
    pConfig->tiers[2].priceFactor = 1.3f;       /**< 电价系数 1.3       */

    /* =================================================================== *
     * 规则0: 1/1 ~ 6/30 — 春/冬季标准表（6个时段：谷/平/峰/平/峰/平）    *
     * 无尖峰时段，适用于气温较低、空调负荷不高的季节                        *
     * =================================================================== */
    pConfig->monthRules[0].startMonth = 1;
    pConfig->monthRules[0].startDay   = 1;
    pConfig->monthRules[0].endMonth   = 6;
    pConfig->monthRules[0].endDay     = 30;
    pConfig->monthRules[0].slotCount  = 6;

    pConfig->monthRules[0].dailyRules[0].startHour = 0;
    pConfig->monthRules[0].dailyRules[0].endHour   = 8;
    pConfig->monthRules[0].dailyRules[0].rateIdx   = TOU_RATE_VALLEY_IDX;  /* 00:00-08:00 谷 */

    pConfig->monthRules[0].dailyRules[1].startHour = 8;
    pConfig->monthRules[0].dailyRules[1].endHour   = 10;
    pConfig->monthRules[0].dailyRules[1].rateIdx   = TOU_RATE_FLAT_IDX;    /* 08:00-10:00 平 */

    pConfig->monthRules[0].dailyRules[2].startHour = 10;
    pConfig->monthRules[0].dailyRules[2].endHour   = 12;
    pConfig->monthRules[0].dailyRules[2].rateIdx   = TOU_RATE_PEAK_IDX;   /* 10:00-12:00 峰 */

    pConfig->monthRules[0].dailyRules[3].startHour = 12;
    pConfig->monthRules[0].dailyRules[3].endHour   = 14;
    pConfig->monthRules[0].dailyRules[3].rateIdx   = TOU_RATE_FLAT_IDX;    /* 12:00-14:00 平 */

    pConfig->monthRules[0].dailyRules[4].startHour = 14;
    pConfig->monthRules[0].dailyRules[4].endHour   = 19;
    pConfig->monthRules[0].dailyRules[4].rateIdx   = TOU_RATE_PEAK_IDX;   /* 14:00-19:00 峰 */

    pConfig->monthRules[0].dailyRules[5].startHour = 19;
    pConfig->monthRules[0].dailyRules[5].endHour   = 24;
    pConfig->monthRules[0].dailyRules[5].rateIdx   = TOU_RATE_FLAT_IDX;    /* 19:00-24:00 平 */

    /* =================================================================== *
     * 规则1: 7/1 ~ 9/30 — 夏季尖峰表（9个时段：谷/平/峰/尖/平/峰/尖/峰/平） *
     * 夏季空调负荷大，增加尖峰时段(11-12, 15-17)，电价最高                  *
     * =================================================================== */
    pConfig->monthRules[1].startMonth = 7;
    pConfig->monthRules[1].startDay   = 1;
    pConfig->monthRules[1].endMonth   = 9;
    pConfig->monthRules[1].endDay     = 30;
    pConfig->monthRules[1].slotCount  = 9;

    pConfig->monthRules[1].dailyRules[0].startHour = 0;
    pConfig->monthRules[1].dailyRules[0].endHour   = 8;
    pConfig->monthRules[1].dailyRules[0].rateIdx   = TOU_RATE_VALLEY_IDX;  /* 00:00-08:00 谷 */

    pConfig->monthRules[1].dailyRules[1].startHour = 8;
    pConfig->monthRules[1].dailyRules[1].endHour   = 10;
    pConfig->monthRules[1].dailyRules[1].rateIdx   = TOU_RATE_FLAT_IDX;    /* 08:00-10:00 平 */

    pConfig->monthRules[1].dailyRules[2].startHour = 10;
    pConfig->monthRules[1].dailyRules[2].endHour   = 11;
    pConfig->monthRules[1].dailyRules[2].rateIdx   = TOU_RATE_PEAK_IDX;   /* 10:00-11:00 峰 */

    pConfig->monthRules[1].dailyRules[3].startHour = 11;
    pConfig->monthRules[1].dailyRules[3].endHour   = 12;
    pConfig->monthRules[1].dailyRules[3].rateIdx   = TOU_RATE_TOP_IDX;    /* 11:00-12:00 尖 */

    pConfig->monthRules[1].dailyRules[4].startHour = 12;
    pConfig->monthRules[1].dailyRules[4].endHour   = 14;
    pConfig->monthRules[1].dailyRules[4].rateIdx   = TOU_RATE_FLAT_IDX;    /* 12:00-14:00 平 */

    pConfig->monthRules[1].dailyRules[5].startHour = 14;
    pConfig->monthRules[1].dailyRules[5].endHour   = 15;
    pConfig->monthRules[1].dailyRules[5].rateIdx   = TOU_RATE_PEAK_IDX;   /* 14:00-15:00 峰 */

    pConfig->monthRules[1].dailyRules[6].startHour = 15;
    pConfig->monthRules[1].dailyRules[6].endHour   = 17;
    pConfig->monthRules[1].dailyRules[6].rateIdx   = TOU_RATE_TOP_IDX;    /* 15:00-17:00 尖 */

    pConfig->monthRules[1].dailyRules[7].startHour = 17;
    pConfig->monthRules[1].dailyRules[7].endHour   = 19;
    pConfig->monthRules[1].dailyRules[7].rateIdx   = TOU_RATE_PEAK_IDX;   /* 17:00-19:00 峰 */

    pConfig->monthRules[1].dailyRules[8].startHour = 19;
    pConfig->monthRules[1].dailyRules[8].endHour   = 24;
    pConfig->monthRules[1].dailyRules[8].rateIdx   = TOU_RATE_FLAT_IDX;    /* 19:00-24:00 平 */

    /* =================================================================== *
     * 规则2: 10/1 ~ 12/31 — 秋季标准表（6个时段，同规则0）                  *
     * =================================================================== */
    pConfig->monthRules[2].startMonth = 10;
    pConfig->monthRules[2].startDay   = 1;
    pConfig->monthRules[2].endMonth   = 12;
    pConfig->monthRules[2].endDay     = 31;
    pConfig->monthRules[2].slotCount  = 6;

    pConfig->monthRules[2].dailyRules[0].startHour = 0;
    pConfig->monthRules[2].dailyRules[0].endHour   = 8;
    pConfig->monthRules[2].dailyRules[0].rateIdx   = TOU_RATE_VALLEY_IDX;  /* 00:00-08:00 谷 */

    pConfig->monthRules[2].dailyRules[1].startHour = 8;
    pConfig->monthRules[2].dailyRules[1].endHour   = 10;
    pConfig->monthRules[2].dailyRules[1].rateIdx   = TOU_RATE_FLAT_IDX;    /* 08:00-10:00 平 */

    pConfig->monthRules[2].dailyRules[2].startHour = 10;
    pConfig->monthRules[2].dailyRules[2].endHour   = 12;
    pConfig->monthRules[2].dailyRules[2].rateIdx   = TOU_RATE_PEAK_IDX;   /* 10:00-12:00 峰 */

    pConfig->monthRules[2].dailyRules[3].startHour = 12;
    pConfig->monthRules[2].dailyRules[3].endHour   = 14;
    pConfig->monthRules[2].dailyRules[3].rateIdx   = TOU_RATE_FLAT_IDX;    /* 12:00-14:00 平 */

    pConfig->monthRules[2].dailyRules[4].startHour = 14;
    pConfig->monthRules[2].dailyRules[4].endHour   = 19;
    pConfig->monthRules[2].dailyRules[4].rateIdx   = TOU_RATE_PEAK_IDX;   /* 14:00-19:00 峰 */

    pConfig->monthRules[2].dailyRules[5].startHour = 19;
    pConfig->monthRules[2].dailyRules[5].endHour   = 24;
    pConfig->monthRules[2].dailyRules[5].rateIdx   = TOU_RATE_FLAT_IDX;    /* 19:00-24:00 平 */

    pConfig->monthRules[3].startMonth = 3;
    pConfig->monthRules[3].startDay   = 3;

    pConfig->monthRules[4].startMonth = 4;
    pConfig->monthRules[4].startDay   = 4;
    
    pConfig->monthRules[5].startMonth = 5;
    pConfig->monthRules[5].startDay   = 5;

    pConfig->monthRules[6].startMonth = 6;
    pConfig->monthRules[6].startDay   = 6;

    pConfig->monthRules[7].startMonth = 7;
    pConfig->monthRules[7].startDay   = 7;

}

#endif /* PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA */
