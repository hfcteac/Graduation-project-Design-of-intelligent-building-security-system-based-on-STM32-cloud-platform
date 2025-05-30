/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LTZL1_Pin GPIO_PIN_2
#define LTZL1_GPIO_Port GPIOE
#define LTZL2_Pin GPIO_PIN_3
#define LTZL2_GPIO_Port GPIOE
#define LTBGR_Pin GPIO_PIN_4
#define LTBGR_GPIO_Port GPIOE
#define LTBGL_Pin GPIO_PIN_5
#define LTBGL_GPIO_Port GPIOE
#define PEOPLEBGR_Pin GPIO_PIN_14
#define PEOPLEBGR_GPIO_Port GPIOB
#define PEOPLEBGL_Pin GPIO_PIN_15
#define PEOPLEBGL_GPIO_Port GPIOB
#define PEOPLEZL2_Pin GPIO_PIN_8
#define PEOPLEZL2_GPIO_Port GPIOD
#define PEOPLEZL1_Pin GPIO_PIN_9
#define PEOPLEZL1_GPIO_Port GPIOD
#define SMOKBGR_Pin GPIO_PIN_10
#define SMOKBGR_GPIO_Port GPIOD
#define SMOKBGL_Pin GPIO_PIN_11
#define SMOKBGL_GPIO_Port GPIOD
#define SMOKZL3_Pin GPIO_PIN_12
#define SMOKZL3_GPIO_Port GPIOD
#define SMOKZL2_Pin GPIO_PIN_13
#define SMOKZL2_GPIO_Port GPIOD
#define SMOKZL1_Pin GPIO_PIN_14
#define SMOKZL1_GPIO_Port GPIOD
#define FIREBGR_Pin GPIO_PIN_15
#define FIREBGR_GPIO_Port GPIOD
#define FIREBGL_Pin GPIO_PIN_6
#define FIREBGL_GPIO_Port GPIOC
#define FIREZL3_Pin GPIO_PIN_7
#define FIREZL3_GPIO_Port GPIOC
#define FIREZL2_Pin GPIO_PIN_8
#define FIREZL2_GPIO_Port GPIOC
#define FIREZL1_Pin GPIO_PIN_9
#define FIREZL1_GPIO_Port GPIOC
#define FANOUT_SLAVE_Pin GPIO_PIN_10
#define FANOUT_SLAVE_GPIO_Port GPIOC
#define FANOUT_Pin GPIO_PIN_11
#define FANOUT_GPIO_Port GPIOC
#define AC_COLD_Pin GPIO_PIN_12
#define AC_COLD_GPIO_Port GPIOC
#define DCFZL1_Pin GPIO_PIN_0
#define DCFZL1_GPIO_Port GPIOD
#define DCFZL2_Pin GPIO_PIN_1
#define DCFZL2_GPIO_Port GPIOD
#define DCFZL3_Pin GPIO_PIN_2
#define DCFZL3_GPIO_Port GPIOD
#define DCFBGL_Pin GPIO_PIN_3
#define DCFBGL_GPIO_Port GPIOD
#define DCFBGR_Pin GPIO_PIN_4
#define DCFBGR_GPIO_Port GPIOD
#define warn_Pin GPIO_PIN_7
#define warn_GPIO_Port GPIOD
#define PUMPCONT2_Pin GPIO_PIN_3
#define PUMPCONT2_GPIO_Port GPIOB
#define PUMPCONT1_Pin GPIO_PIN_4
#define PUMPCONT1_GPIO_Port GPIOB
#define USERLED_Pin GPIO_PIN_9
#define USERLED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
