
///* Private includes ----------------------------------------------------------*/
//#include "main.h"
//#include "cmsis_os.h"
//#include "timers.h"

//#include "app_key.h"
//#include "app_log.h"
//#include "flexible_button.h"
//#include "app_parameter.h"
//#include "app_tm1650.h"
//#include "app_main.h"
//#include "app_uart.h"
//#include "app_collect.h"


///* Private define ------------------------------------------------------------*/
//#define KEY_SCAN_CYCLE (20)

//#define COUNTDOWN_NUM             5

//#define TIME_OUT_TO_EXIT_SETADDR  5000
//#define TIME_OUT_TO_EXIT_USERMODE 10000
//#define TIME_OUT_TO_EXIT_SETALM   5000

////按键有效电平
//#if PROD_TYPE == PROD_FSS
//#define  KEY_TURNON_LEVEL         1
//#else
//#define  KEY_TURNON_LEVEL         0
//#endif

//typedef enum
//{
//    USER_BUTTON_1 = 0,
//    USER_BUTTON_2,
//    USER_BUTTON_MAX
//} iotb_user_button_t;


///* Private variables ---------------------------------------------------------*/
///* Definitions for keyTask */
//osThreadId_t keyTaskHandle;
//const osThreadAttr_t keyTask_attributes =
//{
//    .name = "keyTask",
//    .priority = (osPriority_t) keyTaskPriority,
//    .stack_size = 1024
//};

//static volatile uint8_t key_scan_cycle = KEY_SCAN_CYCLE;
//static bool iotb_button_scan_enable = true;

//static flex_button_t iotb_user_button[USER_BUTTON_MAX];

//TimerHandle_t gSetAddrTimer = NULL;
//static volatile uint8_t start_set_addr_cnt;
//static volatile uint8_t set_which_addr = 1;  //1：设置个位   2：设置十位

//TimerHandle_t gUserModeTimer = NULL;
//static volatile uint8_t start_user_mode_cnt;
//static volatile uint8_t set_which_mode_num = 1;   //1：设置个位  2：设置十位  3：设置千位（b/c）
//static volatile uint8_t user_mode = 1;       //模式 1：b读取  2：c设置
//static volatile uint8_t user_mode_num = 1;   //序号 1：01
//static volatile uint8_t set_which_num = 1;   //1：设置个位  2：设置十位  3：设置百位  4:设置千位

//TimerHandle_t gSetAlmTimer = NULL;
//static volatile uint8_t start_set_alm_cnt;
//static volatile uint8_t set_which_almNum = 1;  //1：设置个位   2：设置十位  3：设置百位

//volatile uint8_t gKey1Mode = KEY_MODE_DEFAULT;

///* Private function prototypes -----------------------------------------------*/
//void KeyTask(void *argument);

//void set_hex_addr_add(uint8_t flag);
//void reset_set_addr_timer(void);

//void set_user_mode(uint8_t flag);
//void reset_user_mode_timer(void);
//void set_numder_addone(uint16_t *num, uint8_t flag);

//void set_almNum_addone(uint16_t *num, uint8_t flag, uint8_t multiple);
//void reset_set_alm_timer(void);


///* Private user code ---------------------------------------------------------*/
///**
//  * @brief  显示初始化
//  * @param  None
//  * @retval None
//  */
//static void Display_Default(void)
//{
//#if PROD_TYPE == PROD_FSS
//    if(gFlashParam.st.TUBEEnable)
//    {
//        _Dispaly_num(gFlashParam.st.LtTimes);
//    }
//#elif PROD_TYPE == PROD_FL
//    Thunder_Dispaly(gLtElemIdx, gLtList.st.LtData[gLtElemIdx - 1].Peak);
//#elif PROD_TYPE == PROD_FA
//    _Dispaly_num(gFlashParam.st.LtTimes);
//#elif PROD_TYPE == PROD_FS
//    _Dispaly_num(gFlashParam.st.LtTimes);
//#elif PROD_TYPE == PROD_FD
//    _Dispaly_10x_num(gFS_Elem.st.CurA);
//#elif PROD_TYPE == PROD_FG
//    if( (gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_DEFAULT) && (gtv_UartPortStatus[UART_PORT1].mcv_Mode == UART_MODE_DEFAULT) )
//    {
//        Gateway_Dispaly_num(gRtu_Num);
//    }
//#elif PROD_TYPE == PROD_FR
//    _Dispaly_100x_num(gFlashParam.st.GResVal);
//#endif
//}

