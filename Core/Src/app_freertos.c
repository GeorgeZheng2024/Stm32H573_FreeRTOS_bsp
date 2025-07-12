/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "sd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint32_t sdStatus; //SD card status.
typedef struct {
    uint8_t *data;
    size_t length;
} DataMessage_t;


static uint8_t read_buffer[256];
static uint8_t prog_buffer[64];
static uint8_t lookahead_buffer[32];
//static uint8_t block_cycles[8];

/* LittleFS配置和变量 */
lfs_t lfs;
static struct lfs_config lfs_cfg=
{
//  .read = sd_read,
//  .prog = sd_prog,
//  .erase = sd_erase,
//  .sync = sd_sync,
//
//  .read_size = 1,
//  .prog_size = 1,
//  .block_size = 4096,
//  .block_count = 4096,
//  .block_cycles = 500,
//  .cache_size   = 4096,
//  .lookahead_size = 4096,
//
//  .read_buffer = read_buffer,
//  .prog_buffer = prog_buffer,
//  .lookahead_buffer = lookahead_buffer,

  .read = sd_read,
  .prog = sd_prog,
  .erase = sd_erase,
  .sync = sd_sync,

  .read_size = 512,
  .prog_size = 512,
  .block_size = 8192,
  .block_count = 2048,
  .block_cycles = 500,
  .cache_size   = 1024,
  .lookahead_size = 128,

