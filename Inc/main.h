/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#define DUMMY(X) (void)X // For suppressing variable unused gcc warnings
#define NUM_CODES 4096 // Number of valid ADC codes (12 bit ADC)
#define NUM_SAMPLES 128

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"

void PC13_toggle(void);
void PC14_toggle(void);

#endif /*__MAIN_H */
