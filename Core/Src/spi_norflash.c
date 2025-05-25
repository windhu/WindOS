// NOR Flash 
#include <stdio.h>
#include <memory.h>
#include "spi.h"
#include "spi_norflash.h"

#define FLASH_TIMEOUT     100
#define FLASH_RDID        0x9F
#define FLASH_ReadData    0x03
#define FLASH_WriteEnable 0x06
#define FLASH_PageProgram 0x02
#define FLASH_ReadStatusReg1        0x05 
#define FLASH_ReadStatusReg2        0x35 
#define FLASH_ReadStatusReg3        0x15

#define FLASH_WriteStatusReg1       0x01 
#define FLASH_WriteStatusReg2       0x31 
#define FLASH_WriteStatusReg3       0x11 

#define FLASH_SectorErase 0x20
#define SECTOR_LENGTH 4096

#define NORFLASH_CS(x)  do { \
            if (x) \
                HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET); \
            else \
                HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); \
        } while(0);

uint8_t nf_write_buf[SECTOR_LENGTH];
uint8_t nf_sector_buf[SECTOR_LENGTH];

enum {
    NF_STATE_IDEL,
    NF_STATE_WRITE,
    NF_STATE_SECTOR_ERASE,
    NF_STATE_WRITE_AFTER_ERASE,
    NF_STATE_WRITE_NOCHECK
} NORFALSH_STATE;

struct WriteContext {
    uint8_t state;
    uint32_t addr;
    uint32_t len;
    uint32_t offset;
    uint32_t sector;
    uint32_t sector_offset;
    uint32_t sector_remain;
} write_context;

void norflash_init() {
    write_context.state = NF_STATE_IDEL;
}

uint8_t spi1_read_write_byte(uint8_t txdata) {
    uint8_t rxdata = 0;
    uint32_t result = 0;
    result = HAL_SPI_TransmitReceive(&hspi1, &txdata, &rxdata, 1, FLASH_TIMEOUT);
    return rxdata;
}


uint32_t norflash_read_id() {
    uint32_t id = 0;
    NORFLASH_CS(0);
    spi1_read_write_byte(FLASH_RDID);
    id = spi1_read_write_byte(0xFF) << 16; //manufacturer id
    id |= spi1_read_write_byte(0xFF);
    NORFLASH_CS(1);
    return id;
}

void norflash_send_address(uint32_t address) {
    spi1_read_write_byte((uint8_t)((address)>>16));
    spi1_read_write_byte((uint8_t)((address)>>8));
    spi1_read_write_byte((uint8_t)address);
}

uint8_t norflash_read_sr(uint8_t regno) {
    uint8_t byte = 0, command = 0;

    switch (regno) {
        case 1:
            command = FLASH_ReadStatusReg1;
            break;

        case 2:
            command = FLASH_ReadStatusReg2;
            break;

        case 3:
            command = FLASH_ReadStatusReg3;
            break;

        default:
            command = FLASH_ReadStatusReg1;
            break;
    }

    NORFLASH_CS(0);
    spi1_read_write_byte(command);
    byte = spi1_read_write_byte(0Xff);
    NORFLASH_CS(1);
    
    return byte;
}

void norflash_write_sr(uint8_t regno, uint8_t sr) {
    uint8_t command = 0;
    switch (regno) {
        case 1:
            command = FLASH_WriteStatusReg1;
            break;
        case 2:
            command = FLASH_WriteStatusReg2;
            break;
        case 3:
            command = FLASH_WriteStatusReg3;
            break;
        default:
            command = FLASH_WriteStatusReg1;
            break;
    }

    NORFLASH_CS(0);
    spi1_read_write_byte(command); 
    spi1_read_write_byte(sr);
    NORFLASH_CS(1);
}

void norflash_read(uint8_t *pbuf, uint32_t addr, uint16_t datalen) {
    uint16_t i = 0;
    NORFLASH_CS(0);
    spi1_read_write_byte(FLASH_ReadData); 
    norflash_send_address(addr); 
    
    for (i = 0; i < datalen; i++) {
        pbuf[i] = spi1_read_write_byte(0XFF);
    }
    NORFLASH_CS(1);
}

void norflash_write_enable(void)
{
    NORFLASH_CS(0);
    spi1_read_write_byte(FLASH_WriteEnable);   /* 发送写使能 */
    NORFLASH_CS(1);
}

void norflash_wait_busy()
{
    while ((norflash_read_sr(1) & 0x01) == 0x01);
}

