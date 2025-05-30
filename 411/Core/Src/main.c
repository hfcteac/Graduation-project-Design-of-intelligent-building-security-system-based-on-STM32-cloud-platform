/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <arm_math.h>
#ifndef PI
#define PI 3.14159265358979323846f
#endif
#define FILTER_SIZE 5
float AD_samples[FILTER_SIZE] = {0};
uint8_t sample_index = 0;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 等比例缩放函数
float cal_map(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
/**
 * 串口3重定义
 * 锟斤拷锟斤拷锟斤拷锟?: 锟斤拷
 * 锟斤拷 锟斤拷 值: 锟斤拷
 * 说    锟斤拷锟斤拷锟斤拷
 */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}
/**
 * 串口3重定义
 * 锟斤拷锟斤拷锟斤拷锟?: 锟斤拷
 * 锟斤拷 锟斤拷 值: 锟斤拷
 * 说    锟斤拷锟斤拷锟斤拷
 */
int fgetc(FILE *f)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
  return ch;
}

// 添加滤波函数
float FilterADValue(float new_sample)
{
    // 更新样本数组
    AD_samples[sample_index] = new_sample;
    sample_index = (sample_index + 1) % FILTER_SIZE;

    // 计算平均值
    float sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        sum += AD_samples[i];
    }
    return sum / FILTER_SIZE;
}
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
int opentime = 0;
float bright,brightcal;
float AD_Value;
float AD_Turn;
int middle1;
int status=1;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t getBuffer[30];
unsigned char i, j;
int ii;
uint8_t k = 0;
uint16_t old_reg = 0, len = 0;
float V = 0, C = 0, P = 0, E_con = 0;
float cos_value;     // 计算存储余弦值
float angle_radians; // 存储计算出的角度（以弧度为单位）
unsigned long sendecon;//太阳能电量发给主控用的
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Data_Processing(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define TIM_HANDLE &htim2
#define KEY0 HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) //
void delay_us(int nus)
{
  __HAL_TIM_SET_COUNTER(TIM_HANDLE, 0); // 把计数器的值设置为0
  __HAL_TIM_ENABLE(TIM_HANDLE);         // 开启计数
  while (__HAL_TIM_GET_COUNTER(TIM_HANDLE) < nus)
    ;                            // 每计数一次，就是1us，直到计数器值等于我们需要的时间
  __HAL_TIM_DISABLE(TIM_HANDLE); // 关闭计数
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if ((getBuffer[0] == 0xF2) && (getBuffer[1] == 0x5A))
  {
    //    Data_Processing();
    //    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
  else
  {
    for (i = 0; i < 24; i++)
    {
      getBuffer[i] = 0;
    }
    for (int ii = 0; ii < 1100000; ii++)
    {
      __NOP(); // No Operation，确保编译器不会优化掉循环
    }
  }
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  // HAL_UART_Receive_IT(&huart2 ,(uint8_t *)getBuffer,24);
  // 使能接收中断和空闲中断并打开DMA接收
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  __HAL_UART_CLEAR_IDLEFLAG(&huart2);
  HAL_UART_Receive_DMA(&huart2, (uint8_t *)getBuffer, 24);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
  while (1)
  {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 50); // 等待转换完成，第二个参数表示超时时间，单位ms.
    if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
    {
      AD_Value = HAL_ADC_GetValue(&hadc1); // 读取ADC转换数据，数据为12位
      //    printf("v=%.1f\r\n",AD_Value);//打印日志//这里advaule是数据
    }
    Data_Processing();

    HAL_Delay(100);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    // AD_Value(改成转换的电压)分压电阻是8k+1k+2k，检测adc接在2k两侧
    AD_Turn = cal_map(AD_Value, 0, 4096.0f, 0, 18.0f); // adc转换成太阳能板电压
    if(AD_Turn<10.0)
    {
        middle1=1;
        status=1;
    }
    else
    {
        middle1=0;
        status=2;
    }
    cos_value = AD_Turn / 0.9f / V;                    // Ud=0.9*U2*cosa，cosa=Ud/0.9/u2;
    bright = 180-(acosf(cos_value) * (180.0f / PI));         // 算角度，给与时间
    if (bright < 90)
    bright = 90;
    if (bright > 165)
    bright = 165;
    opentime = (unsigned int)cal_map(bright, 0, 180, 0, 10000);
    sendecon=E_con*10000;//99,999.0011*10000=999990011
    printf("%d %09d\r\n",status,(int)sendecon);//status的0是线路损坏，1是关闭，2是开启
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Data_Processing(void) // 电能数据解析
{
  uint32_t VP_REG = 0, V_REG = 0, CP_REG = 0, C_REG = 0, PP_REG = 0, P_REG = 0, PF_COUNT = 0, PF = 0, dat_sum = 0;
  uint8_t i = 0;
  if (getBuffer[0] != 0xaa) // 芯片误差修正功能正常，参数正常
  {
    for (i = 2; i < 23; i++)
    {
      dat_sum = dat_sum + getBuffer[i]; // 计算校验和
    }
    if (dat_sum % 256 == getBuffer[23]) // 检查校验位是否正确
    {
      VP_REG = getBuffer[2] * 65536 + getBuffer[3] * 256 + getBuffer[4]; // 计算电压参数寄存器
      V_REG = getBuffer[5] * 65536 + getBuffer[6] * 256 + getBuffer[7];  // 计算电压寄存器
      V = (VP_REG / V_REG) * 1.88;                                       // 计算电压值，1.88为电压系数，根据所采用的分压电阻大小来确定
      // printf("U:%0.2fV; ",V);

      CP_REG = getBuffer[8] * 65536 + getBuffer[9] * 256 + getBuffer[10];  // 计算电流参数寄存器
      C_REG = getBuffer[11] * 65536 + getBuffer[12] * 256 + getBuffer[13]; // 计算电流寄存器
      C = ((CP_REG * 100) / C_REG) / 100.0;                                // 计算电流值
      // printf("I:%2.3fA; ",C);
      // sprintf((char *)dat,"U:%0.2fV  I:%2.2fA ",V,C);
      // OLED_P6x8Str(0,3,dat);
      // printf((char *)dat);
      if (getBuffer[0] > 0xf0) // 判断实时功率是否未溢出
      {
        // printf("NO Device!");
        // OLED_P6x8Str(30,4,"NO Device");
        P = 0;
      }
      else
      {
        PP_REG = getBuffer[14] * 65536 + getBuffer[15] * 256 + getBuffer[16]; // 计算功率参数寄存
        P_REG = getBuffer[17] * 65536 + getBuffer[18] * 256 + getBuffer[19];  // 计算功率寄存器
        P = (PP_REG / P_REG) * 1.88 * 1;                                      // 计算有效功率
        // sprintf((char *)dat,"P:%0.2fW   ",P);
        // OLED_P6x8Str(30,4,dat);
        // printf((char *)dat);
      }
      if ((getBuffer[20] & 0x80) != old_reg) // 判断数据更新寄存器最高位有没有翻转
      {
        k++;
        old_reg = getBuffer[20] & 0x80;
      }
      PF = (k * 65536) + (getBuffer[21] * 256) + getBuffer[22]; // 计算已用电量脉冲数
      PF_COUNT = ((100000 * 3600) / (PP_REG * 1.88)) * 10000;   // 计算1度电对应的脉冲数量
      E_con = ((PF * 10000) / PF_COUNT) / 10000.0;              // 计算已用电量
      // sprintf((char *)dat,"E:%0.4lf kW.h  ",E_con);
      // OLED_P6x8Str(10,5,dat);
      // printf((char *)dat);
      // printf("\r\n");
    }
  }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_5&&middle1==0) // 有中断就触发B7  B6  B5   B4,对应1234   &&middle1==0
  {
    if (KEY0 == 0)
    {
      delay_us(opentime);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); // 13开
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //
      delay_us(50);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // 13关
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); //
    }
    if (KEY0 == 1)
    {
      delay_us(opentime);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // 24开
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET); //
      delay_us(50);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // 24关
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); //
    }
  }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
