#ifndef __FUN_TEST_H
#define __FUN_TEST_H
#include "stdint.h"

void test_mode_init();
void test_mode_change();
void test_key_handler(uint8_t key_code);

void show_logo_on_lcd();
void draw_test_iic_window();
void test_iic_write_random();
void test_iic_read();

void test_spi_norflash();

#endif