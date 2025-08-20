#ifndef LVGL_GUI_H
#define LVGL_GUI_H

enum LVGLCommand {
    ENTER_IIC_MODE = 1,
    ENTER_SPI_MODE,
    ENTER_CAN_MODE,
    ENTER_LVGL_MODE,

    UPDATE_WRITE = 20,
    UPDATE_READ,
};

void lvgl_gui_init();
void lvgl_gui_main();
#endif // LVGL_GUI_H