/* USER CODE BEGIN Header */
/**盗版的人我操你妈逼，剽窃者死爹死妈，你妈炸了，祝你延毕死全家，我操你妈的。傻逼
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 * daobanwocaonima
 * The ownership of the program belongs to Song Nan, please stop your piracy.
 * Piracy is not a good death, dead parents die, dead whole family.
 * Some secret, equivalent to a watermark.
 * https://pan.baidu.com/s/1uRYBI3bAJ5pFQhadPV13AQ?pswd=8fsg.
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <security.h>
#include <youseeme.mp3>
#include <lsy.h>
#include <lisiyu.iloveyou>
#define ADDR_24LCxx_Write 0xA0
#define ADDR_24LCxx_Read 0xA1
#define BufferSize 256
uint8_t WriteBuffer[BufferSize], ReadBuffer[BufferSize];
uint16_t i;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
unsigned int temp, humi, tempout, humiout; // 室内外温湿度
long numctrl, numact, numeco, numwhether, numwind1, numwind2;
char *leftover, *leftover2, *leftover3, *leftover4;
// 控制 空调   ， 小时（用于节能）,  1天气+1湿度+2天气+2湿度   1风向+风速
char *leftover5, *leftover6, *leftover7, *leftover8, *leftover9, *leftover10, *leftover11, *leftover12;
//  2风向+风速         1低温   ,    1高温，     2低温      2高温      wifi状态    点灯科技状态   时间
char *slpover, *slpover2; // 太阳能没啥说的
char showtime[16];
u8 fireloca;                                                      // 火警置位，用于灯光控制
/*000*/ u8 firetest1, firetest2, firetest3, firetest4, firetest5; //
// 用于如果有着火就置为1，程序从上往下，当全部为0时给关闭提示，此时将上面fireloca归0，关闭火灾指示
u8 ledtwinkle;
int pumpdelay = 10;
int ToBlinkerWeCom = 0; // 发给点灯科技、巴法云、企业微信的数据量信息（780e也通过这个数据流）数据信息介绍在下面
/////////////发送的数据信息，格式为0 0 0 0 0 0 0 0 0
///////////////从低位到高位分别为走廊1，走廊2，走廊3，办公1，办公2，太阳能。前三位如果是119，代表报警。
///////////////暂时先这么用，如果后期有其他想法再说，目前打算在里面加一个发电量显示，实时显示电能质量。
///////////////////xxxxx   1   1   1   1   1   1
///////////////////发电量
//////////////（已实现esp接收字符串可以使用printf
///////////////例子：00100100 111，前六位是ToBlinkerWeCom,加上“ ”后面是发电量）
char sendcontrol[13];
int solarpowernum; // 太阳能发电量最高999
double slpshow;    // （太阳能显示，需要将整数转换成小数）
int solarwork;     // 太阳能工作指示
int sendcount;     // 延时发送专用
int whtese;        // 天气测试
int oledroll;      // 显示平滚动翻页
int window, wifiti, blinker, hand;
int ii;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef struct
{
  int zuigaowendu;
  int zuidiwendu;
  int shidu;
  int tianqitubiao;
  int fengjiaodu;
  int dengji;

} tianqixinxi; // 高温最低温和湿度的结构体

tianqixinxi day1, day2;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define BUFFERSIZE 80            // 串口1可以接收的最大字符个数
uint8_t ReceiveBuff[BUFFERSIZE]; // 串口1接收缓冲区
uint8_t Rx_len;                  // 串口1接收完成中断标志，接收到字符长度
extern DMA_HandleTypeDef hdma_usart1_rx;

