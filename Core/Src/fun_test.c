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
#include "event_groups.h"
#include "message_buffer.h"
#include "sdio.h"
#include "string.h"
#include "ff.h"

extern MessageBufferHandle_t gui_message_handle;
extern uint32_t get_random_number(void);

void test_iic(uint8_t key_code);
void test_spi1(uint8_t key_code);
void test_can1(uint8_t key_code);
void test_sd(uint8_t key_code);
void test_fatfs(uint8_t key_code);
void test_audio(uint8_t key_code);

static void get_4bytes_random(uint8_t *buf);
// buffer shared by all test for saving memory
static uint8_t test_tmp_buf[512];

typedef void (*mode_key_handler) (uint8_t key_code);
typedef struct {
    uint8_t mode_signal;
    mode_key_handler handler;
} TestMode;

TestMode modes[] = {
    {ENTER_IIC_MODE, test_iic},
    {ENTER_SPI_MODE, test_spi1},
    {ENTER_CAN_MODE, test_can1},
    {ENTER_SD_MODE, test_sd},
    {ENTER_FATFS_MODE, test_fatfs},
    {ENTER_AUD_PLAYER, test_audio},
};

static int8_t curr_test_mode = -1;

static void get_4bytes_random(uint8_t *buf) {
    uint32_t data = get_random_number();
    buf[0] = data & 0xff;
    buf[1] = (data >> 8) & 0xff;
    buf[2] = (data >> 16) & 0xff;
    buf[3] = (data >> 24) & 0xff;
}

static FATFS fs;
static uint8_t fs_mounted = 0;
EventGroupHandle_t keyEventGroup;
void test_mode_init() {
    if (fs_mounted == 0) {
        if (f_mount(&fs, "", 1)) {
            printf("failed to mount fs\r\n");
        }
        else {
            fs_mounted = 1;
        }
    }
    keyEventGroup = xEventGroupCreate();
    if (keyEventGroup == NULL) {
        printf("failed to create key event group\r\n");
    }
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

void test_sd(uint8_t key_code)
{   
    HAL_SD_CardStatusTypeDef card_status;
    HAL_SD_CardInfoTypeDef card_info;
    uint8_t buf[32 - 4]; // max message is 28 bytes
    size_t sbyte = 0;
    if (key_code == KEY_CODE_0) {
        // if (sd_read_card_status(&card_status)) {
        //     printf("failed to read sd card status\r\n");
        // }
        // else {
        //     printf("SpeedClass:%d\r\n", card_status.SpeedClass);
        // }

        if (sd_read_card_info(&card_info)) {
            printf("failed to read sd card information\r\n");
        }
        else {
            buf[0] = SD_INFO;
            buf[1] = 1; // card type
            switch (card_info.CardType)
            {
            case CARD_SDSC:
                // sprinf("SDSC (Standard Capacity)\r\n");
                strcpy(&buf[2], "SDSC");
                break;
            case CARD_SDHC_SDXC:
                // printf("SDHC/SDXC (High/Extended Capacity)\r\n");
                strcpy(&buf[2], "SDHC/SDXC");
                break;
            case CARD_SECURED:
                // printf("Secured Digital Card\r\n");
                strcpy(&buf[2], "Secured Digital Card");
                break;
            default:
                // printf("Unknown (%lu)\r\n", card_info.CardType);
                strcpy(&buf[2], "Unknown");
            }
            sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
            if (sbyte != sizeof(buf)) {
                printf("Send message buffer for SD Card information failed, sbyte:%d\r\n", sbyte);
            }
            // printf("Card Version: %lu\r\n", card_info.CardVersion);
            // printf("Class: %lu\r\n", card_info.Class);
            // printf("Relative Card Address (RCA): 0x%04lX\r\n", card_info.RelCardAdd);
            // printf("Block Size: %lu bytes\r\n", card_info.BlockSize);
            // printf("Logical Block Number: %lu\r\n", card_info.LogBlockNbr);
            // printf("Logical Block Size: %lu bytes\r\n", card_info.LogBlockSize);

            // capacity
            uint64_t capacity_bytes = (uint64_t)card_info.LogBlockNbr * card_info.LogBlockSize;
            uint32_t capacity_mb = capacity_bytes / (1024 * 1024);
            uint32_t capacity_gb = capacity_bytes / (1024 * 1024 * 1024);

            // printf("Total Capacity: %llu bytes\r\n", capacity_bytes);
            // printf("Total Capacity: %lu MB\r\n", capacity_mb);
            // printf("Total Capacity: %lu GB\r\n", capacity_gb);
            // printf("================================\r\n");
            buf[1] = 2; // capacity
            snprintf(&buf[2], 16, "%lu GB", capacity_gb);
            sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
            if (sbyte != sizeof(buf)) {
                printf("Send message buffer for SD Card information failed, sbyte:%d\r\n", sbyte);
            }
        }
    }
    else if (key_code == KEY_CODE_1) {
        buf[0] = SD_DATA;
        if (sd_read_disk(test_tmp_buf, 0, 1) != SD_TRANSFER_OK) {
            printf("Read data from sd card failed\r\n");
        }
        else {
            // for (int i = 0; i < 512; i++) {
            //     if (sd_sector_buf[i] != 0) {
            //         printf("%i=%d ", i, sd_sector_buf[i]);
            //     }
            // }
            // printf("\r\n");
            memcpy(&buf[1], test_tmp_buf, sizeof(buf) - 1);
            sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
            if (sbyte != sizeof(buf)) {
                printf("Send message buffer for SD Card data failed, sbyte:%d\r\n", sbyte);
            }
        }
    }
}

void test_fatfs(uint8_t key_code) {
    FIL file;
    FRESULT res;
    UINT bytes_read = 0;
    size_t sbyte = 0;
    if (key_code == KEY_CODE_0 || key_code == KEY_CODE_1) {
        // open file
        res = f_open(&file, "/test/test.txt", FA_READ);
        if (res == FR_OK) {
            test_tmp_buf[0] = FILE_CONTENT;
            f_read(&file, &test_tmp_buf[1], sizeof(test_tmp_buf), &bytes_read);
            f_close(&file);
            test_tmp_buf[bytes_read + 1] = '\0';
            // printf("content: %s\n", test_tmp_buf);
            sbyte = xMessageBufferSend(gui_message_handle,(void *) test_tmp_buf, bytes_read + 1 + 1, 0);
            if (sbyte != bytes_read + 1 + 1) {
                printf("Send message buffer for file content failed, sbyte:%d\r\n", sbyte);
            }
        }
    }
}

void test_audio(uint8_t key_code) {
    EventBits_t uxBits;
    if (key_code == KEY_CODE_0) {
        uxBits = xEventGroupSetBits(keyEventGroup, EVENT_KEY0);
    }
    else if (key_code == KEY_CODE_1) {
        uxBits = xEventGroupSetBits(keyEventGroup, EVENT_KEY1);
    }
    else if (key_code == KEY_CODE_3) {
        uxBits = xEventGroupSetBits(keyEventGroup, EVENT_KEY3);
    }
}