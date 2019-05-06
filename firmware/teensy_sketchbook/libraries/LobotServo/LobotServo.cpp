#include <LobotServo.h>

#define LOBOTSERVO_HEADER 0x55
#define LOBOTSERVO_REPLYTIMEOUT_MS 200

#define LOBOTSERVO_GETBYTE_LO(x) ((uint8_t)((x) & 0xFF))
#define LOBOTSERVO_GETBYTE_HI(x) ((uint8_t)(((x) >> 8) & 0xFF))
#define LOBOTSERVO_GETSHORT_BUFF_IDX(b, i) (((b)[(i)]) + (((b)[(i) + 1]) << 8))
#define LOBOTSERVO_READBACK_STARTIDX 3
#define LOBOTSERVO_READBACK_BASELENGTH 4

#define LOBOTSERVO_BUFFER_LEN_MAX 8
static uint8_t buff[LOBOTSERVO_BUFFER_LEN_MAX];

LobotServo::LobotServo(uint8_t id)
{
	this->id = id;
}

static void LobotServo::attachPort(HardwareSerial* serialPort)
{
	busport = serialPort;
}

static void LobotServo::initPort(void)
{
	serialPort.begin(LOBOTSERVO_LOBOTSERVO_BAUDRATE
	#ifdef SERIAL_8N2_SINGLEWIRE
	, SERIAL_8N2_SINGLEWIRE
	#elif defined(SERIAL_8N1_SINGLEWIRE)
	, SERIAL_8N1_SINGLEWIRE
	#elif defined(SERIAL_8N2)
	, SERIAL_8N2
	#endif
	);
}

static void LobotServo::attachRwPin(int8_t rwpin, int8_t rwpinpolarity, isSerPortBusy_t isserportbusy)
{
	rwPin = rwpin;
	rwPinPolarity = rwpinpolarity;
	isSerPortBusy = isserportbusy;
}

static void LobotServo::initRwPin(void)
{
	if (rwPin >= 0) {
		pinMode(rwPin, OUTPUT);
	}
	rwPinRead();
}

void LobotServo::attach(uint8_t id)
{
	this->id = id;
}

void LobotServo::moveExecute(void)
{
	targetAngleTicks = queuedAngleTicks;
	lastMoveDurationMs = queuedMoveDurationMs;
	write(this->id, LOBOTSERVO_CMD_MOVE_START, NULL, 0);
	lastMoveTimeMs = millis();
}

void LobotServo::moveWrite(uint8_t cmd, uint16_t angleTicks, uint16_t timeMs)
{
	int16_t ang = angleTicks;

	if (this->invert) {
		ang = LOBOTSERVO_ANGLETICKS_MAXIMUM - ang;
	}
	ang += this->minorAngleOffset;

	ang (ang < 0) ? 0 : ((ang > LOBOTSERVO_ANGLETICKS_MAXIMUM) ? LOBOTSERVO_ANGLETICKS_MAXIMUM : ang);

	buff[0] = LOBOTSERVO_GETBYTE_LO(ang);
	buff[1] = LOBOTSERVO_GETBYTE_HI(ang);
	buff[2] = LOBOTSERVO_GETBYTE_LO(timeMs);
	buff[3] = LOBOTSERVO_GETBYTE_HI(timeMs);
	write(this->id, cmd, buff, 4);
}

void LobotServo::moveQueue(uint16_t angleTicks, uint16_t timeMs)
{
	prevAngleTicks = targetAngleTicks;
	queuedAngleTicks = angleTicks;
	queuedMoveDurationMs = timeMs;
	moveWrite(LOBOTSERVO_CMD_MOVE_TIME_WAIT, angleTicks, timeMs);
}

