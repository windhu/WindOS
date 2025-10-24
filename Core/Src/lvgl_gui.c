#include "lcd.h"
#include "lvgl_gui.h"
#include "lvgl_port.h"
#include "lvgl.h"
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "queue.h"
#include "lv_port_indev.h"
#include "key.h"
#include <stdio.h>

#define LOGO_HEIGHT  80
#define MODE_TOP_MARGIN 20
#define MODE_LEFT_MARGIN 20
#define LABEL_HEIGHT 40
#define LABEL_WIDTH  150

#define MESSAGE_BUFFER_SIZE 100
#define MESSAGE_ITEM_SIZE   32

#define TOUCH_POINT_RADIUS  6

extern uint8_t spk_volume_val; // TODO: global variable is not a good idea

MessageBufferHandle_t gui_message_handle;
QueueHandle_t gui_tp_queue_handle;

static char messageItem[32];
static lv_obj_t *welcom_label = NULL;
static lv_obj_t *start_btn_label = NULL;
static lv_obj_t *start_btn = NULL;
static lv_obj_t *row_1 = NULL;
static lv_obj_t *row_2 = NULL;
static lv_obj_t *row_3 = NULL;
static lv_obj_t *row_2_data = NULL;
static lv_obj_t *row_3_data = NULL;
static lv_obj_t *row_4 = NULL;
static lv_obj_t *row_4_data = NULL;
static lv_obj_t *arc_widge;
static lv_obj_t *cir_tp_topleft = NULL;
static lv_obj_t *cir_tp_topright = NULL;
static lv_obj_t *cir_tp_center = NULL;
static lv_obj_t *cir_tp_bottomleft = NULL;
static lv_obj_t *cir_tp_bottomright = NULL;

static void draw_logo();
static void draw_welcome_screen();
static void draw_enter_mode();
static void lv_example_arc_2(void);
static lv_obj_t* draw_touch_point(int32_t x, int32_t y);
static void hide_touch_points();

void lvgl_gui_init() {
    lv_port_init();
    draw_logo();
    draw_welcome_screen();
    gui_message_handle = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
    gui_tp_queue_handle = xQueueCreate(10, sizeof(TP_Point));
    lv_port_indev_init();
}

