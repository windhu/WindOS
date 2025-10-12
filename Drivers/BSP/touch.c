#include "touch.h"
#include "gpio.h"
#include "util.h"
#include "cmsis_os.h"
#include "lcd.h"
#include "iic_at24c02.h"
#include <stdio.h>
#include <stdlib.h>

#define T_PEN           HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin)             /* T_PEN */
#define T_MISO          HAL_GPIO_ReadPin(T_MISO_GPIO_Port, T_MISO_Pin)           /* T_MISO */

#define T_MOSI(x)     do{ x ? \
                          HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, GPIO_PIN_SET) : \
                          HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, GPIO_PIN_RESET); \
                      }while(0)     /* T_MOSI */

#define T_SCK(x)      do{ x ? \
                          HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET) : \
                          HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET); \
                      }while(0)     /* T_CLK */

#define T_CS(x)       do{ x ? \
                          HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET) : \
                          HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET); \
                      }while(0)     /* T_CS */

#define TP_PRES_DOWN    0x8000
#define TP_CATH_PRES    0x4000
#define CT_MAX_TOUCH    10

typedef struct
{
    uint16_t x[CT_MAX_TOUCH]; 
    uint16_t y[CT_MAX_TOUCH]; 
    uint16_t sta;
    float xfac;
    float yfac;
    short xc;
    short yc;

    uint8_t touchtype;
} TpDev;

static TpDev tp_dev = {0};

static void tp_write_byte(uint8_t data)
{
    uint8_t count = 0;

    for (count = 0; count < 8; count++)
    {
        if (data & 0x80) {
            T_MOSI(1);
        }
        else {
            T_MOSI(0);
        }

        data <<= 1;
        T_SCK(0);
        delay_us(1);
        T_SCK(1);
    }
}


static uint16_t tp_read_ad(uint8_t cmd)
{
    uint8_t count = 0;
    uint16_t num = 0;
    
    T_SCK(0);
    T_MOSI(0);
    T_CS(0);
    tp_write_byte(cmd);
    delay_us(6);
    T_SCK(0);
    delay_us(1);
    T_SCK(1);
    delay_us(1);
    T_SCK(0);

    for (count = 0; count < 16; count++)
    {
        num <<= 1;
        T_SCK(0);
        delay_us(1);
        T_SCK(1);

        if (T_MISO) num++;
    }

    num >>= 4;
    T_CS(1);
    return num;
}

#define TP_READ_TIMES   5
#define TP_LOST_VAL     1

