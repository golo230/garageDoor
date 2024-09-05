#include "SPI.h"
#include "SysTimer.h"
#include "accelerometer.h"

void accWrite(uint8_t addr, uint8_t val){
	// TODO access SPI_Transfer_Data
	// 0 0 addr val 11100010
	// addr = addr << 2;
	// addr = addr >> 2;
	//addr = addr & 0b00111111;
	uint16_t write_data = 0;
	write_data = (uint16_t)((addr) << 8);
	write_data = write_data | val;
	SPI_Transfer_Data(write_data);
}

uint8_t accRead(uint8_t addr){
	// access SPI_Transfer_Data
	// addr = addr << 2;
	// addr = addr >> 2;
	//addr = addr & 0b00111111;
	// int16_t readVal = ((uint16_t)0b10) << 14 | ((int16_t)addr) << 8;
	//readVal = readVal | ((int16_t)addr) << 8;
	//int16_t readVal = 0b1011111100000000 & ((int16_t)addr << 8);
	// uint16_t data = SPI_Transfer_Data(readVal);
	// uint8_t data1 = data;
	//return (SPI_Transfer_Data(readVal) & 0xFF); // TODO
	uint16_t read_data1 = 0;
	uint8_t read_data2 = 0;
	
	read_data1 = (uint16_t)(addr << 8);
	read_data1 |= (1UL << 15);
	read_data2 = 0xFF & SPI_Transfer_Data(read_data1);
	return read_data2;
}

void initAcc(void){
	// set full range mode
	// enable measurement
	accWrite(0x31, 0x9);
	accWrite(0x2D, 0x8);
}

void readValues(double* x, double* y, double* z){
	// find scaler from data sheet
	*x = (((int16_t)(accRead(0x33) << 8)) | accRead(0x32)) * 0.0039;
	*y = (((int16_t)(accRead(0x35) << 8)) | accRead(0x34)) * 0.0039;
	*z = (((int16_t)(accRead(0x37) << 8)) | accRead(0x36)) * 0.0039;
	// read values into x,y,z using accRead
}
