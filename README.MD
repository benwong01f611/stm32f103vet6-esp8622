# ESP8622 WiFi module for STM32F103VET6

This is the API for utilizing the ESP8622 WiFi module on STM32F103VET6. Referenced from [this CSDN article](https://blog.csdn.net/qq_26024785/article/details/107352408), modified for using it on STM32F103VET6.

The folder ``original`` stores the original version of the code.

## Requirements
- USART
- DMA
- Timer

## Steps
1. Create a new project with CubeIDE (or other)
2. Set High Speed Clock to "Crystal/Ceramic Resonator"
4. Set the USART mode to "Asychronous"
5. Enable NVIC global interrupt for USART
6. Enable DMA for USART RX
7. Enable a timer
8. Set the timer's clock source to "Internal Clock", prescalar to "72-1", counter period to "10000-1" (depends on your clock configuration, the formula is 1ms = ((1+PRESCALER)/72M)*(1+PERIOD))
9. Under "Project Manager", enable "Generate peripheral initialization as a pair of '.c/.h' files per peripheral"
10. Copy ``wifi.h`` and ``wifi.c`` to folder ``Inc`` and ``Src`` respectively
11. Add the following code to USARTx_IRHandler, where x is your USART number
```c
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
  USER_UART_Handler();
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
  HAL_UART_Receive_DMA(&huart2, Espdatatype.DMARecBuffer, DMA_REC_SIZE);
  /* USER CODE END USART2_IRQn 1 */
}
```
12. Include ``wifi.h`` to ``stm32f1xx.h`` and ``main.h``
13. Add the following function (you can place it in ``main.h``):
```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	static uint8_t count = 0;
	if(htim == (&htim3)){
		count++;
		if(count >= 10){
			recDataHandle();
			count = 0;
		}
	}
}
```
14. Add ``wifiInit(CLIENT)`` or ``wifiInit(SERVER)`` to ``main.cpp``, CLIENT means starting ESP8622 as client (station mode) and SERVER means starting ESP8622 as server (AP)
15. Modify ``wifi.c``, change the UART/USART/timer to your configuration
16. Modify ``wifi.h`` according to your preferences
17. Done!

## Usage
```c
void sendData(char *userdata, uint16_t userLength);
```
``sendData()`` will send the data out after connecting to a server, ``userLength`` is equivilent to  ``strlen(userdata)``.


```c
void onReceiveData();
```
``onReceiveData()`` will be called when data is received, the data is stored in ``Espdatatype.UserBuffer``, and the length of the data is stored in ``Espdatatype.UserRecLen``