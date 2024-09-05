#include "I2C.h"

// Inter-integrated Circuit Interface (I2C)
// up to 100 Kbit/s in the standard mode, 
// up to 400 Kbit/s in the fast mode, and 
// up to 3.4 Mbit/s in the high-speed mode.

// Recommended external pull-up resistance is 
// 4.7 kOmh for low speed, 
// 3.0 kOmh for the standard mode, and 
// 1.0 kOmh for the fast mode
	
//===============================================================================
//                        I2C GPIO Initialization
//===============================================================================
void I2C_GPIO_Init(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	
	//PB8 Initialization
	//Output = open drain (1)
	GPIOB->OTYPER |= GPIO_OTYPER_OT8;
	//Output speed = very high (11)
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED8;
	//Pull up (01)
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8_1);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0;
	// Mode = alternate function (10)
	GPIOB->MODER |= GPIO_MODER_MODE8_1;
	GPIOB->MODER &= ~(GPIO_MODER_MODE8_0);
	// Specific alternate function (0100 - AF4)
	GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL8;
	GPIOB->AFR[1] |= GPIO_AFRH_AFSEL8_2;

	// PB9 Initialization
	//Output = open drain (1)
	GPIOB->OTYPER |= GPIO_OTYPER_OT9;
	//Output speed = very high (11)
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED9;
	//Pull up (01)
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD9_1);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD9_0;
	// Mode = alternate function (10)
	GPIOB->MODER |= GPIO_MODER_MODE9_1;
	GPIOB->MODER &= ~(GPIO_MODER_MODE9_0);
	// Specific alternate function (0100 - AF4)
	GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL9;
	GPIOB->AFR[1] |= GPIO_AFRH_AFSEL9_2;
}
	
#define I2C_TIMINGR_PRESC_POS	28
#define I2C_TIMINGR_SCLDEL_POS	20
#define I2C_TIMINGR_SDADEL_POS	16
#define I2C_TIMINGR_SCLH_POS	8
#define I2C_TIMINGR_SCLL_POS	0

//===============================================================================
//                          I2C Initialization
//===============================================================================
void I2C_Initialization(void){
	uint32_t OwnAddr = 0x52;
	
	// [TODO]
	// Number 1
	// Part A - enable I2C clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
	// Part B - system clock is source clock
	RCC->CCIPR &= ~RCC_CCIPR_I2C1SEL;
	RCC->CCIPR |= RCC_CCIPR_I2C1SEL_0;
	// Part C - Reset I2C1
	RCC->APB1RSTR1 |= RCC_APB1RSTR1_I2C1RST;
	RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_I2C1RST;
	
	//Number 2
	// Part A
	// Disable I2C
	I2C1->CR1 &= ~I2C_CR1_PE;
	// Enable analog noise filter
	I2C1->CR1 &= ~I2C_CR1_ANFOFF;
	// Disable digital noise filter
	I2C1->CR1 &= ~I2C_CR1_DNF;
	// Enable error interrupts
	I2C1->CR1 |= I2C_CR1_ERRIE;
	// Enable clock stretching
	I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;
	// Set master to operate in 7-bit addressing mode
	I2C1->CR2 &= ~I2C_CR2_ADD10;
	// Enable automatic end mode
	I2C1->CR2 |= I2C_CR2_AUTOEND;
	// Enable NACK generation
	I2C1->CR2 |= I2C_CR2_NACK;
	
	// Part B
	// 12 = 11.5 actually (SDADEL)
	I2C1->TIMINGR &= ~(15UL << 28);
	I2C1->TIMINGR &= ~(15UL << 20);
	I2C1->TIMINGR &= ~(15UL << 16);
	I2C1->TIMINGR &= ~(15UL << 8);
	I2C1->TIMINGR &= ~(15UL);
	I2C1->TIMINGR = 7UL << 28 | 10UL << 20 | 12UL << 16 | 40UL << 8 | 47UL;
	
	// Part C
	// Disable own address 1
	I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
	// Disable own address 2
	I2C1->OAR2 &= ~I2C_OAR2_OA2EN;
	// Set to 7-bit mode
	I2C1->OAR1 &= ~I2C_OAR1_OA1MODE;
	//Write own address to register
	I2C1->OAR1 &= ~(127UL << 1);
	I2C1->OAR1 |= OwnAddr << 1;
	// Reenable own address 1
	I2C1->OAR1 |= I2C_OAR1_OA1EN;
	
	// Part D
	// Reenable I2C
	I2C1->CR1 |= I2C_CR1_PE;
}

