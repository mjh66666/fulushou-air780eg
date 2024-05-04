#include "stm32f10x.h"                  // Device header

void Timer2_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimerBaseInitstructure;
	TIM_TimerBaseInitstructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimerBaseInitstructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimerBaseInitstructure.TIM_Period = 10000-1;
	TIM_TimerBaseInitstructure.TIM_Prescaler = 72-1;
	TIM_TimerBaseInitstructure.TIM_RepetitionCounter = 0;
  /*
	设置1秒的定时
	f=1/T=100HZ
	100=72M/(72)/(100)[分频后频率为10k，在10k的频率下记10000个数]
  */
	
	TIM_TimeBaseInit(TIM2,&TIM_TimerBaseInitstructure);
	
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_Initstructure;
	NVIC_Initstructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_Initstructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Initstructure.NVIC_IRQChannelPreemptionPriority =1;
	NVIC_Initstructure.NVIC_IRQChannelSubPriority = 0;//1
	
	NVIC_Init(&NVIC_Initstructure);
	
	TIM_Cmd(TIM2,ENABLE);
}

void Timer3_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	TIM_InternalClockConfig(TIM3);
	
	TIM_TimeBaseInitTypeDef TIM_TimerBaseInitstructure;
	TIM_TimerBaseInitstructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimerBaseInitstructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimerBaseInitstructure.TIM_Period = 40000-1;
	TIM_TimerBaseInitstructure.TIM_Prescaler = 18000-1;
	TIM_TimerBaseInitstructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM3,&TIM_TimerBaseInitstructure);
	
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_Initstructure;
	NVIC_Initstructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_Initstructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Initstructure.NVIC_IRQChannelPreemptionPriority =0;
	NVIC_Initstructure.NVIC_IRQChannelSubPriority =0;//0
	
	NVIC_Init(&NVIC_Initstructure);
	
	TIM_Cmd(TIM3,ENABLE);
}
/*
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2,TIM_IT_Update) == RESET)
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}
*/


