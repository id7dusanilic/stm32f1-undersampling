#include "config.h"
#include "stm32f1xx.h"

void Clock_Init(void)
{
	/*
	 * ===================
	 * Clock configuration
	 * ===================
	 *
	 * Configuring clock to use external clock
	 * and raising frequency to 56MHz
	 *
	 * Uses the external 8MHz clock. The clock is raised to
	 * 56MHz, so the ADC clock can be configured to be 14MHz
	 */

	/* Setting the right number of wait states (2) for flash*/
	CLEAR_BIT(FLASH->ACR, FLASH_ACR_LATENCY);
	SET_BIT(FLASH->ACR, FLASH_ACR_LATENCY_1);

	/* Setting the PLL multiplier */
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_PLLMULL);
	SET_BIT(RCC->CFGR, RCC_CFGR_PLLMULL7);

	/* Setting PLL entry clock source */
	SET_BIT(RCC->CFGR, RCC_CFGR_PLLSRC);

	/* APB1 speed must not exceed 36MHz */
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_PPRE1);
	SET_BIT(RCC->CFGR, RCC_CFGR_PPRE1_2);

	/* PREDIV1 division factor 1 */
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_PLLXTPRE_HSE_DIV2);

	/* Selecting PLL as system clock */
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_SW);
	SET_BIT(RCC->CFGR, RCC_CFGR_SW_PLL);

	/* Turning on HSE */
	SET_BIT(RCC->CR, RCC_CR_HSEON);
	while(!(READ_BIT(RCC->CR, RCC_CR_HSERDY)));

	/* Turning off HSI */
	CLEAR_BIT(RCC->CR, RCC_CR_HSION);

	/* Turning on PLL */
	SET_BIT(RCC->CR, RCC_CR_PLLON);
	while(!(READ_BIT(RCC->CR, RCC_CR_PLLRDY)));
}

void DMA1_Init(uint16_t samples[], int number_of_samples)
{
	/*
	 * ============================
	 * DMA1 Channel 1 Configuration
	 * ============================
	 */

	volatile uint32_t tmpreg; // Used for dummy read operations

	/* Enabling DMA1 clock */
	SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
	tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);

	/* Peripheral register address */
	DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));

	/* Memory address */
	DMA1_Channel1->CMAR = (uint32_t)(samples);

	/* Number of data to be transfered */
	DMA1_Channel1->CNDTR = number_of_samples;

	/* Channel priority */
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_PL);

	/* Data transfer direction */
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_DIR);

	/* Memory increment mode */
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_MINC);

	/* Peripheral increment mode */
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_PINC);

	/* Circular mode */
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_CIRC);

	/* Memory data size */
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_MSIZE);
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_MSIZE_0);

	/* Peripheral data size */
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_PSIZE);
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_PSIZE_0);

	/* Interrupt */
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_TCIE);
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	NVIC_SetPriority(DMA1_Channel1_IRQn, 0);

	/* Activate the channel */
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);

	DUMMY(tmpreg);
}

void ADC1_Init(void)
{
	/*
	 * ==================
	 * ADC1 Configuration
	 * ==================
	 */

	volatile uint32_t tmpreg; // Used for dummy read operations

	/* ADC1 clock prescaler */
	/* Maximum ADC clock frequency is 14MHz */
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_ADCPRE);
	SET_BIT(RCC->CFGR, RCC_CFGR_ADCPRE_DIV4);

	/* Enabling ADC1 clock */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);

	DUMMY(tmpreg);

	/* Configuring conversion sequence */
	CLEAR_BIT(ADC1->SQR3, ADC_SQR3_SQ1);

	/* Right alignment */
	CLEAR_BIT(ADC1->CR2, ADC_CR2_ALIGN);

	/* Sample rate 1.5 cycles on channel0 */
	CLEAR_BIT(ADC1->SMPR2, ADC_SMPR2_SMP0);

	/* Configuring continuous mode */
	SET_BIT(ADC1->CR2, ADC_CR2_CONT);

	/* Enabling DMA mode */
	SET_BIT(ADC1->CR2, ADC_CR2_DMA);

	/* Enabling ADC1 */
	SET_BIT(ADC1->CR2, ADC_CR2_ADON);
	for(int i=0; i<10000; i++);

	/* Calibrating */
	SET_BIT(ADC1->CR2, ADC_CR2_CAL);
	while(READ_BIT(ADC1->CR2, ADC_CR2_CAL));
}

void GPIO_Init(void)
{
	/*
	 * ==================
	 * GPIO Configuration
	 * ==================
	 */

	volatile uint32_t tmpreg; // Used for dummy read operations

	/* GPIOA */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);


	/*
	 * =================
	 * PA0 Configuration
	 * =================
	 *
	 * Used as an analog input (ADC1 Channel 0)
	 * Needs to be configured as a floating input
	 * to be used as an alternate function input
	 */

	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE0_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE0_1);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF0_1);
	SET_BIT(GPIOA->CRL, GPIO_CRL_CNF0_0);

	DUMMY(tmpreg);
}
