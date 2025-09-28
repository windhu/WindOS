#include "player.h"
#include "ff.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include "i2s.h"
#include "es8388.h"
#include "lvgl_gui.h"
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "fun_test.h"
#include <stdio.h>

#define  WAV_I2S_TX_DMA_BUFSIZE 1024

enum {
    player_stop,
    player_playing,
    player_pause,
};

typedef struct {
    uint8_t status;
    FIL file;
    AudioInfo audInfo;
    uint8_t i2sbuf0[WAV_I2S_TX_DMA_BUFSIZE];
    uint8_t i2sbuf1[WAV_I2S_TX_DMA_BUFSIZE];
    uint8_t tbuf[WAV_I2S_TX_DMA_BUFSIZE];
} PlayerManager;

extern MessageBufferHandle_t gui_message_handle;

uint8_t spk_volumn_val = 16;

static PlayerManager playerMng = {0};
static volatile uint8_t current_buff = 0;
static volatile uint8_t buff0_filled = 0;
static volatile uint8_t buff1_filled = 0;
static void player_start(uint8_t start);

uint8_t audio_decode_init(char *fname, AudioInfo *audInfo) {
    uint8_t buf[512] = {0};
    uint8_t res;
    UINT rbyte;

    ChunkRIFF *riff;
    ChunkFMT *fmt;
    ChunkFACT *fact;
    ChunkDATA *data;

    res = f_open(&playerMng.file, fname, FA_READ);
    if (res != FR_OK) {
        printf("failed to open, %d\r\n", res);
        return 1;
    }
    f_read(&playerMng.file, buf, 512, &rbyte);
    riff = (ChunkRIFF *)buf;
    // 0x45564157 => WAVE
    if (riff->Format != 0x45564157) {
        return 2;
    }
    fmt = (ChunkFMT *)(buf + 12);
    fact = (ChunkFACT *)(buf + 12 + 8 + fmt->ChunkSize);
    // don't always have 0x74636166 => fact,  0x5453494C => LIST
    if (fact->ChunkID == 0x74636166 || fact->ChunkID == 0x5453494C) {
        audInfo->datastart = 12 + 8 + fmt->ChunkSize + 8 + fact->ChunkSize;    /* fact/LIST */
    }
    else {
        audInfo->datastart = 12 + 8 + fmt->ChunkSize;
    }
    data = (ChunkDATA *)(buf + audInfo->datastart);
    // 0x61746164 => data
    if (data->ChunkID != 0x61746164) {
        return 3;
    }
    audInfo->audioformat = fmt->AudioFormat;
    audInfo->nchannels = fmt->NumOfChannels;
    audInfo->samplerate = fmt->SampleRate;
    audInfo->bitrate = fmt->ByteRate * 8;
    audInfo->blockalign = fmt->BlockAlign;
    audInfo->bps = fmt->BitsPerSample;

    audInfo->datasize = data->ChunkSize;
    audInfo->datastart = audInfo->datastart + 8;

    printf("audInfo->audioformat:%d\r\n", audInfo->audioformat);
    printf("audInfo->nchannels:%d\r\n", audInfo->nchannels);
    printf("audInfo->samplerate:%lu\r\n", audInfo->samplerate);
    printf("audInfo->bitrate:%lu\r\n", audInfo->bitrate);
    printf("audInfo->blockalign:%d\r\n", audInfo->blockalign);
    printf("audInfo->bps:%d\r\n", audInfo->bps);
    printf("audInfo->datasize:%lu\r\n", audInfo->datasize);
    printf("audInfo->datastart:%lu\r\n", audInfo->datastart);

    return 0;
}

uint32_t wav_buffill(uint8_t *buf, uint16_t size, uint8_t bits) {
    uint16_t readlen = 0;
    uint32_t bread = 0;
    uint16_t i = 0;
    uint8_t *p = NULL;
    FRESULT res = FR_OK;
    if (bits == 24) {
        readlen = (size / 4) * 3;
        res = f_read(&playerMng.file, playerMng.tbuf, readlen, (UINT*)&bread);
        if (res != FR_OK) {
            return 0;
        }
        p = playerMng.tbuf;
        for (i = 0; i < size;) {
            buf[i++] = p[1];
            buf[i] = p[2]; 
            i += 2;
            buf[i++] = p[0];
            p += 3;
        }
        
        bread = (bread * 4) / 3;
    }
    else {
        res = f_read(&playerMng.file, buf, size, (UINT*)&bread);
        if (res != FR_OK) {
            return 0;
        }
        if (bread < size) {
            for (i = bread; i < size - bread; i++) {
                buf[i] = 0;
            }
        }
    }
    return bread;
}

