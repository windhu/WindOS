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
#include "queue.h"
#include "sdio.h"
#include "ff.h"
#include "touch.h"
#include "sram.h"
#include <string.h>

extern MessageBufferHandle_t gui_message_handle;
extern uint32_t get_random_number(void);
extern TIM_HandleTypeDef htim14;

void test_iic(uint8_t key_code);
void test_spi1(uint8_t key_code);
void test_can1(uint8_t key_code);
void test_sd(uint8_t key_code);
void test_fatfs(uint8_t key_code);
void test_audio(uint8_t key_code);
void test_touch(uint8_t key_code);
void test_sram(void);
void test_pwm(uint8_t key_code);

static void get_4bytes_random(uint8_t *buf);
// buffer shared by all test for saving memory
static uint8_t test_tmp_buf[512];
static int8_t curr_test_mode = -1;

void SendModeSignal(uint8_t mode_signal) {
    uint8_t model_signal[1] = { mode_signal };
    size_t sbyte = xMessageBufferSend(gui_message_handle,(void *) model_signal, sizeof( model_signal ), 0);
    if (sbyte != sizeof(model_signal)) {
        printf("Send message buffer failed, sbyte:%d\r\n", sbyte);
    }
}

void enter_iic(int8_t mode_signal) {
    SendModeSignal(mode_signal);
}
void exit_iic(int8_t mode_signal) {
    printf("Exit IIC Test Mode\r\n");
}
void enter_spi1(int8_t mode_signal) {
    SendModeSignal(mode_signal);
}
void exit_spi1(int8_t mode_signal) {
    printf("Exit SPI1 Test Mode\r\n");
}
void enter_can1(int8_t mode_signal) {
    SendModeSignal(mode_signal);
}
void exit_can1(int8_t mode_signal) {
    printf("Exit CAN1 Test Mode\r\n");
}
void enter_sd(int8_t mode_signal) {
    SendModeSignal(mode_signal);
}
void exit_sd(int8_t mode_signal) {
    printf("Exit SD Test Mode\r\n");
}
void enter_fatfs(int8_t mode_signal) {
    SendModeSignal(mode_signal);
}
void exit_fatfs(int8_t mode_signal) {
    printf("Exit FATFS Test Mode\r\n");
}
void enter_audio(int8_t mode_signal) {
    SendModeSignal(mode_signal);
}
void exit_audio(int8_t mode_signal) {
    printf("Exit Audio Test Mode\r\n");
}
void enter_touch(int8_t mode_signal) {
    SendModeSignal(mode_signal);
}
void exit_touch(int8_t mode_signal) {
    printf("Exit Touch Test Mode\r\n");
}
void enter_pwm(int8_t mode_signal) {
    SendModeSignal(mode_signal);
}
void exit_pwm(int8_t mode_signal) {
    printf("Exit PWM Test Mode\r\n");
}

typedef void (*mode_handler) (int8_t mode_signal);
typedef void (*mode_key_handler) (uint8_t key_code);
typedef struct {
    int8_t enter_signal;
    int8_t exit_signal;
    mode_handler mode_enter;
    mode_handler mode_exit;
    mode_key_handler handler;
} TestMode;

TestMode modes[] = {
    {ENTER_IIC_MODE, -1, enter_iic, exit_iic, test_iic},     // 0
    {ENTER_SPI_MODE, -1, enter_spi1, exit_spi1, test_spi1},    // 1
    {ENTER_CAN_MODE, -1, enter_can1, exit_can1, test_can1},    // 2
    {ENTER_SD_MODE, -1, enter_sd, exit_sd, test_sd},       // 3
    {ENTER_FATFS_MODE, -1, enter_fatfs, exit_fatfs, test_fatfs}, // 4
    {ENTER_AUD_PLAYER, -1, enter_audio, exit_audio, test_audio}, // 5
    {ENTER_TOUCH_MODE, -1, enter_touch, exit_touch, test_touch}, // 6
    {ENTER_PWM_MODE, -1, enter_pwm, exit_pwm, test_pwm},     // 7
};

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
    test_sram();
}

#define MODE_COUNT (sizeof(modes)/sizeof(modes[0]))

