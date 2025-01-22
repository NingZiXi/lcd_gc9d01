/**
 * @file lcd_gc9d01.c
 * @author 宁子希 (1589326497@qq.com)
 * @brief   LCD GC9D01 驱动源文件
 * @version 1.0.0
 * @date 2025-01-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <string.h>
#include "lcd_gc9d01.h"

#define TAG "LCD_GC9D01"

spi_device_handle_t spi;

//当前光标位置
static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

// GPIO 初始化
static void gpio_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LCD_PIN_NUM_RST) | (1ULL << LCD_PIN_NUM_DC) | (1ULL << LCD_PIN_NUM_CS) | (1ULL << LCD_PIN_NUM_BL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    gpio_set_level(LCD_PIN_NUM_RST, 0);
    gpio_set_level(LCD_PIN_NUM_DC, 0);
    gpio_set_level(LCD_PIN_NUM_CS, 1);
    gpio_set_level(LCD_PIN_NUM_BL, 1);
}

// SPI 初始化
static void spi_init(void) {
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = LCD_PIN_NUM_MOSI,
        .sclk_io_num = LCD_PIN_NUM_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32 * 8
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 66 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = LCD_PIN_NUM_CS,
        .queue_size = 1,
        .flags = SPI_DEVICE_NO_DUMMY,
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

// SPI 发送数据
static void spi_send_data(uint8_t data) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &data;
    ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
}

// 发送命令到 LCD
static void lcd_send_cmd(uint8_t cmd) {
    gpio_set_level(LCD_PIN_NUM_DC, 0);
    spi_send_data(cmd);
}

// 发送数据到 LCD
static void lcd_send_data(uint8_t data) {
    gpio_set_level(LCD_PIN_NUM_DC, 1);
    spi_send_data(data);
}

// 设置显示区域
void lcd_gc9d01_set_addr(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end) {
    x_start += LCD_COLUMN_OFFSET;
    y_start += LCD_LINE_OFFSET;
    x_end += LCD_COLUMN_OFFSET;
    y_end += LCD_LINE_OFFSET;

    lcd_send_cmd(0x2A); // 列地址设置
    lcd_send_data(x_start >> 8);
    lcd_send_data(x_start);
    lcd_send_data(x_end >> 8);
    lcd_send_data(x_end);

    lcd_send_cmd(0x2B); // 行地址设置
    lcd_send_data(y_start >> 8);
    lcd_send_data(y_start);
    lcd_send_data(y_end >> 8);
    lcd_send_data(y_end);

    lcd_send_cmd(0x2C); // 内存写入
}

// 显示单个像素点
void lcd_gc9d01_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    lcd_gc9d01_set_addr(x, y, x, y); // 设置显示区域为单个像素点
    lcd_send_data(color >> 8);      // 发送颜色高字节
    lcd_send_data(color);           // 发送颜色低字节
}

// 清屏
void lcd_gc9d01_clear(void) {
    uint16_t row, column;
    lcd_gc9d01_set_addr(0, 0, LCD_COLUMN_NUMBER - 1, LCD_LINE_NUMBER - 1);
    for (row = 0; row < LCD_LINE_NUMBER; row++) {
        for (column = 0; column < LCD_COLUMN_NUMBER; column++) {
            lcd_send_data(0x00);
            lcd_send_data(0x00);
        }
    }
}

// 填充屏幕
void lcd_gc9d01_fill(uint16_t color) {
    uint16_t row, column;
    lcd_gc9d01_set_addr(0, 0, LCD_COLUMN_NUMBER - 1, LCD_LINE_NUMBER - 1);
    for (row = 0; row < LCD_LINE_NUMBER; row++) {
        for (column = 0; column < LCD_COLUMN_NUMBER; column++) {
            lcd_send_data(color >> 8);
            lcd_send_data(color);
        }
    }
}

// 显示 16x16 汉字
void lcd_gc9d01_chinese_display(uint16_t x, uint16_t y, uint16_t color, const uint8_t (*chinese_word)[32], uint8_t word_index) {
    uint16_t col;
    uint8_t bit_pos = 0, current_byte = 0, byte_index = 0;
    lcd_gc9d01_set_addr(x, y, x + 15, y + 16);

    for (col = 0; col < 32; col++) {
        current_byte = chinese_word[word_index][byte_index];
        
        for (bit_pos = 0; bit_pos < 8; bit_pos++) {
            if (current_byte & 0x01) {
                lcd_send_data(color >> 8);
                lcd_send_data(color);
            } else {
                lcd_send_data(0x00);
                lcd_send_data(0x00);
            }
            current_byte >>= 1;
        }
        byte_index++;
    }
}

// 画圆函数
void lcd_gc9d01_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 0;

    while (x >= y) {
        lcd_gc9d01_draw_pixel(x0 + x, y0 + y, color);
        lcd_gc9d01_draw_pixel(x0 + y, y0 + x, color);
        lcd_gc9d01_draw_pixel(x0 - y, y0 + x, color);
        lcd_gc9d01_draw_pixel(x0 - x, y0 + y, color);
        lcd_gc9d01_draw_pixel(x0 - x, y0 - y, color);
        lcd_gc9d01_draw_pixel(x0 - y, y0 - x, color);
        lcd_gc9d01_draw_pixel(x0 + y, y0 - x, color);
        lcd_gc9d01_draw_pixel(x0 + x, y0 - y, color);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

// 画线函数
void lcd_gc9d01_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = (dx > dy ? dx : -dy) / 2;
    int16_t e2;

    while (1) {
        lcd_gc9d01_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

// 画三角形函数
void lcd_gc9d01_draw_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    lcd_gc9d01_draw_line(x0, y0, x1, y1, color);
    lcd_gc9d01_draw_line(x1, y1, x2, y2, color);
    lcd_gc9d01_draw_line(x2, y2, x0, y0, color);
}

// 画矩形函数
void lcd_gc9d01_draw_rect(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color) {
    uint16_t x1 = x0 + width - 1;
    uint16_t y1 = y0 + height - 1;

    lcd_gc9d01_draw_line(x0, y0, x1, y0, color); // 上边
    lcd_gc9d01_draw_line(x1, y0, x1, y1, color); // 右边
    lcd_gc9d01_draw_line(x1, y1, x0, y1, color); // 下边
    lcd_gc9d01_draw_line(x0, y1, x0, y0, color); // 左边
}

void lcd_gc9d01_putchar(lcd_string_handle_t *handle, char c) {
    // 换行处理
    if (c == '\n') {
        handle->x = 0;
        handle->y += handle->font->height + handle->line_spacing;
        if (handle->y + handle->font->height > LCD_LINE_NUMBER) {
            handle->y = 0;
        }
        return;
    }
    // 自动换行处理
    if (handle->x + handle->font->width + handle->char_spacing > LCD_COLUMN_NUMBER) {
        handle->x = 0;
        handle->y += handle->font->height + handle->line_spacing;
        if (handle->y + handle->font->height > LCD_LINE_NUMBER) {
            handle->y = 0;
        }
    }

    // 查找字符对应的字模数据
    int char_index = (int)c;
    int char_data_size = (handle->font->width + 7) / 8 * handle->font->height;

    // 仅处理可打印字符（ASCII 32~127）
    if (char_index >= 32 && char_index <= 127) {
        const uint8_t *char_data = &handle->font->data[(char_index - 32) * char_data_size];
        
        // 设置显示区域
        lcd_gc9d01_set_addr(handle->x, handle->y, 
                            handle->x + handle->font->width - 1, 
                            handle->y + handle->font->height - 1);
        // 遍历字模数据
        for (uint16_t col = 0; col < char_data_size; col++) {
            uint8_t current_byte = char_data[col];
            uint8_t bit_count = (handle->font->width <= 8) ? handle->font->width : (col % 2 == 0) ? 8 : (handle->font->width - 8);

            // 逐位绘制像素
            for (uint8_t bit_pos = 0; bit_pos < bit_count; bit_pos++) {
                if (current_byte & 0x01) {
                    lcd_send_data(handle->color >> 8);
                    lcd_send_data(handle->color);
                } else {
                    lcd_send_data(0x00);
                    lcd_send_data(0x00);
                }
                current_byte >>= 1;
            }
        }

        // 更新句柄位置，增加字符间距
        handle->x += handle->font->width + handle->char_spacing;
    }
}

void lcd_gc9d01_printf(lcd_string_handle_t *handle, const char *format, ...) {
    // 格式化字符串
    char buffer[128];  
    va_list args;     
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // 逐个显示字符
    const char *ptr = buffer;
    while (*ptr) {
        lcd_gc9d01_putchar(handle, *ptr++);
    }
}

#ifdef	CONFIG_LCD_GC9D01_IMAGE

// 显示图片
void lcd_gc9d01_picture_display(uint16_t x_start, uint16_t y_start, uint16_t width, uint16_t height, const uint8_t *ptr_pic) {
    uint32_t number;
    uint16_t x_end = x_start + width - 1;
    uint16_t y_end = y_start + height - 1;

    lcd_gc9d01_set_addr(x_start, y_start, x_end, y_end);
    uint32_t total_pixels = width * height*2;
    for (number = 0; number < total_pixels; number++) {
        lcd_send_data(*ptr_pic++);
    }
}

#endif // CONFIG_LCD_GC9D01_IMAGE

void lcd_gc9d01_init(void) {
    gpio_init(); // 初始化 GPIO
    spi_init();  // 初始化 SPI
    // 复位 LCD
    gpio_set_level(LCD_PIN_NUM_RST, 0); // 拉低 RST
    vTaskDelay(pdMS_TO_TICKS(10));      // 延时 10ms
    gpio_set_level(LCD_PIN_NUM_RST, 1); // 拉高 RST
    vTaskDelay(pdMS_TO_TICKS(120));     // 延时 120ms

    // 初始化命令序列
    lcd_send_cmd(0xFE);
    lcd_send_cmd(0xEF);

    lcd_send_cmd(0x80);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x81);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x82);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x83);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x84);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x85);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x86);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x87);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x88);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x89);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x8A);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x8B);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x8C);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x8D);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x8E);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x8F);
    lcd_send_data(0xFF);

    lcd_send_cmd(0x3A);
    lcd_send_data(0x05);

    lcd_send_cmd(0xEC);
    lcd_send_data(0x01);

    lcd_send_cmd(0x74);
    lcd_send_data(0x02);
    lcd_send_data(0x0E);
    lcd_send_data(0x00);
    lcd_send_data(0x00);
    lcd_send_data(0x00);
    lcd_send_data(0x00);
    lcd_send_data(0x00);

    lcd_send_cmd(0x98);
    lcd_send_data(0x3E);
    lcd_send_cmd(0x99);
    lcd_send_data(0x3E);

    lcd_send_cmd(0xB5);
    lcd_send_data(0x0D);
    lcd_send_data(0x0D);

    lcd_send_cmd(0x60);
    lcd_send_data(0x38);
    lcd_send_data(0x0F);
    lcd_send_data(0x79);
    lcd_send_data(0x67);

    lcd_send_cmd(0x61);
    lcd_send_data(0x38);
    lcd_send_data(0x11);
    lcd_send_data(0x79);
    lcd_send_data(0x67);

    lcd_send_cmd(0x64);
    lcd_send_data(0x38);
    lcd_send_data(0x17);
    lcd_send_data(0x71);
    lcd_send_data(0x5F);
    lcd_send_data(0x79);
    lcd_send_data(0x67);

    lcd_send_cmd(0x65);
    lcd_send_data(0x38);
    lcd_send_data(0x13);
    lcd_send_data(0x71);
    lcd_send_data(0x5B);
    lcd_send_data(0x79);
    lcd_send_data(0x67);

    lcd_send_cmd(0x6A);
    lcd_send_data(0x00);
    lcd_send_data(0x00);

    lcd_send_cmd(0x6C);
    lcd_send_data(0x22);
    lcd_send_data(0x02);
    lcd_send_data(0x22);
    lcd_send_data(0x02);
    lcd_send_data(0x22);
    lcd_send_data(0x22);
    lcd_send_data(0x50);

    lcd_send_cmd(0x6E);
    lcd_send_data(0x03);
    lcd_send_data(0x03);
    lcd_send_data(0x01);
    lcd_send_data(0x01);
    lcd_send_data(0x00);
    lcd_send_data(0x00);
    lcd_send_data(0x0F);
    lcd_send_data(0x0F);
    lcd_send_data(0x0D);
    lcd_send_data(0x0D);
    lcd_send_data(0x0B);
    lcd_send_data(0x0B);
    lcd_send_data(0x09);
    lcd_send_data(0x09);
    lcd_send_data(0x00);
    lcd_send_data(0x00);
    lcd_send_data(0x00);
    lcd_send_data(0x00);
    lcd_send_data(0x0A);
    lcd_send_data(0x0A);
    lcd_send_data(0x0C);
    lcd_send_data(0x0C);
    lcd_send_data(0x0E);
    lcd_send_data(0x0E);
    lcd_send_data(0x10);
    lcd_send_data(0x10);
    lcd_send_data(0x00);
    lcd_send_data(0x00);
    lcd_send_data(0x02);
    lcd_send_data(0x02);
    lcd_send_data(0x04);
    lcd_send_data(0x04);

    lcd_send_cmd(0xBF);
    lcd_send_data(0x01);

    lcd_send_cmd(0xF9);
    lcd_send_data(0x40);

    lcd_send_cmd(0x9B);
    lcd_send_data(0x3B);
    lcd_send_cmd(0x93);
    lcd_send_data(0x33);
    lcd_send_data(0x7F);
    lcd_send_data(0x00);

    lcd_send_cmd(0x7E);
    lcd_send_data(0x30);

    lcd_send_cmd(0x70);
    lcd_send_data(0x0D);
    lcd_send_data(0x02);
    lcd_send_data(0x08);
    lcd_send_data(0x0D);
    lcd_send_data(0x02);
    lcd_send_data(0x08);

    lcd_send_cmd(0x71);
    lcd_send_data(0x0D);
    lcd_send_data(0x02);
    lcd_send_data(0x08);

    lcd_send_cmd(0x91);
    lcd_send_data(0x0E);
    lcd_send_data(0x09);

    lcd_send_cmd(0xC3);
    lcd_send_data(0x18);
    lcd_send_cmd(0xC4);
    lcd_send_data(0x18);
    lcd_send_cmd(0xC9);
    lcd_send_data(0x3C);

    lcd_send_cmd(0xF0);
    lcd_send_data(0x13);
    lcd_send_data(0x15);
    lcd_send_data(0x04);
    lcd_send_data(0x05);
    lcd_send_data(0x01);
    lcd_send_data(0x38);

    lcd_send_cmd(0xF2);
    lcd_send_data(0x13);
    lcd_send_data(0x15);
    lcd_send_data(0x04);
    lcd_send_data(0x05);
    lcd_send_data(0x01);
    lcd_send_data(0x34);

    lcd_send_cmd(0xF1);
    lcd_send_data(0x4B);
    lcd_send_data(0xB8);
    lcd_send_data(0x7B);
    lcd_send_data(0x34);
    lcd_send_data(0x35);
    lcd_send_data(0xEF);

    lcd_send_cmd(0xF3);
    lcd_send_data(0x47);
    lcd_send_data(0xB4);
    lcd_send_data(0x72);
    lcd_send_data(0x34);
    lcd_send_data(0x35);
    lcd_send_data(0xDA);

    lcd_send_cmd(0x36);
    lcd_send_data(0x00);

    lcd_send_cmd(0xB4);
    lcd_send_data(0x00);
    lcd_send_data(0x00);

    lcd_send_cmd(0x34);
    lcd_send_cmd(0x11);

    vTaskDelay(pdMS_TO_TICKS(120));
    lcd_send_cmd(0x29); // 打开显示
}
