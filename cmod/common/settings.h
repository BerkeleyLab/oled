#ifndef SETTINGS_H
#define SETTINGS_H
//-----------------------------
// Global settings file
//-----------------------------
// can be pretty much included by anyone (.S, .c, .h)
// Some of the constants here are expected by the startup script / library files
// and need to be defined even if not used

// Base addresses of Peripherals
#define BASE_BRAM               0x00000000
#define BASE_SPI                0x01000000
#define BASE_GPIO               0x02000000
#define BASE_UART0              0x03000000

#define OLED_SPI                BASE_SPI
#define OLED_SPI_CLKDIV         (F_CLK / 10000000 / 2)  // 10 MHz SCK
#define OLED_GPIO               BASE_GPIO
#define OLED_BIT_D_C            0
#define OLED_BIT_RSTN           -1  // set to -1 for ui_board
#define OLED_BIT_CSN            4

// ui_board GPIO expander
#define IO_SPI                  BASE_SPI
#define IO_SPI_CLKDIV           OLED_SPI_CLKDIV
#define IO_GPIO                 BASE_GPIO
#define IO_CSN                  1
#define IO_RSTN                 2
#define IO_INT                  3

#define IRQ_UART0_RX            0x03

#define F_CLK                   (100000000)

// How long to wait in the bootloader for a connection
#define BOOTLOADER_DELAY        (F_CLK / 1000)

#endif
