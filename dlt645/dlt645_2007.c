#include "dlt645_2007.h"
#include <stdio.h>
#include <string.h>
#include "app_parameter.h"
#include <ctype.h>
#include "app_log.h"
#include "app_att7022eu.h"


uint8_t SlaveAddr[6] = {0};
uint8_t SlaveDataID[4] = {0};

#define ENERGY_ACTIVE_CHAR				'A' 					//65
#define ENERGY_REACTIVE_CHAR			'R' 					//82

#define EPI_A_CHAR								'P'+ 'I'+ 'A' //218
#define EPI_B_CHAR								'P'+ 'I'+ 'B' //219
#define EPI_C_CHAR								'P'+ 'I'+ 'C' //220

#define EPE_A_CHAR								'P'+ 'E'+ 'A' //214
#define EPE_B_CHAR								'P'+ 'E'+ 'B' //215
#define EPE_C_CHAR								'P'+ 'E'+ 'C' //216

#define VOLTAGE_A_CHAR						'U'+ 'A' 			//150
#define VOLTAGE_B_CHAR						'U'+ 'B'			//151
#define VOLTAGE_C_CHAR						'U'+ 'C'			//152
#define VOLTAGE_Z_CHAR						'U'+ 'Z'			//175

#define CURRENT_A_CHAR						'I'+ 'A'			//138
#define CURRENT_B_CHAR						'I'+ 'B'			//139
#define CURRENT_C_CHAR						'I'+ 'C'			//140
#define CURRENT_Z_CHAR						'I'+ 'Z'			//163

#define POWER_0_CHAR							'P'						//80
#define POWER_A_CHAR							'P'+ '1'			//129
#define POWER_B_CHAR							'P'+ '2'			//130
#define POWER_C_CHAR							'P'+ '3'			//131
#define POWER_Z_CHAR							'P'+ '9'			//137

#define REACTIVE_POWER_0_CHAR			'Q'						//81
#define REACTIVE_POWER_A_CHAR			'Q'+ 'A'			//146
#define REACTIVE_POWER_B_CHAR			'Q'+ 'B'      //147
#define REACTIVE_POWER_C_CHAR			'Q'+ 'C'			//148
#define REACTIVE_POWER_Z_CHAR			'Q'+ 'Z'			//171

#define APPARENT_POWER_0_CHAR			'S'						//83
#define APPARENT_POWER_A_CHAR			'S'+ 'a'			//180
#define APPARENT_POWER_B_CHAR			'S'+ 'b'			//181
#define APPARENT_POWER_C_CHAR			'S'+ 'c'			//182
#define APPARENT_POWER_Z_CHAR			'S'+ 'z'			//205

#define POWER_FACTOR_0_CHAR				'P'+ 'F'+ '0'	//198
#define POWER_FACTOR_A_CHAR				'P'+ 'F'+ '1'	//199
#define POWER_FACTOR_B_CHAR				'P'+ 'F'+ '2'	//200
#define POWER_FACTOR_C_CHAR				'P'+ 'F'+ '3'	//201
#define POWER_FACTOR_Z_CHAR				'P'+ 'F'+ '9'	//207

#define ZERO_LINE_CHAR						'I'+ '0'			//121
#define FREQ_CHAR									'F'						//70

