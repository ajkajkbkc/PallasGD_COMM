
/* Private includes ----------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "app_tool.h"

#include "main.h"
#include "app_opts.h"

/* Private define ------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/




/* Private function prototypes -----------------------------------------------*/



/* Private user code ---------------------------------------------------------*/
/**
  * @brief  16进制格式打印数据（如：01 03 AA BB）
  * @param  *p   指向数据的指针
  * @param  size 打印数据的长度
  * @retval None
  */
void hexdump(const void *p, uint16_t size)
{
#if ( ( (UART1_AS_LOG == 1) || (UART2_AS_LOG == 1) || (UART3_AS_LOG == 1) || (UART4_AS_LOG == 1) || (UART5_AS_LOG == 1) ) && (PRINT_LOG_OPEN == 1) )
    const uint8_t *c = p;
    printf("Dumping %u bytes from %p:\r\n", size, p);

    while (size > 0)
    {
        unsigned i;

        for (i = 0; i < 16; i++)
        {
            if (i < size)
                printf("%02X ", c[i]);
            else
                printf("   ");
        }
#if 0
        for (i = 0; i < 16; i++)
        {
            if (i < size)
                PRINTF("%c", c[i] >= 32 && c[i] < 127 ? c[i] : '.');
            else
                PRINTF(" ");
        }
#endif
        printf("\r\n");

        c += 16;

        if (size <= 16)
            break;

        size -= 16;
    }
    printf("\r\n");
#endif
}

/**
  * @brief  输入一串数据，分别去掉一定数量的最大最小的数据，剩下的数据求平均后输出
  * @param  输入数据串(*pWBuf)
  * @param  总采集次数(cySmapleNum)
  * @param  分别去掉最大最小的数量(cyCountStart)
  * @param  剩下求平均数的数量(cyCountNum)
  * @retval 求出的平均数输出
  */
uint16_t SMA_Compare(uint16_t *pBuf, uint8_t SmapleNum, uint8_t CountStart, uint8_t CountNum)
{
    uint8_t i;
    uint8_t j;
    uint8_t cyEnd;
    uint16_t wTemp;
    uint32_t dwSum;

    for (i = 0; i < SmapleNum - 1; i++)
    {
        for (j = i + 1; j < SmapleNum; j++)
        {
            if ( *(pBuf + i) > *(pBuf + j) )
            {
                wTemp = *(pBuf + i);
                *(pBuf + i) = *(pBuf + j);
                *(pBuf + j) = wTemp;
            }
        }
    }

    dwSum = 0;
    cyEnd = CountStart + CountNum;
    for (i = CountStart; i < cyEnd; i++)
    {
        dwSum += *(pBuf + i);
    }

    dwSum /= CountNum;
    return (dwSum);
}

///**
//  * @brief  将值转化为字符串，如：值987转化为"987"
//  * @param  Val: 输入值
//  * @param  *pBuf: 输出字符串
//  * @retval 返回字符串长度
//  */
//uint8_t Val_To_String(uint16_t Val, uint8_t *pBuf)
//{
//    uint8_t cylen;

//    sprintf((char *)pBuf, "%u", Val);
//    cylen = strlen((char *)pBuf);

//    return cylen;
//}

/**
  * @brief  将值转化为字符串，如：值987转化为"987"
  * @param  Sign: 1:有符号值  0:无符号值
  * @param  Val: 输入值
  * @param  *pBuf: 输出字符串
  * @retval 返回字符串长度
  */
uint8_t Val_To_String(uint8_t Sign, uint32_t Val, uint8_t *pBuf)
{
    uint8_t cylen;

    if(Sign)
    {
        sprintf((char *)pBuf, "%d", (signed int)Val);
    }
    else
    {
        sprintf((char *)pBuf, "%u", (unsigned int)Val);
    }
    cylen = strlen((char *)pBuf);

    return cylen;
}

///**
//  * @brief  将值缩小10倍再转化为字符串，如：值987转化为"98.7"
//  * @param  Val: 输入值
//  * @param  *pBuf: 输出字符串
//  * @retval 返回字符串长度
//  */
//uint8_t ValDivideBy10_To_String(uint16_t Val, uint8_t *pBuf)
//{
//    uint8_t cylen;

//    sprintf((char *)pBuf, "%u", Val);
//    cylen = strlen((char *)pBuf);

