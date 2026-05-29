
/* Private includes ----------------------------------------------------------*/
#include "cmsis_os.h"

#include "app_log.h"
#include "app_parameter.h"
#include "app_version.h"
#include "app_collect.h"
#include "app_tm1650.h"
#include "app_etcr2900.h"
#include "app_att7022eu.h"

#include "hzudp.h"

#include "module_ESE.h"
/* Private define ------------------------------------------------------------*/
#if PRINT_LOG_OPEN == 1
static const char *TAG = "parameter";
#endif


/* Private variables ---------------------------------------------------------*/
flash_param_t gFlashParam;
//volatile param_t gParam;
__IO param_t gParam;

//PLCList_st gplcList;
//LtList_st gLtList;
//uint16_t gLtElemIdx; //雷电流元素索引

//FS_Elem_st  gFS_Elem;
//FSS_Elem_st gFSS_Elem;
/* Private function prototypes -----------------------------------------------*/
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
flash_offset_t gflashOffset;
#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB || PROD_TYPE == PROD_ESA */



/* Private user code ---------------------------------------------------------*/
/* Flash operation -------------------------------------------------------- */
/**
  * @brief  获取长度，u8单位转换为u32单位，如len_char=5，则len_int=2
  * @param  len_char 数据长度(unit:8-bit)
  * @retval len_int  数据长度(unit:32-bit)
  */
static uint32_t getLen_char2int(uint32_t len_char)
{
    uint32_t len_int;

    if(len_char % 4)
    {
        len_int = len_char / 4 + 1;
    }
    else
    {
        len_int = len_char / 4;
    }

    return len_int;
}
/**
  * @brief  擦除页
  * @param  pageAddress 起始地址
  * @param  pageNum 页数
  * @retval None
  */
void BSP_erase_page(uint32_t pageAddress, uint16_t pageNum)
{
    FLASH_EraseInitTypeDef eit =
    {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .PageAddress = pageAddress,
        .NbPages = pageNum
    };
    uint32_t pageErr = 0;
    HAL_FLASHEx_Erase(&eit, &pageErr);
//    LOGI("parameter", "Erase Address is 0x%04x", (uint32_t)pageAddress);
//    LOGD("parameter", "Leave %s(), pageErr = 0x%04X, pageErr = 0xFFFFFFFF is sucess to erase page", __func__, pageErr);

}

/**
  * @brief  数据写入Flash
  * @param  addr 存入flash的地址
  * @param  *pBuf 指向数据的指针
  * @param  len 数据长度(unit: 8-bit)
  * @retval None
  */
void Parameter_FlashWrite(uint32_t addr, void *pBuf, uint32_t len)
{
    //printf("Enter %s(), startAddr=0x%08X, pBuff = 0x%08X, size=%u\r\n", __func__, addr, pBuf, len);
    uint32_t *pData = pBuf;
    uint32_t length;  //get length(uint: 32-bit)
    uint16_t pageNum;

    if(len % 4)
    {
        length = len / 4 + 1;
    }
    else
    {
        length = len / 4;
    }

    HAL_FLASH_Unlock();

    //erase page
    if(length % FLASH_ONEPAGE_WORDSIZE == 0)
    {
        pageNum = length / FLASH_ONEPAGE_WORDSIZE; //get number of pages
    }
    else
    {
        pageNum = length / FLASH_ONEPAGE_WORDSIZE + 1;
    }
    BSP_erase_page(addr, pageNum);

    //flash program
    for(uint32_t i = 0; i < length; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, pData[i]);
        addr += 4;
    }

    HAL_FLASH_Lock();
}

/**
  * @brief  取Flash数据
  * @param  addr flash的地址
  * @param  *pBuf 指向取出数据的指针
  * @param  len 数据长度(unit: 8-bit)
  * @retval None
  */
void Parameter_FlashRead(uint32_t addr, void *pBuf, uint32_t len)
{
    uint32_t *pData = pBuf;
    uint32_t length;

    //get length(uint: 32-bit)
    if(len % 4)
    {
        length = len / 4 + 1;
    }
    else
    {
        length = len / 4;
    }

    for(uint32_t i = 0; i < length; i++)
    {
        pData[i] = *((uint32_t *)addr);
        addr += 4;
    }
}

/**
 * STM32F103RC的page大小为2K
 *
 * 因为最小erase单元是2K，所以为了充分利用资源，当更新一个变量值时，
 * 不是每次都erase一个2K，而是当这个2K区域都写操作一次之后，再erase，
 * 从而再重新使用这个2K区域。
 * 这个2K区域命名为SECTOR_ID。
 *
 * 每个2K区域，根据所存变量的大小，划分为同等大小的block，需4字节对齐。
 * 当为了存储0xFFFFFFFF这样的特殊变量，最小的block大小应为8字节，每个变量
 * 头4个字节定义为0xABABABAB

 * 如果不需要存储0xFFFFFFFF这样的特殊变量，则block最小可为4字节。
*/
/**
  * @brief  Flash一页内，寻找没有初始化的偏移值
  * @param  addr 存入flash的地址
  * @param  len 数据长度4字节对齐(unit: 8-bit)
  * @param  offset 获取偏移值
  * @retval 0: 没有空片子偏移(这一页用满了)
            1: 有空片子来偏移
  */
static uint8_t Parameter_FindOffset_InOnePage(unsigned int addr, unsigned int len, unsigned int *offset)
{
    uint8_t emptyflg;  //1:找到空片子
    uint32_t i, j;
    uint32_t len_int;
    uint32_t *paddr = (uint32_t *)addr;

    LOGI(TAG, "len_char=%d", len);
    len_int = getLen_char2int(len);
    len = len_int * 4;
    LOGI(TAG, "len_char=%d, len_int=%d", len, len_int);

    for(i = 0; i < FLASH_ONEPAGE_WORDSIZE / len_int; i++)
    {
        emptyflg = 1;
        for(j = 0; j < len_int; j++)
        {
            if(*(paddr + j) != 0xFFFFFFFF)
            {
                emptyflg = 0;
                break;
            }
        }
        if(emptyflg == 1)
        {
            break;
        }
        paddr = paddr + len_int;
    }
    *offset = i * len;

    return emptyflg;
}

/**
  * @brief  数据写入Flash一页，写入多次直到写满一页后再擦除重新写
  * @param  addr 存入flash的地址
  * @param  *pBuf 指向数据的指针
  * @param  len 数据长度4字节对齐(unit: 8-bit)
  * @param  offset 存储偏移，上电前硬初始化位0xFFFFFFFF
  * @retval None
  */
void Parameter_FlashWrite_InOnePage(unsigned int addr, unsigned int len, void *pBuf, unsigned int *offset)
{
    uint32_t i;
    uint32_t len_int;
    uint32_t *pData = pBuf;

    if(*offset == 0xFFFFFFFF)
    {
        Parameter_FindOffset_InOnePage(addr, len, offset);
    }
//    LOGI(TAG, "write flash offset=%d", *offset);

    len_int = getLen_char2int(len);
    len = len_int * 4;

    HAL_FLASH_Unlock();
    if(*offset + len >= FLASH_ONEPAGE_BYTESIZE)
    {
        LOGW(TAG, "erase all, start over!!!");
        BSP_erase_page(addr, 1);
        *offset = 0;
    }

    addr += *offset;
    /* flash program */
    for(i = 0; i < len_int; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, pData[i]);
        addr += 4;
    }
    HAL_FLASH_Lock();

    *offset += len;
}

