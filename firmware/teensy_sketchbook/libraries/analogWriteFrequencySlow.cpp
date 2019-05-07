/*
https://forum.pjrc.com/threads/25164-2Hz-analogWriteFrequency-on-Teensy-3-1?p=80081&viewfull=1#post80081
*/

void analogWriteFrequencySlow(uint8_t pin, float frequency)
{
	uint32_t prescale, mod, ftmClock, ftmClockSource;
	float minfreq;

		if (frequency < (float)(F_TIMER >> 7) / 65536.0f) {
				ftmClockSource = 2;
				ftmClock = 31250;
		} else {
				ftmClockSource = 1;
				ftmClock = F_TIMER;
		}

	for (prescale = 0; prescale < 7; prescale++) {
		minfreq = (float)(ftmClock >> prescale) / 65536.0f;
		if (frequency >= minfreq) break;
	}
	mod = (float)(ftmClock >> prescale) / frequency - 0.5f;
	if (mod > 65535) mod = 65535;
	if (pin == FTM1_CH0_PIN || pin == FTM1_CH1_PIN) {
		FTM1_SC = 0;
		FTM1_CNT = 0;
		FTM1_MOD = mod;
		FTM1_SC = FTM_SC_CLKS(ftmClockSource) | FTM_SC_PS(prescale);
	} else if (pin == FTM0_CH0_PIN || pin == FTM0_CH1_PIN
	  || pin == FTM0_CH2_PIN || pin == FTM0_CH3_PIN
	  || pin == FTM0_CH4_PIN || pin == FTM0_CH5_PIN
#ifdef FTM0_CH6_PIN
	  || pin == FTM0_CH6_PIN || pin == FTM0_CH7_PIN
#endif
	  ) {
		FTM0_SC = 0;
		FTM0_CNT = 0;
		FTM0_MOD = mod;
		FTM0_SC = FTM_SC_CLKS(ftmClockSource) | FTM_SC_PS(prescale);
	}
#ifdef FTM2_CH0_PIN
	  else if (pin == FTM2_CH0_PIN || pin == FTM2_CH1_PIN) {
		FTM2_SC = 0;
		FTM2_CNT = 0;
		FTM2_MOD = mod;
		FTM2_SC = FTM_SC_CLKS(ftmClockSource) | FTM_SC_PS(prescale);
	}
#endif
}