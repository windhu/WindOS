#include "lvgl_port.h"
#include "cmsis_os.h"
#include "lcd.h"
#include "lvgl_gui.h"

void vApplicationTickHook(void) {
    lv_tick_inc(1);
}

/*Copy the rendered image to the screen.
 *It needs to be implemented by the user*/
void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf)
{
    uint16_t * pixels = (uint16_t *)px_buf;
    /*Show the rendered image on the display*/
    for(int16_t y = area->y1; y <= area->y2; y++) {
        for(int16_t x = area->x1; x <= area->x2; x++) {
            /*Set the pixel color at (x, y)*/
            lcd_draw_point(x, y, *pixels);
            pixels++;
        }
    }

    /*Indicate that the buffer is available.
     *If DMA were used, call in the DMA complete interrupt*/
    lv_display_flush_ready(disp);
}


static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

void lv_port_init(void)
{
    lv_init();

    lv_display_t * display = lv_display_create(320, 480);

    /*LVGL will render to this 1/10 screen sized buffer for 2 bytes/pixel*/
    static uint8_t __attribute__((section(".ccmram"))) buf[320 * 480 / 10 * 2];
    lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /*This callback will display the rendered image*/
    lv_display_set_flush_cb(display, my_flush_cb);
}

void lv_port_task() {
    static uint32_t time_till_next = 0;
    for(;;) {
        // draw LVGL GUI
        lvgl_gui_main();
        // LVGL task
        time_till_next = lv_timer_handler();
        if(time_till_next == LV_NO_TIMER_READY) {
            time_till_next = LV_DEF_REFR_PERIOD; /*handle LV_NO_TIMER_READY. Another option is to `sleep` for longer*/
        }
        osDelay(time_till_next);
    }
}