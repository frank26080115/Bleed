#include <Arduino.h>
#include <PXX.h>

#define PPM_CENTER                          1500
#define PPM_LOW                             817
#define PPM_HIGH                            2182
#define PPM_HIGH_ADJUSTED                   (PPM_HIGH - PPM_LOW)
#define PXX_CHANNEL_WIDTH                   2048
#define PXX_UPPER_LOW                       2049
#define PXX_UPPER_HIGH                      4094
#define PXX_LOWER_LOW                       1
#define PXX_LOWER_HIGH                      2046

const uint16_t CRC_Short[] =
{
	0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
	0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7
};

uint16_t CRCTable(uint8_t val)
{
	return CRC_Short[val&0x0F] ^ (0x1081 * (val>>4));
}


void PXX_Class::crc( uint8_t data )
{
	pcmCrc=(pcmCrc<<8) ^ CRCTable((pcmCrc>>8)^data) ;
}

void PXX_Class::begin(HardwareSerial* p)
{
	this->port = p;
	this->port->begin(125000
	#ifdef SERIAL_8N2
	, SERIAL_8N2
	#elif defined(SERIAL_8N1)
	, SERIAL_8N1
	#endif
	);
}

void PXX_Class::putPcmSerialBit(uint8_t bit)
{
	serialByte >>= 1;
	if (bit & 1)
	{
		serialByte |= 0x80;
	}

	if (++serialBitCount >= 8)
	{
		pulses[length++] = serialByte;
		serialBitCount = 0;
	}
}

// 8uS/bit 01 = 0, 001 = 1
void PXX_Class::putPcmPart(uint8_t value)
{
	putPcmSerialBit(0);

	if (value)
	{
		putPcmSerialBit(0);
	}

	putPcmSerialBit(1);
}

void PXX_Class::putPcmFlush()
{
	while (serialBitCount != 0)
	{
		putPcmSerialBit(1);
	}
}

void PXX_Class::putPcmBit(uint8_t bit)
{
	if (bit)
	{
		pcmOnesCount += 1;
		putPcmPart(1);
	}
	else
	{
		pcmOnesCount = 0;
		putPcmPart(0);
	}

	if (pcmOnesCount >= 5)
	{
		putPcmBit(0);                                // Stuff a 0 bit in
	}
}

void PXX_Class::putPcmByte(uint8_t byte)
{
	crc(byte);

	for (uint8_t i=0; i<8; i++)
	{
		putPcmBit(byte & 0x80);
		byte <<= 1;
	}
}

void PXX_Class::putPcmHead()
{
	// send 7E, do not CRC
	// 01111110
	putPcmPart(0);
	putPcmPart(1);
	putPcmPart(1);
	putPcmPart(1);
	putPcmPart(1);
	putPcmPart(1);
	putPcmPart(1);
	putPcmPart(0);
}

void PXX_Class::prepare(int16_t channels[16])
{
	uint16_t chan=0, chan_low=0;

	length = 0;
	pcmCrc = 0;
	pcmOnesCount = 0;

	/* Preamble */
	putPcmPart(0);
	putPcmPart(0);
	putPcmPart(0);
	putPcmPart(0);

	/* Sync */
	putPcmHead();

	// Rx Number
	putPcmByte(16);

	// FLAG1 - Fail Safe Mode, nothing currently set, maybe want to do this
	putPcmByte(0);

	// FLAG2
	putPcmByte(0);

	// PPM
	for (int i=0; i<8; i++)
	{

		int channelPPM = channels[(sendUpperChannels ? (8 + i) : i)];
		float convertedChan = ((float(channelPPM) - float(PPM_LOW)) / (float(PPM_HIGH_ADJUSTED))) * float(PXX_CHANNEL_WIDTH);
		chan = limit((sendUpperChannels ? PXX_UPPER_LOW : PXX_LOWER_LOW),
					 convertedChan,
					 (sendUpperChannels ? PXX_UPPER_HIGH : PXX_LOWER_HIGH));

		if (i & 1)
		{
			putPcmByte(chan_low); // Low byte of channel
			putPcmByte(((chan_low >> 8) & 0x0F) | (chan << 4));  // 4 bits each from 2 channels
			putPcmByte(chan >> 4);  // High byte of channel
		}
		else
		{
			chan_low = chan;
		}
	}

	// CRC16
	putPcmByte(0);
	chan = pcmCrc;
	putPcmByte(chan>>8);
	putPcmByte(chan);

	// Sync
	putPcmHead();
	putPcmFlush();
}

void PXX_Class::send()
{
	for(int i = 0; i < length; i++)
	{
		this->port->write(pulses[i]);
	}

	sendUpperChannels = !sendUpperChannels;
}

uint16_t PXX_Class::limit(uint16_t low, uint16_t val, uint16_t high) {
	if(val < low)
	{
		return low;
	}

	if(val > high)
	{
		return high;
	}

	return val;
}

PXX_Class PXX;
