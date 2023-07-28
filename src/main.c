#include <stdio.h>

#include "uart.h"
#include "xmodem.h"

int main(void)
{
    uart_setup();

    for (;;) {
		uart_transmit_str((uint8_t*)"Please send a new binary file with Xmodem protocol to update the firmware.\n\r");

		xmodem_receive();

		/* We only exit the xmodem protocol, if there are any errors.
		* In that case, notify the user and start over. */
		uart_transmit_str((uint8_t*)"\n\rFailed... Please try again.\n\r");
    }
    return 0;
}
