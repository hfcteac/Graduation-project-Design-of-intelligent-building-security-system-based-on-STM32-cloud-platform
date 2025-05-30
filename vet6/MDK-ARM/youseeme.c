#include <security.h>
#include <youseeme.mp3>
#include <lisiyu.iloveyou>
#include "gpio.h"
#include "usart.h"


/**
* 从这里开始到下一个标志，这片区域是自动行人灯光控制
* 效果：如果有行人经过，自动开启对应区域灯光。经过后灯光自动关闭
* 
* 
*/	
void safety_light(int time)//安全时候执行正常开灯
{
		if(time>6&&time<16)
		{
				HAL_GPIO_WritePin(LTZL1_GPIO_Port ,LTZL1_Pin ,GPIO_PIN_SET);
				HAL_GPIO_WritePin(LTZL2_GPIO_Port ,LTZL2_Pin ,GPIO_PIN_SET);
				HAL_GPIO_WritePin(LTBGL_GPIO_Port ,LTBGL_Pin ,GPIO_PIN_SET);
				HAL_GPIO_WritePin(LTBGR_GPIO_Port ,LTBGR_Pin ,GPIO_PIN_SET);
		}
		else
		{
			if(HAL_GPIO_ReadPin(PEOPLEZL1_GPIO_Port ,PEOPLEZL1_Pin)==1)
			{
				HAL_GPIO_WritePin(LTZL1_GPIO_Port ,LTZL1_Pin ,GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(LTZL1_GPIO_Port ,LTZL1_Pin ,GPIO_PIN_SET);
			}//1传感器1灯
			if(HAL_GPIO_ReadPin(PEOPLEZL2_GPIO_Port ,PEOPLEZL2_Pin)==1)
			{
				HAL_GPIO_WritePin(LTZL2_GPIO_Port ,LTZL2_Pin ,GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(LTZL2_GPIO_Port ,LTZL2_Pin ,GPIO_PIN_SET);
			}//2传感器2灯			
			if(HAL_GPIO_ReadPin(PEOPLEBGL_GPIO_Port ,PEOPLEBGL_Pin)==1)
			{
				HAL_GPIO_WritePin(LTBGL_GPIO_Port ,LTBGL_Pin ,GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(LTBGL_GPIO_Port ,LTBGL_Pin ,GPIO_PIN_SET);
			}//左边办公室左边灯
			if(HAL_GPIO_ReadPin(PEOPLEBGR_GPIO_Port ,PEOPLEBGR_Pin)==1)
			{
				HAL_GPIO_WritePin(LTBGR_GPIO_Port ,LTBGR_Pin ,GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(LTBGR_GPIO_Port ,LTBGR_Pin ,GPIO_PIN_SET);
			}//右边办公室右边灯
		}
}

void initialize(void)
{
  //宋楠毕设
    OLED_ShowCHinese(0, 0, 0);			  
    OLED_ShowCHinese(16, 0, 1);
    OLED_ShowCHinese(32, 0, 2);			  
    OLED_ShowCHinese(48, 0, 3);
  //指导教师常凤筠
    HAL_Delay(1000);
    OLED_ShowCHinese(0, 2, 5);			  
    OLED_ShowCHinese(16, 2, 6);
    OLED_ShowCHinese(32, 2, 7);			  
    OLED_ShowCHinese(48, 2, 8);	
    OLED_ShowCHinese(64, 2, 9);			  
    OLED_ShowCHinese(80, 2, 10);
    OLED_ShowCHinese(96, 2, 11);
  //鸣谢：李思雨
    HAL_Delay(1000);
    OLED_ShowCHinese(0, 4, 130);			  
    OLED_ShowCHinese(16, 4, 131);
    OLED_ShowCHinese(32, 4, 85);			  
    OLED_ShowCHinese(48, 4, 41);
    OLED_ShowCHinese(64, 4, 42);			  
    OLED_ShowCHinese(80, 4, 43);
	//模块连接正常
    HAL_Delay(1000);
    OLED_ShowCHinese(0, 6, 16);			  
    OLED_ShowCHinese(16, 6, 17);
    OLED_ShowCHinese(32, 6, 18);			  
    OLED_ShowCHinese(48, 6, 19);
    OLED_ShowCHinese(64, 6, 20);			  
    OLED_ShowCHinese(80, 6, 21);
  //请不要断电
    HAL_Delay(1000);
    OLED_Clear();
    OLED_ShowCHinese(0, 0, 22);			  
    OLED_ShowCHinese(16, 0, 25);
    OLED_ShowCHinese(32, 0, 26);			  
    OLED_ShowCHinese(48, 0, 27);	
    OLED_ShowCHinese(64, 0, 29);		    
  //初始化中请等待
    HAL_Delay(1000);
    OLED_ShowCHinese(0, 2, 12);			  
    OLED_ShowCHinese(16, 2, 13);
    OLED_ShowCHinese(32, 2, 14);			  
    OLED_ShowCHinese(48, 2, 15);
    OLED_ShowCHinese(64, 2, 22);			  
    OLED_ShowCHinese(80, 2, 23);	
    OLED_ShowCHinese(96, 2, 24);
}

//等比例缩放函数	
u32 cal_map(u32 x, u32 in_min, u32 in_max, u32 out_min, u32 out_max)
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

//对应数值修改函数，用于将火情传出去
int setDigit(int num, int digit, int value) {   //num 进行修改的数 digit 位数  value 对该位置赋值0~9
    int new_num = 0;
    int place_value = pow(10, digit); // 计算位数对应的数值

    // 将原数字对应位清零
    new_num = num - (num / place_value % 10) * place_value;
    // 设置新值
    new_num += value * place_value;

    return new_num;
}

void WhetherReport(int x,int y,int wh)//天气预报显示（最大x从48开始，否则数组越界）
{
    switch(wh)
    {
      case 0: OLED_ShowWte(x,y,0); OLED_ShowWte(x+16,y,1); OLED_ShowWte(x+32,y,4); OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//白天晴
      case 1: OLED_ShowWte(x,y,2); OLED_ShowWte(x+16,y,3); OLED_ShowWte(x+32,y,4); OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//夜晚晴
      case 2: OLED_ShowWte(x,y,0); OLED_ShowWte(x+16,y,1); OLED_ShowWte(x+32,y,4); OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//国网白天
      case 3: OLED_ShowWte(x,y,2); OLED_ShowWte(x+16,y,3); OLED_ShowWte(x+32,y,4); OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//国外夜晚
      case 4: OLED_ShowWte(x,y,7); OLED_ShowWte(x+16,y,8); OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//多云
      case 5: OLED_ShowWte(x,y,4); OLED_ShowWte(x+16,y,5); OLED_ShowWte(x+32,y,6); OLED_ShowWte(x+48,y,7); OLED_ShowWte(x+64,y,8); break;//晴有时多云
      case 6: OLED_ShowWte(x,y,4); OLED_ShowWte(x+16,y,5); OLED_ShowWte(x+32,y,6); OLED_ShowWte(x+48,y,7); OLED_ShowWte(x+64,y,8); break;//夜晚晴有时多云
      case 7: OLED_ShowWte(x,y,9); OLED_ShowWte(x+16,y,10);OLED_ShowWte(x+32,y,7); OLED_ShowWte(x+48,y,8); OLED_ShowWte(x+64,y,50);break;//大部多云
      case 8: OLED_ShowWte(x,y,9); OLED_ShowWte(x+16,y,10);OLED_ShowWte(x+32,y,7); OLED_ShowWte(x+48,y,8); OLED_ShowWte(x+64,y,50);break;//夜晚大部多云
      case 9: OLED_ShowWte(x,y,11);OLED_ShowWte(x+16,y,50);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//阴
      case 10:OLED_ShowWte(x,y,12);OLED_ShowWte(x+16,y,13);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//阵雨
      case 11:OLED_ShowWte(x,y,14);OLED_ShowWte(x+16,y,15);OLED_ShowWte(x+32,y,16);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//雷阵雨
      case 12:OLED_ShowWte(x,y,14);OLED_ShowWte(x+16,y,15);OLED_ShowWte(x+32,y,16);OLED_ShowWte(x+48,y,17);OLED_ShowWte(x+64,y,18);break;//雷阵雨冰雹
      case 13:OLED_ShowWte(x,y,19);OLED_ShowWte(x+16,y,20);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//小雨
      case 14:OLED_ShowWte(x,y,21);OLED_ShowWte(x+16,y,20);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//中雨
      case 15:OLED_ShowWte(x,y,22);OLED_ShowWte(x+16,y,20);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//大雨
      case 16:OLED_ShowWte(x,y,23);OLED_ShowWte(x+16,y,20);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//暴雨
      case 17:OLED_ShowWte(x,y,22);OLED_ShowWte(x+16,y,23);OLED_ShowWte(x+32,y,20);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//大暴雨
      case 18:OLED_ShowWte(x,y,24);OLED_ShowWte(x+16,y,22);OLED_ShowWte(x+32,y,23);OLED_ShowWte(x+48,y,20);OLED_ShowWte(x+64,y,50);break;//特大暴雨
      case 19:OLED_ShowWte(x,y,25);OLED_ShowWte(x+16,y,20);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//冻雨
      case 20:OLED_ShowWte(x,y,20);OLED_ShowWte(x+16,y,26);OLED_ShowWte(x+32,y,27);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//雨夹雪
      case 21:OLED_ShowWte(x,y,12);OLED_ShowWte(x+16,y,27);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//阵雪
      case 22:OLED_ShowWte(x,y,19);OLED_ShowWte(x+16,y,27);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//小雪
      case 23:OLED_ShowWte(x,y,21);OLED_ShowWte(x+16,y,27);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//中雪
      case 24:OLED_ShowWte(x,y,22);OLED_ShowWte(x+16,y,27);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//大雪
      case 25:OLED_ShowWte(x,y,23);OLED_ShowWte(x+16,y,27);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//暴雪
      case 26:OLED_ShowWte(x,y,28);OLED_ShowWte(x+16,y,29);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//浮尘
      case 27:OLED_ShowWte(x,y,30);OLED_ShowWte(x+16,y,29);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//扬尘
      case 28:OLED_ShowWte(x,y,32);OLED_ShowWte(x+16,y,33);OLED_ShowWte(x+32,y,34);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//沙尘暴
      case 29:OLED_ShowWte(x,y,31);OLED_ShowWte(x+16,y,32);OLED_ShowWte(x+32,y,33);OLED_ShowWte(x+48,y,34);OLED_ShowWte(x+64,y,50);break;//强沙尘暴
      case 30:OLED_ShowWte(x,y,35);OLED_ShowWte(x+16,y,50);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//雾
      case 31:OLED_ShowWte(x,y,36);OLED_ShowWte(x+16,y,50);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//霾
      case 32:OLED_ShowWte(x,y,38);OLED_ShowWte(x+16,y,50);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//风
      case 33:OLED_ShowWte(x,y,22);OLED_ShowWte(x+16,y,38);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//大风
      case 34:OLED_ShowWte(x,y,37);OLED_ShowWte(x+16,y,38);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//飓风
      case 35:OLED_ShowWte(x,y,39);OLED_ShowWte(x+16,y,40);OLED_ShowWte(x+32,y,41);OLED_ShowWte(x+48,y,42);OLED_ShowWte(x+64,y,50);break;//热带风暴
      case 36:OLED_ShowWte(x,y,43);OLED_ShowWte(x+16,y,44);OLED_ShowWte(x+32,y,45);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//龙卷风
      case 37:OLED_ShowWte(x,y,46);OLED_ShowWte(x+16,y,50);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//冷
      case 38:OLED_ShowWte(x,y,47);OLED_ShowWte(x+16,y,50);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//热
      case 99:OLED_ShowWte(x,y,48);OLED_ShowWte(x+16,y,49);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//未知
      default:OLED_ShowWte(x,y,48);OLED_ShowWte(x+16,y,49);OLED_ShowWte(x+32,y,50);OLED_ShowWte(x+48,y,50);OLED_ShowWte(x+64,y,50);break;//未知
    }
}

void Winddirection(int x,int y,int wind)//风向显示(下一位要从48开始)
{
  if(wind==0||wind==360)//北
  {
    OLED_Showwind(x,y,3);OLED_Showwind(x+16,y,4);OLED_Showwind(x+32,y,5);
  }
  else if(wind>0&&wind<90)//东北
  {
    OLED_Showwind(x,y,0);OLED_Showwind(x+16,y,3);OLED_Showwind(x+32,y,4); 
  }
  else if(wind==90)//东
  {
    OLED_Showwind(x,y,0);OLED_Showwind(x+16,y,4);OLED_Showwind(x+32,y,5);  
  }
  else if(wind>90&&wind<180)//东南
  {
    OLED_Showwind(x,y,0);OLED_Showwind(x+16,y,1);OLED_Showwind(x+32,y,4);    
  }
  else if(wind==180)//南
  {  
    OLED_Showwind(x,y,1);OLED_Showwind(x+16,y,4);OLED_Showwind(x+32,y,5);    
  }
  else if(wind>180&&wind<270)//西南
  {
    OLED_Showwind(x,y,2);OLED_Showwind(x+16,y,1);OLED_Showwind(x+32,y,4);    
  }
  else if(wind==270)//西
  {  
    OLED_Showwind(x,y,2);OLED_Showwind(x+16,y,4);OLED_Showwind(x+32,y,5);    
  }
  else if(wind>270&&wind<360)//西北
  {
    OLED_Showwind(x,y,2);OLED_Showwind(x+16,y,3);OLED_Showwind(x+32,y,4);    
  }
}

void showwifiwr(int y,int wire)//因为过大，只允许设置y和值，显示wifi状态
{
    OLED_Showwifi(0,y,32);OLED_Showwifi(16,y,33);OLED_Showwifi(32,y,26);//：
    switch(wire)
    {
      case 0: OLED_Showwifi(48,y,0);OLED_Showwifi(64,y,1);OLED_Showwifi(80,y,2);OLED_Showwifi(96,y,3);break;//正在连接
      case 1: OLED_Showwifi(48,y,4);OLED_Showwifi(64,y,5);OLED_Showwifi(80,y,6);OLED_Showwifi(96,y,7);break;//没找到
      case 2: OLED_Showwifi(48,y,27);OLED_Showwifi(64,y,27);OLED_Showwifi(80,y,27);OLED_Showwifi(96,y,27);break;    
      case 3: OLED_Showwifi(48,y,8);OLED_Showwifi(64,y,9);OLED_Showwifi(80,y,10);OLED_Showwifi(96,y,11);break;//连接成功
      case 4: OLED_Showwifi(48,y,12);OLED_Showwifi(64,y,13);OLED_Showwifi(80,y,14);OLED_Showwifi(96,y,15);break;//连接失败
      case 5: OLED_Showwifi(48,y,16);OLED_Showwifi(64,y,17);OLED_Showwifi(80,y,18);OLED_Showwifi(96,y,19);break;//丢失
      case 6: OLED_Showwifi(48,y,20);OLED_Showwifi(64,y,21);OLED_Showwifi(80,y,22);OLED_Showwifi(96,y,23);break;//断开      
      default :OLED_Showwifi(48,y,28);OLED_Showwifi(64,y,29);OLED_Showwifi(80,y,30);OLED_Showwifi(96,y,31);//模块损坏
    }
}
void showblinker(int y,int wire)//因为过大，只允许设置y和值，显示点灯状态
{
    OLED_Showblk(0,y,0);OLED_Showblk(16,y,1);OLED_Showblk(32,y,4);//：
    switch(wire)
    {
      case 0: OLED_Showblk(48,y,5);OLED_Showblk(64,y,6);OLED_Showblk(80,y,9);OLED_Showblk(96,y,10);break;//连接失败
      case 1: OLED_Showblk(48,y,5);OLED_Showblk(64,y,6);OLED_Showblk(80,y,7);OLED_Showblk(96,y,8);break;//连接成功   
      default :OLED_Showblk(48,y,11);OLED_Showblk(64,y,12);OLED_Showblk(80,y,13);OLED_Showblk(96,y,14);//未知错误
    }
}
char* shandiaokongge(char *str)//删去字符串前面的空格
{
    char *s=str,*p=str;
    if(str==NULL) 
        return NULL;
    while(*s!='\0'&&*s==' ') 
        s++;/*跳过开头的空格*/
    while(*s!='\0') 
        *p++=*s++;/*复制剩余字符到新位置,p总是前于s刚才跳过空格个数那么多位置*/
    *p='\0';/*字串结束标识符*/
    return str;
}