///**
//  * @brief button 1 Callback
//  * @retval None
//  */
//static void btn_1_cb(flex_button_t *btn)
//{
//#if PROD_TYPE == PROD_FSS
//    if(gKey1Mode == KEY_MODE_SETADDR)
//    {
//        if(btn->event == FLEX_BTN_PRESS_DOWN)
//        {
//        }
//        else if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            set_hex_addr_add(set_which_addr);
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            set_which_addr = (set_which_addr == 1) ? 2 : 1;
//        }
//        reset_set_addr_timer();  //重置退出时间
//    }
//    else if(gKey1Mode == KEY_MODE_SELECTMODE)
//    {
//        if(btn->event == FLEX_BTN_PRESS_DOWN)
//        {
//        }
//        else if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            set_user_mode(set_which_mode_num);
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            if(set_which_mode_num >= 3)
//            {
//                set_which_mode_num = 1;
//            }
//            else
//            {
//                set_which_mode_num++;
//            }
//        }
//        reset_user_mode_timer();  //重置退出时间
//    }
//    else if(gKey1Mode == KEY_MODE_SETNUM)
//    {
//        if(btn->event == FLEX_BTN_PRESS_DOWN)
//        {
//        }
//        else if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            if(user_mode_num == 1) //雷击计数
//            {
//                set_numder_addone(&gFSS_Elem.st.LtNum, set_which_num);
//            }
//            else if(user_mode_num == 2)  //温度阈值
//            {
//                set_numder_addone(&gFlashParam.st.SPD_Temp_Up, set_which_num);
//            }
//            else if(user_mode_num == 3)  //恢复出厂设置
//            {
//                set_numder_addone(&gFlashParam.st.ResetFactory, set_which_num);
//            }
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            if(set_which_num >= 4)
//            {
//                set_which_num = 1;
//            }
//            else
//            {
//                set_which_num++;
//            }
//        }
//        reset_user_mode_timer();  //重置退出时间
//    }
//#elif PROD_TYPE == PROD_FL
//    if(gKey1Mode == KEY_MODE_SETADDR)
//    {
//        if(btn->event == FLEX_BTN_PRESS_DOWN)
//        {
//        }
//        else if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            set_hex_addr_add(set_which_addr);
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            set_which_addr = (set_which_addr == 1) ? 2 : 1;
//        }
//        reset_set_addr_timer();  //重置退出时间
//    }
//    else if(gKey1Mode == KEY_MODE_DEFAULT)
//    {
//        if( (btn->event == FLEX_BTN_PRESS_CLICK) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            if(gLtList.st.length == 0)
//            {
//                return ;
//            }
//            gLtElemIdx--;
//            if(gLtElemIdx < 1)
//            {
//                gLtElemIdx = gLtList.st.length;
//            }
//            Thunder_Dispaly(gLtElemIdx, gLtList.st.LtData[gLtElemIdx - 1].Peak);
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            LtListInit();
//        }
//    }
//#elif PROD_TYPE == PROD_FS
//    if(gKey1Mode == KEY_MODE_SETADDR)
//    {
//        if(btn->event == FLEX_BTN_PRESS_DOWN)
//        {
//        }
//        else if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            set_hex_addr_add(set_which_addr);
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            set_which_addr = (set_which_addr == 1) ? 2 : 1;
//        }
//        reset_set_addr_timer();  //重置退出时间
//    }
//    else if(gKey1Mode == KEY_MODE_DEFAULT)
//    {
//        if( (btn->event == FLEX_BTN_PRESS_CLICK) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            gFS_Elem.st.LtNum++;
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            gFS_Elem.st.LtNum = 0;
//        }
//    }
//#elif PROD_TYPE == PROD_FD
//    if(gKey1Mode == KEY_MODE_SETADDR)
//    {
//        if(btn->event == FLEX_BTN_PRESS_DOWN)
//        {
//        }
//        else if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            set_hex_addr_add(set_which_addr);
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            set_which_addr = (set_which_addr == 1) ? 2 : 1;
//        }
//        reset_set_addr_timer();  //重置退出时间
//    }
//    else if(gKey1Mode == KEY_MODE_SETALM)
//    {
//        if(btn->event == FLEX_BTN_PRESS_DOWN)
//        {
//        }
//        else if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            set_almNum_addone(&gFlashParam.st.SPD_Cur_Up, set_which_almNum, 1);
//        }
//        else if(btn->event == FLEX_BTN_PRESS_SHORT_START)
//        {
//            if(set_which_almNum >= 3)
//            {
//                set_which_almNum = 1;
//            }
//            else
//            {
//                set_which_almNum++;
//            }
//        }
//        reset_set_alm_timer();  //重置退出时间
//    }
//#elif PROD_TYPE == PROD_FG
//    if(gKey1Mode == KEY_MODE_DEFAULT)
//    {
//        if( (btn->event == FLEX_BTN_PRESS_CLICK) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            if(gParam.st.SecCnt < 30)
//            {
//                AutoFindRtu();
//            }
//        }
//    }
//#endif
//}

