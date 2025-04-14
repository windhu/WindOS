#ifndef __LCD_H
#define __LCD_H
#include "stm32f4xx_hal.h"

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t id;
    uint8_t dir;
    uint16_t wramcmd;
    uint16_t setxcmd;
    uint16_t setycmd;
} LcdDev;

#define WHITE           0xFFFF      /* ��ɫ */
#define BLACK           0x0000      /* ��ɫ */
#define RED             0xF800      /* ��ɫ */
#define GREEN           0x07E0      /* ��ɫ */
#define BLUE            0x001F      /* ��ɫ */ 
#define MAGENTA         0xF81F      /* Ʒ��ɫ/�Ϻ�ɫ = BLUE + RED */
#define YELLOW          0xFFE0      /* ��ɫ = GREEN + RED */
#define CYAN            0x07FF      /* ��ɫ = GREEN + BLUE */  

void LcdDev_init();
void lcd_clear(uint16_t color);

void test_lcd();
#endif