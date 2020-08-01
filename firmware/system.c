#include <stdint.h>
#include <stdbool.h>
#include "settings.h"
#include "irqs.h"
#include "print.h"
#include "uart.h"
#include "gpio.h"
#include "spi_memio.h"
#include "spi.h"
#include "timer.h"
#include "frame_buffer.h"
#include "ssd1322.h"
#include "lvgl/lvgl.h"

#define LED(val) SET_GPIO8(BASE_GPIO, GPIO_OUT_REG, 0, ((~val) & 0x7))

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
    _picorv32_irq_enable(1 << IRQ_UART0_RX);

    SET_GPIO8(BASE_GPIO, GPIO_OUT_REG, 0, 0);
    SET_GPIO8(BASE_GPIO, GPIO_OE_REG, 0, 0xFF);  // Enable outputs
    LED(0b000);  // LED off
    SPI_INIT(BASE_SPI, 0, 1, 0, 0, 0, 8, 1);

    init_ssd1322();
    set_brightness(0);

    lv_init();
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * LV_VER_RES_MAX / 10];                     /*Declare a buffer for 1/10 screen size*/
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX / 10);    /*Initialize the display buffer*/

    lv_disp_drv_t disp_drv;               /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
    // disp_drv.flush_cb = my_disp_flush;    /*Set your driver function*/
    disp_drv.buffer = &disp_buf;          /*Assign the buffer to the display*/
    lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/

    while (1) {
        lv_task_handler();
    }
}
