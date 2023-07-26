#include <stdio.h>

#include "uart.h"

int main(void)
{
    uart_setup();

	uart_transmit_str((uint8_t *)"Hello, world!\n");
    for (;;) {

    }
    return 0;
}