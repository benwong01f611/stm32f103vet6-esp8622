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
#include "usart.h"
#include "lcd.h"

/* demoap -> Your AP name
 * A12345678 -> password
 */
#define CLIENT_WIFI "AT+CWJAP=\"demoap\",\"A12345678\""

#define CLIENT_CONNECT_STR1 "AT+CIPSTART=\""
#define CLIENT_CONNECT_STR2 "\",\""
#define CLIENT_CONNECT_STR3 "\","

#define CLIENT_CONNECTION "UDP" // Recommend using UDP, TDP has a lot of handshakes which causes latencies
#define CLIENT_USE_GATEWAY 1 // If you want to use the gateway as the target server, set this to 1, otherwise 0
#define CLIENT_IP "192.168.1.1" // The server you want to connect to, doesn't matter if you want to use gateway as target serverַ
#define CLIENT_PORT "80" // The server port you want to connect to

/*  demoap -> AP name
 *  A12345678 -> password
 *  5 -> channel
 *  4 -> protection method (see below)
 *  0 OPEN
 *  1 WEP
 *  2 WPA_PSK
 *  3 WPA2_PSK
 *  4 WPA_WPA2_PSK
 */
#define SERVER_AP "AT+CWSAP=\"demoap\",\"A12345678\",5,4"
#define SERVER_START "AT+CIPSERVER=1,80" // Change 80 to the port you want to expose

#define SERVER_CREATE 1 // Create server?
#define SERVER_CONNECT 1 // Connect to server?

#define PASSTHROUGH 0		// idk but better keep this 0
#define MULTIPLE_CON 1 // Keep this on if you create a server

// Buffer size
#define DMA_REC_SIZE 1024   // DMA buffer size
#define USER_BUFF_SIZE 512	// User data buffer size
#define AT_BUFF_SIZE	512		// ATָ data buffer size

#define CLIENT 1
#define SERVER 2
#define CS 3

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

extern UART_HandleTypeDef huart2; // CHANGE THIS
extern DMA_HandleTypeDef hdma_usart2_rx; // CHANGE THIS

extern uint8_t persent_mode;
extern uint8_t currentMode;

typedef struct
{   
	uint8_t  UserRecFlag;  		 //�û����ݽ��յ���־λ
	uint8_t  AtRecFlag;  			 //AT���ݽ��յ���־λ
	uint16_t DMARecLen; 			 //DMA���ܳ���
	uint16_t UserRecLen;   		 //�û�δ�������ݳ���
	uint16_t AtRecLen;    		 //ATδ�������ݳ���
	uint8_t  DMARecBuffer[DMA_REC_SIZE];  //DMAbuffer
	uint8_t  UserBuffer[USER_BUFF_SIZE];         // �û����ݴ���buffer
	uint8_t  AtBuffer[AT_BUFF_SIZE];				// ATָ�����ݴ���buffer
}Userdatatype;

typedef enum {FALSE = 0,TRUE = 1} bool;
extern bool serverConnectFlag;
extern Userdatatype Espdatatype;  //�ṹ��ȫ�ֶ���
extern bool dataAnalyzeFlag;
extern char ipAddr[16];
extern char gateway[16];
extern UART_HandleTypeDef *wifiCom;

void onReceiveData();
void USER_UART_Handler(void);
void EnableUsart_IT(void);
void clientStart(void);
void serverStart(void);
void wifiInit(uint8_t MODE);
uint8_t Send_AT_commend(char *at_commend, bool *re_flag, uint16_t time_out);
uint8_t findStr(char *a);
uint8_t wifiStart(void);
void clientStart(void);
void serverStart(void);
void recDataHandle(void);
void sendData(char *userdata, uint16_t userLength);
void sendCommandCreate(uint16_t length);
void recDataAnalyze(uint8_t *recData);
void serverCreate(void);
void serverConnect(void);
void serverDisConnect(void);
void getIP();
void getCWMode();
#endif

/*
****************wifi_demo v1.0*****************
* ����������ʱ��ʾip��ַ�Ͷ˿ں�
*	�������ڣ�2021-04-25
* ������־��
* 1.�������������ʱ������ݺ������������
* 2.���wifiģ���ظ�RST������
* 3.������ģʽ������豸��������ʾ
*	4.�ֽ�ͻ���/����˳�ʼ�����룬ʵ��stationģʽ��̬�������Ͽ����������Ӻ�apģʽ��̬�������رշ�����
* 5.�Ż��������������ȫ������ͷ�ļ���
* �������ڣ�2020-07-12
* �汾���ܣ�
* 1.�ͻ���ģʽ���Զ�����WiFi�ȵ�  
* 2.�ͻ���ģʽ���Զ����ӷ�����  
* 3.������ģʽ���Զ�����wifi�ȵ�  
* 4.������ģʽ���Զ�����TCP������
* 5.������ģʽ�����TCP��������IP��ַ���˿ںţ�DISPLAY_AT_INFOΪ1�������  
* 6.��ʵʱ�򿪺͹ر�͸��ģʽ  
* 7.��ʵʱ���Ľ������ݵĽ���ģʽ����Ҫ�ֶ�����dataAnalyzeFlag��ֵ��
* 8.�ṩ�������ݷ�װģʽ��101Э���ʽ���޷�װ��ʽ���ֱ�ͨ������sendData101��sendData����ʵ��
* ����ģʽ�������ܣ�101Э���װΪģʽ1���޷�װΪģʽ2����
* �ڷ�͸��ģʽ�£�ģʽ1���300ms/�Ρ�ģʽ2���200ms/��
* ��͸��ģʽ�£�ģʽ1��ģʽ2���ܴﵽ30ms/�Σ�ʵ��50ms/��ʱ�����ܹ������������ݣ�С��50msʱ���ܻ����
* *********************************************
*/
