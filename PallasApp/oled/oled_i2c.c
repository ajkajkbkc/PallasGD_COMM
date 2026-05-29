/*
 * Copyright (c) 2006-2018, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-23     Arrbow       first implementation
 */

/* Private includes ----------------------------------------------------------*/
#include "oled_i2c.h"
#include "oled_codetab.h"
#include "cmsis_os.h"
#include "main.h"


/* Private define ------------------------------------------------------------*/




/* Private variables ---------------------------------------------------------*/





/* Private function prototypes -----------------------------------------------*/






/* Private user code ---------------------------------------------------------*/

/**
  * @brief  向OLED写入命令
  * @param  I2C_Command：命令代码
  * @retval 无
  */
static void WriteCmd(unsigned char I2C_Command)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, &I2C_Command, 1, 0x100);
}

/**
  * @brief  向OLED写入数据
  * @param  I2C_Data：数据
  * @retval 无
  */
static void WriteData(unsigned char I2C_Data)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDRESS, 0x40, I2C_MEMADD_SIZE_8BIT, &I2C_Data, 1, 0x100);
}

/**
  * @brief  OLED_Init，初始化OLED
  * @param  无
  * @retval 0：正常  1：异常
  */
unsigned char OLED_Init(void)
{
    osDelay(1);     // 1s,这里的延时很重要,上电后延时，没有错误的冗余设计

    unsigned char I2C_Command = 0xAE;
    if(HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, &I2C_Command, 1, 0x100) != HAL_OK)  //display off
    {
        return 1;  //OLED disconnect
    }

    WriteCmd(0xAE); //display off
    WriteCmd(0x20);	//Set Memory Addressing Mode
    WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
    WriteCmd(0xc8);	//Set COM Output Scan Direction
    WriteCmd(0x00); //---set low column address
    WriteCmd(0x10); //---set high column address
    WriteCmd(0x40); //--set start line address
    WriteCmd(0x81); //--set contrast control register
    WriteCmd(0xff); //亮度调节 0x00~0xff
    WriteCmd(0xa1); //--set segment re-map 0 to 127
    WriteCmd(0xa6); //--set normal display
    WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
    WriteCmd(0x3F); //
    WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    WriteCmd(0xd3); //-set display offset
    WriteCmd(0x00); //-not offset
    WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
    WriteCmd(0xf0); //--set divide ratio
    WriteCmd(0xd9); //--set pre-charge period
    WriteCmd(0x22); //
    WriteCmd(0xda); //--set com pins hardware configuration
    WriteCmd(0x12);
    WriteCmd(0xdb); //--set vcomh
    WriteCmd(0x20); //0x20,0.77xVcc
    WriteCmd(0x8d); //--set DC-DC enable
    WriteCmd(0x14); //
    WriteCmd(0xaf); //--turn on oled panel

    OLED_CLS();

    return 0;
}

/**
  * @brief  OLED_SetPos，设置光标 设置起始点坐标
  * @param  x,光标x位置
  *         y,光标y位置
  * @retval 无
  */
//坐标设置1.3寸
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
void OLED_SetPos(unsigned char x, unsigned char y)
{
    x += 2;
    WriteCmd(0xB0 + y);        //设置页地址（0~7）
    WriteCmd((x >> 4) | 0x10); //设置显示位置—列高地址
    WriteCmd(x & 0x0F);        //设置显示位置—列低地址
}
//void OLED_SetPos(uint8_t x, uint8_t y)
//{
//    WriteCmd(0xB0 + y);        //设置页地址（0~7）
//    WriteCmd(((x & 0xF0) >> 4) | 0x10); //设置显示位置—列高地址
//    WriteCmd((x & 0x0F) | 0x02);        //设置显示位置—列低地址
//}
#else
//坐标设置0.96寸
void OLED_SetPos(unsigned char x, unsigned char y)
{
    WriteCmd(0xb0 + y);
    WriteCmd(((x & 0xf0) >> 4) | 0x10);
    WriteCmd((x & 0x0f) | 0x01);
}
#endif
/**
  * @brief  OLED_Fill，填充整个屏幕 全屏填充
  * @param  fill_Data:要填充的数据
  *            @arg OLED_DISPLAY_FILL: 点亮所有
  *            @arg OLED_DISPLAY_CLEAR: 清屏
  * @retval 无
  */
#if PROD_TYPE == PROD_SFE || PROD_TYPE == PROD_SFB || PROD_TYPE == PROD_SFA
void OLED_Fill(unsigned char fill_Data)
{
    unsigned char m, n;
    for(m = 0; m < 8; m++)
    {
        WriteCmd(0xb0 + m); //page0-page1
        WriteCmd(0x02); //low column start address
        WriteCmd(0x10); //high column start address
        for(n = 0; n < 128; n++)
        {
            WriteData(fill_Data);
        }
    }
}
#else
void OLED_Fill(unsigned char fill_Data)
{
    unsigned char m, n;
    for(m = 0; m < 8; m++)
    {
        WriteCmd(0xb0 + m); //page0-page1
        WriteCmd(0x00); //low column start address
        WriteCmd(0x10); //high column start address
        for(n = 0; n < 128; n++)
        {
            WriteData(fill_Data);
        }
    }
}
#endif

