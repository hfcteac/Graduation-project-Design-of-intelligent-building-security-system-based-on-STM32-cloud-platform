#ifndef _DH11_H_
#define _DH11_H_

#include "main.h"

#define u8 unsigned char 
#define u16 unsigned short
#define u32 unsigned int

//DATA IO的定义
#define DATA_SET() HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin, GPIO_PIN_SET)
#define DATA_RESET() HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin, GPIO_PIN_RESET)

#define DATA_READ() HAL_GPIO_ReadPin(DATA_GPIO_Port,DATA_Pin)

typedef struct
{
  u8 Data[5];//数据存放数组
  u8 index;
  u8 temp;//温度
  u8 humidity;//湿度
  
}DH11_DATA;

extern DH11_DATA DH11_data;//DH11属性封装

void DH11_Task(void);//后台轮询调用

#define DATA_Pin GPIO_PIN_5
#define DATA_GPIO_Port GPIOA
/////////////////////////////////////
//DATA IO的定义
#define DATA_SET2() HAL_GPIO_WritePin(DATA_GPIO_Port2, DATA_Pin2, GPIO_PIN_SET)
#define DATA_RESET2() HAL_GPIO_WritePin(DATA_GPIO_Port2, DATA_Pin2, GPIO_PIN_RESET)

#define DATA_READ2() HAL_GPIO_ReadPin(DATA_GPIO_Port2,DATA_Pin2)

typedef struct
{
  u8 Data2[5];//数据存放数组
  u8 index2;
  u8 temp2;//温度
  u8 humidity2;//湿度
  
}DH11_DATA2;

extern DH11_DATA2 DH11_data2;//DH11属性封装

void DH11_Task2(void);//后台轮询调用

#define DATA_Pin2 GPIO_PIN_6
#define DATA_GPIO_Port2 GPIOA


#endif


