#ifndef UART_H_
#define UART_H_

#include <libopencm3/stm32/usart.h>

typedef enum {
  UART_OK     = 0,
  UART_ERROR  = 1
} uart_status;

uart_status uart_setup(void);
void uart_deinit(void);

uart_status uart_receive(uint8_t *data, uint16_t length);
uart_status uart_transmit_str(uint8_t *data);
uart_status uart_transmit_ch(uint8_t data);

#endif /* UART_H_ */
