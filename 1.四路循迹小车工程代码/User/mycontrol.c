#include "stm32f10x.h"
#include "Delay.h"
#include "Trace.h"
#include "MOTOR.h"





//Ñ°¼£ÈÎÎñ
void Trace_task(void)
{

	if(X1==0&&X3==0) Set_Car_Speed(4000,4000);
		
	if(X1==1&&X3==0) Set_Car_Speed(4200,0);
		
	if(X1==0&&X3==1) Set_Car_Speed(0,4200);
		
	if(X2==0&&X1==1&&X3==1&&X4==1) Set_Car_Speed(0,4800);
		
	if(X2==1&&X1==1&&X3==1&&X4==0) Set_Car_Speed(4800,0);
	
	//if(X2==1&&X1==1&&X3==1&&X4==1) Set_Car_Speed(4000,4000);

}