uint8_t norflash_busy() {
    if (norflash_read_sr(1) & 0x01) {
        return 1;
    }
    return 0;
}

void norflash_erase_sector(uint16_t sector) {
    uint32_t addr = sector * 4096;
    norflash_write_enable();
    norflash_wait_busy();
    NORFLASH_CS(0);
    spi1_read_write_byte(FLASH_SectorErase);
    norflash_send_address(addr);
    NORFLASH_CS(1);
}

void norflash_write_page(uint8_t *pbuf, uint32_t addr, uint16_t datalen) {
    uint16_t i;

    norflash_write_enable(); 

    NORFLASH_CS(0);
    spi1_read_write_byte(FLASH_PageProgram); 
    norflash_send_address(addr);

    for (i = 0; i < datalen; i++) {
        spi1_read_write_byte(pbuf[i]); 
    }
    
    NORFLASH_CS(1);
    norflash_wait_busy();
}

// one page = 256 bytes
void norflash_write_nocheck(uint8_t *pbuf, uint32_t addr, uint16_t datalen) {
    uint16_t page_remain = 256 - addr%256;
    if (datalen <= page_remain) {
        page_remain = datalen;
    }
    while(1) {
        norflash_write_page(pbuf, addr, page_remain);
        if (datalen == page_remain) {
            break;
        }
        else {
            pbuf += page_remain;
            addr +=  page_remain;
            datalen -= page_remain;
            if (datalen > 256) {
                page_remain = 256;
            }
            else{
                page_remain = datalen;
            }
        }
    }
}

void norflash_write(uint8_t *pbuf, uint32_t addr, uint16_t datalen) {
    if (datalen > SECTOR_LENGTH) {
        // only support sector length write one time
        return;
    }
    write_context.addr = addr;
    write_context.len = datalen;
    write_context.offset = 0;
    memcpy(nf_write_buf, pbuf, datalen);
    write_context.sector = write_context.addr/SECTOR_LENGTH;
    write_context.sector_offset = write_context.addr%SECTOR_LENGTH;
    write_context.sector_remain = SECTOR_LENGTH - write_context.sector_offset;
    if (write_context.len <= write_context.sector_remain) {
        write_context.sector_remain = write_context.len;
    }
    write_context.state = NF_STATE_WRITE;
}

void norflash_write_task() {
    printf("in norflash_write\r\n");
    int i = 0;
    if (write_context.state == NF_STATE_IDEL) {
        return;
    }

    if (write_context.state == NF_STATE_WRITE) {
        norflash_read(nf_sector_buf, write_context.sector*SECTOR_LENGTH, SECTOR_LENGTH);
        for (i = 0; i < write_context.sector_remain; i++) {
            if (nf_sector_buf[i + write_context.sector_offset] != 0xFF) {
                break;
            }
        }
        if (i < write_context.sector_remain) {
            for (i = 0; i < write_context.sector_remain; i++) {
                nf_sector_buf[i + write_context.sector_offset] = nf_write_buf[i + write_context.offset];
            }
            norflash_erase_sector(write_context.sector);
            write_context.state = NF_STATE_SECTOR_ERASE;
        }
        else {
            write_context.state = NF_STATE_WRITE_NOCHECK;
        }
        return;
    }
    else if (write_context.state == NF_STATE_SECTOR_ERASE){
        if (norflash_busy()) {
            return;
        }
        write_context.state = NF_STATE_WRITE_AFTER_ERASE;
        return;
    }
    else if (write_context.state == NF_STATE_WRITE_NOCHECK || write_context.state == NF_STATE_WRITE_AFTER_ERASE) {
        if (write_context.state == NF_STATE_WRITE_AFTER_ERASE) {
            norflash_write_nocheck(nf_sector_buf, write_context.sector*SECTOR_LENGTH, SECTOR_LENGTH);
        }
        else {
            norflash_write_nocheck(nf_write_buf+write_context.offset, write_context.addr, write_context.sector_remain);
        }
        if (write_context.len == write_context.sector_remain) {
            write_context.state = NF_STATE_IDEL;
        }
        else {
            write_context.sector++;
            write_context.sector_offset = 0;
            write_context.offset += write_context.sector_remain;
            write_context.len -= write_context.sector_remain;
            write_context.sector_remain = SECTOR_LENGTH;
            if (write_context.len <= write_context.sector_remain) {
                write_context.sector_remain = write_context.len;
            }
            write_context.state = NF_STATE_WRITE;
        }
        return;
    }
}
