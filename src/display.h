#ifndef DISPLAY_H
#define DISPLAY_H

#include <cstddef>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "rtc.h"
#include "module_pira.h"
#include "module.h"


#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

extern Adafruit_SSD1306 display;

void init_display();
void boot_screen();
void info_screen();

#endif
