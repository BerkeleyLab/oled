#include <stdint.h>
#include <stdbool.h>
#include "settings.h"
#include "timer.h"
#include "spi.h"
#include "gpio.h"
#include "frame_buffer.h"

// set data/command pin of display, 1 = command, 0 = data
#define D_C(val) SET_GPIO1(BASE_GPIO, GPIO_OUT_REG, 3, val)

// set /RESET pin of display, 0 = Reset, 1 = Run
#define NRST(val) SET_GPIO1(BASE_GPIO, GPIO_OUT_REG, 4, val)

// send 1 byte of data to display
#define SPI(val) SPI_SET_DAT_BLOCK(BASE_SPI, val)

// Initialization for NHD-2.8-25664UCB2 OLED display
// negative = command, positive = data
static const int16_t init[] = {
    -0xFD, 0x12,     // Unlock OLED driver IC
    -0xAE,           // Display OFF (blank)
    -0x15, 0x1C, 0x5B,  // Set column address to 1C, 5B
    -0x75, 0x00, 0x3F,  // Set row address to 00, 3F
    -0xB3, 0x91,     // set clock to 80 fps
    -0xCA, 0x3F,     // Multiplex ratio, 1/64, 64 COMS enabled
    -0xA2, 0x00,     // Set offset, the display map starting line is COM0
    -0xA1, 0x00,     // Set start line position
    -0xA0, 0x14, 0x11,   // Set remap, horiz address increment, disable colum address remap,
                        //  enable nibble remap, scan from com[N-1] to COM0, disable COM split odd even
    -0xAB, 0x01,     // Select external VDD
    -0xB4, 0xA0, 0xFD,   // Display enhancement A, external VSL, enhanced low GS display quality
    -0xC1, 0x9F,     // Contrast current, 256 steps, default is 0x7F
    -0xC7, 0x02,     // Master contrast current, 16 steps, default is 0x0F
    -0xB9,           // load linear gamma table
    -0xB1, 0xE2,     // Phase Length
    -0xD1, 0x82, 0x20    // Display enhancement B
    -0xBB, 0x1F,     // Pre-charge voltage
    -0xB6, 0x08,     // Pre-charge period = 8 clks
    -0xBE, 0x07,     // Set VCOMH
    -0xA6,           // Normal display
    -0xA9,           // Disable partial display mode
    -0xAF            // Display ON
};

static void send_cmd(uint8_t val)
{
    D_C(0);
    SPI(val);
    D_C(1);
}

static void send_init(const int16_t *init, unsigned len)
{
    for (unsigned i=0; i<len; i++) {
        if (*init < 0)
            send_cmd(-(*init));
        else
            SPI(*init);
        init++;
    }
    send_cmd(0x5C); // write VRAM command
}

void init_ssd1322(void)
{
    // Enable outputs
    SET_GPIO8(BASE_GPIO, GPIO_OUT_REG, 0, 7);
    SET_GPIO8(BASE_GPIO, GPIO_OE_REG, 0, 0xFF);
    SPI_INIT(BASE_SPI, 0, 1, 0, 0, 0, 8, 1);

    NRST(0);
    DELAY_MS(1);

    NRST(1);
    DELAY_MS(120);

    send_init(init, sizeof(init) / sizeof(init[0]));
}

// from 0 - 15
void set_brightness(uint8_t val)
{
    send_cmd(0xC7);
    SPI(val);
    send_cmd(0x5C);  // write VRAM command
}

void send_fb(void)
{
    uint8_t *p = g_frameBuff;
    for (unsigned i=0; i<(DISPLAY_WIDTH * DISPLAY_HEIGHT / 2); i++)
        SPI(*p++);
}

// x1, y1, x2, y2: the rectangle to update in [pixels]
// note that ssd1322 works with columns of 4 pixels horizontally
// so the lower 2 bits of x1 and x2 will be truncated
// data in 4 bits / pixel, 2 pixels / byte
void send_window_4(unsigned x1, unsigned y1, unsigned x2, unsigned y2, uint8_t *data)
{
    x1 >>= 2;
    x2 >>= 2;
    send_cmd(0x15);  // Set column address range
    SPI(0x1C + x1);
    SPI(0x1C + x2);

    send_cmd(0x75);  // Set row address range
    SPI(y1);
    SPI(y2);

    // number of bytes to transfer over SPI
    unsigned n_bytes = (x2 - x1 + 1) * 2 * (y2 - y1 + 1);
    send_cmd(0x5C);  // write VRAM
    for (unsigned i=0; i<n_bytes; i++)
        SPI(*data++);
}

static unsigned getBrightness(uint8_t color)
{
    unsigned tmp = (color & 0x03) + ((color >> 2) & 0x07) + ((color >> 5) & 0x07);
    return (tmp > 0x0F ? 0x0F : tmp);
}

// x1, y1, x2, y2: the rectangle to update in [pixels]
// note that ssd1322 works with columns of 4 pixels horizontally
// so the lower 2 bits of x1 and x2 will be truncated
// data in 8 bits / pixel, 8 pixels / byte, 3:3:2 RGB format
// TODO replace with special VRAM in fabric which does DMA SPI transfers
void send_window_8(unsigned x1, unsigned y1, unsigned x2, unsigned y2, uint8_t *data)
{
    x1 >>= 2;
    x2 >>= 2;
    send_cmd(0x15);  // Set column address range
    SPI(0x1C + x1);
    SPI(0x1C + x2);

    send_cmd(0x75);  // Set row address range
    SPI(y1);
    SPI(y2);

    // number of bytes to transfer over SPI
    unsigned n_bytes = (x2 - x1 + 1) * 2 * (y2 - y1 + 1);
    send_cmd(0x5C);  // write VRAM
    for (unsigned i=0; i<n_bytes; i++) {
        uint8_t tmp = getBrightness(*data++) << 4;
        tmp |= getBrightness(*data++);
        SPI(tmp);
    }
}
