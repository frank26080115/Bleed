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
#include "arduino_mpu9250_spi.h"
#include <Arduino.h>
#include <SPI.h>
#include <DmaSpi.h>

#define DMASPIn DMASPI0

static SPISettings spiSettingsSlow(1000000, MSBFIRST, SPI_MODE3);
static SPISettings spiSettingsFast(20000000, MSBFIRST, SPI_MODE3);
static uint8_t mpu_csPin;
static uint8_t mpu_intPin;
static DmaSpi::Transfer mpu_dmaTrx(nullptr, 0, nullptr);

int mpu_spi_write(unsigned char reg_addr, unsigned char length, unsigned char * data)
{
	SPI.beginTransaction(spiSettingsSlow);

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
	#if 0
	if (length <= 2) {
		SPI.beginTransaction(spiSettingsSlow);
	}
	else
	#endif
	{
		SPI.beginTransaction(spiSettingsFast);
	}

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

int mpu_spi_read_dma(unsigned char reg_addr, unsigned char length, unsigned char * data)
{
	data[0] = reg_addr;

	static ActiveLowChipSelect cs(mpu_csPin, 
		spiSettingsFast
		//spiSettingsSlow
	);

	DMASPIn.begin();
	DMASPIn.start();

	mpu_dmaTrx = DmaSpi::Transfer(nullptr, length + 1, data, 0, cs);
	DMASPIn.registerTransfer(mpu_dmaTrx);

	return 0;
}

bool mpu_spi_read_dmaIsBusy(void)
{
	return mpu_dmaTrx.busy();
}

void mpu_spi_init(uint8_t csPin, uint8_t intPin)
{
	mpu_csPin = csPin;
	mpu_intPin = intPin;

	pinMode(mpu_csPin, OUTPUT);
	digitalWrite(mpu_csPin, HIGH);

	pinMode(mpu_intPin, INPUT_PULLUP);
}
