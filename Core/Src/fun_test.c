#include <stdio.h>
#include <stdlib.h>
#include "key.h"
#include "fun_test.h"
#include "lcd.h"
#include "iic_at24c02.h"
#include "charcode.h"
#include "spi_norflash.h"
#include "lvgl_gui.h"
#include "FreeRTOS.h"
#include "message_buffer.h"

extern MessageBufferHandle_t gui_message_handle;
extern uint32_t get_random_number(void);

void test_iic(uint8_t key_code);
void test_spi1(uint8_t key_code);
void test_can1(uint8_t key_code);
static void get_4bytes_random(uint8_t *buf);

typedef void (*mode_draw_window) (void); 
typedef void (*mode_key_handler) (uint8_t key_code);
typedef struct {
    uint8_t mode_signal;
    mode_key_handler handler;
} TestMode;

TestMode modes[] = {
    {ENTER_IIC_MODE, test_iic},
    {ENTER_SPI_MODE, test_spi1},
    {ENTER_CAN_MODE, test_can1},
};

static int8_t curr_test_mode = -1;

void test_mode_init() {
    norflash_init();
}

void test_mode_change() {
    if (++curr_test_mode >= sizeof(modes)/sizeof(TestMode)) {
        curr_test_mode = 0;
    }
    uint8_t model_signal[1] = { modes[curr_test_mode].mode_signal };
    size_t sbyte = xMessageBufferSend(gui_message_handle,(void *) model_signal, sizeof( model_signal ), 0);
    if (sbyte != sizeof(model_signal)) {
        printf("Send message buffer failed, sbyte:%d\r\n", sbyte);
    }
}

void test_key_handler(uint8_t key_code) {
    if (curr_test_mode >= 0) {
        modes[curr_test_mode].handler(key_code);
    }
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
    buf[0] = UPDATE_WRITE;
    get_4bytes_random(&buf[1]);
    at24c02_write(0x05, &buf[1], 4);
    size_t sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof( buf ), 0);
    if (sbyte != sizeof(buf)) {
        printf("Send message buffer for iic write failed, sbyte:%d\r\n", sbyte);
    }
}

void test_iic_read() {
    uint8_t buf[5] = {0};
    buf[0] = UPDATE_READ;
    at24c02_read(0x05, &buf[1], 4);
    size_t sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof( buf ), 0);
    if (sbyte != sizeof(buf)) {
        printf("Send message buffer for iic read failed, sbyte:%d\r\n", sbyte);
    }
}

void test_spi1(uint8_t key_code) {
    uint8_t buf[5] = {0};
    uint16_t write_addr = 4095;
    if (key_code == KEY_CODE_0) {
        buf[0] = UPDATE_WRITE;
        get_4bytes_random(&buf[1]);
        norflash_write(&buf[1], write_addr, 4);
    }
    else if (key_code == KEY_CODE_1) {
        buf[0] = UPDATE_READ;
        norflash_read(&buf[1], write_addr, 4);
    }
    size_t sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof( buf ), 0);
    if (sbyte != sizeof(buf)) {
        printf("Send message buffer for SPI failed, sbyte:%d\r\n", sbyte);
    }
}

// uint8_t can_send_msg(uint32_t id, uint8_t *msg, uint8_t len);
// uint8_t can_receive_msg(uint32_t id, uint8_t *buf);
void test_can1(uint8_t key_code) {
    uint8_t buf[5] = {0};
    if (key_code == KEY_CODE_0) {
        //write
        buf[0] = UPDATE_WRITE;
        get_4bytes_random(&buf[1]);
        if (can_send_msg(0x1234, &buf[1], 4)) {
            //fails
            printf("failed to write to CAN1\r\n");
        }
    }
    else if (key_code == KEY_CODE_1) {
        //read
        buf[0] = UPDATE_READ;
        can_receive_msg(0x1234, &buf[1]);
    }
    size_t sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
    if (sbyte != sizeof(buf)) {
        printf("Send message buffer for CAN failed, sbyte:%d\r\n", sbyte);
    }
}

static void get_4bytes_random(uint8_t *buf) {
    uint32_t data = get_random_number();
    buf[0] = data & 0xff;
    buf[1] = (data >> 8) & 0xff;
    buf[2] = (data >> 16) & 0xff;
    buf[3] = (data >> 24) & 0xff;
}