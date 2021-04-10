#include "stm32f1xx.h"
#include "config.h"
#include "analog_bw_init.h"
#include "usart.h"

#define ANALOG_BW_MEASURMENT

#ifdef ANALOG_BW_MEASURMENT
/* Global variables */
uint16_t samples[NUM_SAMPLES] = {0};

volatile int PC13_state = 0;

volatile int finished_sampling = 0;
volatile int data_sent = 0;

/* Function declaration */
static void PC13_Init(void);
void PC13_toggle(void);

/* Interrupt handlers */
void DMA1_Channel1_IRQHandler(void)
{
	if(READ_BIT(DMA1->ISR, DMA_ISR_TCIF1))
	{
		/* Disabling ADC */
		CLEAR_BIT(ADC1->CR2, ADC_CR2_ADON);
		/* Clearing the interrupt flag */
		SET_BIT(DMA1->IFCR, DMA_IFCR_CTCIF1);
		/* Disabling DMA Channel */
		CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);

		finished_sampling = 1;
	}
}

int main(void)
{
	/* Initialize everything */
	Clock_Init();
	GPIO_Init();
	USART1_Init(CLOCK_FREQUENCY, 115200);
	USART1_EnableTx();
	DMA1_Init(samples, NUM_SAMPLES);
	ADC1_Init();
	PC13_Init();

	/* Starting conversion */
	SET_BIT(ADC1->CR2, ADC_CR2_ADON);
	for(int i=0; i<100000; i++);

	while (1)
	{
		if(finished_sampling && !data_sent)
		{
			for(int i=0; i<NUM_SAMPLES; i++)
			{
				USART1_Tx_int(samples[i]);
				USART1_Tx_string("\r\n");
			}
			USART1_Tx_string("END\r\n");
			USART1_DisableTx();
			SET_BIT(GPIOC->BSRR, GPIO_BSRR_BS14);
			data_sent = 1;
		}
	}
}
#else
int main(void)
{

}
#endif


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
