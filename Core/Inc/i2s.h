/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2s.h
  * @brief   This file contains all the function prototypes for
  *          the i2s.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2S_H__
#define __I2S_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern I2S_HandleTypeDef hi2s2;

/* USER CODE BEGIN Private defines */
#define GPIO_AF_I2S_SPI                 GPIO_AF5_SPI2
#define GPIO_AF_I2S_EXT_SPI             GPIO_AF6_I2S2ext


#define I2S_LRCK_GPIO_PORT              GPIOB
#define I2S_LRCK_GPIO_PIN               GPIO_PIN_12
#define I2S_LRCK_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)     /* PB��ʱ��ʹ�� */

#define I2S_SCLK_GPIO_PORT              GPIOB
#define I2S_SCLK_GPIO_PIN               GPIO_PIN_13
#define I2S_SCLK_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)     /* PB��ʱ��ʹ�� */

#define I2S_SDOUT_GPIO_PORT             GPIOC
#define I2S_SDOUT_GPIO_PIN              GPIO_PIN_2
#define I2S_SDOUT_GPIO_CLK_ENABLE()     do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)     /* PC��ʱ��ʹ�� */

#define I2S_SDIN_GPIO_PORT              GPIOC
#define I2S_SDIN_GPIO_PIN               GPIO_PIN_3
#define I2S_SDIN_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)     /* PC��ʱ��ʹ�� */

#define I2S_MCLK_GPIO_PORT              GPIOC
#define I2S_MCLK_GPIO_PIN               GPIO_PIN_6
#define I2S_MCLK_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)     /* PC��ʱ��ʹ�� */

#define I2S_SPI                         SPI2
#define I2S_SPI_CLK_ENABLE()            do{ __HAL_RCC_SPI2_CLK_ENABLE(); }while(0)      /* I2S2ʱ��ʹ��*/

#define I2S_TX_DMASx                    DMA1_Stream4
#define I2S_TX_DMASx_Channel            DMA_CHANNEL_0
#define I2S_TX_DMASx_Handle             DMA1_Stream4_IRQHandler
#define I2S_TX_DMASx_IRQ                DMA1_Stream4_IRQn
#define I2S_TX_DMASx_FLAG               DMA_FLAG_TCIF0_4
#define I2S_TX_DMA_CLK_ENABLE()         do{ __HAL_RCC_DMA1_CLK_ENABLE(); }while(0)   /* I2S2 TX DMAʱ��ʹ�� */
/* USER CODE END Private defines */

void MX_I2S2_Init(void);

/* USER CODE BEGIN Prototypes */
extern void (*i2s_tx_callback)(void); 
typedef void(*DMA_Callback)(struct __DMA_HandleTypeDef * hdma, uint8_t bufferId);
void i2s_init(uint32_t i2s_standard, uint32_t i2s_mode, uint32_t i2s_clock_polarity, uint32_t i2s_dataformat);
void i2s_tx_dma_init(uint8_t* buf0, uint8_t *buf1, uint16_t num, DMA_Callback cb); 
void i2s_play_start(void); 
void i2s_set_format(uint32_t data_format);
uint8_t i2s_set_sample_rate(uint32_t sample_rate);
void i2s_set_tx_dma(uint8_t *buf0, uint8_t *buf1, uint32_t size, DMA_Callback cb);
void i2s_play_start();
void i2s_play_stop();
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2S_H__ */

