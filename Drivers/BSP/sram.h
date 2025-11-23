#ifndef __SRAM_H__
#include <stdint.h>
// BANK 1 is the most frequently used because it connects to NOR Flash, SRAM, and LCDs (via an 8080 parallel interface). 
// It internally provides 4 independent chip-select signals (NE1, NE2, NE3, NE4), each corresponding to a sub-BANK.

// NE1 -> Sub-BANK 1 (Address: 0x6000 0000 - 0x63FF FFFF)
// NE2 -> Sub-BANK 2 (Address: 0x6400 0000 - 0x67FF FFFF)
// NE3 -> Sub-BANK 3 (Address: 0x6800 0000 - 0x6BFF FFFF)
// NE4 -> Sub-BANK 4 (Address: 0x6C00 0000 - 0x6FFF FFFF)
// This means you can connect up to 4 different devices to BANK 1 simultaneously (e.g., one NOR Flash, one SRAM, and 
//     two LCDs) and configure independent timings for each.
// My sram use sub-bank3
#define SRAM_BASE_ADDR 0x68000000
void sram_write(uint8_t *pbuf, uint32_t addr, uint32_t datalen);
void sram_read(uint8_t *pbuf, uint32_t addr, uint32_t datalen);
void sram_write_one(uint32_t addr, uint8_t data);
uint8_t sram_read_one(uint32_t addr);
#endif