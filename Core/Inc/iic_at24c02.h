#ifndef __IIC_AT24C02
#define __IIC_AT24C02
#include "stm32f407xx.h"

void at24c02_write_one_byte(uint16_t addr, uint8_t data);
uint8_t at24c02_read_one_byte(uint16_t addr);

#endif