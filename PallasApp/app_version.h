
#ifndef __APP_VERSION_H
#define __APP_VERSION_H

///*
//  0.0.0606.01
//  0    ： 主版本号
//  0    ： 副版本号
//  0606 ： 日期月日
//  01   ： 一日的临时版本号
//*/
//#define SW_VERSION  "1.4.0621.01"

/*实际上飞凌核心板GPIO_AD_B0_09是作为LED使用的，
 *Kalyke的P1板将不再使用该脚作为X0
 */
//#define X0_AS_LED

#define SW_VERSION  "1.2.0312.01"

#define PROGRAM_CAPACITY    64
/**
 * MiStudio收到该值后，会除以1000得到PLC版本号
 * 例如：1002 -> 1.002
 *          1 -> 0.001
 *
 * 每次提供新的OTA升级固件时，该值必须增加（每次加一即可）。
 * 每个具体客户的版本号可达10000个，从0.000至9.999
 */
#define FIRMWARE_IMAGE_ID    55U

#endif
