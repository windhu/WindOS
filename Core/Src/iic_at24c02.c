#include "iic_at24c02.h"
#include "gpio.h"
#include "util.h"

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

uint8_t at24c02_read_one_byte(uint16_t addr) {
    uint8_t temp = 0;
    iic_start(); 

    iic_write_byte(0xA0);       /* write */
    
    iic_wait_ack();
    iic_write_byte(addr % 256); /* word address, at24c02 can store 256 bytes*/
    iic_wait_ack();
    
    iic_start();
    iic_write_byte(0xA1);      /* read */
    iic_wait_ack();
    temp = iic_read_byte(0);
    iic_stop();
    return temp;
}


void at24c02_write_one_byte(uint16_t addr, uint8_t data) {
    iic_start();

    iic_write_byte(0xA0);        /* write */
    
    iic_wait_ack();
    iic_write_byte(addr % 256);  /* word address, at24c02 can store 256 bytes*/
    iic_wait_ack();
    
    iic_write_byte(data);       /* write directly */
    iic_wait_ack();
    iic_stop();
    //delay_ms(10);             /* need to wait eeprom finish */
}
 

uint8_t at24c02_check(void) {
    uint8_t temp;
    uint16_t addr = 0;

    temp = at24c02_read_one_byte(addr); 
    if (temp == 0x55) {
        return 0;
    }
    else {
        at24c02_write_one_byte(addr, 0x55);
        temp = at24c02_read_one_byte(255);

        if (temp == 0x55)return 0;
    }

    return 1;
}


void at24c02_read(uint16_t addr, uint8_t *pbuf, uint16_t datalen) {
    while (datalen--) {
        *pbuf++ = at24c02_read_one_byte(addr++);
    }
}


void at24c02_write(uint16_t addr, uint8_t *pbuf, uint16_t datalen) {
    while (datalen--) {
        at24c02_write_one_byte(addr, *pbuf);
        addr++;
        pbuf++;
    }
}
