#include <Arduino.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "bleedheadtilt.h"

#define USE_ANALOGWRITE
#define USE_PWMSERVO

#ifdef USE_PWMSERVO
#include <PWMServo.h>
#endif

#define PIN_TILT_SERVO_PWM 1
#ifdef USE_ANALOGWRITE
#define SERVO_PWM_CENTER 9830
#define SERVO_PWM_SPREAD (SERVO_PWM_CENTER / 3)
#endif

#define PIN_TILT_SERVO_DAC A14
#define SERVO_DAC_WRITE(x) analogWriteDAC0((x))

#define HEAD_TILT_SPREAD 15.0f
#define HEAD_CTRL_LIMIT_MIN -32000
#define HEAD_CTRL_LIMIT_MAX 32000

#ifdef USE_ANALOGWRITE
uint32_t
#endif
#ifdef USE_PWMSERVO
float
#endif
         headtilt_ctrlToPwm(int16_t x);
uint32_t headtilt_pitchDegToDac(float x);

#ifdef USE_PWMSERVO
PWMServo tiltservo;
#endif

void headtilt_init(void)
{
	digitalWrite(PIN_TILT_SERVO_PWM, LOW);
	pinMode(PIN_TILT_SERVO_PWM, OUTPUT);
	digitalWrite(PIN_TILT_SERVO_DAC, LOW);
	pinMode(PIN_TILT_SERVO_DAC, OUTPUT);

	#ifdef USE_ANALOGWRITE
	analogWriteResolution(16);
	analogWriteFrequencySlow(PIN_TILT_SERVO_PWM, 100);
	#endif

	#ifdef USE_PWMSERVO
	tiltservo.attach(PIN_TILT_SERVO_PWM);
	#endif

	analogReference(EXTERNAL);
}

void headtilt_setTarget(int16_t x)
{
	#ifdef USE_ANALOGWRITE
	analogWrite(PIN_TILT_SERVO_PWM, headtilt_ctrlToPwm(x));
	#endif
	#ifdef USE_PWMSERVO
	tiltservo.writef(headtilt_ctrlToPwm(x));
	#endif
}

void headtilt_setPitchDeg(float x)
{
	SERVO_DAC_WRITE(headtilt_pitchDegToDac(x));
}

#ifdef USE_ANALOGWRITE
uint32_t headtilt_ctrlToPwm(int16_t x)
{
	float in_min = -32767;
	float in_max = 32767;
	float out_min = SERVO_PWM_CENTER - SERVO_PWM_SPREAD;
	float out_max = SERVO_PWM_CENTER + SERVO_PWM_SPREAD;

	x = constrain(x, HEAD_CTRL_LIMIT_MIN, HEAD_CTRL_LIMIT_MAX);

	float yf = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	int32_t yl = lroundf(yf);
	yl = constrain(yl, lroundf(out_min), lroundf(out_max));
	return yl;
}
#endif

#ifdef USE_PWMSERVO
float headtilt_ctrlToPwm(int16_t x)
{
	float in_min  = -32767;
	float in_max  =  32767;
	float out_min =  0.0f;
	float out_max =  180.0f;

	x = constrain(x, HEAD_CTRL_LIMIT_MIN, HEAD_CTRL_LIMIT_MAX);

	float yf = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

	return yf;
}
#endif

uint32_t headtilt_pitchDegToDac(float x)
{
	float in_min  = -HEAD_TILT_SPREAD;
	float in_max  =  HEAD_TILT_SPREAD;
	float out_min =  0;
	float out_max =  4095;

	// wrap
	if (x > 180.0f) {
		x = x - 360.0f;
	}

	float yf = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	int32_t yl = lroundf(yf);
	yl = constrain(yl, 0, 4095);
	return (uint32_t)yl;
}