/**
  * @brief  数据读取Flash
  * @param  addr 存入flash的地址
  * @param  *pBuf 指向数据的指针
  * @param  len 数据长度4字节对齐(unit: 8-bit)
  * @retval 0: ok
            1: err
  */
void Parameter_FlashRead_InOnePage(unsigned int addr, unsigned int len, void *pBuf, unsigned int *offset)
{
    uint32_t i;
    uint32_t len_int;
    uint32_t *pData = pBuf;
    uint32_t offset_read;

    if(*offset == 0xFFFFFFFF)
    {
        Parameter_FindOffset_InOnePage(addr, len, offset);
    }
    LOGD(TAG, "read flash offset=%d", *offset);

    len_int = getLen_char2int(len);
    len = len_int * 4;

    offset_read = *offset == 0 ? *offset : *offset - len;
    addr += offset_read;
    for(i = 0; i < len_int; i++)
    {
        pData[i] = *((uint32_t *)addr);
        addr += 4;
    }
}


/**
  * @brief  打印flash地址所存内容
  * @param  addr：打印flash起始地址
  * @param  len：打印的长度(unit: byte)
  * @retval 结束地址
  */
uint32_t printf_Flash(uint32_t addr, uint32_t len)
{
    uint32_t *paddr = (uint32_t *)addr;

    len /= 4;
    for(uint32_t i = 0; i < len; i++)
    {
        paddr = (uint32_t *)(addr + 4 * i);
        LOGI("parameter", "address = 0x%08x, Value = 0x%08x", (uint32_t)paddr, *paddr);
    }
    LOGI("", "\r\n\r\n");
    return (uint32_t)paddr;
}
/* End of Flash operation ------------------------------------------------- */


/* Thunder current monitor ------------------------------------------------ */
/**
  * @brief  初始化线性表
  * @param  表(LtList_st LtList)
  * @retval None
  */
//void LtList_Init(LtList_st *LtList)
//{
//    LtList->st.length = 0;
//    for(uint16_t i = 0; i < LT_MAX_NUM; i++)
//    {
//        memset(&LtList->st.LtData[i], 0, sizeof(LtElem_st));
//        //PrintLtElem(LtList->LtData[i]);
//    }
//}

/**
  * @brief  判断线性表是否为空
  * @param  表(LtList_st LtList)
  * @retval true/false
  */
//bool LtListEmpty(LtList_st LtList)
//{
//    return (LtList.st.length == 0);
//}

/**
  * @brief  求表长
  * @param  表(LtList_st LtList)
  * @retval 表长
  */
//uint16_t LtListLength(LtList_st LtList)
//{
//    return LtList.st.length;
//}

/**
  * @brief  输出雷击元素
  * @param  雷击元素(LtElem_st LtElem)
  * @retval None
  */
//void PrintLtElem(LtElem_st LtElem)
//{
//    LOGI("thunder_parameter", "Peak: %d, Polar: %d, Keeptime: %d, Time: %d-%d-%d %d:%d:%d, Q: %d, W/R: %d", LtElem.Peak, LtElem.Polar, LtElem.KeepTime,
//         LtElem.Year, LtElem.Month, LtElem.Day, LtElem.Hour, LtElem.Minute, LtElem.Second, LtElem.usQ, LtElem.usWR);
//}

/**
  * @brief  输出线性表
  * @param  表(LtList_st LtList)
  * @retval None
  */
//void PrintLtList(LtList_st LtList)
//{
//    for(uint16_t i = 0; i < LtList.st.length; i++)
//    {
//        LOGI("thunder_parameter", "LtNum: %d, Peak: %d, Polar: %d, Keeptime: %d, Time: %d-%d-%d %d:%d:%d, Q: %d, W/R: %d", i + 1, LtList.st.LtData[i].Peak, LtList.st.LtData[i].Polar, LtList.st.LtData[i].KeepTime
//             , LtList.st.LtData[i].Year, LtList.st.LtData[i].Month, LtList.st.LtData[i].Day, LtList.st.LtData[i].Hour, LtList.st.LtData[i].Minute, LtList.st.LtData[i].Second, LtList.st.LtData[i].usQ, LtList.st.LtData[i].usWR);
//    }
//    LOGI("", "\r\n");
//}

/**
  * @brief  取表中第几个雷击的信息
  * @param  表(LtList_st LtList)
  * @param  查询第i个位置(uint16_t i)
  * @param  返回该次雷击元素(LtElem_st LtElem)
  * @retval true/false
  */
//bool GetLtElem(LtList_st LtList, uint16_t i, LtElem_st *LtElem)
//{
//    if(i < 1 || i > LtList.st.length)
//    {
//        return false;
//    }
//    *LtElem = LtList.st.LtData[i - 1];
//    return true;
//}

/**
  * @brief  删除数据元素
  * @param  表(LtList_st LtList)
  * @param  第i个位置删除(uint16_t i)
  * @param  删除的元素(LtElem_st LtElem)
  * @retval true/false
  */
//bool LtListDelete(LtList_st *LtList, uint16_t i, LtElem_st *LtElem)
//{
//    uint16_t j;
//    if(i < 1 || i > LtList->st.length)
//    {
//        return false;
//    }
//    i--;
//    *LtElem = LtList->st.LtData[i];
//    for(j = i; j < LtList->st.length; j++)
//    {
//        LtList->st.LtData[j] = LtList->st.LtData[j + 1];
//    }
//    LtList->st.length --;
//    return true;
//}

/**
  * @brief  插入数据元素
  * @param  表(LtList_st LtList)
  * @param  第i个位置插入(uint16_t i)
  * @param  插入的新元素(LtElem_st LtElem)
  * @retval true/false
  */
//bool LtListInsert(LtList_st *LtList, uint16_t i, LtElem_st LtElem)
//{
//    LtElem_st LtElemTemp;
//    uint16_t j;
//    if(i < 1 || i > LtList->st.length)
//    {
//        return false;
//    }
//    else if(LtList->st.length == LT_MAX_NUM)
//    {
//        LtListDelete(LtList, 1, &LtElemTemp);  //删除最早发生的雷击（第一个数据）
//        LOGW("thunder_parameter", "len = %d out of MAX_NUM, the following LtElem will be deleted: ", LtList->st.length);
//        PrintLtElem(LtElemTemp);
//    }
//    i--;
//    for(j = LtList->st.length; j > i; j--)
//    {
//        LtList->st.LtData[j] = LtList->st.LtData[j - 1];
//    }
//    LtList->st.LtData[i] = LtElem;
//    LtList->st.length ++;
//    return true;
//}

/**
  * @brief  从表尾插入数据元素
  * @param  表(LtList_st LtList)
  * @param  插入的新元素(LtElem_st LtElem)
  * @retval true/false
  */