void LobotServo::moveImmediate(uint16_t angleTicks, uint16_t timeMs)
{
	prevAngleTicks = targetAngleTicks;
	targetAngleTicks = queuedAngleTicks = angleTicks;
	lastMoveDurationMs = queuedMoveDurationMs = timeMs;
	moveWrite(LOBOTSERVO_CMD_MOVE_TIME, angleTicks, timeMs);
	lastMoveTimeMs = millis();
}

void LobotServo::moveExecuteAll(void)
{
	write(LOBOTSERVO_ID_GLOBAL, LOBOTSERVO_CMD_MOVE_START, NULL, 0);
}

void LobotServo::setServoMode(void) {
	buff[0] = 0;
	buff[1] = 0;
	buff[2] = 0;
	buff[3] = 0;
	write(this->id, LOBOTSERVO_CMD_SERVO_OR_MOTOR_MODE, buff, 4);
}

void LobotServo::setServoModeAll(void) {
	buff[0] = 0;
	buff[1] = 0;
	buff[2] = 0;
	buff[3] = 0;
	write(LOBOTSERVO_ID_GLOBAL, LOBOTSERVO_CMD_SERVO_OR_MOTOR_MODE, buff, 4);
}

void LobotServo::moveMotor(int16_t spd = 0)
{
	int16_t lim = 1000;
	if (this->invert) {
		spd *= -1;
	}
	spd = (spd < (-lim)) ? (-lim) : ((spd > lim) ? lim : spd);
	buff[0] = 1;
	buff[1] = 0;
	buff[2] = LOBOTSERVO_GETBYTE_LO(spd);
	buff[3] = LOBOTSERVO_GETBYTE_HI(spd);
	write(this->id, LOBOTSERVO_CMD_SERVO_OR_MOTOR_MODE, buff, 4);
}

bool LobotServo::getIsMotorMode(bool* success)
{
	bool ret;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	ret = read(this->id, LOBOTSERVO_CMD_SERVO_OR_MOTOR_MODE_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 4)) {
		ret = false;
	}
	if (success != NULL) {
		*success = ret;
	}
	if (ret == false) {
		return -1;
	}
	return (buff[LOBOTSERVO_READBACK_STARTIDX] == 0 ? false : true);
}

void LobotServo::stop(void)
{
	write(this->id, LOBOTSERVO_CMD_MOVE_STOP, NULL, 0);
}

void LobotServo::stopAll(void)
{
	write(LOBOTSERVO_ID_GLOBAL, LOBOTSERVO_CMD_MOVE_STOP, NULL, 0);
}

void LobotServo::powerOn(void) {
	setPowered(true);
}

void LobotServo::powerOff(void) {
	setPowered(false);
}

void LobotServo::powerOnAll(void) {
	setPoweredAll(true);
}

void LobotServo::powerOffAll(void) {
	setPoweredAll(false);
}

void LobotServo::setPowered(bool x)
{
	buff[0] = x ? 1 : 0;
	write(this->id, LOBOTSERVO_CMD_SERVO_OR_MOTOR_MODE, buff, 1);
}

void LobotServo::setPoweredAll(bool x)
{
	buff[0] = x ? 1 : 0;
	write(LOBOTSERVO_ID_GLOBAL, LOBOTSERVO_CMD_SERVO_OR_MOTOR_MODE, buff, 1);
}

bool LobotServo::getPowered(bool* success);
{
	bool ret;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	ret = read(this->id, LOBOTSERVO_CMD_LOAD_OR_UNLOAD_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 1)) {
		ret = false;
	}
	if (success != NULL) {
		*success = ret;
	}
	if (ret == false) {
		return -1;
	}
	return (buff[LOBOTSERVO_READBACK_STARTIDX] == 0 ? false : true);
}

void LobotServo::assignId(uint8_t id)
{
	if (id >= LOBOTSERVO_ID_GLOBAL) {
		return;
	}
	if (id == LOBOTSERVO_HEADER) {
		return;
	}

	write(this->id, LOBOTSERVO_CMD_ID_WRITE, &id, 1);

	this->id = id;
}

