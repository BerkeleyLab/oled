#include <stdint.h>
#include <stdbool.h>
#include "settings.h"
#include "irqs.h"
#include "print.h"
#include "uart.h"
#include "gpio.h"
#include "spi.h"
#include "spi_memio.h"
#include "timer.h"
#include "frame_buffer.h"
#include "ssd1322.h"
#include "lv_font.h"
#include "ui_board.h"
#include "demo.h"

uint32_t *irq(uint32_t *regs, uint32_t irqs)
{
    // called for all 32 interrupts
    // *regs = context save X-registers
    // irqs = q1 = bitmask of all IRQs to be handled
    if (irqs & (1 << IRQ_UART0_RX)) {
        switch (UART_GETC(BASE_UART0)) {
            case 0x14:  // Ctrl + T = reset
                _picorv32_irq_reset();  // reboot from interrupt
                break;
        }
    }
    return regs;
}

// void _putchar(char c)
// {
//     // hook for all print_* functions
//     UART_PUTC(BASE_UART0, c);
// }

int main(void)
{
    // TODO add support for the ui_board
    UART_INIT(BASE_UART0, BOOTLOADER_BAUDRATE);  // Debug print (USB serial)
    _picorv32_irq_enable(1 << IRQ_UART0_RX);

    uiBoardInit();
    set_brightness(9);

    while (1) {
        unsigned btns = uiBoardPoll();  // returns encoder state (btns)
        demo(btns);
        send_fb();
        // DELAY_MS(100);
    }
}
