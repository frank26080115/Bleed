/******************************************************************************
arduino_mpu9250_spi.cpp
MPU-9250 Digital Motion Processor Arduino Library 
Jim Lindblom @ SparkFun Electronics
original creation date: November 23, 2016
https://github.com/sparkfun/SparkFun_MPU9250_DMP_Arduino_Library

modified by Frank Zhao 2019 to add SPI

This library implements motion processing functions of Invensense's MPU-9250.
It is based on their Emedded MotionDriver 6.12 library.
	https://www.invensense.com/developers/software-downloads/

Development environment specifics:
Arduino IDE 1.6.12
SparkFun 9DoF Razor IMU M0

******************************************************************************/
#include "arduino_mpu9250_spi.h"
#include <Arduino.h>
#include <SPI.h>

static SPISettings spiSettings(1000000, MSBFIRST, SPI_MODE3);
static uint8_t mpu_csPin;
static uint8_t mpu_intPin;

int mpu_spi_write(unsigned char reg_addr, unsigned char length, unsigned char * data)
{
	SPI.beginTransaction(spiSettings);

	digitalWrite(mpu_csPin, LOW);

	SPI.transfer(reg_addr & 0x7F);
	for (unsigned char i = 0; i < length; i++)
	{
		SPI.transfer(data[i]);
	}

	digitalWrite(mpu_csPin, HIGH);

	SPI.endTransaction();

	return 0;
}

int mpu_spi_read(unsigned char reg_addr, unsigned char length, unsigned char * data)
{
	SPI.beginTransaction(spiSettings);

	digitalWrite(mpu_csPin, LOW);

	SPI.transfer(reg_addr | 0x80);

	for (unsigned char i = 0; i < length; i++)
	{
		data[i] = SPI.transfer(0xFF);
	}

	digitalWrite(mpu_csPin, HIGH);

	SPI.endTransaction();

	return 0;
}

void mpu_spi_init(uint8_t csPin, uint8_t intPin)
{
	mpu_csPin = csPin;
	mpu_intPin = intPin;

	pinMode(mpu_csPin, OUTPUT);
	digitalWrite(mpu_csPin, HIGH);

	pinMode(mpu_intPin, INPUT_PULLUP);
}
