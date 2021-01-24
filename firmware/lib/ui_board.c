#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "gpio.h"
#include "timer.h"
#include "spi.h"
#include "ssd1322.h"
// #include "print.h"
#include "settings.h"

// Pins directly controlled over PMOD
#define CS_N(val) SET_GPIO1(IO_GPIO, GPIO_OUT_REG, IO_CSN, val)
#define RST_N(val) SET_GPIO1(IO_GPIO, GPIO_OUT_REG, IO_RSTN, val)
#define SPI(val) SPI_SET_DAT_BLOCK(IO_SPI, val)

// on-board SPI GPIO expander (MCP23508)
#define MCP_ADDR 0x20  // 7-bit chip address

// Pin assignment
#define P_OLED_NRST 0
#define P_ENCB 1
#define P_ENCA 2
#define P_ENCSW 3
#define P_LEDR 4
#define P_LEDG 5
#define P_AUXA 6
#define P_AUXB 7

// MCP23508 registers
#define IODIR 0x00
#define IPOL 0x01
#define GPINTEN 0x02
#define DEFVAL 0x03
#define INTCON 0x04
#define IOCON 0x05
#define GPPU 0x06
#define INTF 0x07
#define INTCAP 0x08
#define GPIO 0x09
#define OLAT 0x0A

static void mcpWriteReg(uint8_t addr, uint8_t val)
{
	CS_N(0);
	SPI((MCP_ADDR << 1) | 0);
	SPI(addr);
	SPI(val);
	CS_N(1);
}

static uint8_t mcpReadReg(uint8_t addr)
{
	CS_N(0);
	SPI((MCP_ADDR << 1) | 1);
	SPI(addr);
	SPI(0);
	CS_N(1);
	return SPI_GET_DAT(IO_SPI);
}

static void mcpInit(void)
{
	//           ss_man, ss_ctrl, cpol, cpha, lsb, nbits, clk_div
	SPI_INIT(IO_SPI,  1,       1,    0,    0,   0,     8, IO_SPI_CLKDIV);

	CS_N(1);
	RST_N(0);
	SET_GPIO1(IO_GPIO, GPIO_OE_REG, IO_CSN, 1);
	SET_GPIO1(IO_GPIO, GPIO_OE_REG, IO_RSTN, 1);
	DELAY_US(1);

	RST_N(1);
	DELAY_US(1);

	// 1 = input
	mcpWriteReg(
		IODIR,
		(1 << P_AUXB) | (1 << P_AUXA) |
		(1 << P_ENCA) | (1 << P_ENCB) | (1 << P_ENCSW)
	);

	// 1 = pullup
	mcpWriteReg(GPPU, (1 << P_AUXB) | (1 << P_AUXA));
}

// 0 = off, 1 = red, 2 = green, 3 = yellow
void setLed(unsigned val)
{
	val &= 3;
	mcpWriteReg(OLAT, (val << P_LEDR) | (1 << P_OLED_NRST));
}

// returns {push, right, left}
static uint8_t encoderPoll(void)
{
	static uint8_t enc_last = 0x01;
	static bool btn_last;
	uint8_t ret=0, enc=0;

	uint8_t val = mcpReadReg(GPIO);
	// for (int i=7; i>=0; i--)
	// 	_putchar(val & (1 << i) ? '1' : '0');
	// _putchar('\n');

	// Make button edge sensitive
	bool btn = (val & (1 << P_ENCSW)) == 0;
	if (!btn_last && btn)
		ret |= 4;

	// convert gray to binary
	if (val & (1 << P_ENCA))
		enc = 1;

	if (val & (1 << P_ENCB))
		enc ^= 3;

	uint8_t diff = enc - enc_last;

	if (diff & 1) {  // did we make a single step?
		if (diff & 2)  // then bit 1 gives the direction
			ret |= 2;
		else
			ret |= 1;
	}

	enc_last = enc;
	btn_last = btn;
	return ret;
}

static bool oled_inverse = false;

void uiBoardInit(void)
{
	mcpInit();
	DELAY_MS(1);
	mcpWriteReg(OLAT, (1 << P_OLED_NRST));  // Un-reset OLED

	init_ssd1322();
	set_brightness(9);

	// Use random initial value
	oled_inverse = rand() & 1;
	set_inverted(oled_inverse);
}

// returns encoder state: {push, right, left}
uint8_t uiBoardPoll(void)
{
	static uint64_t last_ts = 0;

	// Invert display every 1 h to reduce burn-in
	uint64_t ts = _picorv32_rd_cycle_64();
	if (ts - last_ts > 60ll * 60 * F_CLK) {
		oled_inverse = !oled_inverse;
		set_inverted(oled_inverse);
		last_ts = ts;
	}

	return encoderPoll();
}