///**
//  * @brief button 2 Callback
//  * @retval None
//  */
//static void btn_2_cb(flex_button_t *btn)
//{
//#if PROD_TYPE == PROD_FSS
//    if(gKey1Mode == KEY_MODE_SELECTMODE)
//    {
//        if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            if(user_mode == 1)  //读取模式
//            {
//                if( (user_mode_num <= 11) && (user_mode_num > 0) )  //11以内序号有效
//                {
//                    gKey1Mode = KEY_MODE_READNUM;
//                }
//                else  //无效序号 则退出用户模式
//                {
//                    gKey1Mode = KEY_MODE_DEFAULT;
//                    start_user_mode_cnt = 0;
//                }
//            }
//            else if(user_mode == 2)  //设置模式
//            {
//                if( (user_mode_num <= 3) && (user_mode_num > 0) )  //3以内序号有效
//                {
//                    gKey1Mode = KEY_MODE_SETNUM;
//                }
//                else
//                {
//                    gKey1Mode = KEY_MODE_DEFAULT;
//                    start_user_mode_cnt = 0;
//                }
//            }
//        }
//        reset_user_mode_timer();  //重置退出时间
//    }
//    else if( (gKey1Mode == KEY_MODE_READNUM) || (gKey1Mode == KEY_MODE_SETNUM) )
//    {
//        if( ( btn->event == FLEX_BTN_PRESS_CLICK ) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) )
//        {
//            if( (gKey1Mode == KEY_MODE_SETNUM) || (user_mode_num == 2) )  //设置温度阈值后保存
//            {
//                Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//            }
//            gKey1Mode = KEY_MODE_DEFAULT;
//            start_user_mode_cnt = 0;
//            if( (gKey1Mode == KEY_MODE_SETNUM) || (user_mode_num == 3) )  //恢复出厂设置
//            {
//                if(gFlashParam.st.ResetFactory)
//                {
//                    gFlashParam.st.magicNum = 0;
//                    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//                    NVIC_SystemReset();
//                }
//            }
//        }
//    }
//#endif
//}

///**
//  * @brief button 1 read Pin
//  * @retval None
//  */
//static uint8_t button_key1_read(void)
//{
//    return HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin);
//}

///**
//  * @brief button 2 read Pin
//  * @retval None
//  */
//static uint8_t button_key2_read(void)
//{
//#if PROD_TYPE == PROD_FSS
//    return HAL_GPIO_ReadPin(KEY_2_GPIO_Port, KEY_2_Pin);
//#else
//    return !KEY_TURNON_LEVEL;
//#endif
//}

///**
//  * @brief set scan cycle time
//  * @retval None
//  */
//void key_thr_set_cycle(uint16_t time)
//{
//    uint32_t ulReturn = taskENTER_CRITICAL_FROM_ISR();
//    key_scan_cycle = time;
//    taskEXIT_CRITICAL_FROM_ISR( ulReturn );
//}

///**
//  * @brief button scan disable
//  * @retval None
//  */
//void iotb_btn_scan_disable(void)
//{
//    uint32_t ulReturn = taskENTER_CRITICAL_FROM_ISR();
//    iotb_button_scan_enable = false;
//    taskEXIT_CRITICAL_FROM_ISR( ulReturn );
//}

///**
//  * @brief button scan enable
//  * @retval None
//  */
//void iotb_btn_scan_enable(void)
//{
//    uint32_t ulReturn = taskENTER_CRITICAL_FROM_ISR();
//    iotb_button_scan_enable = true;
//    taskEXIT_CRITICAL_FROM_ISR( ulReturn );
//}

///**
//  * @brief key init
//  * @retval None
//  */
//void iotb_key_init(void)
//{
//    uint8_t i;

//    memset(&iotb_user_button[0], 0x0, sizeof(iotb_user_button));

//    iotb_user_button[USER_BUTTON_1].usr_button_read = button_key1_read;
//    iotb_user_button[USER_BUTTON_1].cb = (flex_button_response_callback)btn_1_cb;

