#include <stdlib.h>

#include "lcd.h"
#include "main.h"
#include "stdio.h"

#include "charcode.c"

#define LCD_BASE_ADDRESS     0x60000000
#define FSMC_NE4_BASE_ADDRESS 0x6C000000
/* RS 2^6*2 */
#define FSMC_A6          (FSMC_NE4_BASE_ADDRESS+0x80)
#define FSMC_ADDR_CMD    ((uint32_t)(FSMC_NE4_BASE_ADDRESS))
#define FSMC_ADDR_DATA   ((uint32_t)(FSMC_A6))

LcdDev lcddev;

void lcd_wr_regno(volatile uint16_t cmd) {
    cmd = cmd;
    *(uint16_t *)(FSMC_ADDR_CMD) = cmd;
}

void lcd_wr_data(volatile uint16_t data) {
    data = data;
    *(uint16_t *)(FSMC_ADDR_DATA) = data;
}

uint16_t lcd_rd_data(void) {
    volatile uint16_t ram = 0;
    ram = *(uint16_t *)(FSMC_ADDR_DATA);
    return ram;
}

void lcd_write_reg(uint16_t regno, uint16_t data) {
    lcd_wr_regno(regno);
    lcd_wr_data(data);
}

void LcdDev_init() {
    lcd_wr_regno(0xD3);
    lcddev.id = lcd_rd_data();
    lcddev.id = lcd_rd_data();
    lcddev.id = lcd_rd_data();
    lcddev.id <<= 8;
    lcddev.id |= lcd_rd_data(); 
    printf("LCD ID:%x\r\n", lcddev.id); // id = 0x7796
    /* LCD reg init start */
    lcd_wr_regno(0x11);

    HAL_Delay(120); 

    lcd_wr_regno(0x36); /* Memory Data Access Control MY,MX~~ */
    lcd_wr_data(0x48);
    
    lcd_wr_regno(0x3A);
    lcd_wr_data(0x55);
    
    lcd_wr_regno(0xF0);
    lcd_wr_data(0xC3);
    
    lcd_wr_regno(0xF0);
    lcd_wr_data(0x96);

    lcd_wr_regno(0xB4);
    lcd_wr_data(0x01);
    
    lcd_wr_regno(0xB6); /* Display Function Control */
    lcd_wr_data(0x0A);
    lcd_wr_data(0xA2);

    lcd_wr_regno(0xB7);
    lcd_wr_data(0xC6);

    lcd_wr_regno(0xB9);
    lcd_wr_data(0x02);
    lcd_wr_data(0xE0);

    lcd_wr_regno(0xC0);
    lcd_wr_data(0x80);
    lcd_wr_data(0x16);

    lcd_wr_regno(0xC1);
    lcd_wr_data(0x19);

    lcd_wr_regno(0xC2);
    lcd_wr_data(0xA7);

    lcd_wr_regno(0xC5);
    lcd_wr_data(0x16);   

    lcd_wr_regno(0xE8);
    lcd_wr_data(0x40);
    lcd_wr_data(0x8A);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0x29);
    lcd_wr_data(0x19);
    lcd_wr_data(0xA5);
    lcd_wr_data(0x33);

    lcd_wr_regno(0xE0);
    lcd_wr_data(0xF0);
    lcd_wr_data(0x07);
    lcd_wr_data(0x0D);
    lcd_wr_data(0x04);
    lcd_wr_data(0x05);
    lcd_wr_data(0x14);
    lcd_wr_data(0x36);
    lcd_wr_data(0x54);
    lcd_wr_data(0x4C);
    lcd_wr_data(0x38);
    lcd_wr_data(0x13);
    lcd_wr_data(0x14);
    lcd_wr_data(0x2E);
    lcd_wr_data(0x34);

    lcd_wr_regno(0xE1);
    lcd_wr_data(0xF0);
    lcd_wr_data(0x10);
    lcd_wr_data(0x14);
    lcd_wr_data(0x0E);
    lcd_wr_data(0x0C);
    lcd_wr_data(0x08);
    lcd_wr_data(0x35);
    lcd_wr_data(0x44);
    lcd_wr_data(0x4C);
    lcd_wr_data(0x26);
    lcd_wr_data(0x10);
    lcd_wr_data(0x12);
    lcd_wr_data(0x2C);
    lcd_wr_data(0x32);

    lcd_wr_regno(0xF0);
    lcd_wr_data(0x3C);

    lcd_wr_regno(0xF0);
    lcd_wr_data(0x69);

    HAL_Delay(120);

    lcd_wr_regno(0x21);
    lcd_wr_regno(0x29);
    /* LCD reg init end */
    lcddev.dir = 0;
    lcddev.width = 320;
    lcddev.height = 480;
    lcddev.wramcmd = 0x2C;
    lcddev.setxcmd = 0x2A;
    lcddev.setycmd = 0x2B;

    uint16_t regval = 0;
    uint16_t dirreg = 0;
    dirreg = 0x36;

    regval |= (0 << 7) | (0 << 6) | (0 << 5);
    regval |= 0x08;
    lcd_write_reg(dirreg, regval);

    lcd_wr_regno(lcddev.setxcmd);
    lcd_wr_data(0);
    lcd_wr_data(0);
    lcd_wr_data((lcddev.width - 1) >> 8);
    lcd_wr_data((lcddev.width - 1) & 0xFF);
    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(0);
    lcd_wr_data(0);
    lcd_wr_data((lcddev.height - 1) >> 8);
    lcd_wr_data((lcddev.height - 1) & 0xFF);

    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
    lcd_clear(WHITE);
}