static uint16_t tp_read_xoy(uint8_t cmd)
{
    uint16_t i, j;
    uint16_t buf[TP_READ_TIMES];
    uint16_t sum = 0;
    uint16_t temp;

    for (i = 0; i < TP_READ_TIMES; i++) {
        buf[i] = tp_read_ad(cmd);
    }

    for (i = 0; i < TP_READ_TIMES - 1; i++) {
        for (j = i + 1; j < TP_READ_TIMES; j++) {
            if (buf[i] > buf[j]) {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }

    sum = 0;

    for (i = TP_LOST_VAL; i < TP_READ_TIMES - TP_LOST_VAL; i++) {
        sum += buf[i];
    }

    temp = sum / (TP_READ_TIMES - 2 * TP_LOST_VAL);
    return temp;
}


static void tp_read_xy(uint16_t *x, uint16_t *y) {
    uint16_t xval, yval;

    if (tp_dev.touchtype & 0X01) {
        /* screen x,y direction is not same with touch */
        xval = tp_read_xoy(0X90);
        yval = tp_read_xoy(0XD0);
    } 
    else {
        xval = tp_read_xoy(0XD0);
        yval = tp_read_xoy(0X90);
    }

    *x = xval;
    *y = yval;
}


#define TP_ERR_RANGE    50 


static uint8_t tp_read_xy2(uint16_t *x, uint16_t *y)
{
    uint16_t x1, y1;
    uint16_t x2, y2;

    tp_read_xy(&x1, &y1);
    tp_read_xy(&x2, &y2);

    if (((x2 <= x1 && x1 < x2 + TP_ERR_RANGE) || (x1 <= x2 && x2 < x1 + TP_ERR_RANGE)) &&
            ((y2 <= y1 && y1 < y2 + TP_ERR_RANGE) || (y1 <= y2 && y2 < y1 + TP_ERR_RANGE)))
    {
        *x = (x1 + x2) / 2;
        *y = (y1 + y2) / 2;
        return 1;
    }

    return 0;
}

/* mode 0:normal   1:adjust */
uint16_t tp_scan(uint8_t mode, uint16_t *x, uint16_t *y)
{
    *x = *y = 0xFFFF;
    if (T_PEN == 0) {
        // detect pressed
        if (mode) {
            tp_read_xy2(&tp_dev.x[0], &tp_dev.y[0]);
        }
        else if (tp_read_xy2(&tp_dev.x[0], &tp_dev.y[0])) {
            tp_dev.x[0] = (signed short)(tp_dev.x[0] - tp_dev.xc) / tp_dev.xfac + lcd_get_width() / 2;

            tp_dev.y[0] = (signed short)(tp_dev.y[0] - tp_dev.yc) / tp_dev.yfac + lcd_get_heigth() / 2;
        }

        if ((tp_dev.sta & TP_PRES_DOWN) == 0) {
            tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES;
            tp_dev.x[CT_MAX_TOUCH - 1] = tp_dev.x[0]; 
            tp_dev.y[CT_MAX_TOUCH - 1] = tp_dev.y[0];
            *x = tp_dev.x[0];
            *y = tp_dev.y[0];
        }
    }
    else {
        if (tp_dev.sta & TP_PRES_DOWN) {
            tp_dev.sta &= ~TP_PRES_DOWN;
        }
        else {
            tp_dev.x[CT_MAX_TOUCH - 1] = 0;
            tp_dev.y[CT_MAX_TOUCH - 1] = 0;
            tp_dev.x[0] = 0xFFFF;
            tp_dev.y[0] = 0xFFFF;
        }
    }

    return tp_dev.sta & TP_PRES_DOWN;
}

#define TP_SAVE_ADDR_BASE   100

void tp_save_adjust_data(void) {
    uint8_t *p = (uint8_t *)&tp_dev.xfac;
    printf("save adjust x=%d, y=%d\r\n", (int32_t)(tp_dev.xfac*100), (int32_t)(tp_dev.yfac*100));
    at24c02_write(TP_SAVE_ADDR_BASE, p, 12);
    at24c02_write_one_byte(TP_SAVE_ADDR_BASE + 12, 0X0A);
}

uint8_t tp_get_adjust_data(void) {
    uint8_t *p = (uint8_t *)&tp_dev.xfac;
    uint8_t temp = 0;

    at24c02_read(TP_SAVE_ADDR_BASE, p, 12);
    temp = at24c02_read_one_byte(TP_SAVE_ADDR_BASE + 12);

    if (temp == 0X0A) {
        printf("read adjust x=%d, y=%d\r\n", (int32_t)(tp_dev.xfac*100), (int32_t)(tp_dev.yfac*100));
        return 1;
    }

    return 0;
}

void tp_adjust(tp_adjust_cb cb) {
    uint16_t pxy[5][2], tmpx, tmpy;
    uint8_t  cnt = 0;
    short s1, s2, s3, s4;
    double px, py; 
    uint16_t outtime = 0;
    cnt = 0;

    if(cb) {
        cb(0, 1);
    }
    tp_dev.sta = 0;

    while (1) {
        tp_scan(1, &tmpx, &tmpy);

        if ((tp_dev.sta & 0xc000) == TP_CATH_PRES) {
            outtime = 0;
            tp_dev.sta &= ~TP_CATH_PRES;

            pxy[cnt][0] = tp_dev.x[0];
            pxy[cnt][1] = tp_dev.y[0];
            cnt++;

            switch (cnt) {
                case 1:
                    if (cb) cb(1, 1);
                    break;

                case 2:
                    if (cb) cb(2, 1);
                    break;

                case 3:
                    if (cb) cb(3, 1);
                    break;

                case 4:
                    if (cb) cb(4, 1);
                    break;

                case 5:
                    s1 = pxy[1][0] - pxy[0][0]; 
                    s3 = pxy[3][0] - pxy[2][0]; 
                    s2 = pxy[3][1] - pxy[1][1]; 
                    s4 = pxy[2][1] - pxy[0][1]; 

                    px = (double)s1 / s3;
                    py = (double)s2 / s4;

                    if (px < 0) px = -px;
                    if (py < 0) py = -py; 

                    if (px < 0.95 || px > 1.05 || py < 0.95 || py > 1.05 || 
                            abs(s1) > 4095 || abs(s2) > 4095 || abs(s3) > 4095 || abs(s4) > 4095 ||
                            abs(s1) == 0 || abs(s2) == 0 || abs(s3) == 0 || abs(s4) == 0 
                       ) {
                        cnt = 0;
                        if (cb) cb(5, 0);
                        continue;
                    }

                    tp_dev.xfac = (float)(s1 + s3) / (2 * (lcd_get_width() - 40));
                    tp_dev.yfac = (float)(s2 + s4) / (2 * (lcd_get_heigth() - 40));

                    tp_dev.xc = pxy[4][0];
                    tp_dev.yc = pxy[4][1];
                    // done
                    if (cb) cb(5, 1);
                    osDelay(1000);
                    tp_save_adjust_data();
                    tp_dev.sta = 0;
                    return;
            }
        }

        osDelay(10);
        outtime++;

        if (outtime > 1000) {
            tp_get_adjust_data();
            break;
        }
    }

}

uint8_t tp_init() {
    tp_dev.touchtype = 0x00;
    if (tp_get_adjust_data()) {
        // adjust has been done
        return 0;
    }
    // need adjust
    return 1;
}
