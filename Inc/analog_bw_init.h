#ifndef INC_ANALOG_BW_INIT_H
#define INC_ANALOG_BW_INIT_H

void Clock_Init(void);
void GPIO_Init(void);
void DMA1_Init(uint16_t samples[], int number_of_samples);
void ADC1_Init(void);

#endif /* INC_ANALOG_BW_INIT_H */
