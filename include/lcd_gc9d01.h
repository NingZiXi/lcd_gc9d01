/**
 * @file lcd_gc9d01.h
 * @author 宁子希 (1589326497@qq.com)
 * @brief    LCD GC9D01 驱动头文件
 * @version 1.0.0
 * @date 2025-01-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef LCD_GC9D01_H
#define LCD_GC9D01_H

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "image.h"
#include "fonts.h"

/*
 * ==================================================
 *               ↓↓ 用户参数配置 ↓↓                   
 * ==================================================
 */

/* LCD GC9D01 驱动接口 */
#define LCD_PIN_NUM_SCK  CONFIG_LCD_PIN_NUM_SCK
#define LCD_PIN_NUM_MOSI CONFIG_LCD_PIN_NUM_MOSI
#define LCD_PIN_NUM_RST  CONFIG_LCD_PIN_NUM_RST
#define LCD_PIN_NUM_DC   CONFIG_LCD_PIN_NUM_DC
#define LCD_PIN_NUM_CS   CONFIG_LCD_PIN_NUM_CS
#define LCD_PIN_NUM_BL   CONFIG_LCD_PIN_NUM_BL

/* LCD GC9D01 屏幕参数 */
#define LCD_COLUMN_NUMBER CONFIG_LCD_COLUMN_NUMBER
#define LCD_LINE_NUMBER   CONFIG_LCD_LINE_NUMBER
#define LCD_COLUMN_OFFSET CONFIG_LCD_COLUMN_OFFSET
#define LCD_LINE_OFFSET   CONFIG_LCD_LINE_OFFSET

/*----------------------↑↑↑↑↑----------------------*/


// 颜色定义（RGB565 格式）
#define COLOR_RGB(R, G, B) (((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3))
#define LCD_RED   0xF800        // 红色
#define LCD_GREEN 0x07E0        // 绿色
#define LCD_BLUE  0x001F        // 蓝色
#define LCD_YELLOW 0xFFE0       // 黄色
#define LCD_WHITE 0xFFFF        // 白色
#define LCD_BLACK 0x0000        // 黑色      


typedef struct {    // 字体结构体
    const uint8_t *data; 
    uint8_t width;        
    uint8_t height;     
} lcd_font_t;

typedef struct {    // 字符串句柄结构体
    uint16_t x;
    uint16_t y;     
    const lcd_font_t *font;
    uint16_t color;  
    uint8_t char_spacing;
    uint8_t line_spacing;
} lcd_string_handle_t;

// 字符串句柄预设配置宏
#define LCD_STRING_HANDLE_DEFAULT(x_pos, y_pos) \
    {                                          \
        .x = (x_pos),                          \
        .y = (y_pos),                          \
        .font = FONT_6x12,                     \
        .color = 0xFFFF,                       \
        .char_spacing = 1,                     \
        .line_spacing = 1                      \
    }

/**
 * @brief GC9D01 LCD 初始化
 * 
 */
void lcd_gc9d01_init(void);

/**
 * @brief GC9D01 LCD 清屏
 * 
 */
void lcd_gc9d01_clear(void);

/**
 * @brief GC9D01 LCD 填充屏幕
 * 
 * @param color 填充颜色（RGB565 格式）
 */
void lcd_gc9d01_fill(uint16_t color);

/**
 * @brief GC9D01 LCD 设置显示区域
 * 
 * @param x_start 显示区域的 x 起始坐标
 * @param y_start 显示区域的 y 起始坐标
 * @param x_end 显示区域的 x 结束坐标
 * @param y_end 显示区域的 y 结束坐标
 */
void lcd_gc9d01_set_addr(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

/**
 * @brief 绘制单个像素点
 * @param x 像素点的 x 坐标
 * @param y 像素点的 y 坐标
 * @param color 像素点的颜色（RGB565 格式）
 */
void lcd_gc9d01_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief 绘制一个圆
 * 
 * @param x0     圆心的 x 坐标
 * @param y0     圆心的 y 坐标
 * @param r      圆的半径
 * @param color  圆的颜色（RGB565 格式）
 */
void lcd_gc9d01_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

/**
 * @brief 绘制一条线
 * 
 * @param x0     线段的起始 x 坐标
 * @param y0     线段的起始 y 坐标
 * @param x1     线段的结束 x 坐标
 * @param y1     线段的结束 y 坐标
 * @param color     线段的颜色（RGB565 格式）
 */
void lcd_gc9d01_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/**
 * @brief 绘制一个三角形
 * 
 * @param x0     三角形的第一个顶点 x 坐标
 * @param y0     三角形的第一个顶点 y 坐标
 * @param x1     三角形的第二个顶点 x 坐标
 * @param y1     三角形的第二个顶点 y 坐标
 * @param x2     三角形的第三个顶点 x 坐标
 * @param y2     三角形的第三个顶点 y 坐标
 * @param color  三角形的颜色（RGB565 格式）
 */
void lcd_gc9d01_draw_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief 绘制一个矩形
 * 
 * @param x0     矩形的左上角 x 坐标
 * @param y0     矩形的左上角 y 坐标
 * @param width     矩形的宽度
 * @param height    矩形的高度
 * @param color     矩形的颜色（RGB565 格式）
 */
void lcd_gc9d01_draw_rect(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color);


/**
 * @brief GC9D01 LCD 显示字符串
 * 
 * @param handle     字符串句柄
 * @param format     字符串
 * @param ...        可变参数 格式化字符串参数
 */
void lcd_gc9d01_printf(lcd_string_handle_t *handle, const char *format, ...);

/**
 * @brief GC9D01 LCD 显示中文字符
 * 
 * @param x         字符的 x 坐标
 * @param y         字符的 y 坐标
 * @param color     字符的颜色（RGB565 格式）
 * @param chinese_word     中文字符数据指针
 * @param word_index     中文字符索引
 */
void lcd_gc9d01_chinese_display(uint16_t x, uint16_t y, uint16_t color, const uint8_t (*chinese_word)[32], uint8_t word_index);


#ifdef	CONFIG_LCD_GC9D01_IMAGE

/**
 * @brief GC9D01 LCD 显示图片
 * 
 * @param x_start   图片的 x 起始坐标
 * @param y_start   图片的 y 起始坐标
 * @param width     图片的宽度
 * @param height    图片的高度
 * @param ptr_pic   图片数据指针
 */
void lcd_gc9d01_picture_display(uint16_t x_start, uint16_t y_start, uint16_t width, uint16_t height, const uint8_t *ptr_pic);

#endif // CONFIG_LCD_GC9D01_IMAGE

#endif // LCD_GC9D01_H
