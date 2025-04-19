#include <stdio.h>
#include <stdlib.h>

#include "fun_test.h"
#include "lcd.h"
#include "iic_at24c02.h"
#include "charcode.h"

static uint16_t curr_pos_y = 0;
static uint16_t wr_pos_y = 0;
static uint16_t rd_pos_y = 0;

void show_logo_on_lcd() {
    uint16_t startx = (lcd_get_width() - 64*4)/2;
    uint16_t starty = curr_pos_y;
    lcd_draw_rectagle(0, 0, lcd_get_width(), 64, BLUE);
    lcd_draw_char(startx,starty, 64, RED, BLUE, char_64x64_feng);
    startx += 64;
    lcd_draw_char(startx,starty, 64, RED, BLUE, char_64x64_qing);
    startx += 64;
    lcd_draw_char(startx,starty, 64, RED, BLUE, char_64x64_yun);
    startx += 64;
    lcd_draw_char(startx,starty, 64, RED, BLUE, char_64x64_dan);
    curr_pos_y += 64;
}

void draw_test_iic_window() {
    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "IIC");
    wr_pos_y = curr_pos_y;
    curr_pos_y += 32;

    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "Write:");
    wr_pos_y = curr_pos_y;
    curr_pos_y += 32;

    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "Read :");
    rd_pos_y = curr_pos_y;
    curr_pos_y += 32;
}

void test_iic_write_random() {
    uint8_t buf[5];
    uint8_t data = rand()%256;
    sprintf((char *)buf, "%X", data);
    lcd_draw_str(200, wr_pos_y, 32, RED, WHITE, buf);
    at24c02_write_one_byte(0x0, data);
    printf("Write at24c02 addr 0 data:%x\r\n", data);
}

void test_iic_read() {
    uint8_t buf[5];
    uint8_t data = 0;
    data = at24c02_read_one_byte(0x0);
    sprintf((char *)buf, "%X", data);
    lcd_draw_str(200, rd_pos_y, 32, RED, WHITE, buf);
    printf("Read at24c02 addr 0 data:%x\r\n", data);
}
