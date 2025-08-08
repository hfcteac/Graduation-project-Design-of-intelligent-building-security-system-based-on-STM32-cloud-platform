#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "sys.h" 

void Buzzer_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;//输出低电代表开启
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;//人脸低电开启
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//wifi低电开启
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	GPIO_SetBits(GPIOB, GPIO_Pin_14);//B14是状态输出
  GPIO_SetBits(GPIOB, GPIO_Pin_15);
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

void Buzzer_ON()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_14);
}

void Buzzer_OFF()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

void Buzzer1()	//蜂鸣器响一声
{
	Buzzer_ON();
	Delay_ms(500);
	Buzzer_OFF();
}

void Buzzer2()	//蜂鸣器响两声
{
	Buzzer_ON();
	Delay_ms(100);
	Buzzer_OFF();
	Delay_ms(100);
	Buzzer_ON();
	Delay_ms(100);
	Buzzer_OFF();
}

void Buzzer_Alarm()	//蜂鸣器发出警报
{
	Buzzer_ON();
	Delay_ms(50);
	Buzzer_OFF();
	Delay_ms(50);
	Buzzer_ON();
	Delay_ms(50);
	Buzzer_OFF();
	Delay_ms(50);
	Buzzer_ON();
	Delay_ms(50);
	Buzzer_OFF();
	Delay_ms(50);
	Buzzer_ON();
	Delay_ms(50);
	Buzzer_OFF();
}
