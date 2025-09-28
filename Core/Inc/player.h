#ifndef __PLAYER_H__
#include "stdint.h"
 
typedef struct {
    uint32_t ChunkID;
    uint32_t ChunkSize;
    uint32_t Format;
 } ChunkRIFF;

typedef struct {
    uint32_t ChunkID;
    uint32_t ChunkSize;
    uint16_t AudioFormat;
    uint16_t NumOfChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
//    uint16_t ByteExtraData;
} ChunkFMT;

typedef struct {
    uint32_t ChunkID;
    uint32_t ChunkSize;
    uint32_t NumOfSamples;
} ChunkFACT;

typedef struct {
    uint32_t ChunkID;
    uint32_t ChunkSize;
} ChunkLIST;

typedef struct {
    uint32_t ChunkID;
    uint32_t ChunkSize ;
} ChunkDATA;

typedef struct {
    ChunkRIFF riff;
    ChunkFMT fmt;
//    ChunkFACT fact;
    ChunkDATA data;
} WaveHeader;

typedef struct {
    uint16_t audioformat;
    uint16_t nchannels;
    uint16_t blockalign;
    uint32_t datasize;

    uint32_t totalsec;
    uint32_t cursec;

    uint32_t bitrate;
    uint32_t samplerate;
    uint16_t bps;

    uint32_t datastart;
} AudioInfo;

#endif