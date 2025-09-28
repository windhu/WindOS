#include "iic_at24c02.h"
#include "iic.h"
#include "cmsis_os.h"

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
        osDelay(5);
    }
}
