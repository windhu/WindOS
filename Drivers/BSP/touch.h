#ifndef __TOUCH_H__
#include <stdint.h>

uint8_t tp_init();
uint16_t tp_scan(uint8_t mode, uint16_t *x, uint16_t *y);

typedef void (*tp_adjust_cb)(uint8_t step, uint8_t result);
void tp_adjust(tp_adjust_cb cb);

#endif