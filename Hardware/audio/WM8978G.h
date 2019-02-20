
#ifndef _WM8978G_H
#define _WM8978G_H

#include "public.h"


uint8_t Audio_AmpInit(void);
uint8_t Audio_AmpSetMute(bool mute);
uint8_t Audio_AmpSetChannel(uint8_t ch);
uint8_t Audio_AmpSetVolume(uint8_t vol);
uint8_t Audio_AmpSetEQ(uint8_t eq);
uint8_t Audio_AmpI2SConfig(uint8_t protocal, uint32_t sampleRate, uint8_t dataSize);
uint8_t Audio_AmpPlayStart(void);
uint8_t Audio_AmpPlayStop(void);
uint8_t Audio_DecoderStart(void);
uint8_t Audio_DecoderStop(void);
#endif

