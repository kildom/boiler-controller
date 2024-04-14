/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32l5xx_hal.h"

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
#define RELAY0_Pin GPIO_PIN_13
#define RELAY0_GPIO_Port GPIOF
#define RELAY3_Pin GPIO_PIN_14
#define RELAY3_GPIO_Port GPIOF
#define RELAY5_Pin GPIO_PIN_15
#define RELAY5_GPIO_Port GPIOF
#define RELAY9_Pin GPIO_PIN_7
#define RELAY9_GPIO_Port GPIOE
#define RELAY8_Pin GPIO_PIN_8
#define RELAY8_GPIO_Port GPIOE
#define RELAY1_Pin GPIO_PIN_9
#define RELAY1_GPIO_Port GPIOE
#define RELAY10_Pin GPIO_PIN_10
#define RELAY10_GPIO_Port GPIOE
#define RELAY2_Pin GPIO_PIN_11
#define RELAY2_GPIO_Port GPIOE
#define RELAY11_Pin GPIO_PIN_12
#define RELAY11_GPIO_Port GPIOE
#define RELAY4_Pin GPIO_PIN_13
#define RELAY4_GPIO_Port GPIOE
#define RELAY12_Pin GPIO_PIN_14
#define RELAY12_GPIO_Port GPIOE
#define RELAY13_Pin GPIO_PIN_15
#define RELAY13_GPIO_Port GPIOE
#define RELAY14_Pin GPIO_PIN_10
#define RELAY14_GPIO_Port GPIOB
#define RELAY15_Pin GPIO_PIN_11
#define RELAY15_GPIO_Port GPIOB
#define RELAY6_Pin GPIO_PIN_8
#define RELAY6_GPIO_Port GPIOD
#define RELAY7_Pin GPIO_PIN_9
#define RELAY7_GPIO_Port GPIOD
#define ST_LINK_VCP_TX_Pin GPIO_PIN_7
#define ST_LINK_VCP_TX_GPIO_Port GPIOG
#define ST_LINK_VCP_RX_Pin GPIO_PIN_8
#define ST_LINK_VCP_RX_GPIO_Port GPIOG
#define LED_GREEN_Pin GPIO_PIN_7
#define LED_GREEN_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_9
#define LED_RED_GPIO_Port GPIOA
#define COMM_TX_Pin GPIO_PIN_5
#define COMM_TX_GPIO_Port GPIOD
#define COMM_RX_Pin GPIO_PIN_6
#define COMM_RX_GPIO_Port GPIOD
#define LED_BLUE_Pin GPIO_PIN_7
#define LED_BLUE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

extern uint32_t sysTickEvent;

void main_pre_loop();
void main_loop();
void main_construct_init();
void main_post_init();
void gpio_post_init();

int main(void);

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_lpuart1_rx;
extern DMA_HandleTypeDef hdma_lpuart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim2;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