void dma_done_callback(struct __DMA_HandleTypeDef * hdma, uint8_t bufferId) {
    if (bufferId == 0) {
        current_buff = 1;
        buff0_filled = 0;
    }
    else {
        current_buff = 0;
        buff1_filled = 0;
    }
}

void play_song(uint8_t *song_name) {
    uint32_t fillnum = 0;
    if (audio_decode_init(song_name, &playerMng.audInfo) == 0) {
        es8388_adda_cfg(1, 0);
        es8388_output_cfg(1, 1);
        es8388_headphone_vol_set(25);
        es8388_speaker_vol_set(spk_volumn_val);
        if (playerMng.audInfo.bps == 16) {
            es8388_i2s_cfg(0, 3);
            i2s_set_format(I2S_DATAFORMAT_16B_EXTENDED);
        }
        else if (playerMng.audInfo.bps == 24) {
            es8388_i2s_cfg(0, 0);
            i2s_set_format(I2S_DATAFORMAT_24B);
        }
        i2s_set_sample_rate(playerMng.audInfo.samplerate);
        i2s_set_tx_dma(playerMng.i2sbuf0, playerMng.i2sbuf1, WAV_I2S_TX_DMA_BUFSIZE/2, dma_done_callback);
        i2s_play_stop();
        f_lseek(&playerMng.file, playerMng.audInfo.datastart);
        fillnum = wav_buffill(playerMng.i2sbuf0, WAV_I2S_TX_DMA_BUFSIZE, playerMng.audInfo.bps);
        buff0_filled = 1;
        fillnum = wav_buffill(playerMng.i2sbuf1, WAV_I2S_TX_DMA_BUFSIZE, playerMng.audInfo.bps);
        buff1_filled = 1;
        playerMng.status = player_playing;
        player_start(1);
    }
}

void player_start(uint8_t start) {
    size_t sbyte = 0;
    uint8_t buf[5] = {0};
    if (start == 1) {
        i2s_play_start();
        buf[0] = PLAYER_START;
        sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
    }
    else {
        i2s_play_stop();
        buf[0] = PLAYER_STOP;
        sbyte = xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
    }
}

extern EventGroupHandle_t keyEventGroup;
void player_task() {
    EventBits_t uxBits;
    uint32_t readed = WAV_I2S_TX_DMA_BUFSIZE;
    uint8_t buf[5] = {0};
    es8388_init();
    for(;;) {
        uxBits = xEventGroupWaitBits(keyEventGroup, EVENT_KEY0|EVENT_KEY1|EVENT_KEY3, pdTRUE,pdFALSE,1);
        if (uxBits & EVENT_KEY0) {
            // key 0
            if (playerMng.status == player_stop) {
                play_song("/test/your_16.wav");
            }
            else if (playerMng.status == player_playing) {
                f_close(&playerMng.file);
                playerMng.status = player_stop;
                player_start(0);
            }
        }

        if ((uxBits & EVENT_KEY1) || (uxBits & EVENT_KEY3)) {
            // key 1 down, key 3 up
            if ((uxBits & EVENT_KEY1)) {
                spk_volumn_val = spk_volumn_val - 1;
            }
            else {
                spk_volumn_val = spk_volumn_val + 1;
            }
            spk_volumn_val = es8388_speaker_vol_set(spk_volumn_val);
            buf[0] = SPK_VOLUMN;
            buf[1] = spk_volumn_val;
            xMessageBufferSend(gui_message_handle,(void *) buf, sizeof(buf), 0);
        }

        if (playerMng.status == player_playing) {
            readed = WAV_I2S_TX_DMA_BUFSIZE;
            if (current_buff == 0 && buff1_filled == 0) {
                readed = wav_buffill(playerMng.i2sbuf1, WAV_I2S_TX_DMA_BUFSIZE, playerMng.audInfo.bps);
                buff1_filled = 1;
            }
            else if (current_buff == 1 && buff0_filled == 0) {
                readed = wav_buffill(playerMng.i2sbuf0, WAV_I2S_TX_DMA_BUFSIZE, playerMng.audInfo.bps);
                buff0_filled = 1;
            }
            if (readed != WAV_I2S_TX_DMA_BUFSIZE) {
                f_close(&playerMng.file);
                playerMng.status = player_stop;
                player_start(0);
            }
        }
        else {
            osDelay(10);
        }
    }
}