const uint16_t SlaveDataID_Map[][5] =
{
	/*数据标识										DI3   DI2   DI1   DI0*/
	{ENERGY_ACTIVE_CHAR, 				0x00, 0x01, 0x00, 0x00},	//正向有功总电能		 4字节	2位小数，单位kWh
	{ENERGY_REACTIVE_CHAR, 			0x00, 0x02, 0x00, 0x00},	//反向有功总电能     4字节  2位小数，单位kWh
	
	{EPI_A_CHAR, 								0x00, 0x15, 0x00, 0x00},	//A相正向有功电能		 4字节，2位小数，单位kWh
	{EPE_A_CHAR, 								0x00, 0x16, 0x00, 0x00},	//A相反向有功电能		 4字节，2位小数，单位kWh
	
	{EPI_B_CHAR, 								0x00, 0x29, 0x00, 0x00},	//B相正向有功电能		 4字节，2位小数，单位kWh
	{EPE_B_CHAR, 								0x00, 0x2A, 0x00, 0x00},	//B相反向有功电能		 4字节，2位小数，单位kWh
	
	{EPI_C_CHAR, 								0x00, 0x3D, 0x00, 0x00},	//C相正向有功电能		 4字节，2位小数，单位kWh
	{EPE_C_CHAR, 								0x00, 0x3E, 0x00, 0x00},	//C相反向有功电能		 4字节，2位小数，单位kWh

	{VOLTAGE_A_CHAR, 						0x02, 0x01, 0x01, 0x00},	//A相电压		         2字节，1位小数，单位V
	{VOLTAGE_B_CHAR, 						0x02, 0x01, 0x02, 0x00},	//B相电压		         2字节，1位小数，单位V
	{VOLTAGE_C_CHAR, 						0x02, 0x01, 0x03, 0x00},	//C相电压		         2字节，1位小数，单位V
	{VOLTAGE_Z_CHAR, 						0x02, 0x01, 0xFF, 0x00},	//电压数据块		     6字节，1位小数，单位V
	
	{CURRENT_A_CHAR, 						0x02, 0x02, 0x01, 0x00},	//A相电流		         3字节，3位小数，单位A
	{CURRENT_B_CHAR, 						0x02, 0x02, 0x02, 0x00},	//B相电流		         3字节，3位小数，单位A
	{CURRENT_C_CHAR, 						0x02, 0x02, 0x03, 0x00},	//C相电流		         3字节，3位小数，单位A
	{CURRENT_Z_CHAR, 						0x02, 0x02, 0xFF, 0x00},	//电流数据块		     9字节，3位小数，单位A
	
	{POWER_0_CHAR, 							0x02, 0x03, 0x00, 0x00},	//总有功功率	   	   3字节，4位小数，单位kW
	{POWER_A_CHAR, 							0x02, 0x03, 0x01, 0x00},	//A相有功功率	       3字节，4位小数，单位kW
	{POWER_B_CHAR, 							0x02, 0x03, 0x02, 0x00},	//B相有功功率	       3字节，4位小数，单位kW
	{POWER_C_CHAR, 							0x02, 0x03, 0x03, 0x00},	//C相有功功率	       3字节，4位小数，单位kW
	{POWER_Z_CHAR, 							0x02, 0x03, 0xFF, 0x00},	//有功功率数据块	   12字节，4位小数，单位kW
	
	{REACTIVE_POWER_0_CHAR, 		0x02, 0x04, 0x00, 0x00},	//总无功功率	       3字节，4位小数，单位kvar
	{REACTIVE_POWER_A_CHAR, 		0x02, 0x04, 0x01, 0x00},	//A相无功功率	       3字节，4位小数，单位kvar
	{REACTIVE_POWER_B_CHAR, 		0x02, 0x04, 0x02, 0x00},	//B相无功功率	       3字节，4位小数，单位kvar
	{REACTIVE_POWER_C_CHAR, 		0x02, 0x04, 0x03, 0x00},	//C相无功功率	       3字节，4位小数，单位kvar
	{REACTIVE_POWER_Z_CHAR, 		0x02, 0x04, 0xFF, 0x00},	//无功功率数据块 	   12字节，4位小数，单位kvar
	
	{APPARENT_POWER_0_CHAR, 		0x02, 0x05, 0x00, 0x00},	//总视在功率	       3字节，4位小数，单位kVA
	{APPARENT_POWER_A_CHAR, 		0x02, 0x05, 0x01, 0x00},	//A相视在功率	       3字节，4位小数，单位kVA
	{APPARENT_POWER_B_CHAR, 		0x02, 0x05, 0x02, 0x00},	//B相视在功率	       3字节，4位小数，单位kVA
	{APPARENT_POWER_C_CHAR, 		0x02, 0x05, 0x03, 0x00},	//C相视在功率	       3字节，4位小数，单位kVA
	{APPARENT_POWER_Z_CHAR, 		0x02, 0x05, 0xFF, 0x00},	//视在功率数据块	   12字节，4位小数，单位kVA
					
	{POWER_FACTOR_0_CHAR, 			0x02, 0x06, 0x00, 0x00},	//总功率因数	       2字节，3位小数
	{POWER_FACTOR_A_CHAR, 			0x02, 0x06, 0x01, 0x00},	//A相功率因数	       2字节，3位小数
	{POWER_FACTOR_B_CHAR, 			0x02, 0x06, 0x02, 0x00},	//B相功率因数	       2字节，3位小数
	{POWER_FACTOR_C_CHAR, 			0x02, 0x06, 0x03, 0x00},	//C相功率因数	       2字节，3位小数
	{POWER_FACTOR_Z_CHAR, 			0x02, 0x06, 0xFF, 0x00},	//功率因数数据块	   8字节，3位小数
	
	{ZERO_LINE_CHAR, 						0x02, 0x80, 0x00, 0x01},	//零线电流		       2字节，3位小数，单位A
	{FREQ_CHAR, 								0x02, 0x80, 0x00, 0x02},	//电网频率		       2字节，2位小数，单位Hz
};
#define DATA_ID_MAP_NUM		(sizeof(SlaveDataID_Map) / 5)

void hex_to_decimal_str(unsigned char hex_value, char *output) {
    snprintf(output, 4, "%03d", hex_value);
}

void convert_string(const char *input, char *output) {
    for (int i = 0; i < 6; i++) {
        unsigned char high, low;
        char c_high = input[2 * i];
        char c_low = input[2 * i + 1];

        // 处理高位字符
        if (c_high >= '0' && c_high <= '9') {
            high = c_high - '0';
        } else if (c_high >= 'A' && c_high <= 'F') {
            high = 10 + (c_high - 'A');
        } else if (c_high >= 'a' && c_high <= 'f') {
            high = 10 + (c_high - 'a');
        } else {
            high = 0; // 处理非法字符，默认设为0
        }

        // 处理低位字符
        if (c_low >= '0' && c_low <= '9') {
            low = c_low - '0';
        } else if (c_low >= 'A' && c_low <= 'F') {
            low = 10 + (c_low - 'A');
        } else if (c_low >= 'a' && c_low <= 'f') {
            low = 10 + (c_low - 'a');
        } else {
            low = 0; // 处理非法字符，默认设为0
        }

        output[i] = (high << 4) | low;
    }
}

/**
  * @brief  判断是否是dlt645协议
  * @param  *pData 数据
  * @param  len 数据长度
  * @retval true/false
  */
