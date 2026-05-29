
#ifndef __INTERNET_H
#define __INTERNET_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "socket.h"
#include "main.h"
#include "app_opts.h"
#include "cmsis_os.h"
/* Exported types ------------------------------------------------------------*/
extern osThreadId_t internetTaskHandle;


/* Exported constants --------------------------------------------------------*/


/* Private defines -----------------------------------------------------------*/
#define   APP_SOCKET_HUAZIUDP  0
#define   APP_SOCKET_MQTT      1
#define   APP_SOCKET_SNTP      2
#define   APP_SOCKET_MODBUS    3
#define   APP_SOCKET_DHCP      4
#define   APP_SOCKET_DNS       5
#define   APP_SOCKET_HTTPS     6
#define   APP_SOCKET_NETBIOS   7


/*
    0: normal   1: error
    NETLINK_SPI       //MCU与W5500_SPI通信
    NETLINK_PHYLINK   //物理连接
    NETLINK_DHCP      //DHCP获取
*/
/*!< NetLink_State registers */
/*******************  Bit definition for NetLink_State register  *******************/
#define NET_STATE_SPI_Pos                      (0U)
#define NET_STATE_SPI_Msk                      (0x1UL << NET_STATE_SPI_Pos)
#define NET_STATE_SPI                          NET_STATE_SPI_Msk
#define NET_STATE_PHYLINK_Pos                  (1U)
#define NET_STATE_PHYLINK_Msk                  (0x1UL << NET_STATE_PHYLINK_Pos)
#define NET_STATE_PHYLINK                      NET_STATE_PHYLINK_Msk
#define NET_STATE_DHCP_Pos                     (2U)
#define NET_STATE_DHCP_Msk                     (0x1UL << NET_STATE_DHCP_Pos)
#define NET_STATE_DHCP                         NET_STATE_DHCP_Msk
#define NET_STATE_DNS_Pos                      (3U)
#define NET_STATE_DNS_Msk                      (0x1UL << NET_STATE_DNS_Pos)
#define NET_STATE_DNS                          NET_STATE_DNS_Msk
#define NET_STATE_SNTP_Pos                     (4U)
#define NET_STATE_SNTP_Msk                     (0x1UL << NET_STATE_SNTP_Pos)
#define NET_STATE_SNTP                         NET_STATE_SNTP_Msk
#define NET_STATE_MQTT_Pos                     (5U)
#define NET_STATE_MQTT_Msk                     (0x1UL << NET_STATE_MQTT_Pos)
#define NET_STATE_MQTT                         NET_STATE_MQTT_Msk
#define NET_STATE_UDP_Pos                      (6U)
#define NET_STATE_UDP_Msk                      (0x1UL << NET_STATE_UDP_Pos)
#define NET_STATE_UDP                          NET_STATE_UDP_Msk

/* (gParam.st.NetLink_State & NET_STATE_OK) == 0  is net ok ------------- */
#if (USE_INTERNET_DHCP == 1)
#define NET_STATE_OK                           (NET_STATE_SPI | NET_STATE_PHYLINK | NET_STATE_DHCP)
#else
#define NET_STATE_OK                           (NET_STATE_SPI | NET_STATE_PHYLINK)
#endif

/* Private functions ---------------------------------------------------------*/
void W5500_Init(void);

bool Get_PHYLINK(void);

uint8_t Check_Socket_State(void);

void W5500MutexInit(void);
int W5500MutexLock(void);
int W5500MutexUnlock(void);

void osThreadNew_internetTask(void);






#endif /* __INTERNET_H */
