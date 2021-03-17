#include "main.h"
#include "usart.h"

/* Global variables */
volatile uint16_t sample_value;
volatile uint16_t histogram[NUM_CODES] = {0};
volatile uint16_t sample_count = 0;

volatile int PC13_state = 0;
volatile int PC14_state = 0;

volatile int finished_sampling = 0;
volatile int data_sent = 0;

/* Function declarations */
static void Clock_Init(void);
static void GPIO_Init(void);
static void ADC1_Init(void);
static void PC13_Init(void);
static void PC14_Init(void);

void ADC1_2_IRQHandler(void)
{
	if(READ_BIT(ADC1->SR, ADC_SR_EOC))
	{
		sample_value = ADC1->DR & ADC_DR_DATA;
		histogram[sample_value]++;
		sample_count++;
		if(NUM_SAMPLES == sample_count)
			finished_sampling = 1;
	}
}

int main(void)
{
	/* Initialize everything */
	Clock_Init();
	GPIO_Init();
	ADC1_Init();
	PC13_Init();
	PC14_Init();
	USART1_Init();

	while (1)
	{
		// Emulating ADC Sampling clock
		if(!finished_sampling)
		{
			for(volatile int i=0; i<50000; i++);
			PC14_toggle();
		}

		// Sending data over UART
		if(finished_sampling && !data_sent)
		{
			USART1_EnableTx();
			for(int i=0; i<NUM_CODES; i++)
			{
				USART1_Tx_int(histogram[i]);
				USART1_Tx_string("\r\n");
			}
			USART1_DisableTx();
			SET_BIT(GPIOC->BSRR, GPIO_BSRR_BS14);
			data_sent = 1;
		}
	}
}

static void Clock_Init(void)
{
	/*
	 * Configuring clock to use external clock
	 * and raising frequency to 72MHz
	 */

	/* Setting the right number of wait states for flash*/
	CLEAR_BIT(FLASH->ACR, FLASH_ACR_LATENCY);
	SET_BIT(FLASH->ACR, FLASH_ACR_LATENCY_1);

	/* Setting the PLL multiplier */
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_PLLMULL);
	SET_BIT(RCC->CFGR, RCC_CFGR_PLLMULL9);

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

static void ADC1_Init(void)
{
	/*
	 * ==================
	 * ADC1 Configuration
	 * ==================
	 *
	 * Predivison of reference clock for the ADC (RCC_CFGR_ADCPRE)
	 * ADCPRE[1:0] = 10 - PCLK2 divided by 6
	 * Maximum ADC clock frequency is 14MHz
	 *
	 * Enabling clock for ADC1
	 * After enabling each clock, a dummy read operation
	 * is performed to insert some delay after clock
	 * enabling
	 *
	 *
	 *	Enabling external trigger (ADC_CR2_EXTTRIG)
	 *	and selecting EXTI11 line
	 *	ADC_CR2_EXTSEL[2:0] = 110
	 *
	 *	Configuring sequence (which ADC channel to convert)
	 *	ADC_SQR3_SQ1[4:0] = 0 (Channel 0)
	 *	- kept at reset value
	 *
	 *	Enabling ADC1 EOC interrupt
	 *
	 *	Enabling ADC
	 *
	 *	Calibrating
	 */

	volatile uint32_t tmpreg; // Used for dummy read operations

	/* ADC1 clock prescaler */
	SET_BIT(RCC->CFGR, RCC_CFGR_ADCPRE_1);
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_ADCPRE_0);

	/* Enabling ADC1 clock */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);

	DUMMY(tmpreg);

	/* Enabling external trigger */
	SET_BIT(ADC1->CR2, ADC_CR2_EXTTRIG);

	SET_BIT(ADC1->CR2, ADC_CR2_EXTSEL_2);
	SET_BIT(ADC1->CR2, ADC_CR2_EXTSEL_1);
	CLEAR_BIT(ADC1->CR2, ADC_CR2_EXTSEL_0);


	/* Enabling ADC1 interrupt */
	SET_BIT(ADC1->CR1, ADC_CR1_EOCIE);
	NVIC_EnableIRQ(ADC1_2_IRQn);
	NVIC_SetPriority(ADC1_2_IRQn, 0);
	__enable_irq();

	/* Enabling ADC1 */
	SET_BIT(ADC1->CR2, ADC_CR2_ADON);

	/* Calibrating */
	SET_BIT(ADC1->CR2, ADC_CR2_CAL);
	while(READ_BIT(ADC1->CR2, ADC_CR2_CAL));
}