//    iotb_user_button[USER_BUTTON_2].usr_button_read = button_key2_read;
//    iotb_user_button[USER_BUTTON_2].cb = (flex_button_response_callback)btn_2_cb;

//    for (i = 0; i < USER_BUTTON_MAX; i ++)
//    {
//        iotb_user_button[i].status = 0;
//        iotb_user_button[i].pressed_logic_level = KEY_TURNON_LEVEL;

//#if 0 //default
//        iotb_user_button[i].click_start_tick = 20;         //click press: keep time > click_start_tick * iotb_key_scan_cycle
//        iotb_user_button[i].short_press_start_tick = 100;  //short press: keep time > short_press_start_tick * iotb_key_scan_cycle
//        iotb_user_button[i].long_press_start_tick = 200;   //long press: keep time > long_press_start_tick * iotb_key_scan_cycle
//        iotb_user_button[i].long_hold_start_tick = 300;    //long hold: keep time > long_hold_start_tick * iotb_key_scan_cycle
//#else
//        iotb_user_button[i].click_start_tick = 12;         //click press: keep time > click_start_tick * iotb_key_scan_cycle
//        iotb_user_button[i].short_press_start_tick = 60;  //short press: keep time > short_press_start_tick * iotb_key_scan_cycle
//        iotb_user_button[i].long_press_start_tick = 120;   //long press: keep time > long_press_start_tick * iotb_key_scan_cycle
//        iotb_user_button[i].long_hold_start_tick = 180;    //long hold: keep time > long_hold_start_tick * iotb_key_scan_cycle
//#endif

//        flex_button_register(&iotb_user_button[i]);
//    }
//}

///* --------------------------------- key in set addr mode ------------------------------------------- */
///**
//  * @brief  退出设置地址模式
//  * @param  None
//  * @retval None
//  */
//void exit_set_addr(void *arg)
//{
//    gKey1Mode = KEY_MODE_DEFAULT;

//    start_set_addr_cnt = 0;
//}

///**
//  * @brief  set addr timer init
//  * @param  None
//  * @retval None
//  */
//void set_addr_timer_init(void)
//{
//    gSetAddrTimer = xTimerCreate((const char *)"gSetAddrTimer",
//                                 (TickType_t  )TIME_OUT_TO_EXIT_SETADDR,
//                                 (UBaseType_t )pdFALSE,
//                                 (void *      )1,
//                                 (TimerCallbackFunction_t)exit_set_addr);
//}

///**
//  * @brief  重置退出时间
//  * @param  None
//  * @retval None
//  */
//void reset_set_addr_timer(void)
//{
//    xTimerReset(gSetAddrTimer, 0);
//}

///**
//  * @brief  设置地址位加一
//  * @param  flag 1 个位加一
//                 2 十位加一
//  * @retval None
//  */
//void set_hex_addr_add(uint8_t flag)
//{
//    if(flag == 1)
//    {
//        if(gFlashParam.st.idNum % 0x10 != 0x0F)
//        {
//            gFlashParam.st.idNum += 0x01;
//        }
//        else
//        {
//            gFlashParam.st.idNum -= 0x0F;
//        }
//    }
//    else
//    {
//        if(gFlashParam.st.idNum / 0x10 != 0x0F)
//        {
//            gFlashParam.st.idNum += 0x10;
//        }
//        else
//        {
//            gFlashParam.st.idNum -= 0xF0;
//        }
//    }
//}

///**
//  * @brief  设置地址进程
//  * @param  None
//  * @retval None
//  */
//void set_addr_process(void)
//{
//    uint16_t dp_tick = 0;

//    _Dispaly_hex_addrNum(gFlashParam.st.idNum, 0);

//    while(start_set_addr_cnt)
//    {
//        if (iotb_button_scan_enable)
//        {
//            flex_button_scan();
//        }

//        dp_tick++;
//        if(dp_tick < 10)   //配置位在闪烁
//        {
//            _Dispaly_hex_addrNum(gFlashParam.st.idNum, set_which_addr);
//        }
//        else if(dp_tick < 20)
//        {
//            _Dispaly_hex_addrNum(gFlashParam.st.idNum, 0);
//        }
//        else
//        {
//            dp_tick = 0;
//        }

//        osDelay(key_scan_cycle);
//    }

//    if(gFlashParam.st.idNum == 0)
//    {
//        gFlashParam.st.idNum = 1;
//    }

