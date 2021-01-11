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
#include "psu_board_gui.h"

uint16_t reg_map[64] = {
         6, // uptime (1 h)
   25 << 8, // PSU_TEMP (25 degC)
         0, // PSU_VCCINT (0 V)
      1000, // PSU_VCCAUX  (1 V)
      1800, // PSU_VCCBRAM  (1.8 V)
     12546, // PSU_IN_VOLTAGE
      1234, // PSU_IN_CURRENT
      5535, // PSU_A_VOLTAGE
       385, // PSU_A_CURRENT
      5522, // PSU_B_VOLTAGE
       119, // PSU_B_CURRENT
      5627, // PSU_C_VOLTAGE
       110, // PSU_C_CURRENT
      5124, // PSU_D_VOLTAGE
        50, // PSU_D_CURRENT
        30, // PSU_E_VOLTAGE
       198, // PSU_E_CURRENT
  125 << 8, // DC_A_TEMP (125 degC)
      1000, // DC_A_VOLTAGE (1 V)
    0xE6F0, // DC_B_TEMP (-25 degC)
      5000, // DC_B_VOLTAGE (5 V)
};

volatile unsigned chars_received=0, btns=0;
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

            case 'a':  // encoder left
                btns |= 1;
                break;

            case 'd': // encoder right
                btns |= 2;
                break;

            case 's':  // encoder push
                btns |= 4;
                break;
        }
        chars_received++;
    }
    return regs;
}

void _putchar(char c)
{
    UART_PUTC(BASE_UART0, c);
}

int main(void)
{
    // TODO add support for the ui_board
    UART_INIT(BASE_UART0, BOOTLOADER_BAUDRATE);  // Debug print (USB serial)
    _picorv32_irq_enable(1 << IRQ_UART0_RX);

    uiBoardInit();
    set_brightness(5);

    unsigned frm=0;
    while (1) {
        uiBoardPoll();  // returns encoder state (btns)

        if (btns & 4)
            for (unsigned i=0; i < sizeof(reg_map) / sizeof(reg_map[0]); i++)
                reg_map[i] += 1;

        draw_psu_gui(btns);
        btns = 0;
        send_fb();

        DELAY_MS(100);
        frm++;
    }
}
