#include "lcd.h"
#include "lvgl_gui.h"
#include "lvgl_port.h"
#include "lvgl.h"
#include "FreeRTOS.h"
#include "message_buffer.h"
#include <stdio.h>

#define LOGO_HEIGHT  80
#define MODE_TOP_MARGIN 20
#define MODE_LEFT_MARGIN 20
#define LABEL_HEIGHT 40
#define LABEL_WIDTH  150

#define MESSAGE_BUFFER_SIZE 100
#define MESSAGE_ITEM_SIZE   32

extern uint8_t spk_volumn_val; // TODO: global variable is not a good idea

MessageBufferHandle_t gui_message_handle;

static char messageItem[32];
static lv_obj_t *welcom_label = NULL;
static lv_obj_t *start_btn_label = NULL;
static lv_obj_t *start_btn = NULL;
static lv_obj_t *mode_label = NULL;
static lv_obj_t *write_label = NULL;
static lv_obj_t *read_label = NULL;
static lv_obj_t *write_data_label = NULL;
static lv_obj_t *read_data_label = NULL;
static lv_obj_t *sd_label = NULL;
static lv_obj_t *sd_data_label = NULL;
static lv_obj_t *arc_widge;
static void draw_logo();
static void draw_welcome_screen();
static void draw_enter_mode();
static void lv_example_arc_2(void);

void lvgl_gui_init() {
    lv_port_init();
    draw_logo();
    draw_welcome_screen();
    gui_message_handle = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
}