void test_mode_set(int8_t new_mode) {
    if (new_mode < 0 || new_mode >= MODE_COUNT) return;
    taskENTER_CRITICAL();
    int8_t prev = curr_test_mode;
    if (prev == new_mode) {
        taskEXIT_CRITICAL();
        return;
    }
    if (prev >= 0 && modes[prev].mode_exit) {
        int8_t sig = (modes[prev].exit_signal >= 0) ? modes[prev].exit_signal : modes[prev].enter_signal;
        modes[prev].mode_exit(sig);
    }
    curr_test_mode = new_mode;
    if (modes[curr_test_mode].mode_enter) {
        modes[curr_test_mode].mode_enter(modes[curr_test_mode].enter_signal);
    }
    taskEXIT_CRITICAL();
}

void test_mode_next(void) {
    int8_t next;
    taskENTER_CRITICAL();
    if (curr_test_mode < 0) next = 0;
    else next = (curr_test_mode + 1) % MODE_COUNT;
    taskEXIT_CRITICAL();
    test_mode_set(next);
}

void test_mode_change(void) {
    test_mode_next();
}

int8_t test_get_current_mode(void) {
    return curr_test_mode;
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
            // f_read(&file, &test_tmp_buf[1], sizeof(test_tmp_buf), &bytes_read);
            f_read(&file, &test_tmp_buf[1], 24, &bytes_read);
            f_close(&file);
            test_tmp_buf[bytes_read + 1] = '\0';
            // printf("content: %s, size=%d\r\n", test_tmp_buf, bytes_read);
            sbyte = xMessageBufferSend(gui_message_handle,(void *) test_tmp_buf, bytes_read + 1 + 1, 0);
            if (sbyte != (bytes_read + 1 + 1)) {
                printf("Send message buffer for file content failed, sbyte:%d\r\n", sbyte);
            }
        }
        else {
            printf("Failed to open file\r\n");
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

void td_adjust_callback(uint8_t step, uint8_t result) {
    uint8_t buf[5] = {0};
    buf[0] = TP_ADJUST;
    buf[1] = step;
    buf[2] = result;
    xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
}

void test_touch(uint8_t key_code) {
    EventBits_t uxBits;
    if (key_code == KEY_CODE_0) {
        // uxBits = xEventGroupSetBits(keyEventGroup, EVENT_KEY0);
        tp_adjust(td_adjust_callback);
    }
    else if (key_code == KEY_CODE_1) {
        // uxBits = xEventGroupSetBits(keyEventGroup, EVENT_KEY1);
    }
    else if (key_code == KEY_CODE_3) {
        // uxBits = xEventGroupSetBits(keyEventGroup, EVENT_KEY3);
    }
}

void test_touch_tp_pressed(uint16_t x, uint16_t y) {
    uint8_t buf[5] = {0};
    TP_Point tpp;
    extern QueueHandle_t gui_tp_queue_handle;
    if (curr_test_mode == 6) {
        // Touch Adjust mode
        buf[0] = TP_PRESSED;
        *(uint16_t *)(&buf[1]) = x;
        *(uint16_t *)(&buf[3]) = y;
        xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
    }
    else if (curr_test_mode == 5){
        // Audio mode send it to LVGL
        tpp.x = x;
        tpp.y = y;
        xQueueSend(gui_tp_queue_handle, (void *)(&tpp), 0);
    }
}

uint32_t __attribute__((section(".sram"))) g_test_sram_buffer[250000];
void test_sram(void) {
    uint8_t write_buf[16];
    uint8_t write_buf_above[16];
    uint8_t read_buf[16];
    g_test_sram_buffer[0] = 0x12345678; // prevent optimization
    printf("SRAM base address value verify: 0x%08X\r\n", (unsigned int)g_test_sram_buffer[0]);
    printf("SRAM test start...\r\n");
    // write random data below 512KB
    for (int i = 0; i < sizeof(write_buf); i++) {
        write_buf[i] = rand() % 256;
        write_buf_above[i] = rand() % 256;
    }
    printf("SRAM write data below 512KB:");
    sram_write(write_buf, 0, sizeof(write_buf));
    for (int i = 0; i < sizeof(write_buf); i++) {
        printf(" %02X", write_buf[i]);
    }
    printf("\r\n");
    printf("SRAM write data above 512KB:");
    sram_write(write_buf_above, 512*1024, sizeof(write_buf_above));
    for (int i = 0; i < sizeof(write_buf_above); i++) {
        printf(" %02X", write_buf_above[i]);
    }
    printf("\r\n");

    // read back
    sram_read(read_buf, 0, sizeof(read_buf));
    printf("SRAM read data below 512KB:");
    for (int i = 0; i < sizeof(read_buf); i++) {
        printf(" %02X", read_buf[i]);
    }
    printf("\r\n");

    // compare
    if (memcmp(write_buf, read_buf, sizeof(write_buf)) == 0) {
        printf("SRAM test passed for below 512KB!\r\n");
    }
    else {
        printf("SRAM test failed for below 512KB!\r\n");
    }

    // read back above 512KB
    sram_read(read_buf, 512*1024, sizeof(write_buf_above));
    printf("SRAM read data above 512KB:");
    for (int i = 0; i < sizeof(read_buf); i++) {
        printf(" %02X", read_buf[i]);
    }
    printf("\r\n");

    // compare
    if (memcmp(write_buf_above, read_buf, sizeof(write_buf_above)) == 0) {
        printf("SRAM test passed for above 512KB!\r\n");
    }
    else {
        printf("SRAM test failed for above 512KB!\r\n");
    }
}

typedef struct {
    uint32_t freq_hz;
    uint32_t prescaler;
    uint32_t period;  // ARR
    const char* description;
} freq_config_t;
    
static freq_config_t freq_configs[] = {
    {1000, 168-1,   1000-1,  "1kHz"},
    {500,  336-1,   1000-1,  "500Hz"},
    {50,   3360-1,  1000-1,  "50Hz"},
    
    {20,   8400-1,  1000-1,  "20Hz"},   // PSC=8399 < 65535 ✓
    {5,    33600-1, 1000-1,  "5Hz"},    // PSC=33599 < 65535 ✓
    
    {2,    8400-1,   5000-1,  "2Hz"},    // PSC=8399, ARR=4999
    {1,    8400-1,   10000-1, "1Hz"}     // PSC=8399, ARR=9999
};

void test_pwm(uint8_t key_code) {
    static uint8_t curr_freq_index = 0;
    
    if (key_code == KEY_CODE_0) {
        if (curr_freq_index >= sizeof(freq_configs)/sizeof(freq_configs[0])) {
            curr_freq_index = 0;
        }
        if(freq_configs[curr_freq_index].prescaler > 65535 || freq_configs[curr_freq_index].period > 65535)
        {
            printf("Skip: %s (parameter)\r\n", freq_configs[curr_freq_index].description);
            curr_freq_index++;
            return;
        }
        HAL_TIM_PWM_Stop(&htim14, TIM_CHANNEL_1);

        __HAL_TIM_SET_COUNTER(&htim14, 0);
        __HAL_TIM_SET_PRESCALER(&htim14, freq_configs[curr_freq_index].prescaler);
        __HAL_TIM_SET_AUTORELOAD(&htim14, freq_configs[curr_freq_index].period);
        uint32_t half_duty = freq_configs[curr_freq_index].period / 2;
        __HAL_TIM_SET_COMPARE(&htim14, TIM_CHANNEL_1, half_duty);

        HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
        // Update GUI
        test_tmp_buf[0] = UPDATE_PWM;
        strcpy((char*)&test_tmp_buf[1], freq_configs[curr_freq_index].description);
        size_t sbyte = xMessageBufferSend(gui_message_handle,(void *) test_tmp_buf, strlen((char*)&test_tmp_buf[1]) + 1 + 1, 0);
        if (sbyte != (strlen((char*)&test_tmp_buf[1]) + 1 + 1)) {
            printf("Send message buffer for PWM update failed, sbyte:%d\r\n", sbyte);
        }
        // Next frequency
        curr_freq_index++;
    }
    else if (key_code == KEY_CODE_1) {
        HAL_TIM_PWM_Stop(&htim14, TIM_CHANNEL_1);
        // Update GUI
        test_tmp_buf[0] = UPDATE_PWM;
        strcpy((char*)&test_tmp_buf[1], "Stop");
        size_t sbyte = xMessageBufferSend(gui_message_handle,(void *) test_tmp_buf, strlen((char*)&test_tmp_buf[1]) + 1 + 1, 0);
        if (sbyte != (strlen((char*)&test_tmp_buf[1]) + 1 + 1)) {
            printf("Send message buffer for PWM update failed, sbyte:%d\r\n", sbyte);
        }
    }
}