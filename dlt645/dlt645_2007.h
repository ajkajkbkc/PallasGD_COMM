#ifndef __DLT645_2007_H
#define __DLT645_2007_H

#include <stdbool.h>
#include <stdint.h>
#include "mb.h"


/*------------------------------------------------------------------------------
*   DLT645 -2007 子功能码定义
*-----------------------------------------------------------------------------*/
/*主站请求帧:请求读电能表数据*/
#define DLT645_READ          					0x11
/*从站正常应答帧 */
#define DLT645_READ_RESPOND          	0x91
/*从站异常应答帧*/
#define DLT645_READ_ERR         			0xD1

/*主站请求帧:读后续数据*/
#define DLT645_READ_FOLLOW          	0x12
/*从站正常应答帧 */
#define DLT645__READ_RESPOND_FOLLOW   0x92
/*从站异常应答帧*/
#define DLT645_READ_FOLLOW_ERR        0xD2

/*主站请求帧:写数据*/
#define DLT645_WRITE          				0x14
/*从站正常应答帧 */
#define DLT645__WRITE_RESPOND    			0x94
/*从站异常应答帧*/
#define DLT645_WRITE_ERR          		0xD4

/*主站请求帧:读通信地址*/
#define DLT645_READ_ADDR          		0x13
/*从站正常应答帧 */
#define DLT645_READ_ADDR_RESPOND    	0x93
/*从站异常应答帧*/
#define DLT645_READ_ADDR_ERR         	0xD3

/*主站请求帧:写通信地址*/
#define DLT645_WRITE_ADDR      				0x15
/*从站正常应答帧 */
#define DLT645__WRITE_ADDR_RESPOND   	0x95
/*从站异常不应答*/

bool is_dlt645_protocol(uint8_t *pData, uint16_t len);
void DLT645_2007_DataAnalysis(md_slave_msg_pack *pMsg);

#endif
