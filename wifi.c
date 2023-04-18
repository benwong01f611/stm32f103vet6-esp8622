
#include "wifi.h"

Userdatatype Espdatatype;

UART_HandleTypeDef *wifiCom = &huart2; // CHANGE THIS

uint8_t persent_mode;
bool atFlag = FALSE;
bool rstFlag = FALSE;
bool modeFlag = FALSE;
bool sendReadyFlag = FALSE;
bool sendOkFlag = FALSE;
bool wifiConnectFlag = FALSE;
bool serverConnectFlag = FALSE;
bool serverDisConnectFlag = FALSE;
bool serverCreateFlag = FALSE;
bool hotspotFlag = FALSE;
bool mulConFlag = FALSE;

bool dummy = FALSE;

char ipAddr[16];
char gateway[16];
int addr_ind = 0;
uint8_t currentMode = -1;

extern DMA_HandleTypeDef hdma_usart2_rx; // CHANGE THIS

// Handle received data
void onReceiveData(){
	//Espdatatype.UserBuffer;
	//Espdatatype.UserRecLen;
	// Do whatever you want!
}


//
void EnableUsart_IT(void) {
    __HAL_UART_ENABLE_IT(wifiCom, UART_IT_RXNE); // Start UART receive interrupt
    __HAL_UART_ENABLE_IT(wifiCom, UART_IT_IDLE); // Start idle interrupt
    __HAL_UART_CLEAR_IDLEFLAG(wifiCom);         // Clear interrupt flag
    HAL_UART_Receive_DMA(wifiCom,Espdatatype.DMARecBuffer,DMA_REC_SIZE);  // Make DMA can receive data
	HAL_TIM_Base_Start_IT(&htim3);		// Start timer
}

//
void USER_UART_Handler(void) {
	if(__HAL_UART_GET_FLAG(wifiCom,UART_FLAG_IDLE) ==SET) { // Trigger idle interrupt
        uint16_t temp = 0;                        
        __HAL_UART_CLEAR_IDLEFLAG(wifiCom);       // Clear interrupt flag
        HAL_UART_DMAStop(wifiCom);                // Turn off DMA
        temp = wifiCom->Instance->SR;              // Clear SR status register F0  ISR
        temp = wifiCom->Instance->DR;              //Read DR data register F0 RDR, for clearing interrupt
        temp = hdma_usart2_rx.Instance->CNDTR;   // CHANGE THIS, obtain untransferred data in DMA
        Espdatatype.DMARecLen = DMA_REC_SIZE - temp;           // Calculate received data length
        HAL_UART_RxCpltCallback(wifiCom);		  // Call the function to save data
    }
}

// Initialize ESP8266
void wifiInit(uint8_t MODE) {
	currentMode = MODE;
	EnableUsart_IT();
	HAL_Delay(1000);
	getCWMode();
	if(!wifiStart()){
		// Failed to start wifi
		while(1);
	}
	if(MODE == CLIENT)
		clientStart();
	else if(MODE == SERVER)
		serverStart();
	#if(PASSTHROUGH)
		HAL_UART_Transmit(wifiCom, (uint8_t *)"AT+CIPMODE=1\r\n", 14, 0xFFFF); // Set as passthrough
		HAL_Delay(20);
		HAL_UART_Transmit(wifiCom, (uint8_t *)"AT+CIPSEND\r\n", 12, 0xFFFF); // Set passthrough
	#endif
}

// idk
void closePassThrough(void) {
	HAL_UART_Transmit(wifiCom, (uint8_t *)"+++", 3, 0xFFFF); // Enable passthrough
	HAL_Delay(20);
	HAL_UART_Transmit(wifiCom, (uint8_t *)"AT+CIPMODE=0\r\n", 14, 0xFFFF); // Set mode
}

// Start as station mode
void clientStart(void) {
	// Set as station mode
	Send_AT_commend(CWMODE1, &modeFlag, 500);
	// Reset
	if(Send_AT_commend(RST, &rstFlag, 3000))
		rstFlag = FALSE;

	// Get CWMode after setting to station mode
	getCWMode();

	// Connect to AP
	if(!wifiConnectFlag)
		Send_AT_commend(CLIENT_WIFI, &wifiConnectFlag, 5000);

	// Get IP address
	do {
		getIP();
	} while(strlen(gateway) == 0);
	// No muxing (multi connection)
	if(Send_AT_commend(CIPMUX0, &mulConFlag, 200)){
	}
	serverConnect();
}

// Connect to server
void serverConnect(void) {
	char cmd[42];
	strcpy(cmd,CLIENT_CONNECT_STR1);
	strcat(cmd,CLIENT_CONNECTION);
	strcat(cmd,CLIENT_CONNECT_STR2);
	#if(CLIENT_USE_GATEWAY)
		strcat(cmd, gateway);
	#else
		strcat(cmd, CLIENT_IP)
	#endif
	strcat(cmd,CLIENT_CONNECT_STR3);
	strcat(cmd, CLIENT_PORT);
	if(Send_AT_commend(cmd, &serverConnectFlag, 3000))
		serverDisConnectFlag = FALSE;
}


