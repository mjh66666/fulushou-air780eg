/*
 *                        _oo0oo_
 *                       o8888888o
 *                       88" . "88
 *                       (| -_- |)
 *                       0\  =  /0
 *                     ___/`---'\___
 *                   .' \\|     |// '.
 *                  / \\|||  :  |||// \
 *                 / _||||| -:- |||||- \
 *                |   | \\\  - /// |   |
 *                | \_|  ''\---/''  |_/ |
 *                \  .-\__  '-'  ___/-. /
 *              ___'. .'  /--.--\  `. .'___
 *           ."" '<  `.___\_<|>_/___.' >' "".
 *          | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *          \  \ `_.   \_ __\ /__ _/   .-` /  /
 *      =====`-.____`.___ \_____/___.-`___.-'=====
 *                        `=---='
 *
 *
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *            佛祖保佑     永不宕机     永无BUG
 *
 * @Author: mojionghao
 * @Date: 2024-04-08 11:17:38
 * @LastEditors: mojionghao
 * @LastEditTime: 2024-10-14 18:15:17
 * @FilePath: \重构v1.3 - 780eg（GPS）没调好6.27\User\main.c
 * @Description:
 */

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
#include <string.h>

float Pitch, Roll, Yaw;
uint8_t i;
uint16_t flag, count = 0;
int fall_value, SPO2, temperature;

int fall_value;
int test1 = 10;
float longtitued, latitude = 0;

int main(void)
{

	int test_int1 = 6, test_int2 = 1;
	// OLED_Init(); // 显示屏初始化
	float test_float = 9.0;

	Usart1_Init(115200); // 串口初始化
	Usart2_Init(115200);

	MPU6050_DMP_Init(); // MPU6050姿态解算初始化

	IIC_GPIO_INIT();   // MAX30102硬件IIC初始化
	MAX30102_GPIO();   // MAX30102GPIO口初始化
	Max30102_reset();  // MAX30102模式初始化 清除模式寄存器Do not use
	MAX30102_Config(); // MAX30102寄存器配置

	/*
	// OLED显示
	// OLED_ShowString(1, 1, "pitch:");
	// OLED_ShowString(2, 1, "roll:");
	// OLED_ShowString(3, 1, "yaw:");

	// OLED_ShowCHinese(1, 5, 12);
	// OLED_ShowCHinese(1, 6, 13);
	// OLED_ShowCHinese(2, 5, 14);
	// OLED_ShowCHinese(2, 6, 15);
	*/

	Air780EG_Init();
	Delay_ms(500);
	Air780EG_Sendonemqttdata(Int, "test2", 0, 0, &test1, "msg"); // 发送上线消息

	Air780EG_GNSSInit();
	Air780EG_sendGNSSdata(NULL, NULL);

	Air780EG_Clear();

	struct Data data1[1];

	strcpy(data1[0].msg, "HI");
	data1[0].type  = Float;
	data1[0].value = (void *)&test_float;

	Air780EG_Sendmqttdata("test2", 0, 0, &data1[0], 1, NULL);

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
		/*oled显示数据
		// OLED_ShowSignedNum(1, 5, Pitch, 3);
		// OLED_ShowSignedNum(2, 5, Roll, 3);
		// OLED_ShowSignedNum(3, 5, Yaw, 3);
		//          UsartPrintf(USART1,"Pitch:%f\r\n",Pitch);
		//          UsartPrintf(USART1,"Roll:%f\r\n",Roll);
		//          UsartPrintf(USART1, "Yaw:%f\r\n", Yaw);
		*/
		count++;
		if (count % 3000 == 0) { // count

			// 构建结构体数组
			struct Data data[3];

			strcpy(data[0].msg, "SPO2");
			data[0].type  = Float;
			data[0].value = (void *)&g_blooddata.SpO2;

			strcpy(data[1].msg, "heart");
			data[1].type  = Int;
			data[1].value = (void *)&g_blooddata.heart;

			strcpy(data[2].msg, "temp");
			data[2].type  = Int;
			data[2].value = (void *)&temperature;

			// 发送结构体数组
			Air780EG_Sendmqttdata("l00000001", 0, 0, data, 3, NULL);
			Air780EG_Clear();

			// Air780EG_Sendtheremqttdata("l00000001", &g_blooddata.SpO2, &g_blooddata.heart, &temperature);

			count = 0;
		}
		if (fabs(Pitch) > 40 || fabs(Roll) > 40 || fabs(Yaw) > 40) {

			fall_value++;
		}
		if (fall_value > 0 && fall_value % 100 == 0) { // 防止跌倒后不断向EMQX发送消息
			fall_value++;
			Air780EG_Sendonemqttdata(Int, "w00000001", 0, 0, &test1, "fall");
			Air780EG_Clear();
			UsartPrintf(USART1, "fall_value:%d\r\n", fall_value);
			fall_value = 0; // 跌倒标志位清0
		}
		// UsartPrintf(USART1, "没有进入判断fall_value:%d\r\n", fall_value);

		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
		flag++;
		if (flag % 5 == 0) {

			blood_Loop();
			temperature = (int)MAX30102_read_temp();
			UsartPrintf(USART_DEBUG, "temp:%d\r\n", temperature);

			UsartPrintf(USART1, "clean\r\n");

			Air780EG_Clear();
			Air780EG_sendGNSSdata(&longtitued, &latitude);

			struct Data GNSS_data[2];

			strcpy(GNSS_data[0].msg, "longitude");
			GNSS_data[0].type  = Float;
			GNSS_data[0].value = (void *)&longtitued;

			strcpy(GNSS_data[1].msg, "latitude");
			GNSS_data[1].type  = Float;
			GNSS_data[1].value = (void *)&latitude;


			UsartPrintf(USART1, "gnssSENDOK\r\n");
			UsartPrintf(USART1, "sendgnss_data!");
			Air780EG_Clear();

			flag = 0;
		}
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}