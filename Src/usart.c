#define DUMMY(X) (void)X

#include "stm32f1xx.h"
#include <stdio.h>


void USART1_EnableTx(void)
{
	SET_BIT(USART1->CR1, USART_CR1_TE);
}

void USART1_DisableTx(void)
{
	while(!(READ_BIT(USART1->SR, USART_SR_TC)));
	CLEAR_BIT(USART1->CR1, USART_CR1_TE);
}

void USART1_Tx_byte(uint8_t *bytes, uint32_t count)
{
	for(uint32_t i=0; i<count; i++)
	{
		while(!(READ_BIT(USART1->SR, USART_SR_TXE)));
		USART1->DR = bytes[i];
	}
}

void USART1_Tx_string(char *buffer)
{
	while(*buffer)
	{
		while(!(READ_BIT(USART1->SR, USART_SR_TXE)));
		USART1->DR = *buffer++;
	}
}

void USART1_Tx_int(int value)
{
	char buffer[16] = "";
	sprintf(buffer, "%d", value);
	USART1_Tx_string(buffer);
}

void USART1_Tx_float(float value)
{
	char buffer[16] = "";
	snprintf(buffer, 16, "%.2f", value);
	USART1_Tx_string(buffer);
}

void USART1_Set_Baud_Rate(uint32_t clock_speed, uint32_t baud_rate)
{
	uint32_t integer_part;
	uint32_t fractional_part;
	uint32_t tmp;

	integer_part = (25 * clock_speed) / (4 * baud_rate);
	tmp = (integer_part / 100) << 4;
	fractional_part = integer_part - (100 * (tmp >> 4));
	tmp |= (((fractional_part * 16) + 50) / 100) & ((uint8_t)0x0F);

	USART1->BRR = (uint16_t)tmp;
}

void USART1_Init(uint32_t clock_speed, uint32_t baud_rate)
{
	/*
	 * ====================
	 * USART1 Configuration
	 * ====================
	 *
	 * PA9  - Tx
	 * PA10 - Rx
	 *
	 * data word - 8
	 * stop bits - 1
	 * parity    - none
	 */

	volatile uint32_t tmpreg; // Used for dummy read operations

	/* Enabling GPIOA port clock */
	if(!READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN))
	{
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
		tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
	}

	/* Enabling USART1 clock */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);


	/* Configuring PA9 - Tx */
	// Output mode, max speed 10MHz        - MODE9[1:0] = 01
	// Alternate function output Push-pull - CNF9[1:0]  = 10
	SET_BIT(GPIOA->CRH, GPIO_CRH_MODE9_0);
	CLEAR_BIT(GPIOA->CRH, GPIO_CRH_MODE9_1);

	SET_BIT(GPIOA->CRH, GPIO_CRH_CNF9_1);
	CLEAR_BIT(GPIOA->CRH, GPIO_CRH_CNF9_0);

	/* Configuring PA10 - Rx */
	// Input mode     - MODE10[1:0] = 00
	// Floating input - CNF10[1:0]  = 01
	CLEAR_BIT(GPIOA->CRH, GPIO_CRH_MODE10_0);
	CLEAR_BIT(GPIOA->CRH, GPIO_CRH_MODE10_1);
	CLEAR_BIT(GPIOA->CRH, GPIO_CRH_CNF10_1);
	SET_BIT(GPIOA->CRH, GPIO_CRH_CNF10_0);

	/* Enabling USART1 */
	SET_BIT(USART1->CR1, USART_CR1_UE);

	/* Setting baud rate to 115200 */
	USART1_Set_Baud_Rate(clock_speed, baud_rate);

	DUMMY(tmpreg);
}
