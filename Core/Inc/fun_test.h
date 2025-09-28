#ifndef __FUN_TEST_H
#define __FUN_TEST_H
#include "stdint.h"

#define EVENT_KEY0  (1<<0)
#define EVENT_KEY1  (1<<1)
#define EVENT_KEY2  (1<<2)
#define EVENT_KEY3  (1<<3)

void test_mode_init();
void test_mode_change();
void test_key_handler(uint8_t key_code);

void show_logo_on_lcd();
void draw_test_iic_window();
void test_iic_write_random();
void test_iic_read();

void test_spi_norflash();

uint8_t can_send_msg(uint32_t id, uint8_t *msg, uint8_t len);
uint8_t can_receive_msg(uint32_t id, uint8_t *buf);
#endif