//    Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//}

///**
//  * @brief  检测是否需要改地址
//  * @param  None
//  * @retval None
//  */
//void set_addr_check(void)
//{
//    set_addr_timer_init();

//    gKey1Mode = KEY_MODE_SETADDR;
//    start_set_addr_cnt = COUNTDOWN_NUM;

//    xTimerStart(gSetAddrTimer, 0);  //开启软件定时器，x ms后开启中断
//    while(start_set_addr_cnt)
//    {
//        if( button_key1_read() == KEY_TURNON_LEVEL )
//        {
//            reset_set_addr_timer();  //重置退出时间
//            _Dispaly_same_Num(start_set_addr_cnt);  //显示5555->4444->3333->2222->1111后进入修改地址界面
//            osDelay(500);

//            if(start_set_addr_cnt <= 1)
//            {
//                set_addr_process();  //start to set addr

//                break;
//            }
//            start_set_addr_cnt--;

//            continue;
//        }

//        _Dispaly_hex_addrNum(gFlashParam.st.idNum, 0);
//        osDelay(100);
//    }

//    xTimerDelete(gSetAddrTimer, 0);  //删除软件定时器
//}

///* ---------------------------------- key in user mode ---------------------------------------------- */
///**
//  * @brief  退出设置地址模式
//  * @param  None
//  * @retval None
//  */
//void exit_user_mode(void *arg)
//{
//    if( (start_user_mode_cnt == 0) && (gKey1Mode == KEY_MODE_DEFAULT) )
//    {
//        return;
//    }

//    if( (gKey1Mode == KEY_MODE_SETNUM) || (user_mode_num == 2) )  //设置温度阈值后保存
//    {
//        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//    }
//    gKey1Mode = KEY_MODE_DEFAULT;
//    start_user_mode_cnt = 0;
//}

///**
//  * @brief  set addr timer init
//  * @param  None
//  * @retval None
//  */
//void set_user_timer_init(void)
//{
//    gUserModeTimer = xTimerCreate((const char *)"gUserModeTimer",
//                                  (TickType_t  )TIME_OUT_TO_EXIT_USERMODE,
//                                  (UBaseType_t )pdFALSE,
//                                  (void *      )1,
//                                  (TimerCallbackFunction_t)exit_user_mode);
//}

///**
//  * @brief  重置退出时间
//  * @param  None
//  * @retval None
//  */
//void reset_user_mode_timer(void)
//{
//    xTimerReset(gUserModeTimer, 0);
//}

///**
//  * @brief  设置选择模式的值
//  * @param  flag 1 个位加一
//                 2 十位加一
//                 3 模式切换（b <-> c）
//  * @retval None
//  */
//void set_user_mode(uint8_t flag)
//{
//    if(flag == 1)
//    {
//        if(user_mode_num % 10 != 9)
//        {
//            user_mode_num += 1;
//        }
//        else
//        {
//            user_mode_num -= 9;
//        }
//    }
//    else if(flag == 2)
//    {
//        if( ((user_mode_num / 10) % 10) != 9)
//        {
//            user_mode_num += 10;
//        }
//        else
//        {
//            user_mode_num -= 90;
//        }
//    }
//    else if(flag == 3)
//    {
//        user_mode = (user_mode == 1 ? 2 : 1);
//    }
//}

///**
//  * @brief  设置数字位加一 数字为9时加一变为0
//  * @param  num 数字
//  * @param  flag 1 个位加一
//                 2 十位加一
//                 3 百位加一
//                 4 千位加一
//  * @retval None
//  */
//void set_numder_addone(uint16_t *num, uint8_t flag)
//{
//    if(flag == 4)
//    {
//        if( ((*num / 1000) % 10) != 9)
//        {
//            *num += 1000;
//        }
//        else
//        {
//            *num -= 9000;
//        }
//    }
//    else if(flag == 3)
//    {
//        if( ((*num / 100) % 10) != 9)
//        {
//            *num += 100;
//        }
//        else
//        {
//            *num -= 900;
//        }
//    }
//    else if(flag == 2)
//    {
//        if( ((*num / 10) % 10) != 9)
//        {
//            *num += 10;
//        }
//        else
//        {
//            *num -= 90;
//        }
//    }
//    else
//    {
//        if( (*num % 10) != 9)
//        {
//            *num += 1;
//        }
//        else
//        {
//            *num -= 9;
//        }
//    }
//}

