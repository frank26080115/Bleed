#ifndef _LOBOT_SERVO_H_
#define _LOBOT_SERVO_H_

#include <Arduino.h>
#include <HardwareSerial.h>

#define LOBOTSERVO_BAUDRATE 115200

#define LOBOTSERVO_ID_GLOBAL   254
#define LOBOTSERVO_ID_DEFAULT  1

#define LOBOTSERVO_ERRORBIT_NONE        0x0
#define LOBOTSERVO_ERRORBIT_TEMPERATURE 0x1
#define LOBOTSERVO_ERRORBIT_VOLTAGE     0x2
#define LOBOTSERVO_ERRORBIT_STALL       0x4

#define LOBOTSERVO_ANGLETICKS_MAXIMUM 10000

typedef bool (*isSerPortBusy_t)(void);

class LobotServo
{
	pubic:
		LobotServo(uint8_t id = LOBOTSERVO_ID_DEFAULT);

		static void attachPort(HardwareSerial* serialPort);
		static void initPort(void);
		static void attachRwPin(int8_t rwpin = -1, int8_t rwpinpolarity = 0, isSerPortBusy_t isserportbusy = NULL);

		void attach(uint8_t id = LOBOTSERVO_ID_DEFAULT);

		void moveExecute(void);
		void moveQueue(uint16_t angleTicks, uint16_t timeMs);
		void moveImmediate(uint16_t angleTicks, uint16_t timeMs);
		void moveExecuteAll(void);

		void setServoMode(void);
		void setServoModeAll(void);
		void moveMotor(int16_t spd = 0);
		bool getIsMotorMode(bool* success);

		void stop(void);
		void stopAll(void);

		void powerOn(void);
		void powerOff(void);
		void powerOnAll(void);
		void powerOffAll(void);
		void setPowered(bool x);
		void setPoweredAll(bool x);
		bool getPowered(bool* success);

		void assignId(uint8_t id);

		int16_t readId(void);

		uint16_t readAngle(bool* success);
		uint16_t estimateAngle(void);
		bool estimateIsDone(void);

		void angleOffsetAdjust(int8_t x);
		int8_t readAngleOffset(bool* success);

		void setInvert(bool x);
		bool getInvert(void);

		void setAngleLimit(uint16_t minimum, uint16_t maximum, bool all = false);
		void getAngleLimit(bool* success, uint16_t* minimum, uint16_t* maximum);
		void setVoltageLimit(uint16_t minimum, uint16_t maximum, bool all = false);
		void getVoltageLimit(bool* success, uint16_t* minimum, uint16_t* maximum);
		void setTemperatureLimit(uint8_t maximum, bool all = false);
		int8_t getTemperatureLimit();

		int16_t readVoltage(void);
		int8_t readTemperature(void);

		void setLed(uint8_t x, bool all = false);
		int8_t getLed(void);

		void setError(uint8_t x, bool all = false);
		int8_t getError(void);

		static float ticksToDegreesF(int16_t x);
		static double ticksToDegreesD(int16_t x);
		static int16_t degreesToTicksF(float x);
		static int16_t degreesToTicksD(double x);

	private:
		uint8_t id;
		static int8_t rwPin = -1;
		static int8_t rwPinPolarity;
		bool invert;
		int16_t minorAngleOffset;
		uint32_t lastMoveTimeMs;
		uint32_t lastMoveDurationMs;
		uint32_t queuedMoveDurationMs;
		uint32_t previousAngleTicks;
		uint32_t targetAngleTicks;
		uint32_t queuedAngleTicks;
		static HardwareSerial* busport;
		static isSerPortBusy_t isSerPortBusy = NULL;
		void moveWrite(uint8_t cmd, uint16_t angleTicks, uint16_t timeMs);
		static void write(uint8_t id, uint8_t cmd, uint8_t* data, uint8_t len);
		static bool read(uint8_t id, uint8_t cmd, uint8_t* data, uint8_t* len);
};

#endif