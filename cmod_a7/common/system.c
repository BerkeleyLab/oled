#include <stdint.h>
#include <stdbool.h>
#include "settings.h"
#include "irqs.h"
#include "print.h"
#include "uart.h"
#include "gpio.h"
#include "spi.h"
#include "timer.h"
#include "frame_buffer.h"
#include "ssd1322.h"
#include "demo.h"

void _putchar(char c)
{
    // hook for all print_* functions
    UART_PUTC(BASE_UART0, c);
}

volatile unsigned chars_received = 0;
uint32_t *irq(uint32_t *regs, uint32_t irqs)
{
    // called for all 32 interrupts
    // *regs = context save X-registers
    // irqs = q1 = bitmask of all IRQs to be handled
    if (irqs & (1 << IRQ_TIMER0)) {
        _picorv32_irq_timer(US_TO_CYCLES(5000));
    }
    if (irqs & (1 << IRQ_UART0_RX)) {
        // Ctrl + T = reset
        if (UART_GETC(BASE_UART0) == 0x14) {
            // reboot from interrupt
            _picorv32_irq_reset();
        }
        chars_received++;
    }
    return regs;
}

int main(void)
{
    UART_INIT(BASE_UART0, BOOTLOADER_BAUDRATE);  // Debug print (USB serial)

    _picorv32_irq_enable((1 << IRQ_UART0_RX) | (1 << IRQ_TIMER0));
    _picorv32_irq_timer(US_TO_CYCLES(5000));

    init_ssd1322();

    set_brightness(10);

    // setAll(0);
    // send_fb();

    while (1) {
        draw_test_frame();
        // send_fb();
        send_window_4(0, 0, 255, 63, g_frameBuff);
        DELAY_MS(5);

    }

    // test();

    while(1);
}
