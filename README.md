# HEXIWEAR OLED SSD1351 (Mbed-OS)

This library drives the OLED display SSD1351 included in the [HEXIWEAR](https://www.mikroe.com/hexiwear) under Mbed-Os platform. 

This library is a re-work of the [Hexi_OLED_SSD1351](https://os.mbed.com/teams/Hexiwear/code/Hexi_OLED_SSD1351/) made by the HEXIEAR Project team made to fix some problems and customize it to my personal needs.

## Getting Started 

To use this library you can use the mbed-cli to include it in you project, just run this command on the console:

```console
$ mbed add url/of/library
``` 

## Features

- Fill Screen with a color
- Draw Pixels at given coordinates
- Fill a Rectangle with given dimensions
- Draw a Bitmap image at given coordinates
- Draw a Bitmap image full-screen with entering transitions
- Draw Text Box and Label at given coordinates:
    * Define Alignment: Left, Right, Center, Top, Bottom
    * Set custom Font-face (more on Fonts below)

## Fonts

Fonts are stored in a .h/.c file using a custom format made by the HEXIWEAR team. To generate new fonts you can use their custom tool that can be found at [this link]().

## Usage

Following is an example use of this library to display something on the OLED:

```c++
#include "oled/oled_ssd1351.h"
#include "oled/font/opensans_font.h"

int main()
{
    oled::SSD1351 oled(PTB22, PTB21, PTC13, PTB20, PTE6, PTD15);

    oled.fill_screen(oled::Color::BLACK);
    oled.draw_screen(image, oled::Transition::NONE);
    
    oled::TextProperties text_prop = {0};
    oled.get_text_properties(&text_prop);
    text_prop.font = OpenSans_18_Regular;    
    oled.set_text_properties(&text_prop);

    oled.label("hello", 0, 0);
    oled.label("word", 0, 20);
    
    oled::DynamicArea text_area = {
        .xCrd = 0,
        .yCrd = 40,
        .width = 30,
        .height = 20
    };
    oled.set_dynamic_area(text_area);
    oled.text_box("Test");

    oled::DynamicArea rect_area = {
        .xCrd = 10,
        .yCrd = 50,
        .width = 25,
        .height = 30
    };
    oled.set_dynamic_area(rect_area);
    oled.draw_box(oled::Color::RED);

    DigitalOut led(LED_RED);
    while (true)
    {
        led = !led;
        wait_ms(500);
    }
    return 0;
}
```

## TODO

- [ ] Draw Lines/Circles/Triangles
- [ ] Draw Box with stroke
- [ ] Use a more open font format (Like the one used by [Adafruit GFX](https://learn.adafruit.com/adafruit-gfx-graphics-library))
- [ ] Draw text longer than the display width by automatically create new lines