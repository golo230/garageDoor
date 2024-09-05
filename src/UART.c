/*
 * ECE 153B
 *
 * Name(s): Derek Dela Cruz, Josephine Nghiem
 * Section: Tuesday 7pm
 * Project
 */


#include "UART.h"
#include "DMA.h"
#include "motor.h"

static volatile DMA_Channel_TypeDef * tx;
static volatile char inputs[IO_SIZE];
static volatile uint8_t data_t_0[IO_SIZE];
static volatile uint8_t data_t_1[IO_SIZE];
static volatile uint8_t input_size = 0;
static volatile uint8_t pending_size = 0; // TODO
static volatile uint8_t * active = data_t_0;
static volatile uint8_t * pending = data_t_1;

#define SEL_0 1
#define BUF_0_EMPTY 2
#define BUF_1_EMPTY 4
#define BUF_0_PENDING 8
#define BUF_1_PENDING 16

void transfer_data(char ch);
void on_complete_transfer(void);

void UART1_Init(void) {
	for (int i = 0; i < IO_SIZE; i++) {
		active[i] = '\0';
		pending[i] = '\0';
	}
	
	//TODO
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->CCIPR &= ~RCC_CCIPR_USART1SEL_1;
	RCC->CCIPR |= RCC_CCIPR_USART1SEL_0;
	
	tx = DMA1_Channel4;
	DMA_Init_UARTx(DMA1_Channel4, USART1);
	
	DMA1_Channel4->CMAR = (uint32_t) active;
}

void UART2_Init(void) {
	//TODO
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	RCC->CCIPR &= ~RCC_CCIPR_USART2SEL_1;
	RCC->CCIPR |= RCC_CCIPR_USART2SEL_0;
	
	tx = DMA1_Channel7;
	
	DMA_Init_UARTx(DMA1_Channel7, USART2);
	
	DMA1_Channel7->CMAR = (uint32_t) active;
}

void UART1_GPIO_Init(void) {
	//TODO
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	
	GPIOB->MODER |= GPIO_MODER_MODE6_1;
	GPIOB->MODER &= ~GPIO_MODER_MODE6_0;
	
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL6;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL6_3;
	
	GPIOB->MODER |= GPIO_MODER_MODE7_1;
	GPIOB->MODER &= ~GPIO_MODER_MODE7_0;
	
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL7;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL7_3;
	
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED6;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED7;
	
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT6;
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT7;
	
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD6_0;
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD7;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD7_0;
}

void UART2_GPIO_Init(void) {
	//TODO
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	GPIOA->MODER |= GPIO_MODER_MODE2_1;
	GPIOA->MODER &= ~GPIO_MODER_MODE2_0;
	
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2;
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL2_3;
	
	GPIOA->MODER |= GPIO_MODER_MODE3_1;
	GPIOA->MODER &= ~GPIO_MODER_MODE3_0;
	
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3;
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL3_3;
	
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED3;
	
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT2;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT3;
	
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD2;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD2_0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;
}

void USART_Init(USART_TypeDef * USARTx) {
	//TODO
	// Disable USART before configuring settings
	USARTx->CR1 &= ~USART_CR1_UE;
	
	// Set Communication Parameters
	USARTx->CR1 &= ~(USART_CR1_M);     // 00 -> 8 Data Bits
	USARTx->CR1 &= ~(USART_CR1_OVER8); // 0 -> Oversampling by 16
	USARTx->CR2 &= ~(USART_CR2_STOP);  // 00 -> 1 Stop Bit
	
	// Set Baud Rate
	// f_CLK = 80 MHz, Baud Rate = 9600 = 80 MHz / DIV -> DIV = 8333 = 0x208D
	USARTx->BRR = 0x208D;
	
	USARTx->CR1 |= USART_CR1_TCIE;
	USARTx->CR1 |= USART_CR1_RXNEIE;
	
	// Enable Transmitter/Receiver
	USARTx->CR1 |= USART_CR1_TE | USART_CR1_RE;
	USARTx->CR3 |= USART_CR3_DMAT;
	
	// Enable USART
	USARTx->CR1 |= USART_CR1_UE;
	
	if (tx == DMA1_Channel4) {
		NVIC_EnableIRQ(USART1_IRQn);
		NVIC_SetPriority(USART1_IRQn, 1);
	}
	else {
		NVIC_EnableIRQ(USART2_IRQn);
		NVIC_SetPriority(USART2_IRQn, 1);
	}
}

/**
 * This function accepts a string that should be sent through UART
*/
void UART_print(char* data) {
	int i;

	//Transfer char array to buffer
	//Check DMA status. If DMA is not ready, put the data aside
	if(tx->CCR & DMA_CCR_EN){
		pending_size = 0;
		
		for (i = 0; i < IO_SIZE; i++){
			if (data[i] == '\0') {
				break;
			}
			pending[i] = data[i];
			pending_size++;
		}
	}
	//If DMA is ready, send data
	else{
		for (i = 0; i < IO_SIZE; i++){
			if (data[i] == '\0') {
				break;
			}
			active[i] = data[i];
		}
		// USART1->TDR = *active & 0xFF;
		tx->CNDTR = i;
		tx->CCR |= DMA_CCR_EN;
		
		// USART2->CR1 |= USART_CR1_TXEIE;
		// USART2->CR1 |= USART_CR1_RXNEIE;
	}
	//on_complete_transfer();
}

/**
 * This function should be invoked when a character is accepted through UART
*/
void transfer_data(char ch) {
	// If the character is end-of-line, invoke UART_onInput
	if (ch == '\n'){
		UART_onInput(inputs, input_size);
		
		input_size = 0;
		
		for (int z = 0; z < IO_SIZE; z++) {
			inputs[z] = '\0';
		}
	}
	// Append character to input buffer.
	else {
		inputs[input_size] = ch;
		input_size++;
	}
}

/**
 * This function should be invoked when DMA transaction is completed
*/
void on_complete_transfer(void) {
	// If there are pending data to send, switch active and pending buffer, and send data
	for (int i = 0; i < IO_SIZE; i++) {
		active[i] = '\0';
	}
	
	if(pending_size != 0){
		
		for (int j = 0; j < pending_size; j++) {
			active[j] = pending[j];
			pending[j] = '\0';
		}
		
		// send data
		tx->CNDTR = pending_size;
		pending_size = 0;
		tx->CCR |= DMA_CCR_EN;
	}
}

void USART1_IRQHandler(void){
	NVIC_ClearPendingIRQ(USART1_IRQn);
	// When receive a character, invoke transfer_data
	if (USART1->ISR & USART_ISR_RXNE) {
		transfer_data(USART1->RDR & 0xFF);
		// USART1->RQR |= USART_RQR_RXFRQ;
		// on_complete_transfer();
	}
	// When complete sending data, invoke on_complete_transfer
	else if (USART1->ISR & USART_ISR_TC) {
		on_complete_transfer();
		USART1->ICR |= USART_ICR_TCCF;
		DMA1_Channel4->CCR &= ~DMA_CCR_EN;
	}
}

void USART2_IRQHandler(void){
	NVIC_ClearPendingIRQ(USART2_IRQn);
	// When receive a character, invoke transfer_data
	if (USART2->ISR & USART_ISR_RXNE) {
		transfer_data(USART2->RDR & 0xFF);
		// on_complete_transfer();
	}
	// When complete sending data, invoke on_complete_transfer
	else if (USART2->ISR & USART_ISR_TC) {
		on_complete_transfer();
		USART2->ICR |= USART_ICR_TCCF;
	}
}