bool is_dlt645_protocol(uint8_t *pData, uint16_t len)
{
    if (pData[0] != 0x68) //判断帧起始符
    {
        return false;
    }
		
		if (pData[1] == 0xAA && pData[2] == 0xAA && pData[3] == 0xAA && pData[4] == 0xAA
			 && pData[5] == 0xAA && pData[6] == 0xAA) //读取设备地址，固定编号
    {
        return true;
    }
		
		char str[13];
		memcpy(str, &gFlashParam.st.idInfo[7], 9);
		char idstr[4];
    hex_to_decimal_str(gFlashParam.st.idNum, idstr);
		memcpy(&str[9], idstr, 3);
		str[12] = '\0';
	
	  char str_6[6];
		convert_string(str, str_6);
		
		char pDATA_6[6];
		
		pDATA_6[0] = pData[6];
		pDATA_6[1] = pData[5];
		pDATA_6[2] = pData[4];
		pDATA_6[3] = pData[3];
		pDATA_6[4] = pData[2];
		pDATA_6[5] = pData[1];
		
		int result = strcmp(str_6,pDATA_6);

		if (result == 0)
    {
        return true;
    }
		else
		{
				return false;
		}
}

/**
  * @brief  校验和计算
  * @note
  * @param
  * @retval
  * @author PWH
  * @date   2023/10
  */
static uint8_t DLT645_2007_Checksum_Calc(unsigned char *array, uint8_t len)
{
	uint16_t sum = 0;
	uint8_t i;

	for (i = 0; i < len; i++)
	{
		sum += array[i];
	}
	return ((uint8_t)sum);
}

static char DLT645_2007_Get_DataIDChar(uint8_t *array)
{
	uint8_t i;

	for (i = 0; i < DATA_ID_MAP_NUM; i++)
	{
		
		if (array[0] == SlaveDataID_Map[i][1]
			&& array[1] == SlaveDataID_Map[i][2]
				&& array[2] == SlaveDataID_Map[i][3]
					&&array[3] == SlaveDataID_Map[i][4])
		{
			return SlaveDataID_Map[i][0];
		}
	}

	return 0;
}

void float_to_XXX_X(float f, unsigned char *byte1, unsigned char *byte2) {
    char buffer[10];
    // 转换为字符串，保留足够精度
    snprintf(buffer, sizeof(buffer), "%.1f", f);

    // 分割整数和小数部分
    char *int_part = buffer;
    char *decimal_part = strchr(buffer, '.');
    if (decimal_part) {
        *decimal_part = '\0';  // 分离整数部分
        decimal_part++;        // 小数部分指针
    } else {
        decimal_part = "";     // 无小数部分时默认空
    }

    // 处理整数部分：补零到3位，超过则取后三位
    char int_str[4] = "000";
    int int_len = strlen(int_part);
    if (int_len >= 3) {
        strncpy(int_str, int_part + int_len - 3, 3);  // 取后三位
    } else {
        strncpy(int_str + (3 - int_len), int_part, int_len);  // 前补零
    }

    // 处理小数部分：取第一位，无则补零
    char decimal_digit = '0';
    if (*decimal_part != '\0') {
        decimal_digit = *decimal_part;
    }

    // 组合为4位数字：3位整数 + 1位小数
    char digits[5];
    snprintf(digits, sizeof(digits), "%s%c", int_str, decimal_digit);

    // 转换为两个十六进制字节
    *byte1 = (digits[0] - '0') << 4 | (digits[1] - '0');
    *byte2 = (digits[2] - '0') << 4 | (digits[3] - '0');
}

void float_to_XXX_XXX(float f, unsigned char *byte1, unsigned char *byte2, unsigned char *byte3) {
    char buffer[10];
    // 转换为字符串，保留足够精度避免科学计数法
    snprintf(buffer, sizeof(buffer), "%.3f", f);

    // 分割整数和小数部分
    char *int_part = buffer;
    char *decimal_part = strchr(buffer, '.');
    if (decimal_part) {
        *decimal_part = '\0';  // 分离整数部分
        decimal_part++;        // 小数部分指针
    } else {
        decimal_part = "0";    // 无小数时默认0
    }

    // 处理整数部分：补零到3位，超过则取后三位
    char processed_int[4] = "000";
    int int_len = strlen(int_part);
    if (int_len >= 3) {
        strncpy(processed_int, int_part + int_len - 3, 3);
    } else {
        strncpy(processed_int + (3 - int_len), int_part, int_len);
    }

    // 处理小数部分：取前3位，不足补零
    char processed_decimal[4] = "000";
    int decimal_len = strlen(decimal_part);
    if (decimal_len >= 3) {
        strncpy(processed_decimal, decimal_part, 3);
    } else {
        strncpy(processed_decimal, decimal_part, decimal_len);
        memset(processed_decimal + decimal_len, '0', 3 - decimal_len);
    }

    // 组合为6位数字：3位整数 + 3位小数
    char digits[7];
    snprintf(digits, sizeof(digits), "%s%s", processed_int, processed_decimal);

    // 转换为三个十六进制字节
    *byte1 = (digits[0] - '0') << 4 | (digits[1] - '0');
    *byte2 = (digits[2] - '0') << 4 | (digits[3] - '0');
    *byte3 = (digits[4] - '0') << 4 | (digits[5] - '0');
}

