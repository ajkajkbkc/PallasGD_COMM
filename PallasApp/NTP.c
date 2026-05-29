/*******************************************************************************
NTP协议相关函数
 *******************************************************************************/
#include <string.h>
#include <time.h>
#include "NTP.h"
#include "socket.h"
#include <stdio.h>
#include "main.h"
#include "app_log.h"
#include "app_tool.h"

volatile int NTP_day;
int GET_day;
extern RTC_HandleTypeDef hrtc;
#define RTC_BKP_DR0                       0x00000000U
volatile struct tm * Net_time;

uint8_t NTPUpdate_FLAG=0;
//NTP报文定义
typedef struct
{

	uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
													 // li.   Two bits.   Leap indicator.
													 // vn.   Three bits. Version number of the protocol.
													 // mode. Three bits. Client will pick mode 3 for client.

	uint8_t stratum;         // Eight bits. Stratum level of the local clock.
	uint8_t poll;            // Eight bits. Maximum interval between successive messages.
	uint8_t precision;       // Eight bits. Precision of the local clock.

	uint32_t rootDelay;      // 32 bits. Total round trip delay time.
	uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
	uint32_t refId;          // 32 bits. Reference clock identifier.

	uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
	uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

	uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
	uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

	uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
	uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

	uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
	uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total:  48 bytes.


void NTPUpdateFunTask(void)
{
	uint8_t NTP_Data[48]={0};
	//uint8_t addr[4]={202,112,31,197};
	//uint8_t addr[4]={192,168,4,14};
	uint8_t addr[4]={120,24,166,46};
	uint8_t i;
	uint16_t port=0;
	uint16_t *Pport=&port;
	int32_t m;
	m=socket(3,Sn_MR_UDP,7900,0x00);
	if(m==3)LOGE("NTPUpdateFunTask","socket open ok!");	
	
	do{
			for(i=0;i<48;i++)NTP_Data[i]=0;
			NTP_Data[0]=0x1b;
			sendto(3,NTP_Data,48,addr,123);
			m=recvfrom(3,NTP_Data,48,addr,Pport);
		}while(m<48);
	
	NTP_DataHandle(NTP_Data);
	RTC_Update();		

}







void NTP_DataHandle(uint8_t *buf)
{
	uint32_t local_timestamp; 
	ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
 
	packet.txTm_s = buf[40]<<24 | buf[40+1]<<16|buf[40+2]<<8 |buf[40+3];
	
  local_timestamp = packet.txTm_s - NTP_TIMESTAMP_DELTA;
	local_timestamp +=SEC_TIME_ZONE; 

  Net_time = localtime(&local_timestamp); 
	
	//printf("Received Time: ");
	//printf("%d-%02d-%02d   %02d:%02d:%02d\r\n",\
		Net_time->tm_year+1900, Net_time->tm_mon+1, Net_time->tm_mday,\
		Net_time->tm_hour,Net_time->tm_min,Net_time->tm_sec);
	NTP_day= Net_time->tm_mday;// 存取日期值，每次读取日期都进行比较，不一样便更新RTC



	
//	  time_t sec;       //typedef long time_t  
//	  sec=local_timestamp;
//    struct tm * curTime;  
//  
//    //sec = time(NULL);           
//    curTime = localtime(&sec);   
//  
//    printf("asctime(curTime): %s\n", asctime (curTime));  
//        
//  
//    printf("ctime(curTime):   %s\n", ctime (&sec));  
//      
//  
//    printf("%d-%d-%d %d:%d:%d\n", 1900+curTime->tm_year, 1+curTime->tm_mon,  
//    curTime->tm_mday, curTime->tm_hour, curTime->tm_min, curTime->tm_sec);  
//    
//    printf("\nsec = %ld\nmktime(curTime) = %ld\n", sec, mktime(curTime));  
   
	
	
	

//	char *str=ctime(&local_timestamp);	
//	while((&huart1)->gState != HAL_UART_STATE_READY);
//	HAL_UART_Transmit_DMA(&huart1,(uint8_t *)str,26);
	



}



// RTC更新函数
void RTC_Update(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;


	/*##-1- Configure the Date #################################################*/
	sdatestructure.Year =  Net_time->tm_year+1900-2000;
	sdatestructure.Month =  Net_time->tm_mon+1;
	sdatestructure.Date = Net_time->tm_mday;
	sdatestructure.WeekDay = Net_time->tm_wday;
	
	if (HAL_RTC_SetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}

	/*##-2- Configure the Time #################################################*/
	stimestructure.Hours =  Net_time->tm_hour;
	stimestructure.Minutes =  Net_time->tm_min;
	stimestructure.Seconds = Net_time->tm_sec;
//  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
//  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
//  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

	if (HAL_RTC_SetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F6);
}

//RTC打印显示
void RTC_Show(void)
{
		RTC_DateTypeDef sdatestructure;
		RTC_TimeTypeDef stimestructure;	
		HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
		//LOGE("RTC_Show", "%02d/%02d/%02d\r\n",2000 + sdatestructure.Year, sdatestructure.Month, sdatestructure.Date);
		//LOGE("RTC_Show","%02d:%02d:%02d\r\n",stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
		GET_day=sdatestructure.Date;
		if(GET_day!=NTP_day)//日期一改变，就更新RTC
			NTPUpdateFunTask();
}

