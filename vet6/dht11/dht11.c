#include <security.h>

extern TIM_HandleTypeDef htim2;

static void DATA_OUTPUT(u8 flg);//DATA输出
static u8 DATA_INPUT(void);//DATA输入
static u8 DH11_Read_Byte(void);//DH11读信号

u8 DH11_Read(void);//读取DH11温度和湿度

static void Test(void);//测试程序
///////////////////////////////////////
static void DATA_OUTPUT2(u8 flg);//DATA输出
static u8 DATA_INPUT2(void);//DATA输入
static u8 DH11_Read_Byte2(void);//DH11读信号

u8 DH11_Read2(void);//读取DH11温度和湿度

static void Test2(void);//测试程序

DH11_DATA DH11_data;
DH11_DATA2 DH11_data2;

void Delay_us(uint16_t us)
{     //微秒延时
	uint16_t differ = 0xffff-us-5;				
	__HAL_TIM_SET_COUNTER(&htim2,differ);	//设定TIM1计数器起始值
	HAL_TIM_Base_Start(&htim2);		//启动定时器	
	
	while(differ < 0xffff-5){	//判断
		differ = __HAL_TIM_GET_COUNTER(&htim2);		//查询计数器的计数值
	}
	HAL_TIM_Base_Stop(&htim2);
}


void DATA_OUTPUT(u8 flg)
{
  	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = DATA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DATA_GPIO_Port, &GPIO_InitStruct);

	if(flg==0)
	{
		DATA_RESET();
	}
	else
	{
		DATA_SET();
	}
}

u8 DATA_INPUT(void)
{
  	GPIO_InitTypeDef GPIO_InitStruct = {0};
	u8 flg=0;
	
	GPIO_InitStruct.Pin = DATA_Pin;
  	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  	GPIO_InitStruct.Pull = GPIO_PULLUP;
  	HAL_GPIO_Init(DATA_GPIO_Port, &GPIO_InitStruct);

	if(DATA_READ()==GPIO_PIN_RESET)
	{
		flg=0;
	}
	else 
	{
		flg=1;
	}

	return flg;
}

u8 DH11_Read_Byte(void)
{
    u8 ReadDat=0;
    u8 temp=0;
    u8 retry=0;
    u8 i=0;
    
    for(i=0;i<8;i++)
    {
      while(DATA_READ()==0&&retry<100)//等待直到DHT11输出高电平
      {
        Delay_us(1);
        retry++;
      }
      retry=0;
      Delay_us(40);
      if(DATA_READ()==1)
      {
        temp=1;
      }
      else
      {
        temp=0;
      }
      while(DATA_READ()==1&&retry<100)//等待直到DHT11输出低电平，表示退出。本轮1bit信号接收完毕。
      {
        Delay_us(1);
        retry++;
      }
      retry=0;
      ReadDat<<=1;
      ReadDat|=temp;
    }
    
    return ReadDat;
}

u8 DH11_Read(void)
{
  u8 retry=0;
  u8 i=0;
  
  DATA_OUTPUT(0);//设置为输出模式MCU向DH11发送信号
  HAL_Delay(18);
  DATA_SET();
  Delay_us(20);
  
  DATA_INPUT();//设置为输入模式DH11向MCU发送信号
  Delay_us(20);
  if(DATA_READ()==0)
  {
    while(DATA_READ()==0&&retry<100)
    {
      Delay_us(1);
      retry=0;
    }
    retry=0;
    while(DATA_READ()==1&&retry<100)
    {
      Delay_us(1);
      retry++;
    }
    retry=0;
    
    for(i=0;i<5;i++)//Data[0]湿度， Data[2]温度。Data[1]和Data[3]分别为0和2的小数位。Data[4]用于校验。
    {
      DH11_data.Data[i]=DH11_Read_Byte();
    }
    Delay_us(50);
  }
  u32 sum=DH11_data.Data[0]+DH11_data.Data[1]+DH11_data.Data[2]+DH11_data.Data[3];//校验
  if((sum)==DH11_data.Data[4])
  {
    DH11_data.humidity=DH11_data.Data[0];//获取湿度
    DH11_data.temp=DH11_data.Data[2];//获取温度
    return 1;    
  }
  else
  {
    return 0;
  }
}

