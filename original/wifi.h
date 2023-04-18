#ifndef __wifi_H
#define __wifi_H
#ifdef __cplusplus
 extern "C"
#endif

#include "main.h"
#include <stdio.h>
#include "string.h"
#include "stdarg.h"
#include "stdlib.h"	
#include "tim.h"
//#include "usart.h"

// 基础设置
#define CLIENT_WIFI_NAME "hello_world"			//要连接的wifi热点名
#define CLIENT_WIFI_PWD "456258888"					//热点密码
#define CLIENT_IP "192.168.1.101"		//服务器ip地址
#define CLIENT_PORT "5656"				//端口号

#define SERVER_PORT "5656"					//服务器建立端口
#define SERVER_TIMEOUT 300			//服务器连接超时时间
#define SERVER_WIFI_NAME 	"wifidemo"		//服务器WLAN热点名
#define SERVER_WIFI_PASSWORD 	"12345678"	//服务器WLAN热点密码
#define SERVER_CHANNEL "12"						//服务器WLAN热点信道
#define SERVER_ENCRYPTION "4"					//服务器WLAN热点加密方式 0 OPEN 1 WEP 2 WPA_PSK 3 WPA2_PSK 4 WPA_WPA2_PSK

#define SERVER_CREATE 1 // ap模式下开启路由后是否创建服务器，默认创建
#define SERVER_CONNECT 1 // station模式下连接路由器后是否自动连接服务器，默认连接

#define DISPLAY_AT_INFO 0			// 是否显示回显数据，测试时可以打开
#define PASSTHROUGH 0		// 是否为透传模式、只有在*客户端单链接*模式下可打开透传模式！
#define MULTIPLE_CON 1			// 设置单/多链接，默认为多链接(实测只有在多链接模式下才能正常开启服务器)

// 高级设置
#define DMA_REC_SIZE 1024   // DMA缓存大小
#define USER_BUFF_SIZE 512	// 用户数据缓存大小
#define AT_BUFF_SIZE	512		// AT指令缓存大小

#define CLIENT 1
#define SERVER 2
#define CS 3

#define WIFISTRCON(s1,s2,s3,s4) s1##XSTR(s2)##s3##XSTR(s4)
#define XSTR(S) STR(S)
#define STR(S) #S

#define AT "AT"
#define OK "OK"
#define RST "AT+RST"
#define CWMODENUM "AT+CWMODE?"
#define CWMODE1 "AT+CWMODE=1"
#define CWMODE2 "AT+CWMODE=2"
#define CWJAP "AT+CWJAP=\""
#define	CIPSTART "AT+CIPSTART=\"TCP\",\""
#define CWSAP "AT+CWSAP=\""
#define	CIPMUX1 "AT+CIPMUX=1"
#define CIPMUX0 "AT+CIPMUX=0"
#define CIPSERVER "AT+CIPSERVER=1,"
#define CIFSR "AT+CIFSR\r\n"
#define CIPSEND "AT+CIPSEND="

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
//extern UART_HandleTypeDef huart1;
//extern UART_HandleTypeDef huart2;

typedef struct
{   
	uint8_t  UserRecFlag;  		 //用户数据接收到标志位
	uint8_t  AtRecFlag;  			 //AT数据接收到标志位
	uint16_t DMARecLen; 			 //DMA接受长度
	uint16_t UserRecLen;   		 //用户未处理数据长度
	uint16_t AtRecLen;    		 //AT未处理数据长度
	uint8_t  DMARecBuffer[DMA_REC_SIZE];  //DMAbuffer
	uint8_t  UserBuffer[USER_BUFF_SIZE];         // 用户数据处理buffer
	uint8_t  AtBuffer[AT_BUFF_SIZE];				// AT指令数据处理buffer
}Userdatatype;

typedef enum {FALSE = 0,TRUE = 1} bool;
extern Userdatatype Espdatatype;  //结构体全局定义
extern bool dataAnalyzeFlag; 

void USER_UART_Handler(void);
void EnableUsart_IT(void);
void clientStart(void);
void serverStart(void);
void wifiInit(uint8_t MODE);
uint8_t* strConnect(int num, ...);
uint8_t Send_AT_commend(char *at_commend, bool *re_flag, uint16_t time_out);
uint8_t findStr(char *a);
uint8_t wifiStart(void);
void clientStart(void);
void serverStart(void);
void recDataHandle(void);
void sendData(uint8_t *userdata, uint16_t userLength);
void sendData101(uint16_t con, uint16_t addr, uint8_t *data, uint16_t userLength);
void sendCommandCreate(uint16_t length);
void recDataAnalyze(uint8_t *recData);
void serverCreate(void);
void serverConnect(void);
void serverDisConnect(void);
#endif

/*
****************wifi_demo v1.0*****************
* 开启服务器时显示ip地址和端口号
*	更新日期：2021-04-25
* 更新日志：
* 1.解决不解析数据时输出数据含有乱码的问题
* 2.解决wifi模块重复RST的问题
* 3.服务器模式下添加设备上下线提示
*	4.分解客户端/服务端初始化代码，实现station模式动态创建、断开服务器连接和ap模式动态建立、关闭服务器
* 5.优化配置项，功能设置全部放在头文件中
* 更新日期：2020-07-12
* 版本功能：
* 1.客户端模式下自动连接WiFi热点  
* 2.客户端模式下自动连接服务器  
* 3.服务器模式下自动创建wifi热点  
* 4.服务器模式下自动开启TCP服务器
* 5.服务器模式下输出TCP服务器的IP地址及端口号，DISPLAY_AT_INFO为1的情况下  
* 6.可实时打开和关闭透传模式  
* 7.可实时更改接收数据的解析模式（需要手动更改dataAnalyzeFlag的值）
* 8.提供两种数据封装模式，101协议格式和无封装格式，分别通过调用sendData101、sendData函数实现
* 发送模式及其性能（101协议封装为模式1，无封装为模式2）：
* 在非透传模式下，模式1最快300ms/次、模式2最快200ms/次
* 在透传模式下，模式1与模式2都能达到30ms/次，实测50ms/次时串口能够正常回显数据，小于50ms时可能会出错
* *********************************************
*/