//void LtListInsertByTail(LtList_st *LtList, LtElem_st LtElem)
//{
//    LtElem_st LtElemTemp;
//    if(LtList->st.length == LT_MAX_NUM)
//    {
//        LOGW("thunder_parameter", "len = %d out of MAX_NUM, the following LtElem will be deleted: ", LtList->st.length);
//        LtListDelete(LtList, 1, &LtElemTemp);  //删除最早发生的雷击（第一个数据）
//        PrintLtElem(LtElemTemp);
//    }
//    LtList->st.LtData[LtList->st.length] = LtElem;
//    LtList->st.length++;
//}

/**
  * @brief  新元素写入flash
  * @param  第i个位置写入(uint16_t i)
  * @param  新元素(LtElem_st LtElem)
  * @retval true/false
  */
//void LtListFlashWrite(LtElem_st LtElem)
//{
//    LtListInsertByTail(&gLtList, LtElem);
//    Parameter_FlashWrite(PAR_LTLIST_SAVE_ADDR, &gLtList, sizeof(gLtList));

//    gLtElemIdx = gLtList.st.length;
////    Thunder_Dispaly(gLtElemIdx, gLtList.st.LtData[gLtElemIdx - 1].Peak);
//}

/**
  * @brief  取表中第几个雷击的信息
  * @param  查询第i个位置(uint16_t i)
  * @param  返回该次雷击元素(LtElem_st LtElem)
  * @retval true/false
  */
//void LtListFlashRead(uint16_t i, LtElem_st *LtElem)
//{
//    if(GetLtElem(gLtList, i, LtElem) == false)
//    {
//        memset(LtElem, 0, sizeof(LtElem_st));
//    }
//}


/**
  * @brief  删除元素并存入flash
  * @param  第i个位置删除(uint16_t i)
  * @param  新元素(LtElem_st LtElem)
  * @retval true/false
  */
//bool LtListFlashDelete(uint16_t i, LtElem_st *LtElem)
//{
//    if(LtListDelete(&gLtList, i, LtElem) == false)
//    {
//        return false;
//    }
//    Parameter_FlashWrite(PAR_LTLIST_SAVE_ADDR, &gLtList, sizeof(gLtList));

//    gLtElemIdx = gLtList.st.length;
////    Thunder_Dispaly(gLtElemIdx, gLtList.st.LtData[gLtElemIdx - 1].Peak);

//    return true;
//}

/**
  * @brief  初始化线性表（删除所有数据）
  * @param  表(LtList_st *LtList)
  * @retval None
  */
//void LtListInit(void)
//{
//    LtList_Init(&gLtList);
//    Parameter_FlashWrite(PAR_LTLIST_SAVE_ADDR, &gLtList, sizeof(gLtList));

//    gLtElemIdx = gLtList.st.length;
////    Thunder_Dispaly(gLtElemIdx, gLtList.st.LtData[gLtElemIdx - 1].Peak);
//}

//void thunderparam_init(void)
//{
//    Parameter_FlashRead(PAR_LTLIST_SAVE_ADDR, &gLtList, sizeof(gLtList));
//    LOGI("parameter", "sizeof(gLtList) = %u", sizeof(gLtList));

//    if(gLtList.st.length == 0xFFFFFFFF)
//    {
//        LtList_Init(&gLtList);
//        Parameter_FlashWrite(PAR_LTLIST_SAVE_ADDR, &gLtList, sizeof(gLtList));
//    }

//    gLtElemIdx = gLtList.st.length;  //初始化指向最新的雷
//}

/* End of Thunder current monitor ----------------------------------------- */