// Disconnect from server
void serverDisConnect(void) {
	if(Send_AT_commend("AT+CIPCLOSE", &serverDisConnectFlag, 3000))
		serverConnectFlag = FALSE;
}

// Start as AP mode
void serverStart(void) {
	// Set as AP mode
	Send_AT_commend(CWMODE2, &modeFlag, 500);
	// Reset
	if(Send_AT_commend(RST, &rstFlag, 3000))
		rstFlag = FALSE;
	// Create AP
	Send_AT_commend(SERVER_AP, &hotspotFlag, 2000);

	#if(SERVER_CREATE)
		serverCreate();
	#endif
	return;
}

void serverCreate(void) {
	// Enable muxing
	Send_AT_commend("AT+CIPMUX=1", &mulConFlag, 200);
	Send_AT_commend(SERVER_START, &dummy, 500);
}

// Handle data when timer triggers interrupt
void recDataHandle(void)
{
	static bool recRstFlag = FALSE;
	if(Espdatatype.AtRecFlag == 1) {
		// Start ESP8622
		if(findStr("AT\r\r\n\r\nOK"))
			atFlag = TRUE;
		// Reset
		else if(findStr("AT+RST") && findStr(OK)) {
			recRstFlag = TRUE;
			if(findStr("ready")) {
				rstFlag = TRUE;
				recRstFlag = FALSE;
			}
		}
		// Set current working mode
		else if(findStr("AT+CWMODE=") && findStr(OK))
			modeFlag = TRUE;
		// Query current working mode
		else if(findStr("AT+CWMODE?")) {
			char *addr = strstr((char *)Espdatatype.AtBuffer, "+CWMODE:");
			if(*addr) {
				// Skip +CWMODE:
				addr += 8;
				persent_mode = *addr - 0x30; // 0x30 == '0'
			}
			//persent_mode = Espdatatype.AtBuffer[21]-0x30;
		}
		else if(findStr("WIFI CONNECTED"))
			wifiConnectFlag = TRUE;
		// Connect to server
		else if((findStr("AT+CIPSTART") && findStr("CONNECT") && findStr("OK")) || findStr("ALREADY CONNECTED")){
			serverConnectFlag = TRUE;
		}
		// Disconnect from server
		else if((findStr("AT+CIPCLOSE") && findStr("OK")))
			serverDisConnectFlag = TRUE;
		// Sent data
		else if(findStr("AT+CIPSEND") && findStr("OK") && findStr(">"))
			sendReadyFlag = TRUE;
		else if(findStr("SEND OK"))
			sendOkFlag = TRUE;
		else if(findStr("AT+CWSAP=") && findStr(OK))
			hotspotFlag = TRUE;
		else if(findStr("AT+CIPSERVER") && findStr(OK))
			serverCreateFlag = TRUE;
		else if(findStr(CIPMUX1) && findStr(OK))
			mulConFlag = TRUE;
		else if(findStr(CIPMUX0) && findStr(OK))
			mulConFlag = TRUE;
		else if(findStr("AT+CIPMUX") && findStr("link") && findStr("builded"))
			mulConFlag = TRUE;
		else if(findStr(CIFSR) && findStr(OK)) {
			char *addr = strstr((char *)Espdatatype.AtBuffer, "+CIFSR:APIP,\"");
			if(*addr) {
				int k = 0;
				addr += 13;
				do {
					ipAddr[k++] = *addr;
				} while(*(++addr) != '\"');
				ipAddr[k] = '\0';
			}
		}
		// Query IP under station mode
		else if(findStr("+CIPSTA") && findStr("OK")) {
			char *addr = strstr((char *)Espdatatype.AtBuffer, "+CIPSTA:ip:\"");
			if(*addr)
			{
				addr += 12;
				int k = 0;
				do {
					ipAddr[k++] = *addr;
				}while(*(++addr) != '\"');
				ipAddr[k] = '\0';
			}
			// Query the gateway (usually the AP itself, we can use this as the main server)
			addr = strstr((char *)Espdatatype.AtBuffer, "+CIPSTA:gateway:\"");
			if(*addr) {
				addr += 17;
				int k = 0;
				do {
					gateway[k++] = *addr;
				} while(*(++addr) != '\"');
				gateway[k] = '\0';
			}
		}
		// Query IP under AP mode
		else if(findStr("+CIPAP") && findStr("OK")){
			char *addr = strstr((char *)Espdatatype.AtBuffer, "+CIPAP:ip:\"");
			if(*addr) {
				addr += 11;
				int k = 0;
				do {
					ipAddr[k++] = *addr;
				} while(*(++addr) != '\"');
				ipAddr[k] = '\0';
			}
		}
		else if(findStr(",CONNECT")){

		}
		else if(findStr(",CLOSED")){
		}
		if(!recRstFlag) {
			memset(Espdatatype.AtBuffer, 0, AT_BUFF_SIZE);
			Espdatatype.AtRecLen = 0;
		}
		Espdatatype.AtRecFlag = 0;
	}
}

