#ifndef __IIC_H__
#include <stdint.h>

void iic_start();
void iic_stop();
uint8_t iic_wait_ack();
void iic_ack();
void iic_nack();
void iic_write_byte(uint8_t data);
uint8_t iic_read_byte(uint8_t ack);

#endif