void flashparam_init(void)
{
    int i=0; 
   
    Parameter_FlashRead(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
    LOGI("parameter", "sizeof(gFlashParam) = %u", sizeof(gFlashParam));

    if( (gFlashParam.st.magicNum != FLASH_MAGIC_NUMBER) || (gFlashParam.st.EndmagicNum[0] != (uint8_t)FLASH_MAGIC_NUMBER)
            || (gFlashParam.st.EndmagicNum[1] != (uint8_t)(FLASH_MAGIC_NUMBER >> 8)))
    {
        LOGW("parameter", "First runing!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        gFlashParam.st.magicNum = FLASH_MAGIC_NUMBER;
        gFlashParam.st.EndmagicNum[0] = (uint8_t)FLASH_MAGIC_NUMBER;
        gFlashParam.st.EndmagicNum[1] = (uint8_t)(FLASH_MAGIC_NUMBER >> 8);

        if(gFlashParam.st.Prod_Param == 0xFFFFFFFF) //第一次上电
        {
            gFlashParam.st.Prod_Param = 0;
            gFlashParam.st.Prod_Param |= PROD_TYPE;
#if   PARAM_TYPE == PARAM_SFE1
            gFlashParam.st.Prod_Param |= PARAM_SFE1;
            memcpy(gFlashParam.st.idInfo, "KSDSFE1250515001,/SFE1", 22);
#elif   PARAM_TYPE == PARAM_SFE2
            gFlashParam.st.Prod_Param |= PARAM_SFE2;
            memcpy(gFlashParam.st.idInfo, "KSDSFE2250515001,/SFE2", 22);
#elif   PARAM_TYPE == PARAM_SFE3
            gFlashParam.st.Prod_Param |= PARAM_SFE3;
            memcpy(gFlashParam.st.idInfo, "KSDSFE3250515001,/SFE3", 22);
#elif   PARAM_TYPE == PARAM_SFE4
            gFlashParam.st.Prod_Param |= PARAM_SFE4;
            memcpy(gFlashParam.st.idInfo, "KSDSFE4250515001,/SFE4", 22);
#elif   PARAM_TYPE == PARAM_SFE5
            gFlashParam.st.Prod_Param |= PARAM_SFE5;
            memcpy(gFlashParam.st.idInfo, "KSDSFE5250515001,/SFE5", 22);
#elif   PARAM_TYPE == PARAM_SFE6
            gFlashParam.st.Prod_Param |= PARAM_SFE6;
            memcpy(gFlashParam.st.idInfo, "KSDSFE6250515001,/SFE6", 22);
#elif   PARAM_TYPE == PARAM_SFB1
            gFlashParam.st.Prod_Param |= PARAM_SFB1;
            memcpy(gFlashParam.st.idInfo, "KSDSFB1250515001,/SFB1", 22);
#elif   PARAM_TYPE == PARAM_SFB2
            gFlashParam.st.Prod_Param |= PARAM_SFB2;
            memcpy(gFlashParam.st.idInfo, "KSDSFB2250515001,/SFB2", 22);
#elif   PARAM_TYPE == PARAM_SFB3
            gFlashParam.st.Prod_Param |= PARAM_SFB3;
            memcpy(gFlashParam.st.idInfo, "KSDSFB3250515001,/SFB3", 22);
#elif   PARAM_TYPE == PARAM_SFB4
            gFlashParam.st.Prod_Param |= PARAM_SFB4;
            memcpy(gFlashParam.st.idInfo, "KSDSFB4250515001,/SFB4", 22);
#elif   PARAM_TYPE == PARAM_SFB5
            gFlashParam.st.Prod_Param |= PARAM_SFB5;
            memcpy(gFlashParam.st.idInfo, "KSDSFB5250515001,/SFB5", 22);
#elif   PARAM_TYPE == PARAM_SFB6
            gFlashParam.st.Prod_Param |= PARAM_SFB6;
            memcpy(gFlashParam.st.idInfo, "KSDSFB6250515001,/SFB6", 22);
#elif   PARAM_TYPE == PARAM_SFA1
            gFlashParam.st.Prod_Param |= PARAM_SFA1;
            memcpy(gFlashParam.st.idInfo, "KSDSFA1250515001,/SFA1", 22);
#elif   PARAM_TYPE == PARAM_SFA2
            gFlashParam.st.Prod_Param |= PARAM_SFA2;
            memcpy(gFlashParam.st.idInfo, "KSDSFA2250515001,/SFA2", 22);
#elif   PARAM_TYPE == PARAM_SFA3
            gFlashParam.st.Prod_Param |= PARAM_SFA3;
            memcpy(gFlashParam.st.idInfo, "KSDSFA3250515001,/SFA3", 22);
#elif   PARAM_TYPE == PARAM_SFA4
            gFlashParam.st.Prod_Param |= PARAM_SFA4;
            memcpy(gFlashParam.st.idInfo, "KSDSFA4250515001,/SFA4", 22);
#elif   PARAM_TYPE == PARAM_SFA5
            gFlashParam.st.Prod_Param |= PARAM_SFA5;
            memcpy(gFlashParam.st.idInfo, "KSDSFA5250515001,/SFA5", 22);
#elif   PARAM_TYPE == PARAM_SFA6
            gFlashParam.st.Prod_Param |= PARAM_SFA6;
            memcpy(gFlashParam.st.idInfo, "KSDSFA6250515001,/SFA6", 22);            
#endif
        }
        else  //恢复出厂设置
        {
            if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA1 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFA1", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA2 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFA2", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA3 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFA3", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA4 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFA4", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA5 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFA5", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA6 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFA6", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB1 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFB1", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB2 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFB2", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB3 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFB3", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB4 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFB4", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB5 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFB5", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB6 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFB6", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE1 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFE1", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE2 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFE2", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE3 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFE3", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE4 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFE4", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE5 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFE5", 4);
            }
            else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE6 )
            {
                memcpy(&gFlashParam.st.idInfo[3], "SFE6", 4);
            }

            else
            {
                memcpy(&gFlashParam.st.idInfo[3], "ERR0", 4);
            }
        }

        if(gFlashParam.st.Prod_Protocol == 0xFFFFFFFF)
        {
            gFlashParam.st.Prod_Protocol = 0;
#if   PROTOCOL_TYPE & PROTOCOL_MBTCP
            gFlashParam.st.Prod_Protocol |= PROTOCOL_MBTCP;
#endif
#if PROTOCOL_TYPE & PROTOCOL_MBTCP
            gFlashParam.st.Prod_Protocol |= PROTOCOL_MBTCP;
#endif
#if PROTOCOL_TYPE & PROTOCOL_FLKMQTT
            gFlashParam.st.Prod_Protocol |= PROTOCOL_FLKMQTT;
#endif
#if PROTOCOL_TYPE & PROTOCOL_HZUDP
            gFlashParam.st.Prod_Protocol |= PROTOCOL_HZUDP;
#endif
#if PROTOCOL_TYPE & PROTOCOL_LLTUDP
            gFlashParam.st.Prod_Protocol |= PROTOCOL_LLTUDP;
#endif
#if PROTOCOL_TYPE & PROTOCOL_DHCP
            gFlashParam.st.Prod_Protocol |= PROTOCOL_DHCP;
#endif
#if PROTOCOL_TYPE & PROTOCOL_DNS
            gFlashParam.st.Prod_Protocol |= PROTOCOL_DNS;
#endif
#if PROTOCOL_TYPE & PROTOCOL_SNTP
            gFlashParam.st.Prod_Protocol |= PROTOCOL_SNTP;
#endif
#if PROTOCOL_TYPE & PROTOCOL_NETBIOS
            gFlashParam.st.Prod_Protocol |= PROTOCOL_NETBIOS;
#endif
#if PROTOCOL_TYPE & PROTOCOL_HTTPS
            gFlashParam.st.Prod_Protocol |= PROTOCOL_HTTPS;
#endif
        }

        /**************************** TCP/UDP *******************************/
        //获取芯片ID
        gFlashParam.st.macAddr[0] = (HAL_GetUIDw2() & 0x000000FF) + ((HAL_GetUIDw2() & 0x0000FF00) >> 8);     //物理MAC地址
        gFlashParam.st.macAddr[1] = ((HAL_GetUIDw2() & 0x00FF0000) >> 16) + ((HAL_GetUIDw2() & 0xFF000000) >> 24);
        gFlashParam.st.macAddr[2] = (HAL_GetUIDw1() & 0x000000FF) + ((HAL_GetUIDw1() & 0x0000FF00) >> 8);
        gFlashParam.st.macAddr[3] = ((HAL_GetUIDw1() & 0x00FF0000) >> 16) + ((HAL_GetUIDw1() & 0xFF000000) >> 24);
        gFlashParam.st.macAddr[4] = (HAL_GetUIDw0() & 0x000000FF) + ((HAL_GetUIDw0() & 0x0000FF00) >> 8);
        gFlashParam.st.macAddr[5] = ((HAL_GetUIDw0() & 0x00FF0000) >> 16) + ((HAL_GetUIDw0() & 0xFF000000) >> 24);
        if(gFlashParam.st.macAddr[0] % 2 != 0) //MAC地址开头必须是偶数
        {
            gFlashParam.st.macAddr[0] += 1;
        }

//        gFlashParam.st.DnsServerIP[0] = 114;        //DNS Server ip
//        gFlashParam.st.DnsServerIP[1] = 114;
//        gFlashParam.st.DnsServerIP[2] = 114;
//        gFlashParam.st.DnsServerIP[3] = 114;

//        memcpy(gFlashParam.st.domainName, "qcqe.com", 50);

//        gFlashParam.st.maskIP[0] = 255;       //子网掩码
//        gFlashParam.st.maskIP[1] = 255;
//        gFlashParam.st.maskIP[2] = 255;
//        gFlashParam.st.maskIP[3] = 0;

//        gFlashParam.st.gatewayIP[0] = 192;    //本机网关IP
//        gFlashParam.st.gatewayIP[1] = 168;    //192.168.1.1
//        gFlashParam.st.gatewayIP[2] = 0;
//        gFlashParam.st.gatewayIP[3] = 1;

//        gFlashParam.st.localIP[0] = 192;      //本机IP
//        gFlashParam.st.localIP[1] = 168;      //192.168.1.99
//        gFlashParam.st.localIP[2] = 0;
//        gFlashParam.st.localIP[3] = 99;
//        gFlashParam.st.localUDPPort = 6000;   //本机UDP端口

//        gFlashParam.st.s0TargetIP[0] = 192;   //S0目标IP
//        gFlashParam.st.s0TargetIP[1] = 168;   //192.168.0.149
//        gFlashParam.st.s0TargetIP[2] = 1;
//        gFlashParam.st.s0TargetIP[3] = 149;

//        gFlashParam.st.s0LocalPort = 6000;    //S0本机端口
//        gFlashParam.st.s0TargetPort = 4000;   //S0目标端口

//        gFlashParam.st.s1TargetIP[0] = 8;   //S1目标IP
//        gFlashParam.st.s1TargetIP[1] = 129;   //8.129.232.136
//        gFlashParam.st.s1TargetIP[2] = 232;
//        gFlashParam.st.s1TargetIP[3] = 136;

//        gFlashParam.st.s1LocalPort = 13036;   //S1本机端口
//        gFlashParam.st.s1TargetPort = 1883;  //S1目标端口

        /**************************** MQTT *******************************/
        gFlashParam.st.mqttPublishInterval = 5;
        
//        if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_COM2)  //zigbee网关扫描速度要慢点
//        {
//            gFlashParam.st.findRtuInterval = 300;
//            gFlashParam.st.getRtuInterval = 300;
//        }
//        else
//        {
            gFlashParam.st.findRtuInterval = 200;
            gFlashParam.st.getRtuInterval = 200;
//        }
        gFlashParam.st.getRtuCycleTime = 1000;

        gFlashParam.st.Cur_THDown = 0;    //下限0.0mA
        gFlashParam.st.Cur_THUp = 3000;  //上限300.0mA
        
        gFlashParam.st.FREQ_Up = 5005;    //频率上限50.05
        gFlashParam.st.FREQ_Down = 4995;  //频率下限49.95
        
        gFlashParam.st.NTC_LowPassFilterBW = 3;

        gFlashParam.st.Temp_THDown = 0;
        gFlashParam.st.Temp_THUp = 700;   //上限70.0

//        gFlashParam.st.Cur_Startup = 3.0;  //最小触发电流mA
        //gFlashParam.st.N_Ib_Vi = N_Ib3000mA_Vi600mV;

        //gFlashParam.st.meter_cali[offset_SYSCON] = 0xFFFF; //计量芯片初始化
        
//        memset(gFlashParam.st.mqttUserName, 0, sizeof(gFlashParam.st.mqttUserName));
//        memcpy(gFlashParam.st.mqttUserName, "admin", 5);
//        memset(gFlashParam.st.mqttPassword, 0, sizeof(gFlashParam.st.mqttPassword));
//        memcpy(gFlashParam.st.mqttPassword, "password", 8);

//        memset(gFlashParam.st.mqttPub, 0, sizeof(gFlashParam.st.mqttPub));
//        strncpy((char *)gFlashParam.st.mqttPub, "FEXCLOUD/DEFAULTMQTT/PUB", sizeof(gFlashParam.st.mqttPub) - 1);
//        memset(gFlashParam.st.mqttSub, 0, sizeof(gFlashParam.st.mqttSub));
//        strncpy((char *)gFlashParam.st.mqttSub, "FEXCLOUD/DEFAULTMQTT/SUB", sizeof(gFlashParam.st.mqttSub) - 1);
//        memset(gFlashParam.st.mqttAlarmPub, 0, sizeof(gFlashParam.st.mqttAlarmPub));
//        strncpy((char *)gFlashParam.st.mqttAlarmPub, "FEXCLOUD/DEFAULTMQTT/Alarm", sizeof(gFlashParam.st.mqttAlarmPub) - 1);

        for(i = 0; i < 3; i++)
        {
            gFlashParam.st.uartparam[i] = 3;
        }      
        
        gFlashParam.st.mb_word_bytetype = WORD_ByteH_ByteL;
        gFlashParam.st.mb_dword_wordtype = DWORD_WordL_WordH;

        gFlashParam.st.idNum = 1;
				gFlashParam.st.AlarmEN = 0;
				
        gFlashParam.st.Hardware_version = HW_VERSION;

        gFlashParam.st.AlmOutput_THUp0 = BOOL_Close;
        gFlashParam.st.AlmOutput_THUp1 = BOOL_Close;
        
        gFlashParam.st.AlmOutput_SourceLogic0 = 0;
        SET_BIT(gFlashParam.st.AlmOutput_SourceLogic0, Output_OnlyCmd_Msk);
        CLEAR_BIT(gFlashParam.st.AlmOutput_SourceLogic0, Output_AndAlmS_Msk);
        
        gFlashParam.st.AlmOutput_SourceLogic1 = 0;
        SET_BIT(gFlashParam.st.AlmOutput_SourceLogic1, Output_OnlyCmd_Msk);
        CLEAR_BIT(gFlashParam.st.AlmOutput_SourceLogic1, Output_AndAlmS_Msk);
        
        for(i = 0; i < 3; i++)
        {
            gFlashParam.st.AlmOutput_Source[i] = 0;            
        }
        
        for(i = 0; i < 5; i++)
        {
            gFlashParam.st.AlmState_Keep[i] = 0;
        }

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
        gFlashParam.st.AlmOutput_Source[0] = StateAlm0_ALL_Msk;
        gFlashParam.st.AlmOutput_Source[1] = StateAlm0_ALL_Msk;

//        gFlashParam.st.DI_Led_relate = 0;
//        gFlashParam.st.DI_Led_relate |= DI1_LED_Msk;
        gFlashParam.st.DI_THUp[0] = BOOL_Close;
        gFlashParam.st.DI_THUp[1] = BOOL_Close;
        gFlashParam.st.DI_THUp[2] = BOOL_Close;
        gFlashParam.st.DI_THUp[3] = BOOL_Close;
        gFlashParam.st.eUI_THUp[0] = 10000;
        gFlashParam.st.eUI_THUp[1] = 400;
        gFlashParam.st.eUI_THUp[2] = 10000;
        gFlashParam.st.eUI_THUp[3] = 10000;
        gFlashParam.st.THD_THUp[0] = 500;
        gFlashParam.st.THD_THUp[1] = 10000;
        gFlashParam.st.HR_THUp[0] = 400;
        gFlashParam.st.HR_THUp[1] = 200;
        gFlashParam.st.HR_THUp[2] = 10000;
        gFlashParam.st.HR_THUp[3] = 10000;
        
#if PROD_TYPE == PROD_SFE
        gFlashParam.st.Malignantloaden = 1;
        gFlashParam.st.Malignantloadmin = 7000;
        gFlashParam.st.Malignantloadmax = 13500;
        gFlashParam.st.MalignantloadPmin = 50;
        gFlashParam.st.MalignantloadPmax = 300;
        gFlashParam.st.MalignantloadSmax = 300;
        gFlashParam.st.MalignantloadTmin = 6000;
#endif 
        
if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA1 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB1 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE1)
{
    gFlashParam.st.EC = EC_Ib5A;
    gFlashParam.st.N_Ib_Vi[0] = N_Ib5A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[1] = N_Ib5A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[2] = N_Ib5A_Vi330mV;
    gFlashParam.st.K_Pow = K_EC6400_Ib5A;
    gFlashParam.st.Cur_Minimun = ESE_MINIMUM_CUR_Ib5A;
    gFlashParam.st.Pow_Minimun = ESE_MINIMUM_POW_Ib5A;
    gFlashParam.st.fCur_THUp = 5.0;
    gFlashParam.st.fCur_THDown = 0.0;
    gFlashParam.st.fPow_THUp = 3300.0;
    gFlashParam.st.fPow_THDown = 0.0;
}
else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA2 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB2 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE2)
{
    gFlashParam.st.EC = EC_Ib100A;
    gFlashParam.st.N_Ib_Vi[0] = N_Ib100A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[1] = N_Ib100A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[2] = N_Ib100A_Vi330mV;
    gFlashParam.st.K_Pow = K_EC400_Ib100A;
    gFlashParam.st.Cur_Minimun = ESE_MINIMUM_CUR_Ib100A;
    gFlashParam.st.Pow_Minimun = ESE_MINIMUM_POW_Ib100A;
    gFlashParam.st.fCur_THUp = 100.0;
    gFlashParam.st.fCur_THDown = 0.0;
    gFlashParam.st.fPow_THUp = 66000.0;
    gFlashParam.st.fPow_THDown = 0.0;
}
else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA3 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB3 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE3)
{
    gFlashParam.st.EC = EC_Ib200A;
    gFlashParam.st.N_Ib_Vi[0] = N_Ib200A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[1] = N_Ib200A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[2] = N_Ib200A_Vi330mV;
    gFlashParam.st.K_Pow = K_EC300_Ib200A;
    gFlashParam.st.Cur_Minimun = ESE_MINIMUM_CUR_Ib200A;
    gFlashParam.st.Pow_Minimun = ESE_MINIMUM_POW_Ib200A;
    gFlashParam.st.fCur_THUp = 200.0;
    gFlashParam.st.fCur_THDown = 0.0;
    gFlashParam.st.fPow_THUp =13200.0 ;
    gFlashParam.st.fPow_THDown = 0.0;
}
else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA4 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB4 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE4)
{
    gFlashParam.st.EC = EC_Ib400A;
    gFlashParam.st.N_Ib_Vi[0] = N_Ib400A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[1] = N_Ib400A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[2] = N_Ib400A_Vi330mV;
    gFlashParam.st.K_Pow = K_EC100_Ib400A;
    gFlashParam.st.Cur_Minimun = ESE_MINIMUM_CUR_Ib400A;
    gFlashParam.st.Pow_Minimun = ESE_MINIMUM_POW_Ib400A;
    gFlashParam.st.fCur_THUp = 400.0;
    gFlashParam.st.fCur_THDown = 0.0;
    gFlashParam.st.fPow_THUp = 264000.0;
    gFlashParam.st.fPow_THDown = 0.0;
}
else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA5 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB5 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE5)
{
    gFlashParam.st.EC = EC_Ib600A;
    gFlashParam.st.N_Ib_Vi[0] = N_Ib600A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[1] = N_Ib600A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[2] = N_Ib600A_Vi330mV;
    gFlashParam.st.K_Pow = K_EC60_Ib600A;
    gFlashParam.st.Cur_Minimun = ESE_MINIMUM_CUR_Ib600A;
    gFlashParam.st.Pow_Minimun = ESE_MINIMUM_POW_Ib600A;
    gFlashParam.st.fCur_THUp = 600.0;
    gFlashParam.st.fCur_THDown = 0.0;
    gFlashParam.st.fPow_THUp = 396000.0;
    gFlashParam.st.fPow_THDown = 0.0;
}
else
{
    gFlashParam.st.EC = EC_Ib1000A;
    gFlashParam.st.N_Ib_Vi[0] = N_Ib1000A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[1] = N_Ib1000A_Vi330mV;
    gFlashParam.st.N_Ib_Vi[2] = N_Ib1000A_Vi330mV;
    gFlashParam.st.K_Pow = K_EC40_Ib1000A;
    gFlashParam.st.Cur_Minimun = ESE_MINIMUM_CUR_Ib1000A;
    gFlashParam.st.Pow_Minimun = ESE_MINIMUM_POW_Ib1000A;
    gFlashParam.st.fCur_THUp = 1000.0;
    gFlashParam.st.fCur_THDown = 0.0;
    gFlashParam.st.fPow_THUp = 660000.0 ;
    gFlashParam.st.fPow_THDown = 0.0;
}

        gFlashParam.st.Vol_Minimun = ESE_STARTUP_VOL;
        gFlashParam.st.fVol_THUp = 242.0;
        gFlashParam.st.fVol_THDown = 200.0;
        gFlashParam.st.fVol_PNUp = 18;

        for(i = 0; i < 6; i++)
        {
            gFlashParam.st.Alm_Delay[i] = 10; //10(x100ms)
        }

        gFlashParam.st.meter_cali[offset_StartSig] = 0xFFFF; //计量芯片初始化