//    if(cylen == 1)
//    {
//        pBuf[2] = pBuf[0];
//        pBuf[1] = '.';
//        pBuf[0] = '0';
//        cylen = 3;
//    }
//    else
//    {
//        pBuf[cylen] = pBuf[cylen - 1];
//        pBuf[cylen - 1] = '.';
//        cylen += 1;
//    }

//    return cylen;
//}

/**
  * @brief  将值缩小10倍再转化为字符串，如：值987转化为"98.7"
  * @param  Sign: 1:有符号值  0:无符号值
  * @param  Val: 输入值
  * @param  *pBuf: 输出字符串
  * @retval 返回字符串长度
  */
uint8_t ValDivideBy10_To_String(uint8_t Sign, uint32_t Val, uint8_t *pBuf)
{
    uint8_t cylen;

    cylen = Val_To_String(Sign, Val, pBuf);

    if(pBuf[0] == '-' && cylen == 2) //一位数负数
    {
        pBuf[3] = pBuf[1];
        pBuf[2] = '.';
        pBuf[1] = '0';
        cylen = 4;
    }
    else if(cylen == 1) //一位数正数
    {
        pBuf[2] = pBuf[0];
        pBuf[1] = '.';
        pBuf[0] = '0';
        cylen = 3;
    }
    else
    {
        pBuf[cylen] = pBuf[cylen - 1];
        pBuf[cylen - 1] = '.';
        cylen += 1;
    }

    return cylen;
}

/**
  * @brief  将值缩小100倍再转化为字符串，如：值987转化为"9.87"
  * @param  Val: 输入值
  * @param  *pBuf: 输出字符串
  * @retval 返回字符串长度
  */
uint8_t ValDivideBy100_To_String(uint16_t Val, uint8_t *pBuf)
{
    uint8_t cylen;

    sprintf((char *)pBuf, "%u", Val);
    cylen = strlen((char *)pBuf);

    if(cylen == 1)
    {
        pBuf[3] = pBuf[0];
        pBuf[2] = '0';
        pBuf[1] = '.';
        pBuf[0] = '0';
        cylen = 4;
    }
    else if(cylen == 2)
    {
        pBuf[3] = pBuf[1];
        pBuf[2] = pBuf[0];
        pBuf[1] = '.';
        pBuf[0] = '0';
        cylen = 4;
    }
    else
    {
        pBuf[cylen] = pBuf[cylen - 1];
        pBuf[cylen - 1] = pBuf[cylen - 2];
        pBuf[cylen - 2] = '.';
        cylen += 1;
    }

    return cylen;
}

/**
*@brief	 	字符转转化为8位整型函数
*@param		str:要转化字符串， base:
*@return	num:返回转化后的整型数
*/
uint16_t atoi16(char *str, uint16_t base	)
{
    unsigned int num = 0;
    while (*str != 0)
        num = num * base + c2d(*str++);
    return num;
}

/**
*@brief	 	字符转转化为32位整型函数
*@param		str:要转化字符串， base:
*@return	num:返回转化后的整型数
*/
uint32_t atoi32(char *str, uint16_t base	)
{
    uint32_t num = 0;
    while (*str != 0)
        num = num * base + c2d(*str++);
    return num;
}

/**
*@brief	 	整型数转化为字符串函数
*@param		n:要转化整数， str[5]:存放转化后的字符串  len：整型数长度
*@return	无
*/
void itoa(uint16_t n, uint8_t str[5], uint8_t len)
{

    uint8_t i = len - 1;

    memset(str, 0x20, len);
    do
    {
        str[i--] = n % 10 + '0';

    }
    while((n /= 10) > 0);

    return;
}

/**
*@brief	 	把字符串转化为十进制或十六进制数函数
*@param		str:要转化字符串， len：整型数长度
*@return	成功 - 1, 失败 - 0
*/
int validatoi(char *str, int base, int *ret)
{
    int c;
    char *tstr = str;
    if(str == 0 || *str == '\0') return 0;
    while(*tstr != '\0')
    {
        c = c2d(*tstr);
        if( c >= 0 && c < base) tstr++;
        else    return 0;
    }

    *ret = atoi16(str, base);
    return 1;
}

/**
*@brief	 	用新的字符去替换字符串中特殊的字符
*@param		str:替换后字符串，oldchar:特殊的字符，newchar：新的字符
*@return	无
*/
void replacetochar(char *str,	char oldchar, char newchar	)
{
    int x;
    for (x = 0; str[x]; x++)
        if (str[x] == oldchar) str[x] = newchar;
}

