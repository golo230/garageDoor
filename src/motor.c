/*
 * ECE 153B
 *
 * Name(s): Derek Dela Cruz, Josephine Nghiem
 * Section: Tuesday 7pm
 * Project
 */

#include "stm32l476xx.h"
#include "motor.h"

static const uint32_t MASK = 0x0;//TODO
static const uint32_t HalfStep[8] = {0x00000220, 0x00000020, 0x00000120, 0x00000100, 0x00000140, 0x00000040, 0x00000240, 0x00000200};//TODO
static const uint32_t DELAY = 200;

static volatile int8_t dire = 0;
static volatile uint8_t step = 0;

void Motor_Init(void) {	
	RCC->AHB2ENR |= (uint32_t)RCC_AHB2ENR_GPIOCEN;
	
	GPIOC->MODER |= (uint32_t)GPIO_MODER_MODE5_0;
	GPIOC->MODER &= ~(uint32_t)GPIO_MODER_MODE5_1;
	GPIOC->MODER |= (uint32_t)GPIO_MODER_MODE6_0;
	GPIOC->MODER &= ~(uint32_t)GPIO_MODER_MODE6_1;
	GPIOC->MODER |= (uint32_t)GPIO_MODER_MODE8_0;
	GPIOC->MODER &= ~(uint32_t)GPIO_MODER_MODE8_1;
	GPIOC->MODER |= (uint32_t)GPIO_MODER_MODE9_0;
	GPIOC->MODER &= ~(uint32_t)GPIO_MODER_MODE9_1;
	
	GPIOC->OSPEEDR |= (uint32_t)GPIO_OSPEEDR_OSPEED5_1;
	GPIOC->OSPEEDR &= ~(uint32_t)GPIO_OSPEEDR_OSPEED5_0;
	GPIOC->OSPEEDR |= (uint32_t)GPIO_OSPEEDR_OSPEED6_1;
	GPIOC->OSPEEDR &= ~(uint32_t)GPIO_OSPEEDR_OSPEED6_0;
	GPIOC->OSPEEDR |= (uint32_t)GPIO_OSPEEDR_OSPEED8_1;
	GPIOC->OSPEEDR &= ~(uint32_t)GPIO_OSPEEDR_OSPEED8_0;
	GPIOC->OSPEEDR |= (uint32_t)GPIO_OSPEEDR_OSPEED9_1;
	GPIOC->OSPEEDR &= ~(uint32_t)GPIO_OSPEEDR_OSPEED9_0;
	
	GPIOC->OTYPER &= ~(uint32_t)GPIO_OTYPER_OT5;
	GPIOC->OTYPER &= ~(uint32_t)GPIO_OTYPER_OT6;
	GPIOC->OTYPER &= ~(uint32_t)GPIO_OTYPER_OT8;
	GPIOC->OTYPER &= ~(uint32_t)GPIO_OTYPER_OT9;
	
	GPIOC->PUPDR &= ~(uint32_t)GPIO_PUPDR_PUPD5_0;
	GPIOC->PUPDR &= ~(uint32_t)GPIO_PUPDR_PUPD5_1;
	GPIOC->PUPDR &= ~(uint32_t)GPIO_PUPDR_PUPD6_0;
	GPIOC->PUPDR &= ~(uint32_t)GPIO_PUPDR_PUPD6_1;
	GPIOC->PUPDR &= ~(uint32_t)GPIO_PUPDR_PUPD8_0;
	GPIOC->PUPDR &= ~(uint32_t)GPIO_PUPDR_PUPD8_1;
	GPIOC->PUPDR &= ~(uint32_t)GPIO_PUPDR_PUPD9_0;
	GPIOC->PUPDR &= ~(uint32_t)GPIO_PUPDR_PUPD9_1;
}

void rotate(void) {
	GPIOC->ODR &= MASK;
	
	// Clockwise
	if(dire == 1){
		if(step == 7){
			step = 0;
		}
		else{
			step++;
		}
		//printf("test 1\r\n");
		GPIOC->ODR |= HalfStep[step];
	}
	// Counter-Clockwise
	else if (dire == -1){
		if(step == 0){
			step = 7;
		}
		else{
			step--;
		}
		GPIOC->ODR |= HalfStep[step];
	}
	else if (dire == 2){
		//printf("stopping\n");
		GPIOC->ODR &= MASK;
	}
}

void setDire(int8_t direction) {
	dire = direction;
}
	