int16_t LobotServo::readId(void);
{
	bool ret;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	uint8_t x;
	ret = read(this->id, LOBOTSERVO_CMD_ID_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 1)) {
		ret = false;
	}
	if (ret == false) {
		return -1;
	}
	x = (uint8_t)buff[LOBOTSERVO_READBACK_STARTIDX];
	return (int16_t)x;
}

uint16_t LobotServo::readAngle(bool* success)
{
	bool ret;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	ret = read(this->id, LOBOTSERVO_CMD_ANGLE_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 2)) {
		ret = false;
	}
	if (success != NULL) {
		*success = ret;
	}
	if (ret == false) {
		return -127;
	}
	return (uint16_t)LOBOTSERVO_GETSHORT_BUFF_IDX(buff, LOBOTSERVO_READBACK_STARTIDX);
}

uint16_t LobotServo::estimateAngle(void)
{
	/*
	current_angle = prev_angle + ((target_angle - prev_angle) * ((now_time - start_time) / total_time))
	*/
	uint32_t t = millis();
	double dt, tt, ca, pa, ta, da;
	tt = lastMoveDurationMs;
	t -= lastMoveTimeMs;
	dt = t;

	pa = prevAngleTicks;
	ta = targetAngleTicks;
	da = ta - pa;

	da *= dt;
	da /= tt;
	ca = pa + da;

	return (uint16_t)lround(ca);
}

bool LobotServo::estimateIsDone(void)
{
	uint32_t t = millis();
	return ((lastMoveTimeMs + lastMoveDurationMs) <= t);
}

void LobotServo::angleOffsetAdjust(int8_t x)
{
	write(this->id, LOBOTSERVO_CMD_ANGLE_OFFSET, &x, 1);
}

int8_t LobotServo::readAngleOffset(bool* success)
{
	bool ret;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	ret = read(this->id, LOBOTSERVO_CMD_ANGLE_OFFSET_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 1)) {
		ret = false;
	}
	if (success != NULL) {
		*success = ret;
	}
	if (ret == false) {
		return -127;
	}
	return (int8_t)buff[LOBOTSERVO_READBACK_STARTIDX];
}

void LobotServo::setMinorAngleOffset(int16_t x)
{
	this->minorAngleOffset = x;
}

int16_t LobotServo::getMinorAngleOffset(void) {
	return this->minorAngleOffset;
}

void LobotServo::setInvert(bool x) {
	this->invert = x;
}

bool LobotServo::getInvert(void) {
	return this->invert;
}

void LobotServo::setAngleLimit(uint16_t minimum, uint16_t maximum, bool all)
{
	buff[0] = LOBOTSERVO_GETBYTE_LO(minimum);
	buff[1] = LOBOTSERVO_GETBYTE_HI(minimum);
	buff[2] = LOBOTSERVO_GETBYTE_LO(maximum);
	buff[3] = LOBOTSERVO_GETBYTE_HI(maximum);
	write(all ? LOBOTSERVO_ID_GLOBAL : this->id, LOBOTSERVO_CMD_ANGLE_LIMIT, buff, 4);
}

void LobotServo::getAngleLimit(bool* success, uint16_t* minimum, uint16_t* maximum)
{
	bool ret;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	ret = read(this->id, LOBOTSERVO_CMD_ANGLE_LIMIT_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 4)) {
		ret = false;
	}
	if (success != NULL)
	{
		*success = ret;
	}
	if (ret != false)
	{
		if (minimum != NULL) {
			*minimum = (uint16_t)LOBOTSERVO_GETSHORT_BUFF_IDX(buff, LOBOTSERVO_READBACK_STARTIDX);
		}
		if (maximum != NULL) {
			*maximum = (uint16_t)LOBOTSERVO_GETSHORT_BUFF_IDX(buff, LOBOTSERVO_READBACK_STARTIDX + 2);
		}
	}
}

