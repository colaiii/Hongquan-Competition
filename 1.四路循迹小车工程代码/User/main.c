#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "TIM3_PWM.h"
#include "MOTOR.h"
#include "OLED.h"
#include "mycontrol.h"
#include "Trace.h"




/**** main 主函数代码 ***

*** 作者：陈加油嗯 ***

***关注微信公众号学习更多单片机知识：微信搜索“陈加油嗯” ***

*/
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	TIM3_PWM_Init(7199,1);   //定时器PWM模式初始化(5KHZ)，用于控制L298N电机调速
	MOTOR_GPIO_Init();       //接L298N引脚IN1到IN4，控制电机正反转
	LED_Init();              //LED灯初始化
	
	OLED_Init();          //OLED初始化
	OLED_ColorTurn(0);    //0正常显示，1 反色显示
	OLED_DisplayTurn(0);  //0正常显示 1 屏幕翻转显示
	OLED_Clear();         //清屏
	OLED_Refresh();       //更新显存到OLED(即刷新刚写入的数据,刷新屏幕)
	//OLED静态显示字符
	OLED_ShowString(0,0,"chenjiayou!",16,1);
	OLED_ShowString(0,32,"SR04:",16,1);  //超声波数据
	OLED_Refresh();             //更新显存到OLED(即刷新刚写入的数据,刷新屏幕)
	
	Trace_Init();  //循迹模块初始化
	
	while(1)
	{
		
		Trace_task();   //循迹
		
	}
}

