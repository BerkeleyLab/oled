`timescale 1 ns / 1 ns

module system_top (
    input            SYSCLK,
    input [1:0]      BTNS,
    output [1:0]     LEDS,

    output           LED_B,
    output           LED_G,
    output           LED_R,

    input            UART_RXD,
    output           UART_TXD,

    // SPI for OLED
    output        oled_cs,
    output        oled_sck,
    output        oled_copi,
    output        oled_c_d,
    output        oled_reset
);

wire pll_reset, sysclk_buf;
pb_debouncer debouncer_inst(
    .clk     (sysclk_buf),
    .PB      (|BTNS),
    .PB_up   (pll_reset)
);

wire clk, locked;
xilinx7_clocks #(
    .DIFF_CLKIN     ("FALSE"),  // Single ended
    .CLKIN_PERIOD   (83.333),   // 12 MHz
    .MULT           (62.500),   // 750 MHz
    .DIV0           (7.500),    // 100 MHz
    .DIV1           (7.500)     // 100 MHz
) clk_inst(
    .sysclk_p (SYSCLK),
    .sysclk_n (1'b0),
    .sysclk_buf(sysclk_buf),
    .reset    (pll_reset),
    .clk_out0 (clk),
    .clk_out1 (),
    .locked   (locked)
);

wire [31:0] gpio_z;
wire trap;

system #(
    .SYSTEM_HEX_PATH ("system32.dat")
) system_inst (
    .clk        (clk),
    .cpu_reset  (~locked),
    .trap       (trap),

    .uart_tx0   (UART_TXD),
    .uart_rx0   (UART_RXD),

    // SPI for OLED
    .spi_cs      (),
    .spi_sck     (oled_sck),
    .spi_copi    (oled_copi),
    .spi_cipo    (1'b0),

    .gpio_z      ({
        LEDS[1:0], oled_cs, oled_reset, oled_c_d, LED_B, LED_G, LED_R
    })
);

endmodule