void LobotServo::setVoltageLimit(uint16_t minimum, uint16_t maximum, bool all)
{
	buff[0] = LOBOTSERVO_GETBYTE_LO(minimum);
	buff[1] = LOBOTSERVO_GETBYTE_HI(minimum);
	buff[2] = LOBOTSERVO_GETBYTE_LO(maximum);
	buff[3] = LOBOTSERVO_GETBYTE_HI(maximum);
	write(all ? LOBOTSERVO_ID_GLOBAL : this->id, LOBOTSERVO_CMD_VIN_LIMIT, buff, 4);
}

void LobotServo::getVoltageLimit(bool* success, uint16_t* minimum, uint16_t* maximum)
{
	bool ret;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	ret = read(this->id, LOBOTSERVO_CMD_VIN_LIMIT_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 4)) {
		ret = false;
	}
	if (success != NULL)
	{
		*success = ret;
	}
	if (ret != false)
	{
		if (minimum != NULL) {
			*minimum = (uint16_t)LOBOTSERVO_GETSHORT_BUFF_IDX(buff, LOBOTSERVO_READBACK_STARTIDX);
		}
		if (maximum != NULL) {
			*maximum = (uint16_t)LOBOTSERVO_GETSHORT_BUFF_IDX(buff, LOBOTSERVO_READBACK_STARTIDX + 2);
		}
	}
}

void LobotServo::setTemperatureLimit(uint8_t maximum, bool all)
{
	write(all ? LOBOTSERVO_ID_GLOBAL : this->id, LOBOTSERVO_CMD_TEMP_MAX_LIMIT, &maximum, 1);
}

void LobotServo::getTemperatureLimit(bool* success, uint8_t* minimum, uint8_t* maximum)
{
	bool success;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	success = read(this->id, LOBOTSERVO_CMD_TEMP_MAX_LIMIT_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 1)) {
		success = false;
	}
	if (success == false) {
		return -1;
	}
	return (int8_t)buff[LOBOTSERVO_READBACK_STARTIDX];
}

int16_t LobotServo::readVoltage(void)
{
	bool success;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	success = read(this->id, LOBOTSERVO_CMD_VIN_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 2)) {
		success = false;
	}
	if (success == false) {
		return -1;
	}
	return (int16_t)LOBOTSERVO_GETSHORT_BUFF_IDX(buff, LOBOTSERVO_READBACK_STARTIDX);
}

int8_t LobotServo::readTemperature(void)
{
	bool success;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	success = read(this->id, LOBOTSERVO_CMD_TEMP_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 1)) {
		success = false;
	}
	if (success == false) {
		return -1;
	}
	return (int8_t)buff[LOBOTSERVO_READBACK_STARTIDX];
}

void LobotServo::setLed(uint8_t x, bool all) {
	write(all ? LOBOTSERVO_ID_GLOBAL : this->id, LOBOTSERVO_CMD_LED_CTRL_WRITE, &x, 1);
}

int8_t LobotServo::getLed(void)
{
	bool success;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	success = read(this->id, LOBOTSERVO_CMD_LED_CTRL_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 1)) {
		success = false;
	}
	if (success == false) {
		return -1;
	}
	return (int8_t)buff[LOBOTSERVO_READBACK_STARTIDX];
}

void LobotServo::setError(uint8_t x, bool all)
{
	write(all ? LOBOTSERVO_ID_GLOBAL : this->id, LOBOTSERVO_CMD_LED_ERROR_WRITE, &x, 1);
}

int8_t LobotServo::getError(void)
{
	bool success;
	uint8_t len = LOBOTSERVO_BUFFER_LEN_MAX;
	success = read(this->id, LOBOTSERVO_CMD_LED_ERROR_READ, buff, &len);
	if (len != (LOBOTSERVO_READBACK_BASELENGTH + 1)) {
		success = false;
	}
	if (success == false) {
		return -1;
	}
	return (int8_t)buff[LOBOTSERVO_READBACK_STARTIDX];
}

