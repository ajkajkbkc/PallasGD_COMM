
#ifndef __NETBIOS_H
#define __NETBIOS_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private defines -----------------------------------------------------------*/
#define NETBIOS_W5500_NAME    "MODULE"        /* 定义NetBIOS名字*/

#define NETBIOS_SOCK     APP_SOCKET_NETBIOS   /* 定义NetBIOS采用的socket*/
#define NETBIOS_PORT     137                  /* "NetBIOS Name service"的默认端口 */


#define NETBIOS_NAME_LEN 16                   /*NetBIOS名字最大长度*/

#define NETBIOS_MSG_MAX_LEN 512               /*NetBIOS报文的最大长度 */

#define NETBIOS_NAME_TTL 10                   //300000 /*NetBIOS响应时间*/


/** NetBIOS header flags */
#define NETB_HFLAG_RESPONSE           0x8000U
#define NETB_HFLAG_OPCODE             0x7800U
#define NETB_HFLAG_OPCODE_NAME_QUERY  0x0000U
#define NETB_HFLAG_AUTHORATIVE        0x0400U
#define NETB_HFLAG_TRUNCATED          0x0200U
#define NETB_HFLAG_RECURS_DESIRED     0x0100U
#define NETB_HFLAG_RECURS_AVAILABLE   0x0080U
#define NETB_HFLAG_BROADCAST          0x0010U
#define NETB_HFLAG_REPLYCODE          0x0008U
#define NETB_HFLAG_REPLYCODE_NOERROR  0x0000U

/** NetBIOS name flags */
#define NETB_NFLAG_UNIQUE             0x8000U
#define NETB_NFLAG_NODETYPE           0x6000U
#define NETB_NFLAG_NODETYPE_HNODE     0x6000U
#define NETB_NFLAG_NODETYPE_MNODE     0x4000U
#define NETB_NFLAG_NODETYPE_PNODE     0x2000U
#define NETB_NFLAG_NODETYPE_BNODE     0x0000U


/* Exported types ------------------------------------------------------------*/
/** NetBIOS message header */
#pragma pack(1)
typedef struct _NETBIOS_HDR
{
    uint16_t trans_id;
    uint16_t flags;
    uint16_t questions;
    uint16_t answerRRs;
    uint16_t authorityRRs;
    uint16_t additionalRRs;
} NETBIOS_HDR;


/** NetBIOS message name part */
typedef struct _NETBIOS_NAME_HDR
{
    uint8_t  nametype;
    uint8_t  encname[(NETBIOS_NAME_LEN * 2) + 1];
    uint16_t type;
    uint16_t cls;
    uint32_t ttl;
    uint16_t datalen;
    uint16_t flags;
    //ip_addr_p_t addr;
    uint8_t addr[4];
} NETBIOS_NAME_HDR;


/** NetBIOS 报文结构体 */
typedef struct _NETBIOS_RESP
{
    NETBIOS_HDR      resp_hdr;
    NETBIOS_NAME_HDR resp_name;
} NETBIOS_RESP;
#pragma pack()



/* Exported constants --------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
void do_netbios(void);
void osThreadNew_netbiosTask(void);

#endif /* __NETBIOS_H */