void Test(void)
{
  if(DH11_Read())
  {
    DH11_data.index++;
    if(DH11_data.index>=128)
    {
      DH11_data.index=0;
    }
  }
   
}


void DH11_Task(void)
{
     Test();
}
//////////////////.....................///////////////////////////////////////

void DATA_OUTPUT2(u8 flg)
{
  	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = DATA_Pin2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DATA_GPIO_Port2, &GPIO_InitStruct);

	if(flg==0)
	{
		DATA_RESET2();
	}
	else
	{
		DATA_SET2();
	}
}

u8 DATA_INPUT2(void)
{
  	GPIO_InitTypeDef GPIO_InitStruct = {0};
	u8 flg2=0;
	
	GPIO_InitStruct.Pin = DATA_Pin2;
  	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  	GPIO_InitStruct.Pull = GPIO_PULLUP;
  	HAL_GPIO_Init(DATA_GPIO_Port2, &GPIO_InitStruct);

	if(DATA_READ2()==GPIO_PIN_RESET)
	{
		flg2=0;
	}
	else 
	{
		flg2=1;
	}

	return flg2;
}

u8 DH11_Read_Byte2(void)
{
    u8 ReadDat2=0;
    u8 temp2=0;
    u8 retry2=0;
    u8 i2=0;
    
    for(i2=0;i2<8;i2++)
    {
      while(DATA_READ2()==0&&retry2<100)//等待直到DHT11输出高电平
      {
        Delay_us(1);
        retry2++;
      }
      retry2=0;
      Delay_us(40);
      if(DATA_READ2()==1)
      {
        temp2=1;
      }
      else
      {
        temp2=0;
      }
      while(DATA_READ2()==1&&retry2<100)//等待直到DHT11输出低电平，表示退出。本轮1bit信号接收完毕。
      {
        Delay_us(1);
        retry2++;
      }
      retry2=0;
      ReadDat2<<=1;
      ReadDat2|=temp2;
    }
    
    return ReadDat2;
}

u8 DH11_Read2(void)
{
  u8 retry=0;
  u8 i=0;
  
  DATA_OUTPUT2(0);//设置为输出模式MCU向DH11发送信号
  HAL_Delay(18);
  DATA_SET2();
  Delay_us(20);
  
  DATA_INPUT2();//设置为输入模式DH11向MCU发送信号
  Delay_us(20);
  if(DATA_READ2()==0)
  {
    while(DATA_READ2()==0&&retry<100)
    {
      Delay_us(1);
      retry=0;
    }
    retry=0;
    while(DATA_READ2()==1&&retry<100)
    {
      Delay_us(1);
      retry++;
    }
    retry=0;
    
    for(i=0;i<5;i++)//Data[0]湿度， Data[2]温度。Data[1]和Data[3]分别为0和2的小数位。Data[4]用于校验。
    {
      DH11_data2.Data2[i]=DH11_Read_Byte2();
    }
    Delay_us(50);
  }
  u32 sum=DH11_data2.Data2[0]+DH11_data2.Data2[1]+DH11_data2.Data2[2]+DH11_data2.Data2[3];//校验
  if((sum)==DH11_data2.Data2[4])
  {
    DH11_data2.humidity2=DH11_data2.Data2[0];//获取湿度
    DH11_data2.temp2=DH11_data2.Data2[2];//获取温度
    return 1;    
  }
  else
  {
    return 0;
  }
}

void Test2(void)
{
  if(DH11_Read2())
  {
    DH11_data2.index2++;
    if(DH11_data2.index2>=128)
    {
      DH11_data2.index2=0;
    }
  }
   
}


void DH11_Task2(void)
{
     Test2();
}