//===============================================================================
//                           I2C Start
// Master generates START condition:
//    -- Slave address: 7 bits
//    -- Automatically generate a STOP condition after all bytes have been transmitted 
// Direction = 0: Master requests a write transfer
// Direction = 1: Master requests a read transfer
//=============================================================================== 
int8_t I2C_Start(I2C_TypeDef * I2Cx, uint32_t DevAddress, uint8_t Size, uint8_t Direction) {	
	
	// Direction = 0: Master requests a write transfer
	// Direction = 1: Master requests a read transfer
	
	uint32_t tmpreg = 0;
	
	// This bit is set by software, and cleared by hardware after the Start followed by the address
	// sequence is sent, by an arbitration loss, by a timeout error detection, or when PE = 0.
	tmpreg = I2Cx->CR2;
	
	tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
	
	if (Direction == READ_FROM_SLAVE)
		tmpreg |= I2C_CR2_RD_WRN;  // Read from Slave
	else
		tmpreg &= ~I2C_CR2_RD_WRN; // Write to Slave
		
	tmpreg |= (uint32_t)(((uint32_t)DevAddress & I2C_CR2_SADD) | (((uint32_t)Size << 16 ) & I2C_CR2_NBYTES));
	
	tmpreg |= I2C_CR2_START;
	
	I2Cx->CR2 = tmpreg; 
	
   	return 0;  // Success
}

//===============================================================================
//                           I2C Stop
//=============================================================================== 
void I2C_Stop(I2C_TypeDef * I2Cx){
	// Master: Generate STOP bit after the current byte has been transferred 
	I2Cx->CR2 |= I2C_CR2_STOP;								
	// Wait until STOPF flag is reset
	while( (I2Cx->ISR & I2C_ISR_STOPF) == 0 ); 
}

//===============================================================================
//                           Wait for the bus is ready
//=============================================================================== 
void I2C_WaitLineIdle(I2C_TypeDef * I2Cx){
	// Wait until I2C bus is ready
	while( (I2Cx->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY );	// If busy, wait
}

//===============================================================================
//                           I2C Send Data
//=============================================================================== 
int8_t I2C_SendData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size) {
	int i;
	
	if (Size <= 0 || pData == NULL) return -1;
	
	I2C_WaitLineIdle(I2Cx);
	
	if (I2C_Start(I2Cx, DeviceAddress, Size, WRITE_TO_SLAVE) < 0 ) return -1;

	// Send Data
	// Write the first data in DR register
	// while((I2Cx->ISR & I2C_ISR_TXE) == 0);
	// I2Cx->TXDR = pData[0] & I2C_TXDR_TXDATA;  

	for (i = 0; i < Size; i++) {
		// TXE is set by hardware when the I2C_TXDR register is empty. It is cleared when the next
		// data to be sent is written in the I2C_TXDR register.
		// while( (I2Cx->ISR & I2C_ISR_TXE) == 0 ); 

		// TXIS bit is set by hardware when the I2C_TXDR register is empty and the data to be
		// transmitted must be written in the I2C_TXDR register. It is cleared when the next data to be
		// sent is written in the I2C_TXDR register.
		// The TXIS flag is not set when a NACK is received.
		while((I2Cx->ISR & I2C_ISR_TXIS) == 0 );
		I2Cx->TXDR = pData[i] & I2C_TXDR_TXDATA;  // TXE is cleared by writing to the TXDR register.
	}
	
	// Wait until TC flag is set 
	while((I2Cx->ISR & I2C_ISR_TC) == 0 && (I2Cx->ISR & I2C_ISR_NACKF) == 0);
	
	if( (I2Cx->ISR & I2C_ISR_NACKF) != 0 ) return -1;

	I2C_Stop(I2Cx);
	return 0;
}


//===============================================================================
//                           I2C Receive Data
//=============================================================================== 
int8_t I2C_ReceiveData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size) {
	int i;
	
	if(Size <= 0 || pData == NULL) return -1;

	I2C_WaitLineIdle(I2Cx);

	I2C_Start(I2Cx, DeviceAddress, Size, READ_FROM_SLAVE); // 0 = sending data to the slave, 1 = receiving data from the slave
						  	
	for (i = 0; i < Size; i++) {
		// Wait until RXNE flag is set 	
		while( (I2Cx->ISR & I2C_ISR_RXNE) == 0 );
		pData[i] = I2Cx->RXDR & I2C_RXDR_RXDATA;
	}
	
	// Wait until TCR flag is set 
	while((I2Cx->ISR & I2C_ISR_TC) == 0);
	
	I2C_Stop(I2Cx);
	
	return 0;
}