static float LobotServo::ticksToDegreesF(int16_t x) {
	float xx = x;
	return xx * 0.24f;
}
static double LobotServo::ticksToDegreesD(int16_t x) {
	double xx = x;
	return xx * 0.24d;
}
static int16_t LobotServo::degreesToTicksF(float x) {
	return (int16_t)lroundf(x / 0.24f);
}
static int16_t LobotServo::degreesToTicksD(double x) {
	return (int16_t)lround(x / 0.24d);
}

static void LobotServo::write(uint8_t id, uint8_t cmd, uint8_t* data, uint8_t len)
{
	uint8_t chksum;
	uint8_t i;

	rwPinWrite();

	busport->write(LOBOTSERVO_HEADER);
	busport->write(LOBOTSERVO_HEADER);
	chksum = 0;
	#define LOBOTSERVO_WRITE_AND_ADD(x) do { busport->write(x); chksum += (x); } while (0)
	LOBOTSERVO_WRITE_AND_ADD(id);
	LOBOTSERVO_WRITE_AND_ADD(len + 3);
	LOBOTSERVO_WRITE_AND_ADD(cmd);
	if (data != NULL)
	{
		for (i = 0; i < len; i++)
		{
			uint8_t d = data[i];
			LOBOTSERVO_WRITE_AND_ADD(d);
		}
	}
	busport->write(chksum);

	rwPinRead();
}

static bool LobotServo::read(uint8_t id, uint8_t cmd, uint8_t* data, uint8_t* len)
{
	uint32_t ts, now;
	uint8_t headercnt = 0, idx = 0, chksum = 0, lim;
	bool succuess = false;

	write(id, cmd, NULL, 0);
	// assume the RW pin is in read mode now

	ts = millis();
	while ((now = millis()) < (ts + LOBOTSERVO_REPLYTIMEOUT_MS))
	{
		yield();
		if (busport->available() > 0)
		{
			ts = now;
			uint8_t d = busport->read();
			if (headercnt < 2)
			{
				if (d == LOBOTSERVO_HEADER)
				{
					headercnt++;
				}
				else
				{
					headercnt = 0;
				}
				idx = 0;
				chksum = 0;
			}
			else
			{
				buff[idx] = d;
				idx++;
				chksum += d;
				if ((idx == 2 && d <= 3) || (idx == 1 && id != d) || (idx == 3 && cmd != d))
				{
					// mismatch, try again
					headercnt = 0;
					idx = 0;
					continue;
				}
				else if (idx == 2)
				{
					lim = d;
				}
				else if (idx > 2)
				{
					// for example, read ID has length 4
					// [0]     [1]        [2]     [3]     [4]
					// ID      len=4      CMD     param   chksum
					// exit if idx is > 4
					if (idx == (lim + 1))
					{
						if (d == chksum)
						{
							succuess = true;
							break;
						}
						else
						{
							// mismatch, try again
							headercnt = 0;
							idx = 0;
							continue;
						}
					}
				}
			}
		}
	}
	if (success != false && data != NULL && len != NULL)
	{
		uint8_t copyLen = *len;
		if (copyLen > idx)
		{
			copyLen = idx;
		}
		*len = idx;
		if (data != buff) {
			memccpy(data, buff, copyLen);
		}
	}
	return success;
}

static void LobotServo::rwPinRead(void) {
	uint32_t ts, now;

	if (rwPin < 0) {
		return;
	}

	if (isSerPortBusy != NULL)
	{
		while (isSerPortBusy()) {
			yield();
		}
	}

	digitalWrite(rwPin, rwPinPolarity == 0 ? LOW : HIGH);
}

static void LobotServo::rwPinWrite(void) {
	if (rwPin < 0) {
		return;
	}
	digitalWrite(rwPin, rwPinPolarity == 0 ? HIGH : LOW);
}