#ifndef LVGL_GUI_H
#define LVGL_GUI_H

enum LVGLCommand {
    ENTER_IIC_MODE = 1,
    ENTER_SPI_MODE,
    ENTER_CAN_MODE,
    ENTER_SD_MODE,
    ENTER_FATFS_MODE,
    ENTER_AUD_PLAYER,
    ENTER_LVGL_MODE,

    UPDATE_WRITE = 20,
    UPDATE_READ,
    SD_INFO,
    SD_DATA,
    FILE_CONTENT,
    SPK_VOLUMN,
    PLAYER_START,
    PLAYER_STOP,
};

void lvgl_gui_init();
void lvgl_gui_main();
#endif // LVGL_GUI_H