#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB || PROD_TYPE == PROD_ESA */
        
        /************************ COMMON PARAM *************************/
//#if PROD_TYPE == PROD_FSS
//        gFlashParam.st.SPD_PE_Up = 1;
//        gFlashParam.st.SPD_Temp_Up = 700;  //70.0

//        gFlashParam.st.Life_Temp_Up = 600;  //默认寿命温度门限60.0℃
//        gFlashParam.st.Life_Cur_Up = 30000; //默认寿命漏流门限3000.0uA
//        gFlashParam.st.Life_Temp_K = 100;   //默认Kt=1.0
//        gFlashParam.st.Life_Cur_K = 0;     //默认Ki=0.5
//        gFlashParam.st.Life_LT_K = 0;     //默认Km=1.0
//        gFlashParam.st.CurA_K = 16400;
//        gFlashParam.st.CurA_B = 63;
//        gFlashParam.st.CurB_K = 17300;
//        gFlashParam.st.CurB_B = 63;
//#else
//        gFlashParam.st.SPD_PE_Up = 1;
//        gFlashParam.st.SPD_Temp_Up = 0xFFFF;

//        gFlashParam.st.Life_Temp_Up = 500;  //默认寿命温度差门限50.0℃
//        gFlashParam.st.Life_Cur_Up = 30000; //默认寿命漏流门限3000.0uA
//        gFlashParam.st.Life_Temp_K = 100;   //默认Kt=1.0
//        gFlashParam.st.Life_Cur_K = 50;     //默认Ki=0.5
//        gFlashParam.st.Life_LT_K = 100;     //默认Km=1.0
//#endif

