#ifndef UI_BOARD_H
#define UI_BOARD_H

// init GPIO expander and OLED
void uiBoardInit(void);

// returns encoder state: {push, right, left}
uint8_t uiBoardPoll(void);

// 0 = off, 1 = red, 2 = green, 3 = yellow
void setLed(unsigned val);

#endif
