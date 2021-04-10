#include "stm32f1xx.h"

void Timer_Init(TIM_TypeDef* timer, uint32_t clock_frequency)
{
	/*
	 * ===================
	 * Delay configuration
	 * ===================
	 *
	 * Using TIM4 for delay and timing
	 * implementation
	 */


	/* Configuring timer to get 1kHz frequency */

	/* Enabling TIM4 Clock */
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);

	/* Prescaler setting */
	timer->PSC = 0;

	/* Autoreload register */
	timer->ARR = clock_frequency / 1000;

	/* Interrupt generation */
	SET_BIT(timer->CR1, TIM_CR1_URS);
	SET_BIT(timer->DIER, TIM_DIER_UIE);
	SET_BIT(timer->EGR, TIM_EGR_UG);

	NVIC_EnableIRQ(TIM4_IRQn);
}

void TIM4_IRQHandler(void)
{
	ticks++;
	CLEAR_BIT(TIM4->SR, TIM_SR_UIF);
}

void delay_miliseconds(TIM_TypeDef* timer, unsigned int milis)
{
	/* Uses TIM4 to make delays */

	ticks = 0;
	/* Enabling timer */
	SET_BIT(TIM4->CR1, TIM_CR1_CEN);
	while (ticks < milis);
	/* Disabling timer */
	CLEAR_BIT(TIM4->CR1, TIM_CR1_CEN);
}
