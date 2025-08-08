
#include "stm32f10x.h"                 
#include "Delay.h"
#include "Flash.h"
#include "BUZZER.h"
#include "Servo.h"
#include "RC522.h"
#include "sys.h"
#include "oled_iic.h"
#include "stdio.h"
#include "key.h"
#include "timer.h"
#include "as608.h"
#include "usart3.h"

uint8_t cardnumber,KeyNum,tempcard,select=0,flag_scan=1,flag_addcard=0,flag_deletecard=0;

extern uint8_t UID[4],Temp[4];
extern uint8_t UI0[4];							//卡片0ID数组
extern uint8_t UI1[4];							//卡片1ID数组
extern uint8_t UI2[4];							//卡片2ID数组
extern uint8_t UI3[4];							//卡片3ID数组

void RFID_Check(void);
void Read_Card(void);
int openthedoor;
int main(void)
{

	extern const u8 BMP1[];
	HZ= GB16_NUM();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	delay_init();
	usart3_init(57600);
	KEY_Init();
	Buzzer_Init();
	delay_ms(1500);
	OLED_Init();
	OLED_Clear();
	OLED_ShowCH(16,0,"指纹身份认证");
	OLED_ShowCH(16,2,"K1键添加指纹");
	OLED_ShowCH(16,4,"K3键删除指纹");
	OLED_ShowCH(16,6,"K5键验证指纹");	
	Servo_Init();
	RFID_Init();
	
	Servo_SetAngle(0);
	Read_Card();		//上电后从flash中读出已录入的卡数据
	while(1)
	{
		key_num=KEY_Scan(0);
		if(openthedoor==1&&key_num==1)
		{
			key_num=0;
			openthedoor=0;
			OLED_Clear();
			Add_FR();
		}
		if(openthedoor==1&&key_num==3)
		{
			key_num=0;
			openthedoor=0;
			OLED_Clear();
			Del_FR();
		}
		if(key_num==5)
		{
			key_num=0;
			openthedoor=0;
			OLED_Clear();
			OLED_ShowCH(32,2,"请按手指");
			press_FR();
		}			
		if(flag_scan==1)
		{
			RFID_Check();
		}
		if(PBin(15)==0||PAin(8)==0)
		{
		Servo_SetAngle(90);	//舵机旋转90度维持1.5秒
		Buzzer_ON();
		delay_ms(1500); delay_ms(1500);delay_ms(1500);delay_ms(1500); delay_ms(1500);delay_ms(1500);   
		Servo_SetAngle(0);
    Buzzer_OFF();	
		}
	}
}


//读卡函数，读卡并获取卡编号
void RFID_Check()									
{
	cardnumber = Rc522Test();	//获取卡编号
	if(cardnumber == 0)			//如果为0，表示“卡片错误”，系统中没有这张卡
	{
		WaitCardOff();		//等待卡片移开
	}
	else if(cardnumber==1||cardnumber==2||cardnumber==3||cardnumber == 4)			//如果卡编号为1-4，说明是系统中的4张卡
	{	
		openthedoor=1;
		WaitCardOff();		//等待卡片移开
	}	
}

//从flash中读取各卡信息
void Read_Card()
{
	UI0[0]=FLASH_R(FLASH_ADDR1);
	UI0[1]=FLASH_R(FLASH_ADDR1+2);
	UI0[2]=FLASH_R(FLASH_ADDR1+4);
	UI0[3]=FLASH_R(FLASH_ADDR1+6);
	
	UI1[0]=FLASH_R(FLASH_ADDR2);
	UI1[1]=FLASH_R(FLASH_ADDR2+2);
	UI1[2]=FLASH_R(FLASH_ADDR2+4);
	UI1[3]=FLASH_R(FLASH_ADDR2+6);
	
	UI2[0]=FLASH_R(FLASH_ADDR3);
	UI2[1]=FLASH_R(FLASH_ADDR3+2);
	UI2[2]=FLASH_R(FLASH_ADDR3+4);
	UI2[3]=FLASH_R(FLASH_ADDR3+6);
	
	UI3[0]=FLASH_R(FLASH_ADDR4);
	UI3[1]=FLASH_R(FLASH_ADDR4+2);
	UI3[2]=FLASH_R(FLASH_ADDR4+4);
	UI3[3]=FLASH_R(FLASH_ADDR4+6);
}
