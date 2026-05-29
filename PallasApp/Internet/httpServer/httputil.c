
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "internet.h"
#include "app_tool.h"
#include "app_log.h"
#include "app_parameter.h"

#include "http_server.h"
#include "httputil.h"
#include "webpge.h"

#include <string.h>

/* Private define ------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/
char tx_buf[MAX_URI_SIZE];
char rx_buf[MAX_URI_SIZE];
uint8_t pub_buf[1024];

uint8_t login_state = 0;


/* Private function prototypes -----------------------------------------------*/
void proc_http(SOCKET s, uint8_t *buf);


/* Private user code ---------------------------------------------------------*/
/**
*@brief		将基本的配置信息设置到json_callback
*@param		无
*@return	无
*/
static void make_basic_config_setting_json_callback(char *buf)
{
    sprintf(buf, "settingsCallback({\"ver\":\"%d.%d\",\
                \"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\
                \"ip\":\"%d.%d.%d.%d\",\
                \"gw\":\"%d.%d.%d.%d\",\
                \"sub\":\"%d.%d.%d.%d\",\
                });", gParam.st.version[2], gParam.st.version[3],
            gFlashParam.st.macAddr[0], gFlashParam.st.macAddr[1], gFlashParam.st.macAddr[2], gFlashParam.st.macAddr[3], gFlashParam.st.macAddr[4], gFlashParam.st.macAddr[5],
            gFlashParam.st.localIP[0], gFlashParam.st.localIP[1], gFlashParam.st.localIP[2], gFlashParam.st.localIP[3],
            gFlashParam.st.gatewayIP[0], gFlashParam.st.gatewayIP[1], gFlashParam.st.gatewayIP[2], gFlashParam.st.gatewayIP[3],
            gFlashParam.st.maskIP[0], gFlashParam.st.maskIP[1], gFlashParam.st.maskIP[2], gFlashParam.st.maskIP[3]
           );
}


/**
*@brief		执行config http响应
*@param		无
*@return	无
*/
void make_cgi_response(uint16_t delay, char *url, char *cgi_response_buf)
{
    sprintf(cgi_response_buf, "<html><head><title>iWeb - Configuration</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j + ' ';j--;setTimeout('func()',1000);if(j==0)location.href='http://%d.%d.%d.%d/';}</script></head><body onload='func()'>please wait for a while, the module will boot in<span style='color:red;' id='delay'></span> seconds.</body></html>", delay, url[0], url[1], url[2], url[3]);
    return;
}

/**
*@brief		执行login http响应
*@param		无
*@return	无
*/
void login_cgi_respones(uint16_t delay, char *url, char *login_response_buf)
{
    if(!login_state)
        //		sprintf(login_response_buf,"<html><head><script language=javascript>location.href='http://%d.%d.%d.%d/dream.htm';}</script></head></body></html>",url[0],url[1],url[2],url[3]);
        sprintf(login_response_buf, "<html><head><script language=javascript>location.href='http://%d.%d.%d.%d/config.html';</script></head></html>", url[0], url[1], url[2], url[3]);
    else
        sprintf(login_response_buf, "<html><head><title>登录信息出错</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j + ' ';j--;setTimeout('func()',1000);if(j==0)location.href='http://%d.%d.%d.%d/';}</script></head><body onload='func()'>用户名或密码错误，请在<span style='color:red;' id='delay'></span> 秒后重新输入.</body></html>", delay, url[0], url[1], url[2], url[3]);
    login_state = LOGIN_NAME_ERR;
    return;
}


/**
*@brief		  得到响应过程中的下一个参数
*@param		  url：需要转化网页地址
*@param			param_name：
*@return	  返回一个数据
*/
uint8_t *get_http_param_value(char *uri, char *param_name)
{
    uint16_t len;
    uint8_t *pos2;
    uint8_t *name = 0;
    uint8_t *ret = pub_buf;
    uint16_t content_len = 0;
    char tmp_buf[10] = {0x00,};
    if(!uri || !param_name) return 0;
    /***************/
    if(strstr(uri, "\r\n\r\n") != NULL)
    {
        mid(uri, "Content-Length: ", "\r\n", tmp_buf);
        content_len = atoi16(tmp_buf, 10);
        //printf("content len=%d\r\n",content_len);
        uri = (char *)strstr(uri, "\r\n\r\n");
        uri += 4;
        //printf("uri=%s\r\n",uri);
        uri[content_len] = 0;
        /***************/
        name = (uint8_t *)strstr(uri, param_name);
    }

    if(name)
    {
        name += strlen(param_name) + 1;
        pos2 = (uint8_t *)strstr((char *)name, "&");
        if(!pos2)
        {
            pos2 = name + strlen((char *)name);
        }
        len = 0;
        len = pos2 - name;

        if(len)
        {
            ret[len] = 0;
            strncpy((char *)ret, (char *)name, len);
            unescape_http_url((char *)ret);
            replacetochar((char *)ret, '+', ' ');
        }
        else
            ret[0] = 0;
    }
    else
        return 0;
    return ret;
}

