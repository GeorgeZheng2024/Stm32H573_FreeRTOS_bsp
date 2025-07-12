/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.h
  * Description        : FreeRTOS applicative header file
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
#ifndef __APP_FREERTOS_H
#define __APP_FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lfs.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Exported macro -------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */
extern osThreadId_t defaultTaskHandle;
extern osThreadId_t LED1Handle;
extern osThreadId_t SDHandle;
extern osMessageQueueId_t fileRecordHandle;

/* Exported function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
//extern lfs_t lfs;
static int sd_read(const struct lfs_config *c, lfs_block_t block,
                  lfs_off_t off, void *buffer, lfs_size_t size) ;
static int sd_prog(const struct lfs_config *c, lfs_block_t block,
                  lfs_off_t off, const void *buffer, lfs_size_t size) ;
static int sd_erase(const struct lfs_config *c, lfs_block_t block) ;
static int sd_sync(const struct lfs_config *c) ;
static int LFS_Init(void) ;
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskLED1(void *argument);
void StartTaskSD(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void SendDataToFile(const uint8_t *data, size_t length) ;
/* USER CODE END Application */

#ifdef __cplusplus
}
#endif
#endif /* __APP_FREERTOS_H */
