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

void my_log_cb(lv_log_level_t level, const char * file, uint32_t line, const char * fn_name, const char * dsc)
{
    /*Send the logs via serial port*/
    if(level == LV_LOG_LEVEL_ERROR) print_str("ERROR: ");
    if(level == LV_LOG_LEVEL_WARN)  print_str("WARNING: ");
    if(level == LV_LOG_LEVEL_INFO)  print_str("INFO: ");
    if(level == LV_LOG_LEVEL_TRACE) print_str("TRACE: ");

    print_str("File: ");
    print_str(file);

    print_str("#");
    print_dec(line);

    print_str(": ");
    print_str(fn_name);
    print_str(": ");
    print_str(dsc);
    print_str("\n");
}

static void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    send_window_8(area->x1, area->y1, area->x2, area->y2, (uint8_t*)color_p);
    lv_disp_flush_ready(disp_drv);
}

int main(void)
{
    UART_INIT(BASE_UART0, BOOTLOADER_BAUDRATE);  // Debug print (USB serial)
    _picorv32_irq_enable(1 << IRQ_UART0_RX);

    init_ssd1322();
    set_brightness(0);

    lv_log_register_print_cb(my_log_cb);
    lv_init();
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * LV_VER_RES_MAX / 10];                     /*Declare a buffer for 1/10 screen size*/
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX / 10);    /*Initialize the display buffer*/

    lv_disp_drv_t disp_drv;               /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
    disp_drv.flush_cb = my_flush_cb;    /*Set your driver function*/
    disp_drv.buffer = &disp_buf;          /*Assign the buffer to the display*/
    lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/

    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 20);                          /*Set its size*/
    lv_obj_t * label = lv_label_create(btn, NULL);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/

    while (1) {
        lv_task_handler();
    }
}