///**
//  * @brief  选择模式进程
//  * @param  None
//  * @retval None
//  */
//static void select_mode_process(void)
//{
//    uint16_t dp_tick = 0;
//    uint16_t temp;

//    gKey1Mode = KEY_MODE_SELECTMODE;
//    _Dispaly_modeNum(user_mode, user_mode_num, 0);

//    while(start_user_mode_cnt)
//    {
//        if (iotb_button_scan_enable)
//        {
//            flex_button_scan();
//        }

//        dp_tick++;
//        if(dp_tick < 10)   //配置位在闪烁
//        {
//            if(gKey1Mode == KEY_MODE_SELECTMODE)  //select mode
//            {
//                _Dispaly_modeNum(user_mode, user_mode_num, set_which_mode_num);
//            }
//            else if(gKey1Mode == KEY_MODE_SETNUM)
//            {
//                if(user_mode_num == 1)  //雷击计数
//                {
//                    _Dispaly_setNum(gFSS_Elem.st.LtNum, 0, set_which_num);
//                }
//                else if(user_mode_num == 2)   //温度报警
//                {
//                    _Dispaly_setNum(gFlashParam.st.SPD_Temp_Up, 1, set_which_num);
//                }
//                else if(user_mode_num == 3)   //恢复出厂设置
//                {
//                    _Dispaly_setNum(gFlashParam.st.ResetFactory, 0, set_which_num);
//                }
//            }
//            else if(gKey1Mode == KEY_MODE_READNUM)
//            {
//                if(user_mode_num == 1)  //工作状态
//                {
//                    temp = 0;
//                    temp += gFSS_Elem.st.L1 * 1000;
//                    if( ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEM2) || ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEA2) )
//                    {
//                        temp += (FSS_USELESS_PARAM_DEFAULT & 0x0001) * 100;
//                        temp += (FSS_USELESS_PARAM_DEFAULT & 0x0001) * 10;
//                    }
//                    else
//                    {
//                        temp += gFSS_Elem.st.L2 * 100;
//                        temp += gFSS_Elem.st.L3 * 10;
//                    }
//                    temp += gFSS_Elem.st.PE * 1;
//                    _Dispaly_setNum(temp, 0, 0);
//                }
//                else if(user_mode_num == 2)   //脱扣状态
//                {
//                    temp = 0;
//                    temp += gFSS_Elem.st.SW[0] * 1000;
//                    temp += gFSS_Elem.st.SW[1] * 100;
//                    if( ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEM2) || ((gFlashParam.st.Prod_Param & PARAM_XX) == PARAM_DEA2) )
//                    {
//                        temp += (FSS_USELESS_PARAM_DEFAULT & 0x0001) * 10;
//                        temp += (FSS_USELESS_PARAM_DEFAULT & 0x0001) * 1;
//                    }
//                    else
//                    {
//                        temp += gFSS_Elem.st.SW[2] * 10;
//                        temp += gFSS_Elem.st.SW[3] * 1;
//                    }
//                    _Dispaly_setNum(temp, 0, 0);
//                }
//                else if(user_mode_num == 3)   //雷击计数
//                {
//                    _Dispaly_num(gFSS_Elem.st.LtNum);
//                }
//                else if(user_mode_num == 4)   //L1漏流
//                {
//                    _Dispaly_10x_num(gFSS_Elem.st.Cur[0]);
//                }
//                else if(user_mode_num == 5)   //L2漏流
//                {
//                    _Dispaly_10x_num(gFSS_Elem.st.Cur[1]);
//                }
//                else if(user_mode_num == 6)   //L3漏流
//                {
//                    _Dispaly_10x_num(gFSS_Elem.st.Cur[2]);
//                }
//                else if(user_mode_num == 7)   //L1温度
//                {
//                    _Dispaly_10x_num(gFSS_Elem.st.Tmp[0]);
//                }
//                else if(user_mode_num == 8)   //L2温度
//                {
//                    _Dispaly_10x_num(gFSS_Elem.st.Tmp[1]);
//                }
//                else if(user_mode_num == 9)   //L3温度
//                {
//                    _Dispaly_10x_num(gFSS_Elem.st.Tmp[2]);
//                }
//                else if(user_mode_num == 10)   //N温度
//                {
//                    _Dispaly_10x_num(gFSS_Elem.st.Tmp[3]);
//                }
//                else if(user_mode_num == 11)   //预估寿命
//                {
//                    _Dispaly_10x_num(gFSS_Elem.st.Life);
//                }
//            }
//        }
//        else if(dp_tick < 20)
//        {
//            if(gKey1Mode == KEY_MODE_SELECTMODE)  //select mode
//            {
//                _Dispaly_modeNum(user_mode, user_mode_num, 0);
//            }
//            else if(gKey1Mode == KEY_MODE_SETNUM)
//            {
//                if(user_mode_num == 1)  //雷击计数
//                {
//                    _Dispaly_setNum(gFSS_Elem.st.LtNum, 0, 0);
//                }
//                else if(user_mode_num == 2)   //温度报警
//                {
//                    _Dispaly_setNum(gFlashParam.st.SPD_Temp_Up, 1, 0);
//                }
//                else if(user_mode_num == 3)   //恢复出厂设置
//                {
//                    _Dispaly_setNum(gFlashParam.st.ResetFactory, 0, 0);
//                }
//            }
//        }
//        else
//        {
//            dp_tick = 0;
//        }

