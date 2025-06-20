#include <stdio.h>
#include <stdlib.h>
#include "key.h"
#include "fun_test.h"
#include "lcd.h"
#include "iic_at24c02.h"
#include "charcode.h"
#include "spi_norflash.h"

void draw_test_iic_window();
void test_iic(uint8_t key_code);
void draw_test_spi1_window();
void test_spi1(uint8_t key_code);
void draw_test_can_window();
void test_can1(uint8_t key_code);

typedef void (*mode_draw_window) (void); 
typedef void (*mode_key_handler) (uint8_t key_code);
typedef struct {
    mode_draw_window show_window;
    mode_key_handler handler;
} TestMode;

TestMode modes[] = {
    {draw_test_iic_window, test_iic},
    {draw_test_spi1_window, test_spi1},
    {draw_test_can_window, test_can1},
};

static uint16_t curr_pos_y = 0;
static uint16_t wr_pos_y = 0;
static uint16_t rd_pos_y = 0;

static uint8_t curr_test_mode = 0;

void test_mode_init() {
    srand(HAL_GetTick());
    curr_test_mode = 0;
    norflash_init();
    modes[curr_test_mode].show_window();
}

void test_mode_change() {
    if (++curr_test_mode >= sizeof(modes)/sizeof(TestMode)) {
        curr_test_mode = 0;
    }
    modes[curr_test_mode].show_window();
}

void test_key_handler(uint8_t key_code) {
    modes[curr_test_mode].handler(key_code);
}

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
    lcd_clear(WHITE);
    curr_pos_y = 0;
    show_logo_on_lcd();
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

void test_iic(uint8_t key_code) {
    if (key_code == KEY_CODE_0) {
        test_iic_write_random();
    }
    else if (key_code == KEY_CODE_1) {
        test_iic_read();
    }
}

void test_iic_write_random() {
    uint8_t buf[5];
    uint8_t data = rand()%256;
    sprintf((char *)buf, "%02X", data);
    lcd_draw_str(200, wr_pos_y, 32, RED, WHITE, buf);
    at24c02_write_one_byte(0x0, data);
    printf("Write at24c02 addr 0 data:%x\r\n", data);
}

void test_iic_read() {
    uint8_t buf[5];
    uint8_t data = 0;
    data = at24c02_read_one_byte(0x0);
    sprintf((char *)buf, "%02X", data);
    lcd_draw_str(200, rd_pos_y, 32, RED, WHITE, buf);
    printf("Read at24c02 addr 0 data:%x\r\n", data);
}

void draw_test_spi1_window() {
    uint8_t buf[20];
    lcd_clear(WHITE);
    curr_pos_y = 0;
    show_logo_on_lcd();
    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "SPI");
    curr_pos_y += 32;
    curr_pos_y += 32;
    uint32_t devid = norflash_read_id();
    sprintf((char *)buf, "ID:%X", devid);
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, (char *)buf);
    curr_pos_y += 32;
    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "WR:");
    wr_pos_y = curr_pos_y;
    curr_pos_y += 32;
    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "RD:");
    rd_pos_y = curr_pos_y;
}

void test_spi1(uint8_t key_code) {
    uint8_t i = 0;
    uint16_t  y = 0;
    uint8_t buf[4] = {0};
    uint8_t out[10] = {0};
    uint8_t data = rand();
    uint16_t write_addr = 4095;
    if (key_code == KEY_CODE_0) {
        for (i = 0; i < 3; i ++) {
            buf[i] = data + i;
        }
        norflash_write(buf, write_addr, 3);
        y = wr_pos_y;
    }
    else if (key_code == KEY_CODE_1) {
        norflash_read(buf, write_addr, 3);
        y = rd_pos_y;
    }
    sprintf(out, "%02X%02X%02X", buf[0], buf[1], buf[2]);
    lcd_draw_str(32*3, y, 32, RED, WHITE, out);
}

void draw_test_can_window() {
    lcd_clear(WHITE);
    curr_pos_y = 0;
    show_logo_on_lcd();
    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "CAN");
    wr_pos_y = curr_pos_y;
    curr_pos_y += 32;

    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "WR:");
    wr_pos_y = curr_pos_y;
    curr_pos_y += 32;

    curr_pos_y += 32;
    lcd_draw_str(0, curr_pos_y, 32, RED, WHITE, "RD:");
    rd_pos_y = curr_pos_y;
    curr_pos_y += 32;
}
// uint8_t can_send_msg(uint32_t id, uint8_t *msg, uint8_t len);
// uint8_t can_receive_msg(uint32_t id, uint8_t *buf);
void test_can1(uint8_t key_code) {
    uint16_t y = 0;
    uint8_t buf[3] = {0};
    uint8_t out[10] = {0};
    static uint8_t feed = 0;
    if (key_code == KEY_CODE_0) {
        //write
        buf[0] = 0xaa;
        buf[1] = 0x55 + feed++;
        if (can_send_msg(0x1234, buf, 2)) {
            //fails
            printf("failed to write to CAN1\r\n");
        }
        y = wr_pos_y;
    }
    else if (key_code == KEY_CODE_1) {
        //read
        can_receive_msg(0x1234, buf);
        y = rd_pos_y;
    }
    sprintf(out, "%02X%02X", buf[0], buf[1]);
    lcd_draw_str(32*3, y, 32, RED, WHITE, out);
}