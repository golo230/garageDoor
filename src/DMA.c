/*
 * ECE 153B
 *
 * Name(s): Derek Dela Cruz, Josephine Nghiem
 * Section: Tuesday 7pm
 * Project
 */
 
#include "DMA.h"
#include "SysTimer.h"
#include "UART.h"

void DMA_Init_UARTx(DMA_Channel_TypeDef * tx, USART_TypeDef * uart) {
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	
	delay(100);

	tx->CCR &= ~DMA_CCR_EN;
	tx->CCR &= ~DMA_CCR_MEM2MEM;

	// setting priority to high
	tx->CCR &= ~DMA_CCR_PL_0;
	tx->CCR |= DMA_CCR_PL_1;

	tx->CCR &= ~DMA_CCR_PSIZE;

	tx->CCR &= ~DMA_CCR_MSIZE;

	tx->CCR &= ~DMA_CCR_PINC;
	tx->CCR |= DMA_CCR_MINC;
	
	tx->CCR &= ~DMA_CCR_CIRC;

	tx->CCR |= DMA_CCR_DIR;
	
	tx->CPAR = (uint32_t)(&(uart->TDR));

	tx->CCR &= ~DMA_CCR_HTIE;
	tx->CCR &= ~DMA_CCR_TEIE;
	tx->CCR |= DMA_CCR_TCIE;
	
	if (uart == USART1){
		DMA1_CSELR->CSELR = 0x00022000U;
	}
	else if (uart == USART2){
		DMA1_CSELR->CSELR = 0x02200000U;
	}
	
	if (tx == DMA1_Channel4){
		NVIC_EnableIRQ(DMA1_Channel4_IRQn);
		NVIC_SetPriority(DMA1_Channel4_IRQn, 2);
	}
	else if (tx == DMA1_Channel7) {
		NVIC_EnableIRQ(DMA1_Channel7_IRQn);
		NVIC_SetPriority(DMA1_Channel7_IRQn, 2);
	}
	
	// USART_Init(uart);
}

void DMA1_Channel4_IRQHandler() {
	//Clear NVIC interrupt flag
	NVIC_ClearPendingIRQ(DMA1_Channel4_IRQn);
	
	// Check Transfer Complete interrupt flag
	if(DMA1->ISR & DMA_ISR_TCIF4){
		DMA1->IFCR |= DMA_IFCR_CTCIF4;
		// DMA1_Channel4->CCR &= ~DMA_CCR_EN;
		// on_complete_transfer();
	}
	
	// Clear global interrupt flag
	DMA1->IFCR |= DMA_IFCR_CGIF4;
}

void DMA1_Channel7_IRQHandler() {
	//Clear NVIC interrupt flag
	NVIC_ClearPendingIRQ(DMA1_Channel7_IRQn);
	
	// Check Transfer Complete interrupt flag
	if(DMA1->ISR & DMA_ISR_TCIF7){
		DMA1->IFCR |= DMA_IFCR_CTCIF7;
		DMA1_Channel7->CCR &= ~DMA_CCR_EN;
		// on_complete_transfer();
	}
	
	// Clear global interrupt flag
	DMA1->IFCR |= DMA_IFCR_CGIF7;
}