/**
  * @brief  OLED_Fill，填充部分屏幕
  * @param  fill_Data:要填充的数据
  *            @arg OLED_DISPLAY_FILL_WHILE: 点亮所有
  *            @arg OLED_DISPLAY_FILL_BLACK: 熄灭所有
  * @param  x1 [0, 128]
  * @param  x2 [0, 128]
  * @param  y1 [0, 8]
  * @param  y2 [0, 8]
  * @retval 无
  */
void OLED_Fill_Part(unsigned char fill_Data, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2)
{
    unsigned char m, n;

    for(m = y1; m < y2; m++)
    {
        WriteCmd(0xb0 + m); //page0-page1
        WriteCmd((x1 & 0x0f));
        WriteCmd(((x1 & 0xf0) >> 4) | 0x10);
        for(n = x1; n < x2; n++)
        {
            WriteData(fill_Data);
        }
    }
}

/**
  * @brief  OLED_CLS，清屏
  * @param  无
  * @retval 无
  */
void OLED_CLS(void)
{
    OLED_Fill(0x00);
}

/**
  * @brief  OLED_ON，将OLED从休眠中唤醒
  * @param  无
  * @retval 无
  */
void OLED_ON(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X14);  //开启电荷泵
    WriteCmd(0XAF);  //OLED唤醒
}


/**
  * @brief  OLED_OFF，让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
  * @param  无
  * @retval 无
  */
void OLED_OFF(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X10);  //关闭电荷泵
    WriteCmd(0XAE);  //OLED休眠
}

/**
  * @brief  OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
  * @param  x,y : 起始点坐标(x:0~127, y:0~7);
  *         ch[] :- 要显示的字符串;
  *         TextSize : 字符大小(1:6*8 ; 2:8*16)
  *         backlight: OLED_BLACK_ON_WHITE(1)-白底黑字 OLED_WHITE_ON_BLACK(0)-黑底白字
  * @retval 无
 */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize, unsigned char backlight)
{
    unsigned char c = 0, i = 0, j = 0;
    switch(TextSize)
    {
    case 1:
    {
        while(ch[j] != '\0')
        {
            c = ch[j] - 32;
            if(x > 126)
            {
                x = 0;
                y++;
            }
            OLED_SetPos(x, y);
            for(i = 0; i < 6; i++)
            {
                if(backlight)
                {
                    WriteData(~F6x8[c][i]);
                }
                else
                {
                    WriteData(F6x8[c][i]);
                }
            }
            x += 6;
            j++;
        }
    }
    break;
    case 2:
    {
        while(ch[j] != '\0')
        {
            c = ch[j] - 32;
            if(x > 120)
            {
                x = 0;
                y++;
            }
            OLED_SetPos(x, y);
            for(i = 0; i < 8; i++)
            {
                if(backlight)
                {
                    WriteData(~F8X16[c * 16 + i]);
                }
                else
                {
                    WriteData(F8X16[c * 16 + i]);
                }
            }
            OLED_SetPos(x, y + 1);
            for(i = 0; i < 8; i++)
            {
                if(backlight)
                {
                    WriteData(~F8X16[c * 16 + i + 8]);
                }
                else
                {
                    WriteData(F8X16[c * 16 + i + 8]);
                }
            }
            x += 8;
            j++;
        }
    }
    break;
    }
}

/**
  * @brief  OLED_ShowCN，显示codetab.h中的汉字,16*16点阵
  * @param  x,y: 起始点坐标(x:0~127, y:0~7);
  *         N:汉字在codetab.h中的索引
  *         backlight: 1-白底黑字 0-黑底白字
  * @retval 无
  */
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N, unsigned char backlight)
{
    unsigned char wm = 0;
    unsigned int  adder = 32 * N;
    OLED_SetPos(x, y);
    for(wm = 0; wm < 16; wm++)
    {
        if(backlight)
        {
            WriteData(~F16x16[adder]);
        }
        else
        {
            WriteData(F16x16[adder]);
        }
        adder += 1;
    }
    OLED_SetPos(x, y + 1);
    for(wm = 0; wm < 16; wm++)
    {
        if(backlight)
        {
            WriteData(~F16x16[adder]);
        }
        else
        {
            WriteData(F16x16[adder]);
        }
        adder += 1;
    }
}

/**
  * @brief  OLED_DrawBMP，显示BMP位图
  * @param  x0,y0 :起始点坐标(x0:0~127, y0:0~7);
  *         x1,y1 : 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
  * @retval 无
  */
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
    unsigned int j = 0;
    unsigned char x, y;

    if(y1 % 8 == 0)
        y = y1 / 8;
    else
        y = y1 / 8 + 1;
    for(y = y0; y < y1; y++)
    {
        OLED_SetPos(x0, y);
        for(x = x0; x < x1; x++)
        {
            WriteData(BMP[j++]);
        }
    }
}
