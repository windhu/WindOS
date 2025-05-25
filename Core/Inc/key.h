#ifndef __KEY_H
#define __KEY_H
#include "stm32f4xx_hal.h"

#define DEBOUNCE_TIME    3  /* 3*4=12ms */
typedef enum {
    KEY_CODE_0,
    KEY_CODE_1,
    KEY_CODE_2,
}KeyCode;

typedef enum
{
    KEY_IDLE,
    KEY_PRE_DOWN,
    KEY_DOWN,
    KEY_PRE_UP
} KeyState;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    KeyState state;
    uint32_t down_tick;
    uint32_t up_tick;
    void (*key_handle)(KeyState);
} Key;

void key_scan();
#endif