// Send data
void sendData(char *userdata, uint16_t userLength) {
	if(serverConnectFlag == FALSE) // Did not connect to server
		return;
	#if(!PASSTHROUGH)
		// Set the data length
		sendCommandCreate(userLength);
		while(sendReadyFlag == FALSE);
	#endif
	// Send the data
	HAL_UART_Transmit(wifiCom, (uint8_t*) userdata, userLength, 0xFFFF);
	#if(!PASSTHROUGH)
		int counter = 0;
		// If it doesn't change to TRUE after ~500ms, just skip it
		while(sendOkFlag == FALSE){
			HAL_Delay(1);
			if(counter == 500){
				break;
			}
			counter++;
		}
		sendReadyFlag = FALSE;
		sendOkFlag = FALSE;
	#endif
}

// Before sending data, you have to send AT+CIPSEND=<data_len>\r\n
void sendCommandCreate(uint16_t length) {
	uint8_t counter = 0; // how many digits
	char len[4];
	itoa(length, len, 10);
	counter = strlen(len);
	uint8_t cmd_len = counter + 11 + 2; // AT+CIPSEND=<len>\r\n, AT+CIPSEND= is 11 characters and \r\n is 2 characters
	uint8_t* at_send = (uint8_t* )malloc(sizeof(uint8_t) * cmd_len);
	memset(at_send, 0, cmd_len);
	strcat((char*)at_send, "AT+CIPSEND=");
	strcat((char*)at_send, len);
	strcat((char*)at_send, "\r\n");
	HAL_UART_Transmit(wifiCom, at_send, cmd_len, 0xFFFF); // Push the command and ready for sending data
	free(at_send);
	return;
}

// Send AT command
uint8_t Send_AT_commend(char *at_commend, bool *re_flag, uint16_t time_out) {
	for(uint8_t i=0;i<3;i++)
	{
		HAL_UART_Transmit(wifiCom, (uint8_t *)at_commend, strlen(at_commend), 0xFFFF);
		HAL_UART_Transmit(wifiCom, (uint8_t *)"\r\n", 2, 0xFFFF); // Auto send
		HAL_Delay(time_out);
		if(*re_flag) // Set flag
			return 1;
	}
	return 0;
}

// Find is the substring a in AtBuffer
uint8_t findStr(char *a) {
	char *addr = strstr((char *)Espdatatype.AtBuffer,a);
	if(addr)
		return 1;
	return 0;
	
}

uint8_t wifiStart(void) {
	for(int i=0; i<=3; i++) {
		if(i>=3) // Failed
			return 0;
		if(Send_AT_commend("AT", &atFlag, 100))
			break;
		if(Send_AT_commend("AT+RST", &rstFlag, 1000)) // Unable to start wifi, so we try to reset the module
			rstFlag = FALSE; //
		HAL_Delay(500); // Failed to start wifi, try again in 0.5s
	}
	// Successfully started
	return 1;
}
	

// When ESP8266 receives data, this will be called
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// Find "+IPD"
	char *addr = strstr((char *)Espdatatype.DMARecBuffer,"+IPD");
	uint16_t data_len=0;

	if(huart->Instance == USART2) // CHANGE THIS
	{
		if(addr) {
			// Receives data from other client/server
			// The structure is like this: +IPD,<len>:<data>
			// Skipping "+IPD,"
			char* myaddr = addr + 5;
			while(*myaddr != ':'){
				data_len = data_len * 10 + (uint8_t)(*myaddr - '0');
				myaddr++;
			}
			// Skipping the colon
			myaddr++;
			// Copy the data
			memcpy(Espdatatype.UserBuffer, myaddr, data_len);
			// Null-terminated, just in case
			Espdatatype.UserBuffer[data_len] = '\0';
			Espdatatype.UserRecLen = data_len;
			Espdatatype.UserRecFlag = 1;

			onReceiveData();
		}
		else {
			// AT commands
			if(Espdatatype.AtRecLen>0) { // Are there any unhandled AT commands
					memcpy(&Espdatatype.AtBuffer[Espdatatype.AtRecLen],Espdatatype.DMARecBuffer,Espdatatype.DMARecLen);
					Espdatatype.AtRecLen +=  Espdatatype.DMARecLen;
			}
			else {
					memcpy(Espdatatype.AtBuffer,Espdatatype.DMARecBuffer,Espdatatype.DMARecLen);
					Espdatatype.AtRecLen =  Espdatatype.DMARecLen;
			}
			Espdatatype.AtRecFlag = 1;
		}
	}
	// Clear DMARecBuffer
	memset(Espdatatype.DMARecBuffer, 0x00, DMA_REC_SIZE);
}

// Get IP address
void getIP(){
	if(currentMode == CLIENT)
		HAL_UART_Transmit(wifiCom, (uint8_t *)"AT+CIPSTA?\r\n", 12, 0xFFFF);
	else if(currentMode == SERVER)
		HAL_UART_Transmit(wifiCom, (uint8_t *)"AT+CIPAP?\r\n", 11, 0xFFFF);

}

// Get current working mode
void getCWMode(){
	Send_AT_commend("AT+CWMODE?", &dummy, 500);
}
