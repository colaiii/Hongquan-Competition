#include "stm32f10x.h"
#include "Delay.h"
#include "robot.h"
#include "Trace.h"
#include "Key.h"


int Lap_Count = 0; // 当前圈数


#define SPEED_NORMAL 70
#define SPEED_TURN   70
#define TIME_TICK    1 // 每次循迹循环的延时(ms)
#define BLIND_TIME 15000



/**
 * @brief 基础循迹逻辑 (单步)
 * @return 1 表示检测到特殊路口(如十字/全黑)，0 表示普通循迹
 */
void Track_Step(void)
{
    // 读取传感器状态 (0:白/无, 1:黑/有)
    int s1 = X1;
    int s2 = X2;
    int s3 = X3;
    int s4 = X4;

    if (s2 == 1 && s3 == 1) // 直行
    {
        makerobo_run(SPEED_NORMAL, 0);
    }
    else if (s2 == 1 && s3 == 0) // 左偏，向左修
    {

        makerobo_Left(SPEED_TURN, 0);
    }
    else if (s2 == 0 && s3 == 1) // 右偏，向右修
    {
        makerobo_Right(SPEED_TURN, 0);
    }
    else if (s2 == 0 && s3 == 0) // 丢失直线 或 遇到特殊情况
    {
        // 当x2和x3都为0时继续右拐，让车重回轨道
        if (s4 == 1) 
        {
            // 如果X4有信号，强烈右转
            makerobo_Right(SPEED_TURN, 0);
			Delay_ms(200);
        }
        else if (s1 == 1)
        {
            // 如果X1有信号，强烈左转
            makerobo_Left(SPEED_TURN, 0);
			Delay_ms(200);
        }
    }
}

/**
 * @brief 带着条件运行循迹，直到条件满足
 * @param condition_func 返回1则停止循环
 */
typedef int (*ConditionFunc)(void);

void Track_Until(ConditionFunc func)
{
    while (1)
    {
        if (func()) break;
        Track_Step();
        Delay_ms(TIME_TICK);
    }
}

/**
 * @brief 延时等待一段盲走时间，通常用于冲过路口避免重复检测
 */
void Blind_Forward(int ms)
{
    makerobo_run(SPEED_NORMAL, 0);
    Delay_ms(ms);
}



// 强行右转90度或直到检测到线
void Turn_Right_Spin(void)
{
    // 先盲转一小会儿脱离当前线
    makerobo_Right(SPEED_TURN, 0);
    Delay_ms(500); // 根据车速调整

    // 继续转直到中间传感器检测到线
    while (1)
    {
        makerobo_Right(SPEED_TURN, 0);
        if (X2 == 1 || X3 == 1) break;
        Delay_ms(1);
    }
}

// 检测到X4变黑 (用于识别岔路/直角)
int Is_X4_Black(void)
{
    // 简单的滤波可以加在这里，暂时直接返回
    return (X4 == 1);
}

// 检测到停车线 (例如 X1+X2+X3+X4 或者 X1+X4 同时黑)
int Is_Stop_Line(void)
{
    if (X1 == 1 && X4 == 1) return 1;
    return 0;
}



int main(void)
{
    robot_Init();
    Key_Init();
    Trace_Init();

    // 按键启动
    while (Key_GetNum() == 0);
    Delay_ms(1000); // 延时启动

    // ======================== 第一圈 ========================
    Lap_Count = 1;

    // 1. 起步直行，离开起跑线
    Blind_Forward(500); 

    // 2. 寻找第一个直角弯 
    while(1) 
    {
        Track_Step();
        if (X4 == 1 && X3 == 1) 
        {
            Turn_Right_Spin();
            break;
        }
        Delay_ms(TIME_TICK);
    }

    // 3. 寻找六边形入口 (直角弯后的下一个岔路)
    Blind_Forward(200);
    while(1)
    {
        Track_Step();
        if (X4 == 1) // 检测到右侧岔路
        {
             Blind_Forward(1000); 
             Turn_Right_Spin(); 
             break;
        }
        Delay_ms(TIME_TICK);
    }


    Track_Until(Is_X4_Black); 


    while(1)
    {
        Track_Step();
        if (Is_Stop_Line()) 
        {
            // 检测到起点线，跑完一圈
            break;
        }
        Delay_ms(TIME_TICK);
    }

    // ======================== 第二圈 ========================
    Lap_Count = 2;
    Blind_Forward(500); // 冲过起跑线

    // 1. 寻找直角弯
    while(1) 
    {
        Track_Step();
        if (X4 == 1 && X3 == 1) { Turn_Right_Spin(); break; }
        Delay_ms(TIME_TICK);
    }
    Blind_Forward(200);

    // 2. 寻找六边形入口
    {
        Track_Step();
        if (X4 == 1) 
        { 
             Blind_Forward(1200); 
             Turn_Right_Spin(); 
             break;
        }
        Delay_ms(TIME_TICK);
    }

    // 盲走一段时间，避开六边形和波浪路
    long safe_time = 0;
    while(safe_time < BLIND_TIME) // 20秒盲区，只循迹，不判断分叉
    {
        Track_Step();
        Delay_ms(1);
        safe_time++;
    }
    
    // 3.2 现在的 X4 信号应该是 内外圈分叉口
    while(1)
    {
        // 果直行和右转同时存在，优先右转
        int s2 = X2; int s3 = X3; int s4 = X4;
        
        if (s4 == 1) // 发现右边有路
        {
            Turn_Right_Spin();
            break; // 进入内圈模式
        }
        
        // 否则普通循迹
        Track_Step();
        Delay_ms(TIME_TICK);
    }

    // 4. 出内圈并入主路（T字路口右转）
    Blind_Forward(200); // 避开进圈线
    
    while(1)
    {
        Track_Step();
        // 检测到横线 (T字口)
        if (Is_Stop_Line()) 
        {
            // T字口执行右转
            Blind_Forward(200); // 把车头探出去一点
            Turn_Right_Spin();  // 右转并入主路
            break;
        }
        Delay_ms(TIME_TICK);
    }
    
    Blind_Forward(100); // 避开T字线

    // ======================== 停车 ========================
    while(1)
    {
        if (Is_Stop_Line())
        {
            makerobo_brake(0); // 停车
            break;
        }
        
        Track_Step();
        Delay_ms(TIME_TICK);
    }

    while(1); // 结束
}
