
#include "stm32f4xx_hal.h"
#include "key.h"
#include "main.h"
#include "fun_test.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "touch.h"

void on_key0(KeyState state);
void on_key1(KeyState state);
void on_key2(KeyState state);
void on_key3(KeyState state);

Key keys[] = {
    {KEY0_GPIO_Port, KEY0_Pin, KEY_IDLE, 0, 0, &on_key0},
    {KEY1_GPIO_Port, KEY1_Pin, KEY_IDLE, 0, 0, &on_key1},
    {KEY2_GPIO_Port, KEY2_Pin, KEY_IDLE, 0, 0, &on_key2},
    {KEY3_GPIO_Port, KEY3_Pin, KEY_IDLE, 0, 0, &on_key3}
};

void on_key0(KeyState state) {
  if (state == KEY_IDLE) {
    HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
    test_key_handler(KEY_CODE_0);
  }
}
void on_key1(KeyState state) {
  if (state == KEY_IDLE) {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    test_key_handler(KEY_CODE_1);
  }
}
void on_key2(KeyState state) {
  if (state == KEY_IDLE) {
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    test_mode_change();
  }
}
void on_key3(KeyState state) {
  if (state == KEY_IDLE) {
    test_key_handler(KEY_CODE_3);
  }
}

void key_scan()
{
  uint8_t key_num = sizeof(keys) / sizeof(Key);
  uint16_t x = 0, y = 0;
  tp_init();

  for (;;)
  {
    for (uint8_t i = 0; i < key_num; i++)
    {
      Key *key = &keys[i];
      uint8_t level = HAL_GPIO_ReadPin(key->port, key->pin);
      uint32_t tick = xTaskGetTickCount(); //get_4ms_tick();
      switch (key->state)
      {
      case KEY_IDLE:
        if (level == 0)
        {
          key->state = KEY_PRE_DOWN;
          key->down_tick = tick;
        }
        break;
      case KEY_PRE_DOWN:
        if (level == 0)
        {
          if ((tick - key->down_tick) >= DEBOUNCE_TIME)
          {
            key->state = KEY_DOWN;
            key->key_handle(key->state);
          }
        }
        else
        {
          key->state = KEY_IDLE;
        }
        break;
      case KEY_DOWN:
        if (level == 1)
        {
          key->state = KEY_PRE_UP;
          key->up_tick = tick;
        }
        else
        {
          /* do nothing, may long press */
        }
        break;
      case KEY_PRE_UP:
        if (level == 1)
        {
          if ((tick - key->up_tick) >= DEBOUNCE_TIME)
          {
            key->state = KEY_IDLE;
            key->key_handle(key->state);
          }
        }
        else
        {
          key->state = KEY_DOWN;
        }
        break;
      }
    }
    // touch panel
    if (tp_scan(0, &x, &y) > 0) {
      if (x != 0xFFFF && y != 0xFFFF) {
        // printf("touch x=%d, y=%d\r\n", x, y);
        test_touch_tp_pressed(x, y);
      }
    }
    osDelay(4);
  }
}