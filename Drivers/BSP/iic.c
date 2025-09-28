#include "gpio.h"
#include "util.h"
#include "cmsis_os.h"

#define IIC_DELAY()  delay_us(2)

#define IIC_SCL(x)        do{ x ? \
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET) : \
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET); \
}while(0)

#define IIC_SDA(x)        do{ x ? \
    HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, GPIO_PIN_SET) : \
    HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, GPIO_PIN_RESET); \
}while(0)

#define IIC_READ_SDA     HAL_GPIO_ReadPin(IIC_SDA_GPIO_Port, IIC_SDA_Pin)

void iic_start() {
    IIC_SDA(1);
    IIC_SCL(1);
    IIC_DELAY();
    IIC_SDA(0); // high->low start
    IIC_DELAY();
    IIC_SCL(0);
    IIC_DELAY();
}

void iic_stop() {
    IIC_SDA(0);
    IIC_DELAY();
    IIC_SCL(1);
    IIC_DELAY();
    IIC_SDA(1); // low->high stop
    IIC_DELAY();
}

uint8_t iic_wait_ack() {
    uint8_t wait = 0;
    uint8_t rack = 0;
    IIC_SDA(1); // release SDA
    IIC_DELAY();
    IIC_SCL(1);
    IIC_DELAY();
    while(IIC_READ_SDA) {
        wait++;
        if (wait > 250) {
            iic_stop();
            rack = 1;
            break;
        }
    }
    IIC_SCL(0);
    IIC_DELAY();
    return rack;
}

void iic_ack() {
    IIC_SDA(0);
    IIC_DELAY();
    IIC_SCL(1);
    IIC_DELAY();
    IIC_SCL(0);
    IIC_DELAY();
    IIC_SDA(1);
    IIC_DELAY();
}

void iic_nack() {
    IIC_SDA(1);
    IIC_DELAY();
    IIC_SCL(1);
    IIC_DELAY();
    IIC_SCL(0);
    IIC_DELAY();
}

void iic_write_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        IIC_SDA((data&0x80)>>7);
        IIC_DELAY();
        IIC_SCL(1);
        IIC_DELAY();
        IIC_SCL(0);  //high->low
        data <<= 1;
    }
    IIC_SDA(1);
}

uint8_t iic_read_byte(uint8_t ack) {
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        IIC_SCL(1);
        IIC_DELAY();
        if (IIC_READ_SDA) {
            data++;
        }
        IIC_SCL(0);
        IIC_DELAY();
    }
    if (ack) {
        iic_ack();
    }
    else {
        iic_nack();
    }
    return data;
}