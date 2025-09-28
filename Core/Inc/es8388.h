#ifndef __ES8388_H__
#define __ES8388_H__
#include <stdint.h>
void es8388_i2s_cfg(uint8_t fmt, uint8_t len);
uint8_t es8388_headphone_vol_set(uint8_t vol);
uint8_t es8388_speaker_vol_set(uint8_t vol);
void es8388_3d_set(uint8_t depth);
void es8388_adda_cfg(uint8_t dacen, uint8_t adcen);
void es8388_output_cfg(uint8_t o1en, uint8_t o2en);
void es8388_mic_gain(uint8_t gain);
void es8388_alc_ctrl(uint8_t sel, uint8_t maxgain, uint8_t mingain);
void es8388_input_cfg(uint8_t in);
uint8_t es8388_init();

void es8388_Play_Test_Tone(uint8_t tone_type);
void es8388_Stop_Test_Tone();
#endif