//        osDelay(key_scan_cycle);
//    }
//}

///**
//  * @brief  检测是否需要改模式
//  * @param  None
//  * @retval None
//  */
//void select_mode_check(void)
//{
//    static uint8_t key2cnt = 0;

//    if(button_key1_read() == KEY_TURNON_LEVEL)
//    {
//        start_user_mode_cnt = COUNTDOWN_NUM;
//        xTimerStart(gUserModeTimer, 0);  //开启软件定时器，x ms后开启中断
//        gKey1Mode = KEY_MODE_WAITING;
//        osDelay(100);

//        while(button_key1_read() == KEY_TURNON_LEVEL)
//        {
//            _Dispaly_same_Num(start_user_mode_cnt);  //显示5555->4444->3333->2222->1111后进入界面
//            osDelay(500);

//            if(start_user_mode_cnt == 1)
//            {
//                select_mode_process();  //start to select mode
//            }
//            else if( (start_user_mode_cnt <= COUNTDOWN_NUM) && (start_user_mode_cnt > 0) )
//            {
//                start_user_mode_cnt--;
//            }
//            else  //避免刚退出按键又被按
//            {
//                return;
//            }
//        }

//        Display_Default();
//    }
//    if(button_key2_read() == KEY_TURNON_LEVEL)
//    {
//        osDelay(100);
//        while(button_key2_read() == KEY_TURNON_LEVEL)
//        {
//            key2cnt++;
//            osDelay(100);
//            if(key2cnt == 20) //2000ms
//            {
//                key2cnt = 0;
//                gFSS_Elem.st.LtNum = 0;
//                break;
//            }
//        }
//    }
//    key2cnt = 0;
//}

///* ------------------------------ key in set alarm mode --------------------------------------------- */
///**
//  * @brief  退出设置报警模式
//  * @param  None
//  * @retval None
//  */
//void exit_set_alm(void *arg)
//{
//    if(gKey1Mode == KEY_MODE_SETALM)  //设置报警值后保存
//    {
//        Parameter_FlashWrite(PAR_SAVE_ADDR, &gFlashParam, sizeof(gFlashParam));
//    }
//    gKey1Mode = KEY_MODE_DEFAULT;
//    start_set_alm_cnt = 0;
//}

///**
//  * @brief  set alarm timer init
//  * @param  None
//  * @retval None
//  */
//void set_alm_timer_init(void)
//{
//    gSetAlmTimer = xTimerCreate((const char *)"gSetAlmTimer",
//                                (TickType_t  )TIME_OUT_TO_EXIT_SETALM,
//                                (UBaseType_t )pdFALSE,
//                                (void *      )1,
//                                (TimerCallbackFunction_t)exit_set_alm);
//}

///**
//  * @brief  重置退出时间
//  * @param  None
//  * @retval None
//  */
//void reset_set_alm_timer(void)
//{
//    xTimerReset(gSetAlmTimer, 0);
//}

///**
//  * @brief  设置报警数字位加一 数字为9时加一变为0
//  * @param  num 数字
//  * @param  flag 1 个位加一
//                 2 十位加一
//                 3 百位加一
//  * @param  multiple 0 num不变
//                     1 num除以10再计算，计算后再乘以10
//  * @retval None
//  */
//void set_almNum_addone(uint16_t *num, uint8_t flag, uint8_t multiple)
//{
//    if(multiple == 1)
//    {
//        *num /= 10;
//    }

