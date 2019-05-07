#ifndef _BLEEDHEADTILT_H_
#define _BLEEDHEADTILT_H_

#include <stdint.h>

#if defined(__cplusplus) 
extern "C" {
#endif

void headtilt_init(void);
void headtilt_setTarget(int16_t x);
void headtilt_setPitchDeg(float x);

#if defined(__cplusplus) 
}
#endif

#endif
