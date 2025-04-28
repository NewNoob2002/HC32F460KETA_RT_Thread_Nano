#include "lcd.h"

void lcd_write_cmd(uint8_t cmd)
{
    LCD_CS(0);
    LCD_DC(0);
    LCD_SPI_Send(&cmd, 1);
    LCD_CS(1);
}

void lcd_write_byte(uint8_t data)
{
    LCD_CS(0);
    LCD_DC(1);
    LCD_SPI_Send(&data, 1);
    LCD_CS(1);
}
void lcd_write_word(uint16_t data)
{
    uint8_t arry[2] = {0};
    arry[0] = (data >> 8) & 0xFF;
    arry[1] = data & 0xFF;
    LCD_CS(0);
    LCD_DC(1);
    LCD_SPI_Send(arry, 2);
    LCD_CS(1);
}
void lcd_init()
{
    delay(50);
    LCD_RST(1);
    delay(50);
    LCD_RST(0);
    LCD_CS(1);
    delay(50);
    LCD_RST(1);
    delay(150);

    lcd_write_cmd(0xfe);
    lcd_write_byte(0x01);
    lcd_write_cmd(0x6A);
    lcd_write_byte(0x21);
    lcd_write_cmd(0xfe);
    lcd_write_byte(0x00); // CMD1 command

    lcd_write_cmd(0xC4);   // NEW  ADD
    lcd_write_byte(0x80); // CMD1 command     //NEW  ADD

    lcd_write_cmd(0x2a);
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);
    lcd_write_byte(0x7d);

    //		LCD_WR_REG(0x2b);
    //		LCD_WR_DATA8(0x00);
    //		LCD_WR_DATA8(0x00);
    //		LCD_WR_DATA8(0x01);
    //		LCD_WR_DATA8(0x25);

    lcd_write_cmd(0x35);
    lcd_write_byte(0x00);

    lcd_write_cmd(0x51);
    lcd_write_byte(0xff);

    lcd_write_cmd(0x3A);
    lcd_write_byte(0x05);

    lcd_write_cmd(0x11);
    //延迟亮屏
    Ddl_Delay1ms(1500); // DDL_DelayMS(1500);

    lcd_write_cmd(0x29);
}

void lcd_write_ramprepare()
{
    lcd_write_cmd(0x2c);
}

void lcd_set_cursor(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    lcd_write_cmd(0x2a);
    lcd_write_byte(sx >> 8);
    lcd_write_byte(sx & 0xff);
    lcd_write_byte(ex >> 8);
    lcd_write_byte(ex & 0xff);

    // 写入数据
    lcd_write_cmd(0x2b);
    lcd_write_byte(sy >> 8);
    lcd_write_byte(sy & 0xff);
    lcd_write_byte(ey >> 8);
    lcd_write_byte(ey & 0xff);
}

void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    lcd_set_cursor(x, y, x + w, y + h);
    lcd_write_ramprepare();
    for (uint16_t i = 0; i < w * h; i++)
    {
        lcd_write_word(color);
    }
}