void float_to_XX_XXXX(float f, unsigned char *byte1, unsigned char *byte2, unsigned char *byte3) {
    char buffer[10];
    // 转换为字符串，保留足够精度避免科学计数法
    snprintf(buffer, sizeof(buffer), "%.4f", f);

    // 分割整数和小数部分
    char *int_part = buffer;
    char *decimal_part = strchr(buffer, '.');
    if (decimal_part) {
        *decimal_part = '\0';  // 分离整数部分
        decimal_part++;        // 小数部分指针
    } else {
        decimal_part = "0000"; // 无小数时默认补零
    }

    // 处理整数部分：补零到2位，超过则取后两位
    char processed_int[3] = "00";
    int int_len = strlen(int_part);
    if (int_len >= 2) {
        strncpy(processed_int, int_part + int_len - 2, 2);
    } else {
        strncpy(processed_int + (2 - int_len), int_part, int_len);
    }

    // 处理小数部分：取前4位，不足补零
    char processed_decimal[5] = "0000";
    int decimal_len = strlen(decimal_part);
    if (decimal_len >= 4) {
        strncpy(processed_decimal, decimal_part, 4);
    } else {
        strncpy(processed_decimal, decimal_part, decimal_len);
        memset(processed_decimal + decimal_len, '0', 4 - decimal_len);
    }

    // 组合为6位数字：2位整数 + 4位小数
    char digits[7]; // 6位数字 + 终止符
    snprintf(digits, sizeof(digits), "%s%s", processed_int, processed_decimal);

    // 转换为三个十六进制字节
    *byte1 = (digits[0] - '0') << 4 | (digits[1] - '0');
    *byte2 = (digits[2] - '0') << 4 | (digits[3] - '0');
    *byte3 = (digits[4] - '0') << 4 | (digits[5] - '0');
}

void float_to_X_XXX(float f, unsigned char *byte1, unsigned char *byte2) {
    char buffer[10];
    // 转换为字符串，保留足够精度避免科学计数法
    snprintf(buffer, sizeof(buffer), "%.3f", f);

    // 分割整数和小数部分
    char *int_part = buffer;
    char *decimal_part = strchr(buffer, '.');
    if (decimal_part) {
        *decimal_part = '\0';  // 分离整数部分
        decimal_part++;        // 小数部分指针
    } else {
        decimal_part = "000";  // 无小数时默认补零
    }

    // 处理整数部分：取最后一位，若为空则默认为 '0'
    char int_digit = '0';
    if (strlen(int_part) > 0) {
        int int_len = strlen(int_part);
        int_digit = int_part[int_len - 1];  // 取最后一位字符
    }

    // 处理小数部分：取前3位，不足补零
    char decimal_digits[4] = "000";
    int decimal_len = strlen(decimal_part);
    if (decimal_len >= 3) {
        strncpy(decimal_digits, decimal_part, 3);
    } else {
        strncpy(decimal_digits, decimal_part, decimal_len);
        memset(decimal_digits + decimal_len, '0', 3 - decimal_len);
    }

    // 组合为4位数字：1位整数 + 3位小数
    char digits[5];
    snprintf(digits, sizeof(digits), "%c%c%c%c", 
             int_digit, 
             decimal_digits[0], 
             decimal_digits[1], 
             decimal_digits[2]);

    // 转换为两个十六进制字节
    *byte1 = (digits[0] - '0') << 4 | (digits[1] - '0');
    *byte2 = (digits[2] - '0') << 4 | (digits[3] - '0');
}

void float_to_XXXXXX_XX(float f, unsigned char *b1, unsigned char *b2, unsigned char *b3, unsigned char *b4) {
    char buffer[10];
    // 转换为字符串，保留足够精度避免科学计数法
    snprintf(buffer, sizeof(buffer), "%.2f", f);

    // 分割整数和小数部分
    char *int_part = buffer;
    char *decimal_part = strchr(buffer, '.');
    if (decimal_part) {
        *decimal_part = '\0';  // 分离整数部分
        decimal_part++;        // 小数部分指针
    } else {
        decimal_part = "00";   // 无小数时默认补零
    }

    // 处理整数部分：补零到6位，超过则取后6位
    char processed_int[7] = "000000";  // 6位整数 + 终止符
    int int_len = strlen(int_part);
    if (int_len >= 6) {
        strncpy(processed_int, int_part + int_len - 6, 6);
    } else {
        strncpy(processed_int + (6 - int_len), int_part, int_len);
    }

    // 处理小数部分：取前2位，不足补零
    char processed_decimal[3] = "00";
    int decimal_len = strlen(decimal_part);
    if (decimal_len >= 2) {
        strncpy(processed_decimal, decimal_part, 2);
    } else {
        strncpy(processed_decimal, decimal_part, decimal_len);
        memset(processed_decimal + decimal_len, '0', 2 - decimal_len);
    }

    // 组合为8位数字：6位整数 + 2位小数
    char digits[9];
    snprintf(digits, sizeof(digits), "%s%s", processed_int, processed_decimal);

    // 转换为四个十六进制字节
    *b1 = (digits[0] - '0') << 4 | (digits[1] - '0');
    *b2 = (digits[2] - '0') << 4 | (digits[3] - '0');
    *b3 = (digits[4] - '0') << 4 | (digits[5] - '0');
    *b4 = (digits[6] - '0') << 4 | (digits[7] - '0');
}

