#pragma once
#include "settings.h"

#define XDP 126
#define YDP 294

void lcd_init();

void lcd_write_ramprepare();
void lcd_write_cmd(uint8_t cmd);
void lcd_write_byte(uint8_t data);
void lcd_set_cursor(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey);

void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void lcd_clear();
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