  .read_buffer = read_buffer,
  .prog_buffer = prog_buffer,
  .lookahead_buffer = lookahead_buffer,

};


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for LED1 */
osThreadId_t LED1Handle;
const osThreadAttr_t LED1_attributes = {
  .name = "LED1",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for SD */
osThreadId_t SDHandle;
const osThreadAttr_t SD_attributes = {
  .name = "SD",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 256 * 4
};
/* Definitions for fileRecord */
osMessageQueueId_t fileRecordHandle;
const osMessageQueueAttr_t fileRecord_attributes = {
  .name = "fileRecord"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of fileRecord */
  fileRecordHandle = osMessageQueueNew (32, sizeof(uint16_t), &fileRecord_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of LED1 */
  LED1Handle = osThreadNew(StartTaskLED1, NULL, &LED1_attributes);

  /* creation of SD */
  SDHandle = osThreadNew(StartTaskSD, NULL, &SD_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN defaultTask */

  uint8_t *dat = "this is test for SD card operation!";
  /* Infinite loop */

  for(;;)
  {
    SendDataToFile(dat,sizeof(*dat));
    printf("send %s to file\n",dat);
    osDelay(5000);
  }
  /* USER CODE END defaultTask */
}

/* USER CODE BEGIN Header_StartTaskLED1 */
/**
* @brief Function implementing the LED1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskLED1 */
void StartTaskLED1(void *argument)
{
  /* USER CODE BEGIN LED1 */
  HAL_GPIO_WritePin(USER_LED1_GPIO_Port,USER_LED1_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(USER_LED2_GPIO_Port,USER_LED2_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(USER_LED3_GPIO_Port,USER_LED3_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(USER_LED4_GPIO_Port,USER_LED4_Pin,GPIO_PIN_SET);
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(USER_LED1_GPIO_Port,USER_LED1_Pin);
    printf("hello LED1 flashes\n");
    osDelay(500);
  }
  /* USER CODE END LED1 */
}

/* USER CODE BEGIN Header_StartTaskSD */
/**
* @brief Function implementing the SD thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskSD */
void StartTaskSD(void *argument)
{
  /* USER CODE BEGIN SD */

  //uint8_t fileCNT = 0;
  uint8_t fileExists = 0;
  lfs_file_t file;
  DataMessage_t msg;
  struct lfs_info info;
  char filename[LFS_NAME_MAX + 1] = "test.txt"; //example file name
  //char *newline = "\n"; //example file name
  osDelay(100);

  if(sdStatus == SD_NOT_PRESENT)
  {
    printf("Error:SD_NOT_PRESENT\n");
    osThreadExit(); //kill current task
  }

  //init littlefs
  if(LFS_Init()<0)
  {
    printf("Error:LFS_Init\n");
    osThreadExit(); //kill current task
  }
  osDelay(100);
  /* Infinite loop */
  for(;;)
  {
    //osMessageQueueGet
    //if(xQueueReceive(fileRecordHandle, &fileCNT, (TickType_t)portMAX_DELAY) == pdPASS)
    if(osMessageQueueGet(fileRecordHandle, &msg, NULL,(TickType_t)portMAX_DELAY) == pdPASS)
    {
      fileExists = lfs_stat(&lfs, filename, &info);
      if(fileExists)
      {
        /* 打开文件，存在则追加，不存在则创建 */
        if (lfs_file_open(&lfs, &file, filename, \
          LFS_O_WRONLY | LFS_O_CREAT | \
            (fileExists ? LFS_O_APPEND : LFS_O_TRUNC)) == 0)
        {

          /* 如果文件已存在，先写入换行符 */
          if (fileExists)
          {
            lfs_file_write(&lfs, &file, "\n", 1);
          }

          /* 写入新数据 */
          lfs_file_write(&lfs, &file, msg.data, msg.length);

          /* 关闭文件 */
          lfs_file_close(&lfs, &file);

        }

      }
      osDelay(1);
    }
    /* USER CODE END SD */
  }
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/**
  * @brief  BSP SD Callback.
  * @param  Instance SD Instance
  * @param  Status   Pin status
  * @retval None.
  */
void BSP_SD_DetectCallback(uint32_t Instance, uint32_t Status)
{
  /* Prevent unused argument(s) compilation warning */
//  UNUSED(Instance);
//  UNUSED(Status);
  sdStatus = Status;

  /* This function should be implemented by the user application.
     It is called into this driver when an event on JoyPin is triggered. */
}

/* 发送数据到文件的示例函数 */
void SendDataToFile(const uint8_t *data, size_t length)
{
    DataMessage_t msg;

    /* 如果数据是静态的，可以直接发送指针 */
    msg.data = (uint8_t *)data;
    msg.length = length;

    /* 如果数据是动态分配的，这里需要传递拷贝或管理内存 */

    /* 发送消息到队列 */
    //xQueueSend(fileRecordHandle, &msg, portMAX_DELAY);
    osMessageQueuePut(fileRecordHandle, &msg, NULL,(TickType_t)portMAX_DELAY);

    //osMessageQueueGet(fileRecordHandle, &fileCNT, NULL,(TickType_t)portMAX_DELAY)
}


/* SD卡块设备操作函数 */
static int sd_read(const struct lfs_config *c, lfs_block_t block,
                  lfs_off_t off, void *buffer, lfs_size_t size)
{
    HAL_SD_ReadBlocks(&hsd1, (uint8_t*)buffer, block, size / 512, 1000);
    return 0;
}

static int sd_prog(const struct lfs_config *c, lfs_block_t block,
                  lfs_off_t off, const void *buffer, lfs_size_t size)
{
    HAL_SD_WriteBlocks(&hsd1, (uint8_t*)buffer, block, size / 512, 1000);
    return 0;
}

static int sd_erase(const struct lfs_config *c, lfs_block_t block)
{
    HAL_SD_Erase(&hsd1, block, block);
    return 0;
}

static int sd_sync(const struct lfs_config *c)
{
    return 0;
}

/* LittleFS初始化 */
static int LFS_Init(void)
{
    /* 配置LittleFS */
    lfs_cfg.read = sd_read;
    lfs_cfg.prog = sd_prog;
    lfs_cfg.erase = sd_erase;
    lfs_cfg.sync = sd_sync;

    /* SD卡参数 */
//    lfs_cfg.block_size = 512;       /* SD卡扇区大小 */
//    lfs_cfg.block_count = 131072;   /* SD卡扇区数量 (64MB) */
//    lfs_cfg.cache_size = 64;        /* 缓存大小 */
//    lfs_cfg.lookahead_size = 32;    /* 预读大小 */
//    lfs_cfg.block_cycles = 500;     /* 块寿命 */

    lfs_cfg.block_size = 512;       /* SD卡扇区大小 */
    lfs_cfg.block_count = 131072;   /* SD卡扇区数量 (64MB) */
    lfs_cfg.cache_size = 64;        /* 缓存大小 */
    lfs_cfg.lookahead_size = 32;    /* 预读大小 */
    lfs_cfg.block_cycles = 500;     /* 块寿命 */

    /* 分配缓冲区 */
    lfs_cfg.read_buffer = read_buffer;
    lfs_cfg.prog_buffer = prog_buffer;
    lfs_cfg.lookahead_buffer = lookahead_buffer;

    /* 尝试挂载文件系统 */
    int err = lfs_mount(&lfs, &lfs_cfg);

    /* 如果文件系统不存在，创建它 */
    if (err == LFS_ERR_CORRUPT) {
        lfs_format(&lfs, &lfs_cfg);
        err = lfs_mount(&lfs, &lfs_cfg);
    }

    return err;
}
/* USER CODE END Application */

