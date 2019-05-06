/******************************************************************************
arduino_mpu9250_spi.cpp
MPU-9250 Digital Motion Processor Arduino Library 
Jim Lindblom @ SparkFun Electronics
original creation date: November 23, 2016
https://github.com/sparkfun/SparkFun_MPU9250_DMP_Arduino_Library

modified by Frank Zhao 2019 to add SPI and DMA

This library implements motion processing functions of Invensense's MPU-9250.
It is based on their Emedded MotionDriver 6.12 library.
	https://www.invensense.com/developers/software-downloads/

Development environment specifics:
Arduino IDE 1.6.12
SparkFun 9DoF Razor IMU M0

******************************************************************************/
#ifndef _ARDUINO_MPU9250_SPI_H_
#define _ARDUINO_MPU9250_SPI_H_

#if defined(__cplusplus) 
extern "C" {
#endif

int mpu_spi_write(unsigned char reg_addr, unsigned char length, unsigned char * data);
int mpu_spi_read(unsigned char reg_addr, unsigned char length, unsigned char * data);
int mpu_spi_read_dma(unsigned char reg_addr, unsigned char length, unsigned char * data);
bool mpu_spi_read_dmaIsBusy(void);
void mpu_spi_init(uint8_t csPin, uint8_t intPin);

#if defined(__cplusplus) 
}
#endif

#endif // _ARDUINO_MPU9250_SPI_H_