//#if PROD_TYPE == PROD_FD
//#if 0 /* 泄漏电流监测8台出现1台，电流的ADC增益为2倍 */
//        gFlashParam.st.CurA_K = 1654;
//        gFlashParam.st.CurB_K = 1654;
//        gFlashParam.st.CurC_K = 1654;
//        gFlashParam.st.CurA_B = -1772;
//        gFlashParam.st.CurB_B = -1772;
//        gFlashParam.st.CurC_B = -1772;
//#else /* 泄漏电流监测8台出现7台，电流的ADC增益为1倍 */
//        gFlashParam.st.CurA_K = 1811;    //1811 -> 0.1811
//        gFlashParam.st.CurB_K = 1811;
//        gFlashParam.st.CurC_K = 1811;
//        gFlashParam.st.CurA_B = -430;    //430 -> 0.430
//        gFlashParam.st.CurB_B = -430;
//        gFlashParam.st.CurC_B = -430;
//#endif
//#elif   PROD_TYPE == PROD_FS
//        gFlashParam.st.CurA_K = 2272;
//        gFlashParam.st.CurB_K = 2272;
//        gFlashParam.st.CurC_K = 2272;
//        gFlashParam.st.CurA_B = -4631;
//        gFlashParam.st.CurB_B = -4631;
//        gFlashParam.st.CurC_B = -4631;
//#endif