void lvgl_gui_main() {
    char str[128] = {0};
    size_t xReceivedBytes;
    xReceivedBytes = xMessageBufferReceive(gui_message_handle,(void *) messageItem, sizeof(messageItem), 0);
    if (xReceivedBytes <= 0) {
        return;
    }
    // printf("receive bytes: %d\r\n", xReceivedBytes);
    // first is the command
    if (messageItem[0] == ENTER_IIC_MODE) {
        if (row_1 == NULL) {
            draw_enter_mode();
        }
        else {
            lv_label_set_text(row_2, "Write data:");
            lv_label_set_text(row_3, "Read data:");
            lv_obj_clear_flag(arc_widge, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(row_4, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(row_4_data, LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text(row_1, "IIC Mode");
        lv_label_set_text(row_2_data, "");
        lv_label_set_text(row_3_data, "");
    }
    else if (messageItem[0] == ENTER_SPI_MODE) {
        lv_label_set_text(row_1, "SPI Mode");
        lv_label_set_text(row_2_data, "");
        lv_label_set_text(row_3_data, "");
    }
    else if (messageItem[0] == ENTER_CAN_MODE) {
        lv_label_set_text(row_1, "CAN Mode");
        lv_label_set_text(row_2_data, "");
        lv_label_set_text(row_3_data, "");
    }
    else if (messageItem[0] == ENTER_SD_MODE) {
        lv_label_set_text(row_1, "SD Card Mode");
        lv_label_set_text(row_2, "Card Type:");
        lv_label_set_text(row_3, "Capacity:");
        lv_label_set_text(row_2_data, "");
        lv_label_set_text(row_3_data, "");
        lv_label_set_text(row_4, "First 27 byte:");
        lv_label_set_text(row_4_data, "");
        lv_obj_add_flag(arc_widge, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(row_4, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(row_4_data, LV_OBJ_FLAG_HIDDEN);
    }
    else if (messageItem[0] == ENTER_FATFS_MODE) {
        lv_label_set_text(row_1, "FATFS Mode");
        lv_label_set_text(row_2, "Dir:");
        lv_label_set_text(row_3, "File:");
        lv_label_set_text(row_2_data, "test");
        lv_label_set_text(row_3_data, "test.txt");
        lv_label_set_text(row_4, "File Content:");
        lv_label_set_text(row_4_data, "");
    }
    else if (messageItem[0] == ENTER_AUD_PLAYER) {
        lv_label_set_text(row_1, "Player Mode");
        lv_label_set_text(row_3_data, "your_16.wav");
        lv_label_set_text(row_4, "Audio Volume:");
        sprintf(str, "%d", spk_volume_val);
        lv_label_set_text(row_4_data, str);
    }
    else if (messageItem[0] == ENTER_TOUCH_MODE) {
        lv_label_set_text(row_1, "Touch Panel Adjust Mode");
        lv_label_set_text(row_2, "Touch Adjust:");
        lv_label_set_text(row_2_data, "Done");
        lv_label_set_text(row_3, "x, y:");
        lv_label_set_text(row_3_data, "");
        lv_label_set_text(row_4, "");
        lv_label_set_text(row_4_data, "");
    }
    else if (messageItem[0] == ENTER_LVGL_MODE) {
        lv_label_set_text(row_1, "LVGL Mode");
        lv_label_set_text(row_2_data, "");
        lv_label_set_text(row_3_data, "");
        // draw LVGL GUI
        draw_welcome_screen();
    }
    else if (messageItem[0] == UPDATE_WRITE) {
        for (int i = 1; i < xReceivedBytes; i++) {
            sprintf(&str[(i-1)*3], "%02X ", (uint8_t)messageItem[i]);
        }
        lv_label_set_text(row_2_data, str);
    }
    else if (messageItem[0] == UPDATE_READ) {
        for (int i = 1; i < xReceivedBytes; i++) {
            sprintf(&str[(i-1)*3], "%02X ", (uint8_t)messageItem[i]);
        }
        lv_label_set_text(row_3_data, str);
    }
    else if (messageItem[0] == SD_INFO) {
        if (messageItem[1] == 1) {
            lv_label_set_text(row_2_data, &messageItem[2]);
        }
        else if (messageItem[1] == 2) {
            lv_label_set_text(row_3_data, &messageItem[2]);
        }
    }
    else if (messageItem[0] == SD_DATA) {
        for (int i = 1; i < xReceivedBytes; i++) {
            sprintf(&str[(i-1)*3], "%02X ", (uint8_t)messageItem[i]);
        }
        lv_label_set_text(row_4_data, str);
    }
    else if (messageItem[0] == FILE_CONTENT) {
        printf(&messageItem[1]);
        lv_label_set_text(row_4_data, &messageItem[1]);
    }
    else if (messageItem[0] == SPK_VOLUME) {
        sprintf(str, "%d", messageItem[1]);
        lv_label_set_text(row_4_data, str);
    }
    else if (messageItem[0] == PLAYER_START) {
        lv_label_set_text(start_btn_label, "Playing");
    }
    else if (messageItem[0] == PLAYER_STOP) {
        lv_label_set_text(start_btn_label, "Stop");
    }
    else if (messageItem[0] == TP_ADJUST) {
        if (messageItem[1] == 0) {
            lv_label_set_text(row_2_data, "top left");
            lv_label_set_text(row_3_data, "");
            lv_obj_clear_flag(cir_tp_topleft, LV_OBJ_FLAG_HIDDEN);
        }
        else if (messageItem[1] == 1) {
            lv_label_set_text(row_2_data, "top right");
            lv_obj_clear_flag(cir_tp_topright, LV_OBJ_FLAG_HIDDEN);
        }
        else if (messageItem[1] == 2) {
            lv_label_set_text(row_2_data, "bottom left");
            lv_obj_clear_flag(cir_tp_bottomleft, LV_OBJ_FLAG_HIDDEN);
        }
        else if (messageItem[1] == 3) {
            lv_label_set_text(row_2_data, "bottom right");
            lv_obj_clear_flag(cir_tp_bottomright, LV_OBJ_FLAG_HIDDEN);
        }
        else if (messageItem[1] == 4) {
            lv_label_set_text(row_2_data, "center");
            lv_obj_clear_flag(cir_tp_center, LV_OBJ_FLAG_HIDDEN);
        }
        else if (messageItem[1] == 5) {
            hide_touch_points();
            if (messageItem[2] == 1) {
                lv_label_set_text(row_2_data, "Done");
            }
            else {
                lv_label_set_text(row_2_data, "Retry");
            }
        }
    }
    else if (messageItem[0] == TP_PRESSED) {
        sprintf(str, "%d, %d", *((uint16_t*)(&messageItem[1])), *((uint16_t*)(&messageItem[3])));
        lv_label_set_text(row_3_data, str);
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

extern void test_audio(uint8_t key_code);
static void event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        // LV_LOG_USER("Clicked");
        test_audio(KEY_CODE_0);
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

LV_FONT_DECLARE(lv_font_songti_24)

void draw_welcome_screen() {
    draw_logo();
    welcom_label = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(welcom_label, LV_LABEL_LONG_MODE_WRAP);
    // lv_label_set_text(welcom_label, "FENG QING YUN DAN's Demo!");
    lv_obj_set_style_text_font(welcom_label, &lv_font_songti_24, 0);
    lv_label_set_text(welcom_label, "欢迎到 峰轻云淡 公众号");
    lv_obj_align(welcom_label, LV_ALIGN_TOP_MID, 0, LOGO_HEIGHT + 40);

    lv_example_arc_2();

    start_btn = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(start_btn, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(start_btn, LV_ALIGN_CENTER, 0, 160);
    lv_obj_set_width(start_btn, 100);
    // lv_obj_remove_flag(start_btn, LV_OBJ_FLAG_PRESS_LOCK);

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
    row_1 = lv_label_create(lv_scr_act());
    lv_obj_align(row_1, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN, LOGO_HEIGHT + MODE_TOP_MARGIN);
    lv_obj_set_height(row_1, LABEL_HEIGHT);
    lv_obj_set_width(row_1, LABEL_WIDTH*2);

    row_2 = lv_label_create(lv_scr_act());
    lv_label_set_text(row_2, "Write data:");
    lv_obj_align(row_2, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT);
    lv_obj_set_height(row_2, LABEL_HEIGHT);
    lv_obj_set_width(row_2, LABEL_WIDTH);

    row_2_data = lv_label_create(lv_scr_act());
    lv_label_set_text(row_2_data, "");
    lv_obj_align(row_2_data, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN + LABEL_WIDTH, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT);
    lv_obj_set_height(row_2_data, LABEL_HEIGHT);
    lv_obj_set_width(row_2_data, LABEL_WIDTH);

    row_3 = lv_label_create(lv_scr_act());
    lv_label_set_text(row_3, "Read data:");
    lv_obj_align(row_3, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT * 2);
    lv_obj_set_height(row_3, LABEL_HEIGHT);
    lv_obj_set_width(row_3, LABEL_WIDTH);

    row_3_data = lv_label_create(lv_scr_act());
    lv_label_set_text(row_3_data, "");
    lv_obj_align(row_3_data, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN + LABEL_WIDTH, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT * 2);
    lv_obj_set_height(row_3_data, LABEL_HEIGHT);
    lv_obj_set_width(row_3_data, LABEL_WIDTH);

    row_4 = lv_label_create(lv_scr_act());
    lv_label_set_text(row_4, "");
    lv_obj_align(row_4, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT * 3);
    lv_obj_set_height(row_4, LABEL_HEIGHT);
    lv_obj_set_width(row_4, LABEL_WIDTH);
    lv_obj_add_flag(row_4, LV_OBJ_FLAG_HIDDEN);

    row_4_data = lv_label_create(lv_scr_act());
    lv_label_set_text(row_4_data, "");
    lv_obj_align(row_4_data, LV_ALIGN_TOP_LEFT, MODE_LEFT_MARGIN + LABEL_WIDTH, LOGO_HEIGHT + MODE_TOP_MARGIN + LABEL_HEIGHT * 3);
    lv_obj_set_height(row_4_data, LABEL_HEIGHT*3);
    lv_obj_set_width(row_4_data, LABEL_WIDTH);
    lv_obj_add_flag(row_4_data, LV_OBJ_FLAG_HIDDEN);

    lv_label_set_text(start_btn_label, "Stop");

    cir_tp_topleft = draw_touch_point(20, 20);
    cir_tp_topright = draw_touch_point(lcd_get_width() - 20, 20);
    cir_tp_center = draw_touch_point(lcd_get_width()/2, lcd_get_heigth()/2);
    cir_tp_bottomleft = draw_touch_point(20, lcd_get_heigth() - 20);
    cir_tp_bottomright = draw_touch_point(lcd_get_width() - 20, lcd_get_heigth() - 20);
    hide_touch_points();
}

static void hide_touch_points() {
    lv_obj_add_flag(cir_tp_topleft, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cir_tp_topright, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cir_tp_center, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cir_tp_bottomleft, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cir_tp_bottomright, LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t* draw_touch_point(int32_t x, int32_t y) {
    lv_obj_t* circle = lv_obj_create(lv_scr_act());
    lv_obj_set_size(circle, TOUCH_POINT_RADIUS*2, TOUCH_POINT_RADIUS*2);
    lv_obj_align(circle, LV_ALIGN_TOP_LEFT, x - TOUCH_POINT_RADIUS, y - TOUCH_POINT_RADIUS);
    lv_obj_set_style_radius(circle, TOUCH_POINT_RADIUS, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_50, 0);
    lv_obj_set_style_border_width(circle, 2, 0);
    lv_obj_set_style_border_color(circle, lv_color_hex(0x2980b9), 0);
    return circle;
}

static void set_angle(void * obj, int32_t v) {
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
