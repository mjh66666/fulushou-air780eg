#include "stm32f10x.h" // Device header
#include <stdio.h>
#include "Delay.h"
#include "Fall.h"
#include "OLED.h"
#include "inv_mpu.h"
#include "usart.h"
#include "max30102.h"
#include "xiic.h"
#include "Timer.h"
#include "blood.h"
#include "usart.h"
#include "780eg.h"

/*

 */

float Pitch, Roll, Yaw;
uint8_t i;
uint16_t flag = 0;
int fall_value, SPO2, temperature;

int fall_value = 0;
int test1      = 10;

int main(void)
{

    //	OLED_Init();		//显示屏初始化

    Usart1_Init(115200); // 串口初始化
    Usart2_Init(115200);

    MPU6050_DMP_Init(); // MPU6050姿态解算初始化

    IIC_GPIO_INIT();   // MAX30102硬件IIC初始化
    MAX30102_GPIO();   // MAX30102GPIO口初始化
    Max30102_reset();  // MAX30102模式初始化 清除模式寄存器Do not use
    MAX30102_Config(); // MAX30102寄存器配置

    // OLED显示
    //	OLED_ShowString(1,1,"pitch:");
    //	OLED_ShowString(2,1,"roll:");
    //	OLED_ShowString(3,1,"yaw:");

    //	OLED_ShowCHinese(1,5,12); OLED_ShowCHinese(1,6,13);
    //	OLED_ShowCHinese(2,5,14); OLED_ShowCHinese(2,6,15);

    Air780EG_Init();
    Delay_ms(500);
    Air780EG_Sendmqttdata(Int, "test2", 0, 0, &test1, "msg"); // 发送上线消息

    UsartPrintf(USART_DEBUG, "你好\n");

    Timer2_Init();
    Timer3_Init();

    for (i = 0; i < 128; i++) {
        while (MAX30102_INTPin_Read() == 0) {

            max30102_read_fifo();
        }
    }
    while (1) {
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {

        MPU6050_DMP_Get_Data(&Pitch, &Roll, &Yaw);

        //			OLED_ShowSignedNum(1,5,Pitch,3);
        //			OLED_ShowSignedNum(2,5,Roll,3);
        //			OLED_ShowSignedNum(3,5,Yaw,3);
        //			UsartPrintf(USART1,"Pitch:%f\r\n",Pitch);
        //		    UsartPrintf(USART1,"Roll:%f\r\n",Roll);
        //			UsartPrintf(USART1,"Yaw:%f\r\n",Yaw);

        if (fabs(Pitch) > 40 || fabs(Roll) > 40 || fabs(Yaw) > 40) {
            fall_value++;
        }
        if (fall_value > 0 && fall_value % 200 == 0) { // 防止跌倒后不断向EMQX发送消息
            fall_value++;
            Air780EG_Sendmqttdata(Int, "warning", 0, 0, &test1, "fall");
            Air780EG_Clear();
            UsartPrintf(USART1, "fall_value:%d\r\n", fall_value);
        }

        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        flag++;
        if (flag % 10 == 0) {

            blood_Loop();
            temperature = (int)MAX30102_read_temp();
            UsartPrintf(USART_DEBUG, "temp:%d\r\n", temperature);

            // Air780EG_Sendmqttdata(Float, "lbd", 0, 0, &g_blooddata.SpO2, "SPO2");
            // Delay_ms(100);
            // Air780EG_Sendmqttdata(Int, "lbd", 0, 0, &g_blooddata.heart, "heart");
            // Delay_ms(100);
            // Air780EG_Sendmqttdata(Int, "lbd", 0, 0, &temperature, "temp");
            Air780EG_Sendtheremqttdata("lbd", &g_blooddata.SpO2, &g_blooddata.heart, &temperature);
            Air780EG_Clear();

            fall_value = 0;
            flag       = 0;
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}