void float_to_XX_XX(float f, unsigned char *b1, unsigned char *b2) {
    char buffer[50];
    // 转换为字符串，保留足够精度避免科学计数法
    snprintf(buffer, sizeof(buffer), "%.2f", f);

    // 分割整数和小数部分
    char *int_part = buffer;
    char *decimal_part = strchr(buffer, '.');
    if (decimal_part) {
        *decimal_part = '\0';  // 分离整数部分
        decimal_part++;        // 小数部分指针
    } else {
        decimal_part = "00";   // 无小数时默认补零
    }

    // 处理整数部分：补零到2位，超过则取后两位
    char processed_int[3] = "00";
    int int_len = strlen(int_part);
    if (int_len >= 2) {
        strncpy(processed_int, int_part + int_len - 2, 2);  // 取后两位
    } else {
        strncpy(processed_int + (2 - int_len), int_part, int_len);  // 前补零
    }

    // 处理小数部分：取前2位，不足补零
    char processed_decimal[3] = "00";
    int decimal_len = strlen(decimal_part);
    if (decimal_len >= 2) {
        strncpy(processed_decimal, decimal_part, 2);
    } else {
        strncpy(processed_decimal, decimal_part, decimal_len);
        memset(processed_decimal + decimal_len, '0', 2 - decimal_len);
    }

    // 组合为4位数字：2位整数 + 2位小数
    char digits[5];
    snprintf(digits, sizeof(digits), "%s%s", processed_int, processed_decimal);

    // 转换为两个十六进制字节
    *b1 = (digits[0] - '0') << 4 | (digits[1] - '0');
    *b2 = (digits[2] - '0') << 4 | (digits[3] - '0');
}
/**
  * @brief  解析从站dlt645协议
  * @param  *pMsg 数据指针
  * @retval None
  */
