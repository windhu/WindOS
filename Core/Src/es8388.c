
#include "es8388.h"
#include "iic.h"
#include "cmsis_os.h"
#include <stdio.h>

#define ES8388_ADDR     0x10

uint8_t es8388_write_reg(uint8_t reg, uint8_t val) {
    iic_start();
    iic_write_byte((ES8388_ADDR << 1)|0);
    if (iic_wait_ack()) {
        printf("es8388 write failed 1\r\n");
        return 1;
    }

    iic_write_byte(reg);
    if (iic_wait_ack()) {
        printf("es8388 write failed 2\r\n");
        return 2;
    }
    
    iic_write_byte(val & 0xFF);
    if (iic_wait_ack()) {
        printf("es8388 write failed 3\r\n");
        return 3;
    }

    iic_stop();
    return 0;
}

uint8_t es8388_read_reg(uint8_t reg) {
    uint8_t temp = 0;

    iic_start();
    iic_write_byte((ES8388_ADDR<<1)|0);
    if (iic_wait_ack()) {
        return 1;
    }
    iic_write_byte(reg);
    if (iic_wait_ack()) {
        return 2;
    }

    iic_start();
    iic_write_byte((ES8388_ADDR<<1)|1);
    if (iic_wait_ack()) {
        return 3;
    }
    temp = iic_read_byte(0);
    iic_stop();

    return temp;
}

// fmt :
// 0, I2S;
// 1, MSB
// 2, LSB
// 3, PCM/DSP
// len :
// 0, 24bit
// 1, 20bit 
// 2, 18bit 
// 3, 16bit 
// 4, 32bit 
void es8388_i2s_cfg(uint8_t fmt, uint8_t len) {
    fmt &= 0x03;
    len &= 0x07;
    es8388_write_reg(23, (fmt<<1) | (len<<3));
}

uint8_t es8388_headphone_vol_set(uint8_t vol) {
    if (vol > 33) {
        vol = 33;
    }
    es8388_write_reg(46, vol);
    es8388_write_reg(47, vol);
    return vol;
}

uint8_t es8388_speaker_vol_set(uint8_t vol) {
    if (vol > 33) {
        vol = 33;
    }
    es8388_write_reg(48, vol);
    es8388_write_reg(49, vol);
    return vol;
}

// Sidetone Enable strength
void es8388_3d_set(uint8_t depth)
{ 
    depth &= 0x7;
    es8388_write_reg(0x1D, depth << 2);
}

void es8388_adda_cfg(uint8_t dacen, uint8_t adcen)
{
    uint8_t tempreg = 0;
    
    tempreg |= ((!dacen) << 0);
    tempreg |= ((!adcen) << 1);
    tempreg |= ((!dacen) << 2);
    tempreg |= ((!adcen) << 3);
    es8388_write_reg(0x02, tempreg);
}

void es8388_output_cfg(uint8_t o1en, uint8_t o2en)
{
    uint8_t tempreg = 0;
    tempreg |= o1en * (3 << 4);
    tempreg |= o2en * (3 << 2);
    es8388_write_reg(0x04, tempreg);
}

void es8388_mic_gain(uint8_t gain)
{
    gain &= 0x0F;
    gain |= gain << 4;
    es8388_write_reg(0x09, gain);
}

void es8388_alc_ctrl(uint8_t sel, uint8_t maxgain, uint8_t mingain)
{
    uint8_t tempreg = 0;
    
    tempreg = sel << 6;
    tempreg |= (maxgain & 0x07) << 3;
    tempreg |= mingain & 0x07;
    es8388_write_reg(0x12, tempreg); 
}

void es8388_input_cfg(uint8_t in)
{
    es8388_write_reg(0x0A, (5 * in) << 4);
}

uint8_t es8388_init()
{
    es8388_write_reg(0, 0x80);
    es8388_write_reg(0, 0x00);
    osDelay(100);

    es8388_write_reg(0x01, 0x58);
    es8388_write_reg(0x01, 0x50);
    es8388_write_reg(0x02, 0xF3);
    es8388_write_reg(0x02, 0xF0);

    es8388_write_reg(0x03, 0x09);
    es8388_write_reg(0x00, 0x06); 
    es8388_write_reg(0x04, 0x00); 
    es8388_write_reg(0x08, 0x00);
    es8388_write_reg(0x2B, 0x80);

    es8388_write_reg(0x09, 0x88); 
    es8388_write_reg(0x0C, 0x4C);
    es8388_write_reg(0x0D, 0x02);
    es8388_write_reg(0x10, 0x00);
    es8388_write_reg(0x11, 0x00);

    es8388_write_reg(0x17, 0x18);
    es8388_write_reg(0x18, 0x02); 
    es8388_write_reg(0x1A, 0x00);
    es8388_write_reg(0x1B, 0x00);
    es8388_write_reg(0x27, 0xB8);
    es8388_write_reg(0x2A, 0xB8);
    osDelay(100);
    // Checking if write is ok
    // uint8_t rdata = 0;
    // rdata =  es8388_read_reg(0x2A);
    // printf("read es8388 %X\r\n", rdata);
    return 0;
}

void es8388_Play_Test_Tone(uint8_t tone_type)
{
    es8388_write_reg(0x1F, 0x00);
    osDelay(10);
    
    switch(tone_type)
    {
        case 0: // 1kHz
            if (es8388_write_reg(0x1F, 0x20)) {
                printf("failed to write\r\n");
            }
            break;
            
        case 1:
            es8388_write_reg(0x1F, 0x30);
            break;
            
        case 2:
            es8388_write_reg(0x1F, 0x40);
            break;
            
        case 3:
            es8388_write_reg(0x1F, 0x50);
            break;
            
        default:
            es8388_write_reg(0x1F, 0x20);
            break;
    }
}

void es8388_Stop_Test_Tone(void)
{
    es8388_write_reg(0x1F, 0x00);
}