//#if PROD_TYPE == PROD_FD
//        gFlashParam.st.SPD_Cur_Up = 5000;  //500.0mA
//#elif PROD_TYPE == PROD_FL
//        if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_L2AI)
//        {
//            gFlashParam.st.SPD_Cur_Up = 5000;  //500.0A
//        }
//        else  //PARAM_M1AI
//        {
//            gFlashParam.st.SPD_Cur_Up = 200;   //20.0kA
//        }
//#else
//        gFlashParam.st.SPD_Cur_Up = 0xFFFF;
//#endif

//        gFlashParam.st.LtTimes = 0;
//        gFlashParam.st.LtTimesFilter = 100;

//        /**************************** FSS ******************************/
//        gFlashParam.st.SPD_L1_Up = 1;
//        gFlashParam.st.SPD_L2_Up = 1;
//        gFlashParam.st.SPD_L3_Up = 1;

//        //gFlashParam.st.SPD_Pe_Up = 1;
//        //gFlashParam.st.SPD_Temp_Up = 750;  //75.0
//        //gFlashParam.st.Life_Temp_Up = 750;  //默认寿命温度门限75.0℃
//        //gFlashParam.st.Life_Cur_Up = 30000; //默认寿命漏流门限3000.0uA
//        //gFlashParam.st.Life_Temp_K = 100;   //默认Kt=1.0
//        //gFlashParam.st.Life_Cur_K = 50;     //默认Ki=0.5
//        //gFlashParam.st.Life_LT_K = 100;     //默认Km=1.0

//        gFlashParam.st.SPD_SW1_Up = 1;
//        gFlashParam.st.SPD_SW2_Up = 1;
//        gFlashParam.st.SPD_SW3_Up = 1;
//        gFlashParam.st.SPD_SW4_Up = 1;

//        gFlashParam.st.NTC_LowPassFilterBW = 1;
//#if PROD_TYPE == PROD_FSS
//        gFlashParam.st.N_Ib_ATT7022[0] = N_Ib2400uA_Vi480mV;
//        gFlashParam.st.N_Ib_ATT7022[1] = N_Ib2400uA_Vi480mV;
//        gFlashParam.st.N_Ib_ATT7022[2] = N_Ib2400uA_Vi480mV;
//#endif       
//        gFlashParam.st.N_Ib_RN8209D = 3282.62;
//        
//#if PROD_TYPE == PROD_FSS || PROD_TYPE == PROD_FD
//        gFlashParam.st.meter_cali[0] = 0xFFFF; //计量芯片初始化
//#endif
//        /**************************** FS *******************************/
//        gFlashParam.st.SPD_YL_Up = 1;
//        gFlashParam.st.SPD_KL_Up = 1;
//        //gFlashParam.st.SPD_Pe_Up = 1;

//        gFlashParam.st.SPD_Cur_Down = 0;
//        //gFlashParam.st.SPD_Cur_Up = 0xFFFF;
//        gFlashParam.st.SPD_Vol_Down = 0;
//        gFlashParam.st.SPD_Vol_Up = 0xFFFF;
//        gFlashParam.st.SPD_Temp_Down = 0;
//        //gFlashParam.st.SPD_Temp_Up = 0xFFFF;

//        //gFlashParam.st.Life_Temp_Up = 500;  //默认寿命温度差门限50.0℃
//        //gFlashParam.st.Life_Cur_Up = 30000; //默认寿命漏流门限3000.0uA
//        //gFlashParam.st.Life_Temp_K = 100;   //默认Kt=1.0
//        //gFlashParam.st.Life_Cur_K = 50;     //默认Ki=0.5
//        //gFlashParam.st.Life_LT_K = 100;     //默认Km=1.0

//        //gFlashParam.st.CurA_K = 2272;
//        //gFlashParam.st.CurB_K = 2272;
//        //gFlashParam.st.CurC_K = 2272;
//        //gFlashParam.st.CurA_B = -4631;
//        //gFlashParam.st.CurB_B = -4631;
//        //gFlashParam.st.CurC_B = -4631;

//        gFlashParam.st.VolA_K = 1732;
//        gFlashParam.st.VolB_K = 1732;
//        gFlashParam.st.VolC_K = 1732;
//        gFlashParam.st.VolA_B = 184;
//        gFlashParam.st.VolB_B = 184;
//        gFlashParam.st.VolC_B = 184;

//        /**************************** FD *******************************/
//        //gFlashParam.st.SPD_Cur_Up = 5000;  //500.0mA

//        //gFlashParam.st.CurA_K = 1811;    //1811 -> 0.1811
//        //gFlashParam.st.CurB_K = 1811;
//        //gFlashParam.st.CurC_K = 1811;
//        //gFlashParam.st.CurA_B = -430;    //430 -> 0.430
//        //gFlashParam.st.CurB_B = -430;
//        //gFlashParam.st.CurC_B = -430;

//        /**************************** FR *******************************/
//        gFlashParam.st.GResGetInter = 1;
//        gFlashParam.st.GResGetTimes = 0;

//        gFlashParam.st.GRes_K = 100;  //K = 1
//        gFlashParam.st.GRes_B = 0;    //B = 0

//        gFlashParam.st.changeGResFlag = 0;     //default close change GRes
//        gFlashParam.st.changeGResMin = 0;      //if 0<GRes<0xFFFF, change GRes
//        gFlashParam.st.changeGResMax = 0xFFFF;
//        gFlashParam.st.changeGRes = 75;        //change GRes 0.75~1.25
//        gFlashParam.st.changeGResWidth = 50;

//        /**************************** FL *******************************/
//        if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_L2AI)
//        {
//            gFlashParam.st.Adc_K01_K02 = 2000;

//            gFlashParam.st.PAdc01_K = 4900;    //4900  => 0.4900
//            gFlashParam.st.PAdc01_B = -5455;   //-5455 => -54.55
//            gFlashParam.st.NAdc01_K = 4900;
//            gFlashParam.st.NAdc01_B = -5455;
//            gFlashParam.st.PAdc02_K = 5580;    //5580  => 0.5580
//            gFlashParam.st.PAdc02_B = -18773;  //-18773 => -187.73
//            gFlashParam.st.NAdc02_K = 5580;
//            gFlashParam.st.NAdc02_B = -18773;
//        }
//        else  //PARAM_M1AI
//        {
//            gFlashParam.st.Adc_K01_K02 = 300;