static void GPIO_Init(void)
{
	/*
	 * =======================
	 * Peripheral Clock Enable
	 * =======================
	 *
	 * Enabling clocks for GPIO ports used
	 * - GPIOA
	 * - GPIOB
	 *
	 * Enabling clock for AFIO
	 *
	 * After enabling each clock, a dummy read operation
	 * is performed to insert some delay after clock
	 * enabling
	 */

	volatile uint32_t tmpreg; // Used for dummy read operations

	/* GPIOA */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);

	/* GPIOB */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);

	/* AFIO */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);

	DUMMY(tmpreg);

	/*
	 * ==================
	 * PB11 Configuration
	 * ==================
	 *
	 * PB11 needs to be configured as a floating input,
	 * to be used as a trigger for ADC1 conversions.
	 * ADC1 requires EXTI line 11 to be used for conversions
	 * with regular channels, and this is why PB11 is used
	 * for this purpose.
	 *
	 * General configuration (GPIOB_CRH)
	 *
	 * MODE11[1:0] = 00 - Input mode
	 * CNF11[1:0]  = 01 - Floating input
	 *
	 * EXTI configuration
	 *
	 * Configuring AFIO_EXTICR3 to select source input for EXTI11
	 * external interrupt
	 * EXTI11[3:0] = 0001 - PB11 pin
	 *
	 * Configuring EXTI line 11 mask (EXTI_IMR)
	 *
	 * Configuring rising edge (EXTI_RTSR)
	 *
	 */

	// General configuration
	CLEAR_BIT(GPIOB->CRH, GPIO_CRH_MODE11_0);
	CLEAR_BIT(GPIOB->CRH, GPIO_CRH_MODE11_1);
	CLEAR_BIT(GPIOB->CRH, GPIO_CRH_CNF11_1);
	SET_BIT(GPIOB->CRH, GPIO_CRH_CNF11_0);

	// EXTI configuration
	CLEAR_BIT(AFIO->EXTICR[2], AFIO_EXTICR3_EXTI11);
	SET_BIT(AFIO->EXTICR[2], AFIO_EXTICR3_EXTI11_PB);

	SET_BIT(EXTI->IMR, EXTI_IMR_IM11);

	SET_BIT(EXTI->RTSR, EXTI_RTSR_RT11);

	/*
	 * =================
	 * PA0 Configuration
	 * =================
	 *
	 * Used as an analog input (ADC1 Channel 0)
	 * Needs to be configured as a floating input
	 * to be used as an alternate function input
	 *
	 * General configuration (GPIOA_CRL)
	 *
	 * MODE0[1:0] = 00 - Input mode
	 * CNF0[1:0]  = 01 - Floating input
	 */

	// General configuration
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE0_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE0_1);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF0_1);
	SET_BIT(GPIOA->CRL, GPIO_CRL_CNF0_0);
}

static void PC13_Init(void)
{
	volatile uint32_t tmpreg; // Used for dummy read operations

	/* GPIOC port clock enable */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);

	/*
	 * ==================
	 * PC13 Configuration
	 * ==================
	 */

	GPIOC->BSRR = GPIO_BSRR_BS13; // PC13 is active low

	SET_BIT(GPIOC->CRH, GPIO_CRH_MODE13_0);
	CLEAR_BIT(GPIOC->CRH, GPIO_CRH_MODE13_1);
	CLEAR_BIT(GPIOC->CRH, GPIO_CRH_CNF13);

	DUMMY(tmpreg);
}

void PC13_toggle(void)
{
	if (1 == PC13_state) {
		GPIOC->BSRR = GPIO_BSRR_BR13;
		PC13_state = 0;
	} else {
		GPIOC->BSRR = GPIO_BSRR_BS13;
		PC13_state = 1;
	}
}

static void PC14_Init(void)
{
	volatile uint32_t tmpreg; // Used for dummy read operations

	/* GPIOC port clock enable */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);

	/*
	 * ==================
	 * PC14 Configuration
	 * ==================
	 */


	SET_BIT(GPIOC->CRH, GPIO_CRH_MODE14_0);
	CLEAR_BIT(GPIOC->CRH, GPIO_CRH_MODE14_1);
	CLEAR_BIT(GPIOC->CRH, GPIO_CRH_CNF14);

	DUMMY(tmpreg);
}

void PC14_toggle(void)
{
	if (1 == PC14_state) {
		GPIOC->BSRR = GPIO_BSRR_BR14;
		PC14_state = 0;
	} else {
		GPIOC->BSRR = GPIO_BSRR_BS14;
		PC14_state = 1;
	}
}
