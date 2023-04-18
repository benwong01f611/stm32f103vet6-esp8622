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

// ��������
#define CLIENT_WIFI_NAME "hello_world"			//Ҫ���ӵ�wifi�ȵ���
#define CLIENT_WIFI_PWD "456258888"					//�ȵ�����
#define CLIENT_IP "192.168.1.101"		//������ip��ַ
#define CLIENT_PORT "5656"				//�˿ں�

#define SERVER_PORT "5656"					//�����������˿�
#define SERVER_TIMEOUT 300			//���������ӳ�ʱʱ��
#define SERVER_WIFI_NAME 	"wifidemo"		//������WLAN�ȵ���
#define SERVER_WIFI_PASSWORD 	"12345678"	//������WLAN�ȵ�����
#define SERVER_CHANNEL "12"						//������WLAN�ȵ��ŵ�
#define SERVER_ENCRYPTION "4"					//������WLAN�ȵ���ܷ�ʽ 0 OPEN 1 WEP 2 WPA_PSK 3 WPA2_PSK 4 WPA_WPA2_PSK

#define SERVER_CREATE 1 // apģʽ�¿���·�ɺ��Ƿ񴴽���������Ĭ�ϴ���
#define SERVER_CONNECT 1 // stationģʽ������·�������Ƿ��Զ����ӷ�������Ĭ������

#define DISPLAY_AT_INFO 0			// �Ƿ���ʾ�������ݣ�����ʱ���Դ�
#define PASSTHROUGH 0		// �Ƿ�Ϊ͸��ģʽ��ֻ����*�ͻ��˵�����*ģʽ�¿ɴ�͸��ģʽ��
#define MULTIPLE_CON 1			// ���õ�/�����ӣ�Ĭ��Ϊ������(ʵ��ֻ���ڶ�����ģʽ�²�����������������)

// �߼�����
#define DMA_REC_SIZE 1024   // DMA�����С
#define USER_BUFF_SIZE 512	// �û����ݻ����С
#define AT_BUFF_SIZE	512		// ATָ����С

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
extern Userdatatype Espdatatype;  //�ṹ��ȫ�ֶ���
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
