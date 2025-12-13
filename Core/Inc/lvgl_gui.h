#ifndef LVGL_GUI_H
#define LVGL_GUI_H

enum LVGLCommand {
    ENTER_IIC_MODE = 1,
    ENTER_SPI_MODE,
    ENTER_CAN_MODE,
    ENTER_SD_MODE,
    ENTER_FATFS_MODE,
    ENTER_AUD_PLAYER,
    ENTER_TOUCH_MODE,
    ENTER_PWM_MODE,
    ENTER_LVGL_MODE,

    UPDATE_WRITE = 20,
    UPDATE_READ,
    SD_INFO,
    SD_DATA,
    FILE_CONTENT,
    SPK_VOLUME,
    PLAYER_START,
    PLAYER_STOP,
    TP_ADJUST,
    TP_PRESSED,
    UPDATE_PWM,
};

typedef struct {
    uint16_t x;
    uint16_t y;
} TP_Point;

void lvgl_gui_init();
void lvgl_gui_main();
#endif // LVGL_GUI_H