//            gFlashParam.st.PAdc01_K = 280;     //280  => 0.0280
//            gFlashParam.st.PAdc01_B = -278;    //-278 => -2.78
//            gFlashParam.st.NAdc01_K = 280;
//            gFlashParam.st.NAdc01_B = -278;
//            gFlashParam.st.PAdc02_K = 311;     //311  => 0.0311
//            gFlashParam.st.PAdc02_B = -370;    //-370 => -3.70
//            gFlashParam.st.NAdc02_K = 311;
//            gFlashParam.st.NAdc02_B = -370;
//        }

        gFlashParam.st.usADCCh11Threshold = 110;
        gFlashParam.st.usADCCh10Threshold = 110;

        /*****************************************************************/

        gFlashParam.st.SecCntAll = 0;
        gFlashParam.st.SysResetCnt = 0;

        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR41, 0); //备份寄存器清零
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR42, 0);

        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
        //printf_Flash(PAR_SAVE_ADDR, sizeof(gFlashParam));
    }

    if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA1 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFA1", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA2 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFA2", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA3 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFA3", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA4 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFA4", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA5 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFA5", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA6 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFA6", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB1 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFB1", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB2 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFB2", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB3 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFB3", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB4 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFB4", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB5 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFB5", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB6 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFB6", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE1 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFE1", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE2 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFE2", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE3 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFE3", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE4 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFE4", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE5 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFE5", 6);
    }
    else if( (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE6 )
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/SFE6", 6);
    }
    else
    {
        memcpy(&gFlashParam.st.idInfo[16], ",/ERR0", 6);
    }

    if(gFlashParam.st.LtTimesFilter < 1)
    {
        gFlashParam.st.LtTimesFilter = 1;  //雷击计数定时器时间不能为0！！！！
    }

    gFlashParam.st.SysResetCnt++;
    gFlashParam.st.W5500ResetCnt = 0;   //系统重启后W5500重启次数清零

#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
    gflashOffset.energy = 0xFFFFFFFF;
    Parameter_FlashRead_InOnePage(PAR_RTU_INFO_SAVE_ADDR, sizeof(gMeterEnergy), &gMeterEnergy, &gflashOffset.energy);
    if(*(uint32_t *)&gMeterEnergy.EPE[0] == 0xFFFFFFFF)
    {
        memset(&gMeterEnergy, 0, sizeof(gMeterEnergy)); //全部电能清零
        Parameter_FlashWrite_InOnePage(PAR_RTU_INFO_SAVE_ADDR, sizeof(gMeterEnergy), &gMeterEnergy, &gflashOffset.energy);
        LOGW(TAG, "init gMeterEnergy !!!!!!");
    }
#endif /* PROD_TYPE == PROD_ESE || PROD_TYPE == PROD_ESB || PROD_TYPE == PROD_ESA */
    
//    gFS_Elem.st.LtNum = gFlashParam.st.LtTimes;  //雷击计数和电弧次数都存在LtTimes
//    gFSS_Elem.st.LtNum = gFlashParam.st.LtTimes;
    //gtv_AfdDevice.msv_ArcTimes = gFlashParam.st.LtTimes;

//    gR2900ItvTime = gFlashParam.st.GResGetInter * 60;
    //Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));  //尽量避免上电写flash的操作
}

void param_init(void)
{
    LOGI("parameter", "sizeof(gParam) = %u", sizeof(gParam));
    uint8_t i;
    uint32_t lsv_buff[4];

    memset((uint16_t *)gParam.param_buff, 0, sizeof(gParam.param_buff));

    uint32_t secondAllH = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR42);
    LOGI("parameter", "secondAllH = %u", secondAllH);
    uint32_t secondAllL = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR41);
    LOGI("parameter", "secondAllL = %u", secondAllL);
    uint32_t secondAll = (secondAllH << 16) | secondAllL;
    LOGI("parameter", "secondAll = %u", secondAll);
    if(secondAll > gFlashParam.st.SecCntAll)
    {
        gFlashParam.st.SecCntAll = secondAll;
    }

    sscanf(SW_VERSION, "%u.%u.%u.%u", &lsv_buff[0], &lsv_buff[1], &lsv_buff[2], &lsv_buff[3]);
    for(i = 0; i < 4; i++)
    {
        gParam.st.version[i] = lsv_buff[i];
    }

    gParam.st.State_NetLinkErr = 0x0;
    gParam.st.State_SystemErr = 0x0;
    gParam.st.AlmOutput0 = 0;  //默认不进行报警输出
    gParam.st.AlmOutput1 = 0;  //默认不进行报警输出
    for(i = 0; i < 10; i++)
    {
        gParam.st.State_Alarm[i] = 0;
    }
    
    gParam.st.NetLink_State = 0xFFFF;
    gParam.st.SysState01 = 0xFFFF;
    gParam.st.AlmState = 0;

    //gParam.st.fCali_Cur = 150; //FSS默认校准电流uA
    
    gParam.st.State_NtcConnect = 0;
    
    #if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
    if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA1 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB1 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE1)
    {
        gParam.st.fCali_Cur = ESE_CALI_CUR_Ib5A;
        gParam.st.fCali_Pow = ESE_CALI_POW_Ib5A;
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA2 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB2 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE2)
    {   
        gParam.st.fCali_Cur = ESE_CALI_CUR_Ib100A;
        gParam.st.fCali_Pow = ESE_CALI_POW_Ib100A;
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA3 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB3 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE3)
    {
        gParam.st.fCali_Cur = ESE_CALI_CUR_Ib200A;
        gParam.st.fCali_Pow = ESE_CALI_POW_Ib200A;
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA4 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB4 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE4)
    {
        gParam.st.fCali_Cur = ESE_CALI_CUR_Ib400A;
        gParam.st.fCali_Pow = ESE_CALI_POW_Ib400A;
    }
    else if((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFA5 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFB5 || (gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_SFE5)
    {
        gParam.st.fCali_Cur = ESE_CALI_CUR_Ib600A;
        gParam.st.fCali_Pow = ESE_CALI_POW_Ib600A;
    }
    else
    {
        gParam.st.fCali_Cur = ESE_CALI_CUR_Ib1000A;
        gParam.st.fCali_Pow = ESE_CALI_POW_Ib1000A;
    }

    gParam.st.fCali_PF = ESE_CALI_PF;
    gParam.st.fCali_Vol = ESE_CALI_VOL;
    gParam.st.fConfig_Pstartup = 0.001;

    for(i = 0; i < 4; i++)
    {
        gParam.st.fClai_Err[i] = 0;
        gParam.st.Meter_CF_Interval_Time[i] = 0;
    }
    #endif
}



/**
  * @brief  参数初始化
  * @param  None
  * @retval None
  */
void Parameter_Init(void)
{
    LOGV("parameter", "Enter %s()", __func__);
    portENTER_CRITICAL();

    flashparam_init();
//    thunderparam_init();
//    gatewayparam_init();
    param_init();
    udpparam_init();

    portEXIT_CRITICAL();
}
