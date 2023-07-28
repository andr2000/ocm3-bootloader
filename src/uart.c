#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "delay.h"
#include "uart.h"

#define UART_PORT		USART3
#define UART_TIMEOUT_US		1000
#define UART_TO_DELTA_US	1

uart_status uart_setup(void)
{
	delay_setup();
#if UART_PORT == USART3
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_USART3);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART3_TX);
#else
    #error UART_PORT must be defined!!
#endif

	usart_set_baudrate(UART_PORT, 115200);
	usart_set_databits(UART_PORT, 8);
	usart_set_stopbits(UART_PORT, USART_STOPBITS_1);
	usart_set_mode(UART_PORT, USART_MODE_TX);
	usart_set_parity(UART_PORT, USART_PARITY_NONE);
	usart_set_flow_control(UART_PORT, USART_FLOWCONTROL_NONE);

	usart_enable(UART_PORT);

	return UART_OK;
}

void uart_deinit(void)
{
	delay_deinit();

	usart_disable(UART_PORT);
#if UART_PORT == USART3
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT,
		      GPIO_USART3_TX);
	rcc_periph_clock_disable(RCC_USART3);
	rcc_periph_clock_disable(RCC_GPIOB);
	rcc_periph_clock_disable(RCC_AFIO);
#else
	#error UART_PORT must be defined!!
#endif
}

static int usart_is_recv_ready(uint32_t usart)
{
	return (USART_SR(usart) & USART_SR_RXNE) == 0;
}

uart_status uart_receive(uint8_t *data, uint16_t length)
{
	uint16_t to = UART_TIMEOUT_US;

	while (length--) {
		while (to && !usart_is_recv_ready(UART_PORT)) {
			delay_us(UART_TO_DELTA_US);
			to -= UART_TO_DELTA_US;
		}
		if (!usart_is_recv_ready(UART_PORT)) {
			return UART_ERROR;
		}
		*data++ = usart_recv(UART_PORT);
	}

	return UART_OK;
}

uart_status uart_transmit_str(uint8_t *data)
{
	while ('\0' != *data) {
		uart_status ret = uart_transmit_ch(*data++);
		if (ret != UART_OK) {
			return ret;
		}
	}

	return UART_OK;
}

uart_status uart_transmit_ch(uint8_t data)
{
	usart_send_blocking(UART_PORT, data);
	return UART_OK;
}