//    if(flag == 3)
//    {
//        if( ((*num / 100) % 10) != 9)
//        {
//            *num += 100;
//        }
//        else
//        {
//            *num -= 900;
//        }
//    }
//    else if(flag == 2)
//    {
//        if( ((*num / 10) % 10) != 9)
//        {
//            *num += 10;
//        }
//        else
//        {
//            *num -= 90;
//        }
//    }
//    else
//    {
//        if( (*num % 10) != 9)
//        {
//            *num += 1;
//        }
//        else
//        {
//            *num -= 9;
//        }
//    }

//    if(multiple == 1)
//    {
//        *num *= 10;
//    }
//}

///**
//  * @brief  设置报警值进程
//  * @param  None
//  * @retval None
//  */
//void set_alm_process(void)
//{
//    uint16_t dp_tick = 0;

//    gKey1Mode = KEY_MODE_SETALM;
//    _Dispaly_H_Num(gFlashParam.st.SPD_Cur_Up, 0, 1);

//    while(start_set_alm_cnt)
//    {
//        if (iotb_button_scan_enable)
//        {
//            flex_button_scan();
//        }

//        dp_tick++;
//        if(dp_tick < 10)   //配置位在闪烁
//        {
//            _Dispaly_H_Num(gFlashParam.st.SPD_Cur_Up, 0, 1);
//        }
//        else if(dp_tick < 20)
//        {
//            _Dispaly_H_Num(gFlashParam.st.SPD_Cur_Up, set_which_almNum, 1);
//        }
//        else
//        {
//            dp_tick = 0;
//        }

//        osDelay(key_scan_cycle);
//    }
//}

///**
//  * @brief  检测是否需要报警值
//  * @param  None
//  * @retval None
//  */
//void set_alm_check(void)
//{
//    if(button_key1_read() == KEY_TURNON_LEVEL)
//    {
//        start_set_alm_cnt = COUNTDOWN_NUM;
//        xTimerStart(gSetAlmTimer, 0);  //开启软件定时器，x ms后开启中断
//        gKey1Mode = KEY_MODE_WAITING;
//        osDelay(100);

//        while(button_key1_read() == KEY_TURNON_LEVEL)
//        {
//            _Dispaly_H_Num(start_set_alm_cnt, 0, 0);  //显示H005->H004->H003->H002->H001后进入界面
//            osDelay(500);

//            if(start_set_alm_cnt == 1)
//            {
//                set_alm_process();  //start to set alarm mode
//            }
//            else if( (start_set_alm_cnt <= COUNTDOWN_NUM) && (start_set_alm_cnt > 0) )
//            {
//                start_set_alm_cnt--;
//            }
//            else  //避免刚退出按键又被按
//            {
//                return;
//            }
//        }

//        Display_Default();
//    }
//}
///* ---------------------------------- ---------------- ---------------------------------------------- */

///**
//  * @brief  新建按键线程（任务）
//  * @param  None
//  * @retval None
//  */
//void osThreadNew_keyTask(void)
//{
//    iotb_key_init();

//    keyTaskHandle = osThreadNew(KeyTask, NULL, &keyTask_attributes);
//}

///**
//  * @brief  Function implementing the keyTask thread.
//  * @param  argument: Not used
//  * @retval None
//  */
//void KeyTask(void *argument)
//{
//    LOGD("app_key", "%s RUN. Free heap size is %d bytes", __func__, xPortGetFreeHeapSize());

//#if PROD_TYPE == PROD_FSS
//    set_user_timer_init();
//    set_addr_check();
//    Display_Default();

//    for(;;)
//    {
//        select_mode_check();
//        osDelay(key_scan_cycle);
//    }
//#elif PROD_TYPE == PROD_FD
//    set_alm_timer_init();
//    set_addr_check();
//    Display_Default();

//    for(;;)
//    {
//        set_alm_check();
//        osDelay(key_scan_cycle);
//    }
//#elif PROD_TYPE == PROD_FG
//    start_user_mode_cnt = 10;  //1000ms
//    while(start_user_mode_cnt)
//    {
//        if (iotb_button_scan_enable)
//        {
//            flex_button_scan();
//        }
//        Display_Default();
//        start_user_mode_cnt--;
//        osDelay(100);
//    }

//    for(;;)
//    {
//        if (iotb_button_scan_enable)
//        {
//            flex_button_scan();
//        }
//        osDelay(key_scan_cycle);
//    }
//#else
//    set_addr_check();
//    Display_Default();

//    for(;;)
//    {
//        if (iotb_button_scan_enable)
//        {
//            flex_button_scan();
//        }
//        osDelay(key_scan_cycle);
//    }
//#endif

//}