void lvgl_gui_main() {
    char str[128] = {0};
    size_t xReceivedBytes;
    xReceivedBytes = xMessageBufferReceive(gui_message_handle,(void *) messageItem, sizeof(messageItem), 0);
    if (xReceivedBytes <= 0) {
        return;
    }
    printf("receive bytes: %d\r\n", xReceivedBytes);
    // first is the command
    if (messageItem[0] == ENTER_IIC_MODE) {
        if (mode_label == NULL) {
            draw_enter_mode();
        }
        else {
            lv_label_set_text(write_label, "Write data:");
            lv_label_set_text(read_label, "Read data:");
            lv_obj_clear_flag(arc_widge, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(sd_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(sd_data_label, LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text(mode_label, "IIC Mode");
        lv_label_set_text(write_data_label, "");
        lv_label_set_text(read_data_label, "");
    }
    else if (messageItem[0] == ENTER_SPI_MODE) {
        lv_label_set_text(mode_label, "SPI Mode");
        lv_label_set_text(write_data_label, "");
        lv_label_set_text(read_data_label, "");
    }
    else if (messageItem[0] == ENTER_CAN_MODE) {
        lv_label_set_text(mode_label, "CAN Mode");
        lv_label_set_text(write_data_label, "");
        lv_label_set_text(read_data_label, "");
    }
    else if (messageItem[0] == ENTER_SD_MODE) {
        lv_label_set_text(mode_label, "SD Card Mode");
        lv_label_set_text(write_label, "Card Type:");
        lv_label_set_text(read_label, "Capacity:");
        lv_label_set_text(write_data_label, "");
        lv_label_set_text(read_data_label, "");
        lv_label_set_text(sd_label, "First 27 byte:");
        lv_label_set_text(sd_data_label, "");
        lv_obj_add_flag(arc_widge, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sd_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sd_data_label, LV_OBJ_FLAG_HIDDEN);
    }
    else if (messageItem[0] == ENTER_FATFS_MODE) {
        lv_label_set_text(mode_label, "FATFS Mode");
        lv_label_set_text(write_label, "Dir:");
        lv_label_set_text(read_label, "File:");
        lv_label_set_text(write_data_label, "test");
        lv_label_set_text(read_data_label, "test.txt");
        lv_label_set_text(sd_label, "File Content:");
        lv_label_set_text(sd_data_label, "");
    }
    else if (messageItem[0] == ENTER_AUD_PLAYER) {
        lv_label_set_text(mode_label, "Player Mode");
        lv_label_set_text(read_data_label, "your_16.wav");
        lv_label_set_text(sd_label, "Audio Volumn:");
        sprintf(str, "%d", spk_volumn_val);
        lv_label_set_text(sd_data_label, str);
    }
    else if (messageItem[0] == ENTER_LVGL_MODE) {
        lv_label_set_text(mode_label, "LVGL Mode");
        lv_label_set_text(write_data_label, "");
        lv_label_set_text(read_data_label, "");
        // draw LVGL GUI
        draw_welcome_screen();
    }
    else if (messageItem[0] == UPDATE_WRITE) {
        for (int i = 1; i < xReceivedBytes; i++) {
            sprintf(&str[(i-1)*3], "%02X ", (uint8_t)messageItem[i]);
        }
        lv_label_set_text(write_data_label, str);
    }
    else if (messageItem[0] == UPDATE_READ) {
        for (int i = 1; i < xReceivedBytes; i++) {
            sprintf(&str[(i-1)*3], "%02X ", (uint8_t)messageItem[i]);
        }
        lv_label_set_text(read_data_label, str);
    }
    else if (messageItem[0] == SD_INFO) {
        if (messageItem[1] == 1) {
            lv_label_set_text(write_data_label, &messageItem[2]);
        }
        else if (messageItem[1] == 2) {
            lv_label_set_text(read_data_label, &messageItem[2]);
        }
    }
    else if (messageItem[0] == SD_DATA) {
        for (int i = 1; i < xReceivedBytes; i++) {
            sprintf(&str[(i-1)*3], "%02X ", (uint8_t)messageItem[i]);
        }
        lv_label_set_text(sd_data_label, str);
    }
    else if (messageItem[0] == FILE_CONTENT) {
        lv_label_set_text(sd_data_label, &messageItem[1]);
    }
    else if (messageItem[0] == SPK_VOLUMN) {
        sprintf(str, "%d", messageItem[1]);
        lv_label_set_text(sd_data_label, str);
    }
    else if (messageItem[0] == PLAYER_START) {
        lv_label_set_text(start_btn_label, "Playing");
    }
    else if (messageItem[0] == PLAYER_STOP) {
        lv_label_set_text(start_btn_label, "Stop");
    }
}

static void draw_logo() {
    lv_obj_t * rect = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rect, lcd_get_width(), LOGO_HEIGHT);
    lv_obj_set_pos(rect, 0, 0);
    lv_obj_set_style_bg_color(rect, lv_color_hex(0x0066cc), LV_PART_MAIN);
    // lv_obj_set_style_radius(rect, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(rect, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(rect, lv_color_hex(0x003366), LV_PART_MAIN);

    lv_obj_t * label1 = lv_label_create(rect);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_label_set_recolor(label1, true);
    lv_obj_set_width(label1, 200);
    lv_label_set_text(label1, "#ff00ff FreeRTOS and LVGL are running on STM32# #ff0000 FENG QING YUN DAN#");
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
}

void draw_welcome_screen() {
    draw_logo();
    welcom_label = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(welcom_label, LV_LABEL_LONG_MODE_WRAP);
    lv_label_set_text(welcom_label, "FENG QING YUN DAN's Demo!");
    lv_obj_align(welcom_label, LV_ALIGN_TOP_MID, 0, LOGO_HEIGHT + 40);

    lv_example_arc_2();

    start_btn = lv_button_create(lv_screen_active());
    // lv_obj_add_event_cb(start_btn, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(start_btn, LV_ALIGN_CENTER, 0, 160);
    lv_obj_set_width(start_btn, 100);
    lv_obj_remove_flag(start_btn, LV_OBJ_FLAG_PRESS_LOCK);

    start_btn_label = lv_label_create(start_btn);
    lv_label_set_text(start_btn_label, "Start");
    lv_obj_center(start_btn_label);
}

static void draw_enter_mode()
{
    if (welcom_label)
    {
        lv_obj_delete(welcom_label);
        welcom_label = NULL;
    }
    mode_label = lv_label_create(lv_scr_act());
    lv_obj_align(mode_label, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN, LOGO_HEIGHT + MODE_TOP_MARGIN);
    lv_obj_set_height(mode_label, LABEL_HEIGHT);
    lv_obj_set_width(mode_label, LABEL_WIDTH);

    write_label = lv_label_create(lv_scr_act());
    lv_label_set_text(write_label, "Write data:");
    lv_obj_align(write_label, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT);
    lv_obj_set_height(write_label, LABEL_HEIGHT);
    lv_obj_set_width(write_label, LABEL_WIDTH);

    write_data_label = lv_label_create(lv_scr_act());
    lv_label_set_text(write_data_label, "");
    lv_obj_align(write_data_label, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN + LABEL_WIDTH, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT);
    lv_obj_set_height(write_data_label, LABEL_HEIGHT);
    lv_obj_set_width(write_data_label, LABEL_WIDTH);

    read_label = lv_label_create(lv_scr_act());
    lv_label_set_text(read_label, "Read data:");
    lv_obj_align(read_label, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT * 2);
    lv_obj_set_height(read_label, LABEL_HEIGHT);
    lv_obj_set_width(read_label, LABEL_WIDTH);

    read_data_label = lv_label_create(lv_scr_act());
    lv_label_set_text(read_data_label, "");
    lv_obj_align(read_data_label, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN + LABEL_WIDTH, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT * 2);
    lv_obj_set_height(read_data_label, LABEL_HEIGHT);
    lv_obj_set_width(read_data_label, LABEL_WIDTH);

    sd_label = lv_label_create(lv_scr_act());
    lv_label_set_text(sd_label, "");
    lv_obj_align(sd_label, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT * 3);
    lv_obj_set_height(sd_label, LABEL_HEIGHT);
    lv_obj_set_width(sd_label, LABEL_WIDTH);
    lv_obj_add_flag(sd_label, LV_OBJ_FLAG_HIDDEN);

    sd_data_label = lv_label_create(lv_scr_act());
    lv_label_set_text(sd_data_label, "");
    lv_obj_align(sd_data_label, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN + LABEL_WIDTH, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT * 3);
    lv_obj_set_height(sd_data_label, LABEL_HEIGHT*3);
    lv_obj_set_width(sd_data_label, LABEL_WIDTH);
    lv_obj_add_flag(sd_data_label, LV_OBJ_FLAG_HIDDEN);

    lv_label_set_text(start_btn_label, "Stop");
}

static void set_angle(void * obj, int32_t v)
{
    lv_arc_set_value((lv_obj_t *)obj, v);
}

/**
 * Create an arc which acts as a loader.
 */
static void lv_example_arc_2(void)
{
    /*Create an Arc*/
    arc_widge = lv_arc_create(lv_screen_active());
    lv_arc_set_rotation(arc_widge, 270);
    lv_arc_set_bg_angles(arc_widge, 0, 360);
    lv_obj_remove_style(arc_widge, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_remove_flag(arc_widge, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_align(arc_widge, LV_ALIGN_CENTER, 0, 30);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc_widge);
    lv_anim_set_exec_cb(&a, set_angle);
    lv_anim_set_duration(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);    /*Just for the demo*/
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_start(&a);
}