/**
*@brief	 	把十进制数转化为字符型
*@param		c:要转化十进制数据
*@return	返回一个字符型数据
*/
char c2d(uint8_t c	)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    if (c >= 'A' && c <= 'F')
        return 10 + c - 'A';

    return (char)c;
}

/**
*@brief	 	16位字符高8位低8位转换
*@param		i:要转化的数据
*@return	转换后的数据
*/
uint16_t swaps(uint16_t i)
{
    uint16_t ret = 0;
    ret = (i & 0xFF) << 8;
    ret |= ((i >> 8) & 0xFF);
    return ret;
}

/**
*@brief	 	32位字符高低位变换
*@param		i:要转化的数据
*@return	转换后的数据
*/
uint32_t swapl(uint32_t l)
{
    uint32_t ret = 0;
    ret = (l & 0xFF) << 24;
    ret |= ((l >> 8) & 0xFF) << 16;
    ret |= ((l >> 16) & 0xFF) << 8;
    ret |= ((l >> 24) & 0xFF);
    return ret;
}

/**
*@brief	 	字符串处理 取出s1 s2之间的字符串
*@param		src目标字符串 s1 s2操作字符串
*@return	无
*/
void mid(char *src, char *s1, char *s2, char *sub)
{
    char *sub1;
    char *sub2;
    uint16_t n;

    sub1 = strstr(src, s1);
    sub1 += strlen(s1);
    sub2 = strstr(sub1, s2);
    n = sub2 - sub1;
    strncpy(sub, sub1, n);
    sub[n] = 0;
}

/**
*@brief	 	ip网络地址转换
*@param		adr：地址 ip：ip
*@return	无
*/
void inet_addr_(unsigned char *addr, unsigned char *ip)
{
    int i;
    char taddr[30];
    char *nexttok;
    char num;
    strcpy(taddr, (char *)addr);

    nexttok = taddr;
    for(i = 0; i < 4 ; i++)
    {
        nexttok = strtok(nexttok, ".");
        if(nexttok[0] == '0' && nexttok[1] == 'x') num = atoi16(nexttok + 2, 0x10);
        else num = atoi16(nexttok, 10);

        ip[i] = num;
        nexttok = NULL;
    }
}

/**
*@brief	 	验证IP地址
*@param		ip addr
*@return	成功 - 1, 失败 - 0
*/
char verify_ip_address(char *src, uint8_t *ip)
{
    int i;
    int tnum;
    char tsrc[50];
    char *tok = tsrc;

    strcpy(tsrc, src);

    for(i = 0; i < 4; i++)
    {
        tok = strtok(tok, ".");
        if ( !tok ) return 0;
        if(tok[0] == '0' && tok[1] == 'x')
        {
            if(!validatoi(tok + 2, 0x10, &tnum)) return 0;
        }
        else if(!validatoi(tok, 10, &tnum)) return 0;

        ip[i] = tnum;

        if(tnum < 0 || tnum > 255) return 0;
        tok = NULL;
    }
    return 1;
}

/**
*@brief		将一个 主机模式的unsigned short型数据转换到大端模式的TCP/IP 网络字节格式的数据.
*@param		要转换的数据
*@return 	大端模式的数据
*/
uint16_t htons(
    uint16_t hostshort	/**< A 16-bit number in host byte order.  */
)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
    return swaps(hostshort);
#else
    return hostshort;
#endif
}

/**
*@brief		将一个 主机模式的unsigned long型数据转换到大端模式的TCP/IP 网络字节格式的数据.
*@param		要转换的数据
*@return 	大端模式的数据
*/
unsigned long htonl(
    unsigned long hostlong		/**< hostshort  - A 32-bit number in host byte order.  */
)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
    return swapl(hostlong);
#else
    return hostlong;
#endif
}



/**
*@brief		将一个大端模式的TCP/IP 网络字节格式的数据转换到主机模式的unsigned short型数据
*@param		要转换的数据
*@return 	unsigned short模式的数据
*/
unsigned long ntohs(
    unsigned short netshort	/**< netshort - network odering 16bit value */
)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
    return htons(netshort);
#else
    return netshort;
#endif
}


/**
*@brief		将一个大端模式的TCP/IP 网络字节格式的数据转换到主机模式的unsigned long型数据
*@param		要转换的数据
*@return 	unsigned long模式的数据
*/
unsigned long ntohl(unsigned long netlong)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
    return htonl(netlong);
#else
    return netlong;
#endif
}


