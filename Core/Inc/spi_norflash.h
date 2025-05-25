#ifndef __SPI_NM25Q128_H
#define __SPI_NM25Q128_H
#include <stdint.h>

uint32_t norflash_read_id();
void norflash_init();
void norflash_read(uint8_t *pbuf, uint32_t addr, uint16_t datalen);
void norflash_write(uint8_t *pbuf, uint32_t addr, uint16_t datalen);
void norflash_write_task();

#endif