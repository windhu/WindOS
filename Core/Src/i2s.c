/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2s.c
  * @brief   This file provides code for the configuration
  *          of the I2S instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2s.h"

/* USER CODE BEGIN 0 */
#include "cmsis_os.h"
#include <stdio.h>
void dma_m0_callback(struct __DMA_HandleTypeDef * hdma);
void dma_m1_callback(struct __DMA_HandleTypeDef * hdma);
/* USER CODE END 0 */

I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_tx;

/* I2S2 init function */
void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B_EXTENDED;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_8K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_ENABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */
  hi2s2.Instance->CR2 |= 1<<1;
  __HAL_I2S_ENABLE(&hi2s2);
  /* USER CODE END I2S2_Init 2 */

}

void HAL_I2S_MspInit(I2S_HandleTypeDef* i2sHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(i2sHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* I2S2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2S2 GPIO Configuration
    PC2     ------> I2S2_ext_SD
    PC3     ------> I2S2_SD
    PB12     ------> I2S2_WS
    PB13     ------> I2S2_CK
    PC6     ------> I2S2_MCK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_I2S2ext;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2S2 DMA Init */
    /* SPI2_TX Init */
    hdma_spi2_tx.Instance = DMA1_Stream4;
    hdma_spi2_tx.Init.Channel = DMA_CHANNEL_0;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi2_tx.Init.Mode = DMA_CIRCULAR;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2sHandle,hdmatx,hdma_spi2_tx);

  /* USER CODE BEGIN SPI2_MspInit 1 */
    hdma_spi2_tx.XferCpltCallback = dma_m0_callback;
    hdma_spi2_tx.XferM1CpltCallback = dma_m1_callback;
  /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef* i2sHandle)
{

  if(i2sHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**I2S2 GPIO Configuration
    PC2     ------> I2S2_ext_SD
    PC3     ------> I2S2_SD
    PB12     ------> I2S2_WS
    PB13     ------> I2S2_CK
    PC6     ------> I2S2_MCK
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

    /* I2S2 DMA DeInit */
    HAL_DMA_DeInit(i2sHandle->hdmatx);
  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
static DMA_Callback dma_callback = NULL;
void dma_m0_callback(struct __DMA_HandleTypeDef * hdma) {
    if (dma_callback) {
        dma_callback(hdma, 0);
    }
}

void dma_m1_callback(struct __DMA_HandleTypeDef * hdma) {
    if (dma_callback) {
        dma_callback(hdma, 1);
    }
}

void i2s_set_format(uint32_t data_format) {
  hi2s2.Init.DataFormat = data_format;
  HAL_I2S_Init(&hi2s2);

  // SPI2->CR2 |= 1<<1;
  __HAL_I2S_ENABLE(&hi2s2);
}

/**
 * Fs=I2SxCLK/[256*(2*I2SDIV+ODD)]
 * I2SxCLK=(HSE/pllm)*PLLI2SN/PLLI2SR, pllm=2
 * HSE=8Mhz 
 * pllm:Sys_Clock_Set
 * PLLI2SN:192~432
 * PLLI2SR:2~7
 * I2SDIV:2~255
 * ODD:0/1
 * Column:samplerate/10,PLLI2SN,PLLI2SR,I2SDIV,ODD
 */
const uint16_t I2S_PSC_TBL[][5]=
{
    {   800, 256/2, 5, 12, 1 },   /* 8Khz */
    {  1102, 429/2, 4, 19, 0 },   /* 11.025Khz */
    {  1600, 213/2, 2, 13, 0 },   /* 16Khz */
    {  2205, 429/2, 4,  9, 1 },   /* 22.05Khz */
    {  3200, 213/2, 2,  6, 1 },   /* 32Khz */
    {  4410, 271/2, 2,  6, 0 },   /* 44.1Khz */
    {  4800, 258/2, 3,  3, 1 },   /* 48Khz */
    {  8820, 316/2, 2,  3, 1 },   /* 88.2Khz */
    {  9600, 344/2, 2,  3, 1 },   /* 96Khz */
    { 17640, 361/2, 2,  2, 0 },   /* 176.4Khz */
    { 19200, 393/2, 2,  2, 0 },   /* 192Khz */
};

uint8_t i2s_set_sample_rate(uint32_t sample_rate) {
    uint8_t i = 0; 
    uint32_t tempreg = 0;

    RCC_PeriphCLKInitTypeDef rcc_i2s_clk_init;

    for (i = 0; i < (sizeof(I2S_PSC_TBL) / 10); i++) {
        if ((sample_rate / 10) == I2S_PSC_TBL[i][0]) {
            break;
        }
    }
    if (i == (sizeof(I2S_PSC_TBL) / 10)) {
        printf("cannot find sample rate\r\n");
        return 1;
    }

    rcc_i2s_clk_init.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    rcc_i2s_clk_init.PLLI2S.PLLI2SN = (uint32_t)I2S_PSC_TBL[i][1];
    rcc_i2s_clk_init.PLLI2S.PLLI2SR = (uint32_t)I2S_PSC_TBL[i][2];
    HAL_RCCEx_PeriphCLKConfig(&rcc_i2s_clk_init);

    RCC->CR |= 1 << 26; 
    while((RCC->CR & 1 << 27) == 0);
    tempreg = I2S_PSC_TBL[i][3] << 0;
    tempreg |= I2S_PSC_TBL[i][4] << 8;
    tempreg |= 1 << 9;
    hi2s2.Instance->I2SPR = tempreg;
    return 0;
}

void i2s_set_tx_dma(uint8_t *buf0, uint8_t *buf1, uint32_t size, DMA_Callback cb) {
    dma_callback = cb;  
    HAL_DMAEx_MultiBufferStart(&hdma_spi2_tx, (uint32_t)buf0, (uint32_t)&hi2s2.Instance->DR, (uint32_t)buf1, size); 
    osDelay(1);                                      
    __HAL_DMA_ENABLE_IT(&hdma_spi2_tx, DMA_IT_TC);
    __HAL_DMA_CLEAR_FLAG(&hdma_spi2_tx, DMA_FLAG_TCIF0_4);
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);       
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
    __HAL_DMA_DISABLE(&hdma_spi2_tx);
}

void i2s_play_start()
{
    __HAL_DMA_ENABLE(&hdma_spi2_tx);
}

void i2s_play_stop()
{
    __HAL_DMA_DISABLE(&hdma_spi2_tx);
}
/* USER CODE END 1 */
