#include "sram.h"

void sram_write(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    for (uint32_t i = 0; i < datalen; i++) {
        *(volatile uint8_t *)(SRAM_BASE_ADDR + addr + i) = pbuf[i];
    }
}

void sram_read(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    for (uint32_t i = 0; i < datalen; i++) {
        pbuf[i] = *(volatile uint8_t *)(SRAM_BASE_ADDR + addr + i);
    }
}

void sram_write_one(uint32_t addr, uint8_t data)
{
    *(volatile uint8_t *)(SRAM_BASE_ADDR + addr) = data;
}

uint8_t sram_read_one(uint32_t addr)
{
    return *(volatile uint8_t *)(SRAM_BASE_ADDR + addr);
}