#define BUFFERSIZE2 80             // 串口2可以接收的最大字符个数
uint8_t ReceiveBuff2[BUFFERSIZE2]; // 串口2接收缓冲区
uint8_t Rx_len2;                   // 串口2接收完成中断标志，接收到字符长度
extern DMA_HandleTypeDef hdma_usart2_rx;
#ifndef youseeme
#error "必要的注释被删除了，程序无法编译。请不要盗版程序"
#endif

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void workshow(void) // 工作显示
{
  if (oledroll < 15) // 第一页显示连接状态
  {
    if (window == 0)
    {
      OLED_Clear();
      window = 1;
    }
    showwifiwr(0, wifiti);
    showblinker(2, blinker);
    OLED_ShowCHinese(0, 4, 79);  // 太
    OLED_ShowCHinese(16, 4, 80); // 阳
    OLED_ShowCHinese(32, 4, 81); // 能
    OLED_ShowCHinese(48, 4, 85); // ：
    if (solarwork == 0)          
    {
      OLED_ShowString(64, 4, "error", 16);
    }
    else if (solarwork == 1)
    {
      OLED_ShowString(64, 4, "close", 16);
    }
    else if (solarwork == 2)
    {
      OLED_ShowString(64, 4, "  ok ", 16);
    }
    OLED_ShowCHinese(0, 6, 82);
    OLED_ShowCHinese(16, 6, 83);
    OLED_ShowCHinese(32, 6, 84);
    OLED_Show6float(48, 6, slpshow, 16);//发电量
    //      OLED_ShowCHinese (0,6,99);
    //      OLED_ShowCHinese (16,6,100);
    //      OLED_ShowCHinese (32,6,101);
    //      OLED_ShowCHinese (48,6,102);
    //      OLED_ShowCHinese (64,6,103);
    //      OLED_ShowCHinese (80,6,104);
    //      OLED_ShowCHinese (96,6,105);
  }
  if (oledroll >= 15 && oledroll < 30) // 第二页显示今天天气预报
  {
    if (window == 1)
    {
      OLED_Clear();
      window = 0;
    }
    OLED_ShowString(0, 0, (unsigned char *)shandiaokongge(showtime), 16);
    OLED_ShowCHinese(0, 2, 86);
    OLED_ShowCHinese(16, 2, 88);
    OLED_ShowCHinese(32, 2, 85);
    WhetherReport(48, 2, day1.tianqitubiao);
    OLED_ShowCHinese(0, 4, 89);
    OLED_ShowCHinese(16, 4, 98);
    OLED_ShownNum(32, 4, day1.zuidiwendu, 2, 16);
    OLED_ShowChar(56, 4, '-', 16);
    OLED_ShownNum(64, 4, day1.zuigaowendu, 2, 16);
    OLED_ShowChar(88, 4, 'C', 16);
    Winddirection(0, 6, day1.fengjiaodu);
    OLED_ShowNum(48, 6, day1.dengji, 2, 16);
    OLED_ShowCHinese(64, 6, 95);
  }
  if (oledroll >= 30 && oledroll < 45) // 第三页显示明天天气预报
  {
    if (window == 0)
    {
      OLED_Clear();
      window = 1;
    }
    OLED_ShowString(0, 0, (unsigned char *)shandiaokongge(showtime), 16);
    OLED_ShowCHinese(0, 2, 87);
    OLED_ShowCHinese(16, 2, 88);
    OLED_ShowCHinese(32, 2, 85);
    WhetherReport(48, 2, day2.tianqitubiao);
    OLED_ShowCHinese(0, 4, 89);
    OLED_ShowCHinese(16, 4, 98);
    OLED_ShownNum(32, 4, day2.zuidiwendu, 2, 16);
    OLED_ShowChar(56, 4, '-', 16);
    OLED_ShownNum(64, 4, day2.zuigaowendu, 2, 16);
    OLED_ShowChar(88, 4, 'C', 16);
    Winddirection(0, 6, day2.fengjiaodu);
    OLED_ShowNum(48, 6, day2.dengji, 2, 16);
    OLED_ShowCHinese(64, 6, 95);
  }
  if (oledroll >= 45 && oledroll < 60) // 第四页显示温湿度检测和空调
  {
    if (window == 1)
    {
      OLED_Clear();
      window = 0;
    }
    OLED_ShowString(0, 0, (unsigned char *)shandiaokongge(showtime), 16);
    OLED_ShowCHinese(0, 2, 126);  // 空
    OLED_ShowCHinese(16, 2, 127); // 调
    OLED_ShowCHinese(32, 2, 128); // 温
    OLED_ShowCHinese(48, 2, 129); // 度
    OLED_ShowNum(64, 2, numact, 2, 16);
    OLED_ShowCHinese(0, 4, 122);  // 室
    OLED_ShowCHinese(16, 4, 123); // 内
    OLED_ShowCHinese(32, 4, 98);  // 温
    OLED_ShowCHinese(48, 4, 91);  // 度
    OLED_ShowNum(64, 4, temp, 2, 16);
    OLED_ShowCHinese(80, 4, 90); // 湿
    OLED_ShowCHinese(96, 4, 91); // 度
    OLED_ShowNum(112, 4, humi, 2, 16);
    OLED_ShowCHinese(0, 6, 124);  // 户
    OLED_ShowCHinese(16, 6, 125); // 外
    OLED_ShowCHinese(32, 6, 98);  // 温
    OLED_ShowCHinese(48, 6, 91);  // 度
    OLED_ShowNum(64, 6, tempout, 2, 16);
    OLED_ShowCHinese(80, 6, 90); // 湿
    OLED_ShowCHinese(96, 6, 91); // 度
    OLED_ShowNum(112, 6, humiout, 2, 16);
  }
  if (oledroll >= 60) // 循环清空
  {
    oledroll = 0;
    window = 0;
  }
}
void conversion(void) // 数据读取
{
  DH11_Task();                    // 温湿度传感器室内检测
  DH11_Task2();                   // 户外温湿度传感器
  temp = DH11_data.temp;          // 室内温度
  humi = DH11_data.humidity;      // 室内湿度
  tempout = DH11_data2.temp2;     // 户外温度
  humiout = DH11_data2.humidity2; // 户外湿度
  // long numctrl, numact,numeco,numwhether,numwind,numdw,numgw;
  numctrl = strtoul((const char *)ReceiveBuff, &leftover, 10); // 提取点灯科技控制
  numact = strtoul(leftover, &leftover2, 10);                  // 提取空调控制
  numeco = strtoul(leftover2, &leftover3, 10);                 // 提取小时时间控制（用于灯光定时）
  numwhether = strtoul(leftover3, &leftover4, 10);             // 提取天气数据（今天明天天气+2天湿度）
  numwind1 = strtoul(leftover4, &leftover5, 10);               // 提取风速数据（今天风向等级）
  numwind2 = strtoul(leftover5, &leftover6, 10);               // 提取风速数据（明天风向等级）
  day1.zuidiwendu = strtoul(leftover6, &leftover7, 10);        // 提取今天高温数据
  day1.zuigaowendu = strtoul(leftover7, &leftover8, 10);       // 提取今天低温数据
  day2.zuidiwendu = strtoul(leftover8, &leftover9, 10);        // 提取明天高温数据
  day2.zuigaowendu = strtoul(leftover9, &leftover10, 10);      // 提取明天低温数据
  wifiti = strtoul(leftover10, &leftover11, 10);               // 提取wifi状态
  blinker = strtoul(leftover11, &leftover12, 10);              // 提取点灯科技状态
  //		if(leftover3[strlen(leftover3)-1] == '\n') {
  //				leftover3[strlen(leftover3)-1] = '\0';
  //		}//(删掉末尾\n)
  strncpy(showtime, leftover12, 15); // 时间复制到安全区然后刷新在屏幕上
  // 接收储存和数据读取结束^
  // 开始数据提取,主要将天气预报存放到结构体v
  day1.tianqitubiao = numwhether / 1000000;
  day1.shidu = numwhether / 10000 % 100;
  day2.tianqitubiao = numwhether / 100 % 100;
  day2.shidu = numwhether % 100;
  day1.fengjiaodu = numwind1 / 100;
  day1.dengji = numwind1 % 100;
  day2.fengjiaodu = numwind2 / 100;
  day2.dengji = numwind2 % 100;
  solarwork = strtoul((const char *)ReceiveBuff2, &slpover, 10); // 提取太阳能
  solarpowernum = strtoul(slpover, &slpover2, 10);               // 提取发电量12345678=====1234.5678
  if(solarpowernum>494950000)
  {
    solarpowernum=494950000;//限制，防止溢出
  }
  slpshow = solarpowernum * 1.0000 / 10000;
  //*dma发送的代码发送数据，目前是串口2
  //*例子代码HAL_UART_Transmit_DMA(&huart2, (uint8_t *)ReceiveBuff, sizeof(ReceiveBuff));
  // ToBlinkerWeCom=1236100000;//测试实例126100000为发电量126，太阳能正常，区域正常
  // HAL_UART_Transmit_DMA(&huart2, "HELL0", 6);	//串口发送（串口3连接企业微信）
}
void at24c02test(void) // 存储测试
{
  for (i = 0; i < 256; i++)
    WriteBuffer[i] = 'n'; /* WriteBuffer init */

  printf("\r\n***************I2C Example Z小旋测试*******************************\r\n");
  for (int j = 0; j < 32; j++)
  {
    if (HAL_I2C_Mem_Write(&hi2c1, ADDR_24LCxx_Write, 8 * j, I2C_MEMADD_SIZE_8BIT, WriteBuffer + 8 * j, 8, 1000) == HAL_OK)
    {
      printf("\r\n EEPROM 24C02 Write Test OK \r\n");
      HAL_Delay(20);
    }
    else
    {
      HAL_Delay(20);
      printf("\r\n EEPROM 24C02 Write Test False \r\n");
    }
  }
  /*
  // wrinte date to EEPROM   如果要一次写一个字节，写256次，用这里的代码
  for(i=0;i<BufferSize;i++)
  {
      HAL_I2C_Mem_Write(&hi2c1, ADDR_24LCxx_Write, i, I2C_MEMADD_SIZE_8BIT,&WriteBuffer[i],1，0xff);//使用I2C块读，出错。因此采用此种方式，逐个单字节写入
    HAL_Delay(5);//此处延时必加，与AT24C02写时序有关
  }
  printf("\r\n EEPROM 24C02 Write Test OK \r\n");
  */

  HAL_I2C_Mem_Read(&hi2c1, ADDR_24LCxx_Read, 0, I2C_MEMADD_SIZE_8BIT, ReadBuffer, BufferSize, 0xff);

  for (i = 0; i < 256; i++)
    // printf("0x%02X  ",ReadBuffer[i]);
    printf("%c  ", ReadBuffer[i]);
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
  // 测试发送数组uint8_t Senbuff[] = "\r\n**** Serial Output Message by DMA ***\r\n   UART DMA Test \r\n   LI Siyu";
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); // 工作指示灯
  HAL_Delay(3000);
  OLED_Init();
  OLED_Clear();
  // 串口一区域改写
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); // 使能串口1 IDLE中断
  /*第二个参数目前为数组 如果为变量需要加取地址符*/
  HAL_UART_Receive_DMA(&huart1, ReceiveBuff, BUFFERSIZE); // 使能接收
  // 串口二区域改写
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE); // 使能串口1 IDLE中断
  /*第二个参数目前为数组 如果为变量需要加取地址符*/
  HAL_UART_Receive_DMA(&huart2, ReceiveBuff2, BUFFERSIZE2); // 使能接收
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);                 // 水泵pwm

  // 打开初始化在这
  initialize();
  for (ii = 0; ii < 20; ii++) // 获取数据稳定
  {
    numctrl = strtoul((const char *)ReceiveBuff, &leftover, 10); // 提取点灯科技控制
    numact = strtoul(leftover, &leftover2, 10);                  // 提取空调控制
    numeco = strtoul(leftover2, &leftover3, 10);                 // 提取小时时间控制（用于灯光定时）
    numwhether = strtoul(leftover3, &leftover4, 10);             // 提取天气数据（今天明天天气+2天湿度）
    numwind1 = strtoul(leftover4, &leftover5, 10);               // 提取风速数据（今天风向等级）
    numwind2 = strtoul(leftover5, &leftover6, 10);               // 提取风速数据（明天风向等级）
    day1.zuidiwendu = strtoul(leftover6, &leftover7, 10);        // 提取今天高温数据
    day1.zuigaowendu = strtoul(leftover7, &leftover8, 10);       // 提取今天低温数据
    day2.zuidiwendu = strtoul(leftover8, &leftover9, 10);        // 提取明天高温数据
    day2.zuigaowendu = strtoul(leftover9, &leftover10, 10);      // 提取明天低温数据
    wifiti = strtoul(leftover10, &leftover11, 10);               // 提取wifi状态
    blinker = strtoul(leftover11, &leftover12, 10);              // 提取点灯科技状态
    HAL_Delay(500);
    DH11_Task();  // 温湿度传感器室内检测
    DH11_Task2(); // 户外温湿度传感器
    OLED_ShowNum(112, 2, ii, 2, 16);
    showwifiwr(4, wifiti);
    showblinker(6, blinker);
  }
  OLED_Clear();
  // at24c02test();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    HAL_GPIO_TogglePin(USERLED_GPIO_Port, USERLED_Pin); // 工作指示
    conversion();
    workshow();
    if (temp > numact) //&&tempout>temp)//空调
    {
      HAL_GPIO_WritePin(FANOUT_GPIO_Port, FANOUT_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(AC_COLD_GPIO_Port, AC_COLD_Pin, GPIO_PIN_RESET);
    }
    else
    {
      HAL_GPIO_WritePin(FANOUT_GPIO_Port, FANOUT_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(AC_COLD_GPIO_Port, AC_COLD_Pin, GPIO_PIN_SET);
    }

    //			solarpowernum++;(先不用了)
    //			if(solarpowernum>999)solarpowernum=999;
    ledtwinkle++;
    sendcount++;
    oledroll++;
#ifndef stcurity
#error "必要的注释被删除了，程序无法编译。请不要盗版程序。"
#endif
     
    /**
     * 从这里开始到下一个标志，这片区域是关于着火有关的执行程序
     * FIREZL1  火焰检测
     * SMOKZL1  烟雾检测
     * DCFZL1   启动电磁阀
     * 由于程序并行，任意一个着火fireloca会被置为1，将在下一个这种注释的标志处被重置。。。
     */
    if (HAL_GPIO_ReadPin(FIREZL1_GPIO_Port, FIREZL1_Pin) == 0 && HAL_GPIO_ReadPin(SMOKZL1_GPIO_Port, SMOKZL1_Pin) == 0)
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 0, 1);
      fireloca = 1;
      HAL_GPIO_WritePin(DCFZL1_GPIO_Port, DCFZL1_Pin, GPIO_PIN_RESET);
      firetest1 = 1;
    } // 走廊1着火双保险,如果触发，置位为1，同时开启对应区域电磁阀
    else
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 0, 0);
      HAL_GPIO_WritePin(DCFZL1_GPIO_Port, DCFZL1_Pin, GPIO_PIN_SET);
      firetest1 = 0;
    } // 火灭了，阀门关闭，置位为0

    if (HAL_GPIO_ReadPin(FIREZL2_GPIO_Port, FIREZL2_Pin) == 0 && HAL_GPIO_ReadPin(SMOKZL2_GPIO_Port, SMOKZL2_Pin) == 0)
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 1, 1);
      fireloca = 1;
      HAL_GPIO_WritePin(DCFZL2_GPIO_Port, DCFZL2_Pin, GPIO_PIN_RESET);
      firetest2 = 1;
    } // 走廊2着火双保险
    else
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 1, 0);
      HAL_GPIO_WritePin(DCFZL2_GPIO_Port, DCFZL2_Pin, GPIO_PIN_SET);
      firetest2 = 0;
    }

    if (HAL_GPIO_ReadPin(FIREZL3_GPIO_Port, FIREZL3_Pin) == 0 && HAL_GPIO_ReadPin(SMOKZL3_GPIO_Port, SMOKZL3_Pin) == 0)
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 2, 1);
      fireloca = 1;
      HAL_GPIO_WritePin(DCFZL3_GPIO_Port, DCFZL3_Pin, GPIO_PIN_RESET);
      firetest3 = 1;
    } // 走廊3着火双保险
    else
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 2, 0);
      HAL_GPIO_WritePin(DCFZL3_GPIO_Port, DCFZL3_Pin, GPIO_PIN_SET);
      firetest3 = 0;
    }

    if (HAL_GPIO_ReadPin(FIREBGL_GPIO_Port, FIREBGL_Pin) == 0 && HAL_GPIO_ReadPin(SMOKBGL_GPIO_Port, SMOKBGL_Pin) == 0)
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 3, 1);
      fireloca = 1;
      HAL_GPIO_WritePin(DCFBGL_GPIO_Port, DCFBGL_Pin, GPIO_PIN_RESET);
      firetest4 = 1;
    } // 办公左着火双保险
    else
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 3, 0);
      HAL_GPIO_WritePin(DCFBGL_GPIO_Port, DCFBGL_Pin, GPIO_PIN_SET);
      firetest4 = 0;
    }

    if (HAL_GPIO_ReadPin(FIREBGR_GPIO_Port, FIREBGR_Pin) == 0 && HAL_GPIO_ReadPin(SMOKBGR_GPIO_Port, SMOKBGR_Pin) == 0)
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 4, 1);
      fireloca = 1;
      HAL_GPIO_WritePin(DCFBGR_GPIO_Port, DCFBGR_Pin, GPIO_PIN_SET);
      firetest5 = 1;
    } // 办公右着火双保险
    else
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 4, 0);
      HAL_GPIO_WritePin(DCFBGR_GPIO_Port, DCFBGR_Pin, GPIO_PIN_RESET);
      firetest5 = 0;
    }
    if(solarwork==0)
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 5, 1); //(太阳能备用)    
    }
    else
    {
      ToBlinkerWeCom = setDigit(ToBlinkerWeCom, 5, 0); //(太阳能备用)    
    }

    /**
     * 从这里开始到下一个标志，这片区域负责将上一片区域的fireloca改为0
     * 效果：USER CODE BEGIN PTD 000处的firetest作用再此
     * 如果全是0代表没有火灾，将fireloca改为0
     *
     */
    if (firetest1 || firetest2 || firetest3 || firetest4 || firetest5)
    {
    }
    else
    {
      fireloca = 0;
    }
    /**
     * 从这里开始到下一个标志，这片区域是关于灯光的控
     * 效果：如果着火，标志位fireloca为1，启动所有灯光，否则执行手动或者safety_light();自动灯光
     *
     *
     */
    if (fireloca == 0 && hand == 0) // 当检测到没有火灾时候，执行正常开关灯流程
    {
      if (numctrl != 0) // 手动控制
      {
        if (numctrl / 10 % 10 == 1)
        {
          HAL_GPIO_WritePin(LTZL1_GPIO_Port, LTZL1_Pin, GPIO_PIN_RESET);
        }
        else
        {
          HAL_GPIO_WritePin(LTZL1_GPIO_Port, LTZL1_Pin, GPIO_PIN_SET);
        } /////////////////////////////////////////////////////////////////
        if (numctrl / 100 % 10 == 1)
        {
          HAL_GPIO_WritePin(LTZL2_GPIO_Port, LTZL2_Pin, GPIO_PIN_RESET);
        }
        else
        {
          HAL_GPIO_WritePin(LTZL2_GPIO_Port, LTZL2_Pin, GPIO_PIN_SET);
        } /////////////////////////////////////////////////////////
        if (numctrl / 1000 % 10 == 1)
        {
          HAL_GPIO_WritePin(LTBGL_GPIO_Port, LTBGL_Pin, GPIO_PIN_RESET);
        }
        else
        {
          HAL_GPIO_WritePin(LTBGL_GPIO_Port, LTBGL_Pin, GPIO_PIN_SET);
        } ////////////////////////////////////////////////////////////////
        if (numctrl / 10000 % 10 == 1)
        {
          HAL_GPIO_WritePin(LTBGR_GPIO_Port, LTBGR_Pin, GPIO_PIN_RESET);
        }
        else
        {
          HAL_GPIO_WritePin(LTBGR_GPIO_Port, LTBGR_Pin, GPIO_PIN_SET);
        }
      }
      else
      {
        safety_light(numeco);
      }                                                          // 自动控制
      HAL_GPIO_WritePin(warn_GPIO_Port, warn_Pin, GPIO_PIN_SET); // （警报关闭）
      pumpdelay = 10;
      HAL_GPIO_WritePin(PUMPCONT1_GPIO_Port, PUMPCONT1_Pin, GPIO_PIN_RESET); // （水泵全关）
      HAL_GPIO_WritePin(PUMPCONT2_GPIO_Port, PUMPCONT2_Pin, GPIO_PIN_RESET); // （水泵全关）
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 000);                     // （转速归零）
    }
    if (fireloca == 1) // 着火了，所有灯全开,
    {
      HAL_GPIO_WritePin(LTZL1_GPIO_Port, LTZL1_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LTZL2_GPIO_Port, LTZL2_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LTBGL_GPIO_Port, LTBGL_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LTBGR_GPIO_Port, LTBGR_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(warn_GPIO_Port, warn_Pin, GPIO_PIN_RESET); // （警报开启，继电器低电触发）
      pumpdelay--;
      if (pumpdelay == 0)
      {
        HAL_GPIO_WritePin(PUMPCONT1_GPIO_Port, PUMPCONT1_Pin, GPIO_PIN_SET);   // （水泵开启）
        HAL_GPIO_WritePin(PUMPCONT2_GPIO_Port, PUMPCONT2_Pin, GPIO_PIN_RESET); // （水泵开启）
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 900);                     // （开启转速）
        pumpdelay = 1;
      }
    }
    if (sendcount == 4) // 数据上传云端
    {
      printf("%09d %d\r\n", ToBlinkerWeCom, solarpowernum);                                  // 重定义目前定义的是串口1(发点灯科技)!!!!! 太阳能要处理
      sprintf(sendcontrol, "%d", ToBlinkerWeCom);                                            // 控制数字转换字符串
      strcat(sendcontrol, "\r\n");                                                           // 拼接\r\n
      HAL_UART_Transmit(&huart3, (unsigned char *)sendcontrol, sizeof(sendcontrol), 0xffff); // 串口3发给企业微信
      sendcount = 0;
    }
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

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#ifndef NECESSARY_COMMENT_PRESENT
#error "必要的注释被删除了，程序无法编译。请不要盗版程序。"
#endif
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