/**
*@brief		将配置信息写进单片机eeprom
*@param		http_request：定义一个http请求的结构体指针
*@return	无
*/
void cgi_ipconfig(st_http_request *http_request)
{
    LOGI("httputil", "http_request-URI : %s\r\n\r\n", http_request->URI);
    uint8_t *param;
    uint8_t addr[4] = {0, 0, 0, 0};
    param = get_http_param_value(http_request->URI, "ip");		/*获取修改后的IP地址*/
    if(param)
    {
        //LOGI("httputil", "IP Address: %s", param);
        if(verify_ip_address((char *)param, addr))
        {
            LOGI("httputil", "IP Address: %d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
        }
        addr[0] = 0;
        addr[1] = 0;
        addr[2] = 0;
        addr[3] = 0;
        //inet_addr_((uint8_t*)param, ConfigMsg.lip);
    }
    param = get_http_param_value(http_request->URI, "gw");		/*获取修改后的网关*/
    if(param)
    {
        //LOGI("httputil", "Gateway Address: %s", param);
        if(verify_ip_address((char *)param, addr))
        {
            LOGI("httputil", "Gateway Address: %d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
        }
        addr[0] = 0;
        addr[1] = 0;
        addr[2] = 0;
        addr[3] = 0;
        //inet_addr_((uint8_t*)param, ConfigMsg.gw);
    }
    param = get_http_param_value(http_request->URI, "sub");	/*获取修改后的子网掩码*/
    if(param)
    {
        //LOGI("httputil", "Sub Address: %s", param);
        if(verify_ip_address((char *)param, addr))
        {
            LOGI("httputil", "Sub Address: %d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
        }
        addr[0] = 0;
        addr[1] = 0;
        addr[2] = 0;
        addr[3] = 0;
        //inet_addr_((uint8_t*)param, ConfigMsg.sub);
    }
    //write_config_to_eeprom();																/*将获取的网络参数写进eeprom*/
}

/**
  *@brief		完成http响应
  *@param		无
  *@return	无
  */
void do_https(void)
{
    uint8_t ch = APP_SOCKET_HTTPS;    /*定义一个socket*/
    uint16_t len;

    st_http_request *http_request;    /*定义一个结构指针*/
    memset(rx_buf, 0x00, MAX_URI_SIZE);
    http_request = (st_http_request *)rx_buf;

    /* http service start */
    switch(getSn_SR(ch))								/*获取socket状态*/
    {
    case SOCK_INIT:											/*socket处于初始化状态*/
        listen(ch);
        break;

    case SOCK_LISTEN:										/*socket处于监听状态*/
        break;

    case SOCK_ESTABLISHED:																/*socket处于连接状态*/
        if(getSn_IR(ch) & Sn_IR_CON)
        {
            setSn_IR(ch, Sn_IR_CON);													/*清除中断标志位*/
        }
        if ((len = getSn_RX_RSR(ch)) > 0)
        {
            len = recv(ch, (uint8_t *)http_request, len); 				/*接收http请求*/
            //LOGW("httputil", "SOCK_ESTABLISHED recv :");
            //hexdump(http_request, len);
            *(( (uint8_t *)http_request ) + len) = 0;
            proc_http(ch, (uint8_t *)http_request );							/*接收http请求并发送http响应*/
            disconnect(ch);
        }
        break;

    case SOCK_CLOSE_WAIT:   															/*socket处于等待关闭状态*/
        if ((len = getSn_RX_RSR(ch)) > 0)
        {
            len = recv(ch, (uint8_t *)http_request, len);				/*接收http请求*/
            //LOGW("httputil", "SOCK_CLOSE_WAIT recv :");
            //hexdump(http_request, len);
            *(( (uint8_t *)http_request ) + len) = 0;
            proc_http(ch, (uint8_t *)http_request);							/*接收http请求并发送http响应*/
        }
        disconnect(ch);
        break;

    case SOCK_CLOSED:                   									/*socket处于关闭状态*/
        socket(ch, Sn_MR_TCP, HTTP_SERVER_PORT, 0x00);   									/*打开socket*/
        break;

    default:
        break;
    }
}

/**
*@brief		接收http请求报文并发送http响应
*@param		s: http服务器socket
*@param		buf：解析报文内容
*@return	无
*/
void proc_http(SOCKET s, uint8_t *buf)
{
    char *name;
    int8_t req_name[32] = {0x00, };    /*定义一个http响应报文的指针*/
    unsigned long file_len = 0;        /*定义http请求报文头的结构体指针*/
    uint16_t send_len = 0;
    uint8_t *http_response;
    st_http_request *http_request;

    memset(tx_buf, 0x00, MAX_URI_SIZE);
    http_response = (uint8_t *)rx_buf;
    http_request = (st_http_request *)tx_buf;

    parse_http_request(http_request, buf);/*解析http请求报文头*/
    //LOGE("httputil", "after parse_http_request, http_request->URI is :");
    //hexdump(http_request->URI, MAX_URI_SIZE);

    switch (http_request->METHOD)
    {
    case METHOD_ERR :																			/*请求报文头错误*/
        memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
        send(s, (uint8_t *)http_response, strlen((char const *)http_response));
        break;

    case METHOD_HEAD:																			/*HEAD请求方式*/

    case METHOD_GET:																			/*GET请求方式*/
        name = http_request->URI;
        if(strcmp((char *)name, "/index.htm") == 0 || strcmp((char *)name, "/") == 0 || (strcmp((char *)name, "/index.html") == 0))
        {
            file_len = strlen(INDEX_HTML);
            make_http_response_head((uint8_t *)http_response, PTYPE_HTML, file_len);
            send(s, http_response, strlen((char const *)http_response));
            send_len = 0;
            while(file_len)
            {
                if(file_len > 1024)
                {
                    if(getSn_SR(s) != SOCK_ESTABLISHED)
                    {
                        return;
                    }
                    send(s, (uint8_t *)INDEX_HTML + send_len, 1024);
                    send_len += 1024;
                    file_len -= 1024;
                }
                else
                {
                    send(s, (uint8_t *)INDEX_HTML + send_len, file_len);
                    send_len += file_len;
                    file_len -= file_len;
                }
            }
        }
        else if(strcmp((char *)name, "/config.htm") == 0 || strcmp((char *)name, "/") == 0 || (strcmp((char *)name, "/config.html") == 0))
        {
            file_len = strlen(CONFIG_HTML);
            make_http_response_head((uint8_t *)http_response, PTYPE_HTML, file_len);
            send(s, http_response, strlen((char const *)http_response));
            send_len = 0;
            while(file_len)
            {
                if(file_len > 1024)
                {
                    if(getSn_SR(s) != SOCK_ESTABLISHED)
                    {
                        return;
                    }
                    send(s, (uint8_t *)CONFIG_HTML + send_len, 1024);
                    send_len += 1024;
                    file_len -= 1024;
                }
                else
                {
                    send(s, (uint8_t *)CONFIG_HTML + send_len, file_len);
                    send_len += file_len;
                    file_len -= file_len;
                }
            }
        }
        else if(strcmp((char *)name, "/w5500.js") == 0)
        {
            memset(tx_buf, 0, MAX_URI_SIZE);
            make_basic_config_setting_json_callback(tx_buf);
            sprintf((char *)http_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s", strlen(tx_buf), tx_buf);
            send(s, (uint8_t *)http_response, strlen((char const *)http_response));
        }
        break;

    case METHOD_POST:																			/*POST请求*/
        mid((char *)(http_request->URI), "/", " ", (char *)req_name);					/*获取该请求的文件名*/
        if(strcmp((char *)req_name, "config.cgi") == 0)
        {
            cgi_ipconfig(http_request);												/*将配置信息写进单片机eeprom*/
            make_cgi_response(5, (char *)gFlashParam.st.localIP, tx_buf);	/*生成响应的文本部分*/
            sprintf((char *)http_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s", strlen(tx_buf), tx_buf);
            /*发送http响应*/
            send(s, (uint8_t *)http_response, strlen((char *)http_response));
            disconnect(s);																		/*断开socket连接*/
            //reboot_flag=1;																		/*重启标志位置1*/
            return;
        }

        else if(strcmp((char *)req_name, "login.cgi") == 0)
        {
            uint8_t *param;
            param = get_http_param_value(http_request->URI, "user");		/*获取修改后的IP地址*/
            if(param && (strcmp((const char *)param, "admin")) == 0)
                login_state = LOGIN_PASSWORD_ERR;
            else
                login_state = LOGIN_NAME_ERR;
            param = get_http_param_value(http_request->URI, "pword");		/*获取修改后的IP地址*/
            if( (param && (strcmp((const char *)param, "password")) == 0) && (login_state != LOGIN_NAME_ERR) )
                login_state = LOGIN_PASS;
            if(login_state == LOGIN_NAME_ERR)
            {
                LOGE("httputil", "用户错误\r\n");
            }
            else if(login_state == LOGIN_PASSWORD_ERR)
            {
                LOGE("httputil", "密码错误\r\n");
            }

            login_cgi_respones(5, (char *)gFlashParam.st.localIP, tx_buf);	/*生成响应的文本部分*/
            sprintf((char *)http_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s", strlen(tx_buf), tx_buf);
            send(s, (uint8_t *)http_response, strlen((char *)http_response));
            disconnect(s);																		/*断开socket连接*/
            return;
        }

        break;

    default :
        break;

    }
}


