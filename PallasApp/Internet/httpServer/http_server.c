
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "http_server.h"
#include "app_tool.h"

#include <string.h>
#include <stdio.h>

/* Private define ------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/




/* Private function prototypes -----------------------------------------------*/



/* Private user code ---------------------------------------------------------*/





/**
*@brief		  转化转义字符为ascii charater
*@param		  url：需要转化网页地址
*@return	  无
*/
void unescape_http_url(char *url)
{
    int x, y;

    for (x = 0, y = 0; url[y]; ++x, ++y)
    {
        if ((url[x] = url[y]) == '%')
        {
            url[x] = c2d(url[y + 1]) * 0x10 + c2d(url[y + 2]);
            y += 2;
        }
    }
    url[x] = '\0';
}



/**
*@brief		  执行一个答复，如 html, gif, jpeg,etc.
*@param		  buf- 答复数据
*@param			type- 答复数据类型
*@param			len-  答复数据长度
*@return	  无
*/
void make_http_response_head(unsigned char *buf, char type, uint32_t len)
{
    char *head;
    char tmp[10];
    memset(buf, 0x00, MAX_URI_SIZE);
    /* 文件类型*/
    if 	(type == PTYPE_HTML) head = RES_HTMLHEAD_OK;
    else if (type == PTYPE_GIF)	head = RES_GIFHEAD_OK;
    else if (type == PTYPE_TEXT)	head = RES_TEXTHEAD_OK;
    else if (type == PTYPE_JPEG)	head = RES_JPEGHEAD_OK;
    else if (type == PTYPE_FLASH)	head = RES_FLASHHEAD_OK;
    else if (type == PTYPE_MPEG)	head = RES_MPEGHEAD_OK;
    else if (type == PTYPE_PDF)	head = RES_PDFHEAD_OK;

    sprintf(tmp, "%d", len);
    strcpy((char *)buf, head);
    strcat((char *)buf, tmp);
    strcat((char *)buf, "\r\n\r\n");
    //printf("%s\r\n", buf);
}




/**
*@brief		  解析每一个http响应
*@param		  request： 定义一个指针
*@return	  无
*/
void parse_http_request(st_http_request *request, uint8_t *buf)
{
    char *nexttok;
    nexttok = strtok((char *)buf, " ");
    if(!nexttok)
    {
        request->METHOD = METHOD_ERR;
        return;
    }
    if(!strcmp(nexttok, "GET") || !strcmp(nexttok, "get"))
    {
        request->METHOD = METHOD_GET;
        nexttok = strtok(NULL, " ");

    }
    else if (!strcmp(nexttok, "HEAD") || !strcmp(nexttok, "head"))
    {
        request->METHOD = METHOD_HEAD;
        nexttok = strtok(NULL, " ");

    }
    else if (!strcmp(nexttok, "POST") || !strcmp(nexttok, "post"))
    {
        nexttok = strtok(NULL, "\0"); //20120316
        //nexttok = strtok(NULL," ");
        request->METHOD = METHOD_POST;

    }
    else
    {
        request->METHOD = METHOD_ERR;
    }
    if(!nexttok)
    {
        request->METHOD = METHOD_ERR;
        return;
    }
    strcpy(request->URI, nexttok);
}





