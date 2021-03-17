#ifndef INC_USART_H
#define INC_USART_H

void USART1_Init(void);
void USART1_EnableTx(void);
void USART1_DisableTx(void);
void USART1_Tx_byte(uint8_t *bytes, uint32_t count);
void USART1_Tx_string(char *buffer);
void USART1_Tx_float(float value);
void USART1_Tx_int(int value);

#endif /* INC_USART_H */
