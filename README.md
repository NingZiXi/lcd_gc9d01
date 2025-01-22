# ESP32 LCD GC9D01 Driver [中文](README_CN.md)

## Introduction
This project is an LCD GC9D01 display driver library based on the ESP32 IDF framework. The GC9D01 is a common SPI interface LCD display controller, often used in small LCD screens (less than 1 inch) found on Taobao. The GC9D01 supports RGB565 color format.

This project provides basic graphic drawing functions such as drawing pixels, lines, rectangles, circles, triangles, etc., and supports displaying Chinese and English characters as well as images.

![picture](picture/picture.jpg)

To add the component to your project, execute the following command in the IDF terminal:

```cpp
idf.py add-dependency "ningzixi/lcd_gc9d01^1.0.0"
```
Alternatively, you can clone this repository directly into the `components` directory of your project:

```cpp
git clone https://github.com/NingZiXi/lcd_gc9d01.git
```
## Usage Example

In the **menuconfig** or **lcd_gc9d01.h** file, users can configure the pins and screen parameters according to the actual hardware connections.

```cpp
    lcd_gc9d01_init(); // Initialize GC9D01
    lcd_gc9d01_clear(); // Clear the screen
    
    // 1. Initialize the string handle
    lcd_string_handle_t lcd_string_handle = {
        .x = 10,
        .y = 50,
        .font = FONT_6x12,  //FONT_6x12 FONT_8x16  FONT_11x17 FONT_13x22 FONT_16x26
        .color = LCD_RED,
        .char_spacing = 1,  // Character spacing
        .line_spacing = 1   // Line spacing
    };
    
    //lcd_string_handle_t lcd_string_handle=LCD_STRING_HANDLE_DEFAULT(10, 70); // Or use the default macro to initialize the string handle

    // 2. Display a string
    lcd_gc9d01_printf(&lcd_string_handle, "Hello, World!\n");

    int value = 123;
    lcd_gc9d01_printf(&lcd_string_handle, "Value: %d", value);     // Display formatted string

```
For more API interfaces, please refer to [lcd_gc9d01.h](include/lcd_gc9d01.h).

## Contribution
This project is licensed under the **MIT** license. For more details, please refer to the [LICENSE](LICENSE) file.

**Note**: This project is still under development, and some features may not be fully implemented or may contain bugs. If you encounter any issues while using it, please feel free to contact the author or submit an Issue to help improve this project! Thanks to all contributors and the support of the open-source community!