void lcd_set_cursor(uint16_t x, uint16_t y) {
    lcd_wr_regno(0x2A);
    lcd_wr_data(x >> 8);
    lcd_wr_data(x &0xFF);
    lcd_wr_regno(0x2B);
    lcd_wr_data(y >> 8);
    lcd_wr_data(y &0xFF);   
}

void lcd_write_ram_prepare(void) {
    lcd_wr_regno(0x2C);
}

void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color) {
    lcd_set_cursor(x, y);
    lcd_write_ram_prepare();
    lcd_wr_data(color);
}

uint16_t lcd_read_point(uint16_t x, uint16_t y) {
    uint16_t r = 0, g = 0, b = 0;
    lcd_set_cursor(x, y);
    lcd_wr_regno(0x2E);
    r = lcd_rd_data(); //dummy read
    r = lcd_rd_data(); //r g
    b = lcd_rd_data();
    g = r & 0xFF;
    return (((r >> 11) << 11) | ((g >> 2) << 5) | (b >> 11));
}

void lcd_clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;

    totalpoint *= lcddev.height; 
    lcd_set_cursor(0x00, 0x0000); 
    lcd_write_ram_prepare(); 

    for (index = 0; index < totalpoint; index++)
    {
        lcd_wr_data(color);
    }
}

uint16_t lcd_get_width() {
    return lcddev.width;
}

uint16_t lcd_get_heigth() {
    return lcddev.height;
}

void lcd_draw_char(uint16_t startx, uint16_t starty, uint16_t bmpsize, uint16_t fcolor, uint16_t bcolor, const uint8_t *char_bmp) {
    uint16_t w = (bmpsize + 7)/8;
    lcd_wr_regno(0x29);
    for (uint16_t y = 0; y < bmpsize; y++) {
        for (uint16_t x = 0; x < w; x++) {
            uint8_t p = char_bmp[y*w + x];
            for(uint16_t b = 0; b < 8; b++) {
                if (p&0x80) {
                    lcd_draw_point(x*8+b+startx, y+starty, fcolor);
                }
                else {
                    lcd_draw_point(x*8+b+startx, y+starty, bcolor);
                }
                p <<= 1;
            }
        }   
    }
}

void lcd_draw_str(int16_t startx, uint16_t starty, uint16_t bmpsize, uint16_t fcolor, uint16_t bcolor, uint8_t *str) {
    while(str != 0 && *str != '\0') {
        lcd_draw_char(startx, starty, bmpsize, fcolor, bcolor, ascii_32x32_table[*str]);
        startx += bmpsize;
        str++;
    }
}

void lcd_draw_rectagle(uint16_t startx, uint16_t starty, uint16_t w, uint16_t h, uint16_t color) {
    lcd_wr_regno(0x29);
    for (uint16_t x = startx; x < (startx + w); x++) {
        for (uint16_t y = starty; y < (starty + h); y++) {
            lcd_draw_point(x, y, color);
        }
    }
}

uint16_t random_rgb565(void) {
    // srand(HAL_GetTick()); 

    uint8_t r = rand() % 32;
    uint8_t g = rand() % 64;
    uint8_t b = rand() % 32;

    return ((r << 11) | (g << 5) | b);  //RGB565
}