void DLT645_2007_DataAnalysis(md_slave_msg_pack *pMsg)
{
    LOGE("DLT645", "Enter %s\r\n", __func__);

		if(pMsg->mcp_ReceiveBuff[7] != 0x68)
    {
        return;
    }
		
		int8_t i, j;
		unsigned char DataIDChar;
		unsigned char b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12;
		uint8_t pos_68 = 0, pos_16 = 0;
		
		char str[13];
		memcpy(str, &gFlashParam.st.idInfo[7], 9);
		char idstr[4];
		hex_to_decimal_str(gFlashParam.st.idNum, idstr);
		memcpy(&str[9], idstr, 3);
		str[12] = '\0';

		char str_6[6];
		convert_string(str, str_6);
		
		if (pMsg->msv_ReceiveLen)
		{
			for (i = 0; i < pMsg->msv_ReceiveLen; i++)
			{
				if (0x68 == pMsg->mcp_ReceiveBuff[i])
				{
					pos_68 = i;	//寻找到起始码
					break;
				}
			}
			for (i = pMsg->msv_ReceiveLen; i >= 0; i--)
			{
				if (0x16 == pMsg->mcp_ReceiveBuff[i])
				{
					pos_16 = i;	//寻到结束码
					break;
				}
			}

			if (pos_16 > pos_68)
			{
				if(DLT645_2007_Checksum_Calc(pMsg->mcp_ReceiveBuff, pos_16 - pos_68 - 1) == pMsg->mcp_ReceiveBuff[pos_16 - 1])		//校验通过
				{
					switch (pMsg->mcp_ReceiveBuff[pos_68 + 8])	//控制码
					{
					case DLT645_READ_ADDR:	//读地址的应答					
						pMsg->mcp_RespBuff[0] = 0x68;
						pMsg->mcp_RespBuff[1] = str_6[5];
						pMsg->mcp_RespBuff[2] = str_6[4];
						pMsg->mcp_RespBuff[3] = str_6[3];
						pMsg->mcp_RespBuff[4] = str_6[2];
						pMsg->mcp_RespBuff[5] = str_6[1];
						pMsg->mcp_RespBuff[6] = str_6[0];
						pMsg->mcp_RespBuff[7] = 0x68;
						pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
						pMsg->mcp_RespBuff[9] = 0x06;
						pMsg->mcp_RespBuff[10] = str_6[5] + 0x33;
						pMsg->mcp_RespBuff[11] = str_6[4] + 0x33;
						pMsg->mcp_RespBuff[12] = str_6[3] + 0x33;
						pMsg->mcp_RespBuff[13] = str_6[2] + 0x33;
						pMsg->mcp_RespBuff[14] = str_6[1] + 0x33;
						pMsg->mcp_RespBuff[15] = str_6[0] + 0x33;
						pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
						pMsg->mcp_RespBuff[17] = 0x16;
						
						HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
						break;
						
					case DLT645_READ:
						for (i = 3, j = (pos_68 + 10); i >= 0; i--, j++)	//数据标识
						{
							SlaveDataID[i] = pMsg->mcp_ReceiveBuff[j] - 0x33;
						}
						if ((DataIDChar = DLT645_2007_Get_DataIDChar(SlaveDataID)) != 0)
						{
								switch (DataIDChar)
								{
								case ENERGY_ACTIVE_CHAR: //正向有功电能	
									float_to_XXXXXX_XX(gMeterEnergy.EPI[3], &b1, &b2, &b3, &b4);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x08;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b4 + 0x33;
									pMsg->mcp_RespBuff[15] = b3 + 0x33;
									pMsg->mcp_RespBuff[16] = b2 + 0x33; 
									pMsg->mcp_RespBuff[17] = b1 + 0x33; 
									pMsg->mcp_RespBuff[18] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 18);
									pMsg->mcp_RespBuff[19] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 20, HAL_MAX_DELAY);
									break;
									
								case ENERGY_REACTIVE_CHAR: //反向有功电能
									float_to_XXXXXX_XX(gMeterEnergy.EPE[3], &b1, &b2, &b3, &b4);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x08;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b4 + 0x33; 
									pMsg->mcp_RespBuff[15] = b3 + 0x33; 
									pMsg->mcp_RespBuff[16] = b2 + 0x33; 
									pMsg->mcp_RespBuff[17] = b1 + 0x33; 
									pMsg->mcp_RespBuff[18] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 18);
									pMsg->mcp_RespBuff[19] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 20, HAL_MAX_DELAY);
									break;
									
								case EPI_A_CHAR: //A相正向有功电能
									float_to_XXXXXX_XX(gMeterEnergy.EPI[0], &b1, &b2, &b3, &b4);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x08;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b4 + 0x33; 
									pMsg->mcp_RespBuff[15] = b3 + 0x33; 
									pMsg->mcp_RespBuff[16] = b2 + 0x33; 
									pMsg->mcp_RespBuff[17] = b1 + 0x33; 
									pMsg->mcp_RespBuff[18] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 18);
									pMsg->mcp_RespBuff[19] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 20, HAL_MAX_DELAY);
									break;
									
								case EPE_A_CHAR: //A相反向有功电能
									float_to_XXXXXX_XX(gMeterEnergy.EPE[0], &b1, &b2, &b3, &b4);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x08;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b4 + 0x33; 
									pMsg->mcp_RespBuff[15] = b3 + 0x33; 
									pMsg->mcp_RespBuff[16] = b2 + 0x33; 
									pMsg->mcp_RespBuff[17] = b1 + 0x33; 
									pMsg->mcp_RespBuff[18] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 18);
									pMsg->mcp_RespBuff[19] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 20, HAL_MAX_DELAY);
									break;
									
								case EPI_B_CHAR: //B相正向有功电能
									float_to_XXXXXX_XX(gMeterEnergy.EPI[1], &b1, &b2, &b3, &b4);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x08;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b4 + 0x33; 
									pMsg->mcp_RespBuff[15] = b3 + 0x33; 
									pMsg->mcp_RespBuff[16] = b2 + 0x33; 
									pMsg->mcp_RespBuff[17] = b1 + 0x33; 
									pMsg->mcp_RespBuff[18] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 18);
									pMsg->mcp_RespBuff[19] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 20, HAL_MAX_DELAY);
									break;
									
								case EPE_B_CHAR: //B相反向有功电能
									float_to_XXXXXX_XX(gMeterEnergy.EPE[1], &b1, &b2, &b3, &b4);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x08;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b4 + 0x33; 
									pMsg->mcp_RespBuff[15] = b3 + 0x33; 
									pMsg->mcp_RespBuff[16] = b2 + 0x33; 
									pMsg->mcp_RespBuff[17] = b1 + 0x33; 
									pMsg->mcp_RespBuff[18] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 18);
									pMsg->mcp_RespBuff[19] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 20, HAL_MAX_DELAY);
									break;
									
								case EPI_C_CHAR: //C相正向有功电能
									float_to_XXXXXX_XX(gMeterEnergy.EPI[2], &b1, &b2, &b3, &b4);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x08;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b4 + 0x33; 
									pMsg->mcp_RespBuff[15] = b3 + 0x33; 
									pMsg->mcp_RespBuff[16] = b2 + 0x33; 
									pMsg->mcp_RespBuff[17] = b1 + 0x33; 
									pMsg->mcp_RespBuff[18] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 18);
									pMsg->mcp_RespBuff[19] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 20, HAL_MAX_DELAY);
									break;
									
								case EPE_C_CHAR: //C相反向有功电能
									float_to_XXXXXX_XX(gMeterEnergy.EPE[2], &b1, &b2, &b3, &b4);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x08;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b4 + 0x33; 
									pMsg->mcp_RespBuff[15] = b3 + 0x33; 
									pMsg->mcp_RespBuff[16] = b2 + 0x33; 
									pMsg->mcp_RespBuff[17] = b1 + 0x33; 
									pMsg->mcp_RespBuff[18] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 18);
									pMsg->mcp_RespBuff[19] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 20, HAL_MAX_DELAY);
									break;
								
								case VOLTAGE_A_CHAR:	//A相电压
									float_to_XXX_X(gMeterParam.Volt[0], &b1, &b2);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x06;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33; 
									pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
									pMsg->mcp_RespBuff[17] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
									break;
									
								case VOLTAGE_B_CHAR:	//B相电压
									float_to_XXX_X(gMeterParam.Volt[1], &b1, &b2);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x06;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33; 
									pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
									pMsg->mcp_RespBuff[17] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
									break;
									
								case VOLTAGE_C_CHAR:	//C相电压
									float_to_XXX_X(gMeterParam.Volt[2], &b1, &b2);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x06;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33; 
									pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
									pMsg->mcp_RespBuff[17] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
									break;
									
								case VOLTAGE_Z_CHAR:	//三相电压
									float_to_XXX_X(gMeterParam.Volt[0], &b1, &b2);
									float_to_XXX_X(gMeterParam.Volt[1], &b3, &b4);
									float_to_XXX_X(gMeterParam.Volt[2], &b5, &b6);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x0A;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33; 
									pMsg->mcp_RespBuff[16] = b4 + 0x33; 
									pMsg->mcp_RespBuff[17] = b3 + 0x33; 
									pMsg->mcp_RespBuff[18] = b6 + 0x33; 
									pMsg->mcp_RespBuff[19] = b5 + 0x33;
									pMsg->mcp_RespBuff[20] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 20);
									pMsg->mcp_RespBuff[21] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 22, HAL_MAX_DELAY);
									break;
									
								case CURRENT_A_CHAR:	//A相电流
									float_to_XXX_XXX(gMeterParam.Curr[0], &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
									
								case CURRENT_B_CHAR:	//B相电流
									float_to_XXX_XXX(gMeterParam.Curr[1], &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
									
								case CURRENT_C_CHAR:	//C相电流
									float_to_XXX_XXX(gMeterParam.Curr[2], &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
									
								case CURRENT_Z_CHAR:	//三相电流
									float_to_XXX_XXX(gMeterParam.Curr[0], &b1, &b2, &b3);
									float_to_XXX_XXX(gMeterParam.Curr[1], &b4, &b5, &b6);
									float_to_XXX_XXX(gMeterParam.Curr[2], &b7, &b8, &b9);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x0D;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33; 
									pMsg->mcp_RespBuff[17] = b6 + 0x33; 
									pMsg->mcp_RespBuff[18] = b5 + 0x33; 
									pMsg->mcp_RespBuff[19] = b4 + 0x33; 
									pMsg->mcp_RespBuff[20] = b9 + 0x33; 
									pMsg->mcp_RespBuff[21] = b8 + 0x33; 
									pMsg->mcp_RespBuff[22] = b7 + 0x33; 
									pMsg->mcp_RespBuff[23] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 23);
									pMsg->mcp_RespBuff[24] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 25, HAL_MAX_DELAY);
									break;
								
								case POWER_0_CHAR:
									float_to_XX_XXXX((gMeterParam.PowP[3]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
									
								case POWER_A_CHAR:
									float_to_XX_XXXX((gMeterParam.PowP[0]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case POWER_B_CHAR:
									float_to_XX_XXXX((gMeterParam.PowP[1]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case POWER_C_CHAR:
									float_to_XX_XXXX((gMeterParam.PowP[2]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
									
								case POWER_Z_CHAR:
									float_to_XX_XXXX((gMeterParam.PowP[3]*0.001), &b1, &b2, &b3);
									float_to_XX_XXXX((gMeterParam.PowP[0]*0.001), &b4, &b5, &b6);
									float_to_XX_XXXX((gMeterParam.PowP[1]*0.001), &b7, &b8, &b9);
									float_to_XX_XXXX((gMeterParam.PowP[2]*0.001), &b10, &b11, &b12);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x10;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = b6 + 0x33; 
									pMsg->mcp_RespBuff[18] = b5 + 0x33; 
									pMsg->mcp_RespBuff[19] = b4 + 0x33;
									pMsg->mcp_RespBuff[20] = b9 + 0x33; 
									pMsg->mcp_RespBuff[21] = b8 + 0x33; 
									pMsg->mcp_RespBuff[22] = b7 + 0x33;
									pMsg->mcp_RespBuff[23] = b12 + 0x33; 
									pMsg->mcp_RespBuff[24] = b11+ 0x33; 
									pMsg->mcp_RespBuff[25] = b10 + 0x33;
									pMsg->mcp_RespBuff[26] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 26);
									pMsg->mcp_RespBuff[27] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 28, HAL_MAX_DELAY);
									break;
								
								case REACTIVE_POWER_0_CHAR:
									float_to_XX_XXXX((gMeterParam.PowQ[3]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case REACTIVE_POWER_A_CHAR:
									float_to_XX_XXXX((gMeterParam.PowQ[0]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case REACTIVE_POWER_B_CHAR:
									float_to_XX_XXXX((gMeterParam.PowQ[1]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case REACTIVE_POWER_C_CHAR:
									float_to_XX_XXXX((gMeterParam.PowQ[2]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case REACTIVE_POWER_Z_CHAR:
									float_to_XX_XXXX((gMeterParam.PowQ[3]*0.001), &b1, &b2, &b3);
									float_to_XX_XXXX((gMeterParam.PowQ[0]*0.001), &b4, &b5, &b6);
									float_to_XX_XXXX((gMeterParam.PowQ[1]*0.001), &b7, &b8, &b9);
									float_to_XX_XXXX((gMeterParam.PowQ[2]*0.001), &b10, &b11, &b12);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x10;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = b6 + 0x33; 
									pMsg->mcp_RespBuff[18] = b5 + 0x33; 
									pMsg->mcp_RespBuff[19] = b4 + 0x33;
									pMsg->mcp_RespBuff[20] = b9 + 0x33; 
									pMsg->mcp_RespBuff[21] = b8 + 0x33; 
									pMsg->mcp_RespBuff[22] = b7 + 0x33;
									pMsg->mcp_RespBuff[23] = b12 + 0x33; 
									pMsg->mcp_RespBuff[24] = b11+ 0x33; 
									pMsg->mcp_RespBuff[25] = b10 + 0x33;
									pMsg->mcp_RespBuff[26] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 26);
									pMsg->mcp_RespBuff[27] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 28, HAL_MAX_DELAY);
									break;
								
								case APPARENT_POWER_0_CHAR:
									float_to_XX_XXXX((gMeterParam.PowS[3]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case APPARENT_POWER_A_CHAR:
									float_to_XX_XXXX((gMeterParam.PowS[0]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case APPARENT_POWER_B_CHAR:
									float_to_XX_XXXX((gMeterParam.PowS[1]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case APPARENT_POWER_C_CHAR:
									float_to_XX_XXXX((gMeterParam.PowS[2]*0.001), &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
									
								case APPARENT_POWER_Z_CHAR:
									float_to_XX_XXXX((gMeterParam.PowS[3]*0.001), &b1, &b2, &b3);
									float_to_XX_XXXX((gMeterParam.PowS[0]*0.001), &b4, &b5, &b6);
									float_to_XX_XXXX((gMeterParam.PowS[1]*0.001), &b7, &b8, &b9);
									float_to_XX_XXXX((gMeterParam.PowS[2]*0.001), &b10, &b11, &b12);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x10;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = b6 + 0x33; 
									pMsg->mcp_RespBuff[18] = b5 + 0x33; 
									pMsg->mcp_RespBuff[19] = b4 + 0x33;
									pMsg->mcp_RespBuff[20] = b9 + 0x33; 
									pMsg->mcp_RespBuff[21] = b8 + 0x33; 
									pMsg->mcp_RespBuff[22] = b7 + 0x33;
									pMsg->mcp_RespBuff[23] = b12 + 0x33; 
									pMsg->mcp_RespBuff[24] = b11+ 0x33; 
									pMsg->mcp_RespBuff[25] = b10 + 0x33;
									pMsg->mcp_RespBuff[26] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 26);
									pMsg->mcp_RespBuff[27] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 28, HAL_MAX_DELAY);
									break;
								
								case POWER_FACTOR_0_CHAR:
									float_to_X_XXX(gMeterParam.Pf[3], &b1, &b2);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x06;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33;
									pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
									pMsg->mcp_RespBuff[17] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
									break;
									
								case POWER_FACTOR_A_CHAR:
									float_to_X_XXX(gMeterParam.Pf[0], &b1, &b2);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x06;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33;
									pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
									pMsg->mcp_RespBuff[17] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
									break;
								
								case POWER_FACTOR_B_CHAR:
									float_to_X_XXX(gMeterParam.Pf[1], &b1, &b2);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x06;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33;
									pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
									pMsg->mcp_RespBuff[17] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
									break;
								
								case POWER_FACTOR_C_CHAR:
									float_to_X_XXX(gMeterParam.Pf[2], &b1, &b2);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x06;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33;
									pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
									pMsg->mcp_RespBuff[17] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
									break;
								
								case POWER_FACTOR_Z_CHAR:
									float_to_X_XXX(gMeterParam.Pf[3], &b1, &b2);
									float_to_X_XXX(gMeterParam.Pf[0], &b3, &b4);
									float_to_X_XXX(gMeterParam.Pf[1], &b5, &b6);
									float_to_X_XXX(gMeterParam.Pf[2], &b7, &b8);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x0C;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33; 
									pMsg->mcp_RespBuff[16] = b4 + 0x33; 
									pMsg->mcp_RespBuff[17] = b3 + 0x33; 
									pMsg->mcp_RespBuff[18] = b6 + 0x33; 
									pMsg->mcp_RespBuff[19] = b5 + 0x33; 
									pMsg->mcp_RespBuff[20] = b8 + 0x33; 
									pMsg->mcp_RespBuff[21] = b7 + 0x33; 
									pMsg->mcp_RespBuff[22] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 22);
									pMsg->mcp_RespBuff[23] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 24, HAL_MAX_DELAY);
									break;
									
								case ZERO_LINE_CHAR:
									float_to_XXX_XXX(gMeterParam.Curr[3], &b1, &b2, &b3);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x07;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b3 + 0x33; 
									pMsg->mcp_RespBuff[15] = b2 + 0x33; 
									pMsg->mcp_RespBuff[16] = b1 + 0x33;
									pMsg->mcp_RespBuff[17] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 17);
									pMsg->mcp_RespBuff[18] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 19, HAL_MAX_DELAY);
									break;
								
								case FREQ_CHAR:
									float_to_XX_XX(gMeterParam.Freq, &b1, &b2);
									pMsg->mcp_RespBuff[0] = 0x68;
									pMsg->mcp_RespBuff[1] = pMsg->mcp_ReceiveBuff[1];
									pMsg->mcp_RespBuff[2] = pMsg->mcp_ReceiveBuff[2];
									pMsg->mcp_RespBuff[3] = pMsg->mcp_ReceiveBuff[3];
									pMsg->mcp_RespBuff[4] = pMsg->mcp_ReceiveBuff[4];
									pMsg->mcp_RespBuff[5] = pMsg->mcp_ReceiveBuff[5];
									pMsg->mcp_RespBuff[6] = pMsg->mcp_ReceiveBuff[6];
									pMsg->mcp_RespBuff[7] = 0x68;
									pMsg->mcp_RespBuff[8] = pMsg->mcp_ReceiveBuff[pos_68 + 8] + 0x80;
									pMsg->mcp_RespBuff[9] = 0x06;
									pMsg->mcp_RespBuff[10] = pMsg->mcp_ReceiveBuff[10];
									pMsg->mcp_RespBuff[11] = pMsg->mcp_ReceiveBuff[11];
									pMsg->mcp_RespBuff[12] = pMsg->mcp_ReceiveBuff[12];
									pMsg->mcp_RespBuff[13] = pMsg->mcp_ReceiveBuff[13];
									pMsg->mcp_RespBuff[14] = b2 + 0x33; 
									pMsg->mcp_RespBuff[15] = b1 + 0x33;
									pMsg->mcp_RespBuff[16] = DLT645_2007_Checksum_Calc(pMsg->mcp_RespBuff, 16);
									pMsg->mcp_RespBuff[17] = 0x16;
									HAL_UART_Transmit(&huart1, pMsg->mcp_RespBuff, 18, HAL_MAX_DELAY);
									break;
								}
						}
						break;
					}
			 }
		} 
	}
	LOGE("DLT645", "Leave %s\r\n", __func__);
}

