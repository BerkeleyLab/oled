#ifndef SSD1322_H
#define SSD1322_H

void init_ssd1322(void);
void set_brightness(uint8_t val);
void send_fb(void);

void send_window_4(unsigned x1, unsigned y1, unsigned x2, unsigned y2, uint8_t *data);

#endif
