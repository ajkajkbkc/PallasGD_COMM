
#ifndef __APP_TOOL_H
#define __APP_TOOL_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/


/* Private defines -----------------------------------------------------------*/
/** @addtogroup Exported_macros
  * @{
  */
#define PAR_SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define PAR_CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define PAR_READ_BIT(REG, BIT)    ((REG) & (BIT))
#define PAR_CLEAR_REG(REG)        ((REG) = (0x0))
#define PAR_WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define PAR_READ_REG(REG)         ((REG))

//#define P_GET_PIN(LOGIC_LEVEL, READ_PIN, Val_Pin)\
//(\
//    LOGIC_LEVEL == 0 ? \
//    (Val_Pin = READ_PIN) : \
//    (Val_Pin = !READ_PIN) \
//)
//#define N_GET_PIN(LOGIC_LEVEL, READ_PIN, Val_Pin)\
//(\
//    LOGIC_LEVEL == 0 ? \
//    (Val_Pin = !READ_PIN) : \
//    (Val_Pin = READ_PIN) \
//)

#define P_GET_PIN(LOGIC, READ_PIN)  (LOGIC == 0 ? READ_PIN : !READ_PIN)
#define N_GET_PIN(LOGIC, READ_PIN)  (LOGIC == 0 ? !READ_PIN : READ_PIN)

/*取指针地址值*/
#define GET_POINT_ADDR(x)           ((unsigned long)(x))

#define GET_PU8_DATA(x)             (*((unsigned char *)(x)))
#define GET_PS8_DATA(x)             (*((char *)(x)))

#define GET_PU16_DATA(x)             (*((unsigned short *)(x)))
#define GET_PS16_DATA(x)             (*((short *)(x)))

#define GET_PU32_DATA(x)             (*((unsigned long *)(x)))
#define GET_PS32_DATA(x)             (*((long *)(x)))

/* 按照小端序取值*/
#define GET_SMLPU16_DATA(x)          (unsigned short)((*(x + 1)<<8) + (*(x)))
#define GET_SMLPU32_DATA(x)          (unsigned long)((*(x+3)<<24) + (*(x+2)<<16) + (*(x+1)<<8) + (*(x)))

/*按照大端序取值*/
#define GET_BIGPU16_DATA(x)          (unsigned short)((*(x)<<8) + (*(x+1)))
#define GET_BIGPU32_DATA(x)          (unsigned long)((*(x)<<24) + (*(x+1)<<16) + (*(x+2)<<8) + (*(x+3)))



/* Private functions ---------------------------------------------------------*/
void hexdump(const void *p, uint16_t size);
uint16_t SMA_Compare(uint16_t *pBuf, uint8_t SmapleNum, uint8_t CountStart, uint8_t CountNum);

//uint8_t Val_To_String(uint16_t Val, uint8_t *pBuf);
uint8_t Val_To_String(uint8_t Sign, uint32_t Val, uint8_t *pBuf);

//uint8_t ValDivideBy10_To_String(uint16_t Val, uint8_t *pBuf);
uint8_t ValDivideBy10_To_String(uint8_t Sign, uint32_t Val, uint8_t *pBuf);

uint8_t ValDivideBy100_To_String(uint16_t Val, uint8_t *pBuf);

uint16_t atoi16(char *str, uint16_t base);
uint32_t atoi32(char *str, uint16_t base);
void itoa(uint16_t n, uint8_t str[5], uint8_t len);
int validatoi(char *str, int base, int *ret);
void replacetochar(char *str, char oldchar, char newchar);
char c2d(uint8_t c	);
uint16_t swaps(uint16_t i);
uint32_t swapl(uint32_t l);
void mid(char *src, char *s1, char *s2, char *sub);
void inet_addr_(unsigned char *addr, unsigned char *ip);
char verify_ip_address(char *src, uint8_t *ip);

uint16_t htons(uint16_t hostshort);
unsigned long htonl(unsigned long hostlong);
unsigned long ntohs(unsigned short netshort);
unsigned long ntohl(unsigned long netlong);


#endif /* __APP_TOOL_H */
