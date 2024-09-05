/*
 * ECE 153B
 *
 * Name(s): Derek Dela Cruz, Josephine Nghiem
 * Section: Tuesday 7pm
 * Project
 */

#include "stm32l476xx.h"
#include "SysClock.h"
#include "SysTimer.h"
#include "LED.h"
#include "DMA.h"
#include "UART.h"
#include "motor.h"
#include "SPI.h"
#include "I2C.h"
#include "accelerometer.h"
#include <stdio.h>
#include <string.h>

static char buffer[IO_SIZE];
uint8_t commandFlag = 0;
uint8_t dirFlag = 0;
uint8_t count = 0;


void UART_onInput(char* inputs, uint32_t size) {
		char word[size];
	
		for (int i = 0; i < size; i++) {
			word[i] = inputs[i];
		}
	
		if((!strcmp(word, "Open")) || (!strcmp(word, "open"))){
			sprintf(buffer, "Opening Door\n");
			UART_print(buffer);
			setDire(-1);
			commandFlag = 1;
			dirFlag = 11;
		}
		else if ((!strcmp(word, "Close")) || (!strcmp(word, "close"))){
			sprintf(buffer, "Closing Door\n");
			UART_print(buffer);
			setDire(1);
			commandFlag = 1;
			dirFlag = 1;
		}
		else if ((!strcmp(word, "Stop")) || (!strcmp(word, "stop"))){
			sprintf(buffer, "Stopping Door\n");
			UART_print(buffer);
			setDire(2);
			commandFlag = 1;
			dirFlag = 0;
		}
		else {
			sprintf(buffer, "Invalid command :(\n");
			UART_print(buffer);
		}
		
		for (int i = 0; i < size; i++) {
			word[i] = '\0';
		}
}

uint8_t SecondaryAddress;
uint8_t Data_Receive;
uint8_t Data_Send = 0;
double x,y,z;

int main(void) {
	// Switch System Clock = 80 MHz
	System_Clock_Init();

	Motor_Init();
	SysTick_Init();
	//UART2_Init();
	//UART2_GPIO_Init();
	//USART_Init(USART2);
	UART1_Init();
	UART1_GPIO_Init();
	USART_Init(USART1);
	// DMA_Init_UARTx(DMA1_Channel4, USART1);
	LED_Init();	
	SPI1_GPIO_Init();
	SPI1_Init();
	initAcc();
	I2C_GPIO_Init();
	I2C_Initialization();
	
	sprintf(buffer, "RAAAAAAAH GARAGE DOOR TIME\r\n");
	UART_print(buffer);
	while(1) {
		SecondaryAddress = 0b1001000 << 1;
		
		readValues(&x, &y, &z);
		I2C_SendData(I2C1, SecondaryAddress, &Data_Send, 1);		
		I2C_ReceiveData(I2C1, SecondaryAddress, &Data_Receive, 1);
		
		sprintf(buffer, "Temp: %d, Coords: %f, %f, %f\r\n", Data_Receive, x, y, z);
		UART_print(buffer);
		
		if (!commandFlag) {
			if (Data_Receive >= 26) {
				setDire(-1);
				dirFlag = 11;
			}
			else if (Data_Receive <= 25) {
				setDire(1);
				dirFlag = 1;
			}
		}
		
		// if (xyx coords satisified and dirFlag = 1) STOP OPEN
		// else if (xyz coords satisfied and dirFlag = 11) STOP CLOSE
		
		if (((z >= 0.9) || (z <= -0.9)) & (dirFlag == 11)) {
			dirFlag = 0;
			setDire(2);
		}
		
		else if (((y >= 0.9) || (y <= -0.9)) & (dirFlag == 1)) {
			dirFlag = 0;
			setDire(2);
		}
		
		// sprintf(x, y, z, "Acceleration: %.2f, %.2f, %.2f\r\n");
		
		if (commandFlag & (dirFlag == 0)) {
			if (count >= 2) {
				count = 0;
				commandFlag = 0;
			}
			else {
				count++;
			}
		}
		
		LED_Toggle();
		delay(1000);
	}
}


