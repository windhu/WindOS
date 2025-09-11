/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sdio.h
  * @brief   This file contains all the function prototypes for
  *          the sdio.c file
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
#ifndef __SDIO_H__
#define __SDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SD_HandleTypeDef hsd;

/* USER CODE BEGIN Private defines */
#define SD_TIMEOUT             ((uint32_t)100000000)
#define SD_TRANSFER_OK         ((uint8_t)0x00)
#define SD_TRANSFER_BUSY       ((uint8_t)0x01)
/* USER CODE END Private defines */

void MX_SDIO_SD_Init(void);

/* USER CODE BEGIN Prototypes */
uint8_t sd_read_card_status(HAL_SD_CardStatusTypeDef *pStatus);
uint8_t sd_read_card_info(HAL_SD_CardInfoTypeDef *pCardInfo);
uint8_t sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);
uint8_t sd_write_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SDIO_H__ */

