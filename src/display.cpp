#include "display.h"

#define serial_debug Serial

static const unsigned char PROGMEM boot[] = 
{
    0x00, 0x00, 0x0F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xF8, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x1F, 0xC0, 0x00, 0x01, 0xFF, 0xFE, 0x00, 0x00, 0x7F, 0xF0,
    0x00, 0x01, 0xFF, 0xFE, 0x00, 0x00, 0xFF, 0xF8, 0x00, 0x00, 0xFF, 0xFC, 0x00, 0x01, 0xFF, 0xFC,
    0x00, 0x00, 0xFF, 0xFC, 0x00, 0x03, 0xFF, 0xFE, 0x00, 0x00, 0xFF, 0xFC, 0x00, 0x03, 0xFF, 0xFE,
    0x00, 0x01, 0xFF, 0xFC, 0x00, 0x07, 0xFF, 0xFE, 0x00, 0x01, 0xFF, 0xFC, 0x00, 0x07, 0xFF, 0xFF,
    0x00, 0x03, 0xFF, 0xFC, 0x00, 0x07, 0xFF, 0xFF, 0x00, 0x0F, 0xE0, 0xFC, 0x00, 0x07, 0xFF, 0xFF,
    0x07, 0xFF, 0xC0, 0x7E, 0x00, 0x0F, 0xFF, 0xFF, 0x1F, 0xFF, 0x80, 0x3E, 0x00, 0x0F, 0xFF, 0xFF,
    0x3F, 0xFF, 0x80, 0x1F, 0x00, 0x1F, 0xFF, 0xFE, 0x7F, 0xFF, 0x00, 0x1F, 0xC0, 0x3F, 0xFF, 0xFE,
    0x7F, 0xFF, 0x00, 0x0F, 0xF0, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFC,
    0xFF, 0xFF, 0x80, 0x0F, 0xFF, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0x80, 0x0F, 0xFF, 0xF0, 0x7F, 0xE0,
    0xFF, 0xFF, 0x80, 0x0F, 0xFF, 0xC0, 0x0F, 0x80, 0xFF, 0xFF, 0x80, 0x0F, 0xFF, 0x80, 0x00, 0x00,
    0x7F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00,
    0x7F, 0xFE, 0x00, 0x0F, 0xFE, 0x00, 0x00, 0x00, 0x3F, 0xFE, 0x00, 0x07, 0xFE, 0x00, 0x00, 0x00,
    0x1F, 0xF8, 0x00, 0x07, 0xFC, 0x00, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x01, 0xF8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x1C, 0x00, 0xF0, 0x00, 0xE0,
    0xF9, 0xE7, 0x9E, 0xFF, 0x07, 0xFE, 0x07, 0xFC, 0x79, 0xFF, 0x9F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFE,
    0x79, 0xFF, 0x9F, 0xFF, 0x8F, 0xFF, 0x9F, 0x1F, 0x79, 0xFC, 0x1F, 0x8F, 0x8F, 0x0F, 0x9E, 0x0F,
    0x79, 0xF0, 0x1E, 0x07, 0x8F, 0x07, 0x9E, 0x0F, 0x79, 0xF0, 0x1E, 0x07, 0x80, 0x07, 0x9F, 0x80,
    0x79, 0xF0, 0x1E, 0x07, 0x81, 0xFF, 0x8F, 0xF0, 0x79, 0xF0, 0x1E, 0x07, 0x87, 0xFF, 0x87, 0xFC,
    0x79, 0xF0, 0x1E, 0x07, 0x8F, 0xFF, 0x81, 0xFE, 0x79, 0xF0, 0x1E, 0x07, 0x9F, 0x0F, 0x80, 0x3F,
    0x79, 0xF0, 0x1E, 0x07, 0x9E, 0x07, 0x9E, 0x0F, 0x79, 0xF0, 0x1E, 0x07, 0x9E, 0x0F, 0x9E, 0x0F,
    0x79, 0xF0, 0x1E, 0x07, 0x9F, 0x3F, 0x9E, 0x1F, 0x79, 0xF0, 0x1E, 0x07, 0x8F, 0xFF, 0x9F, 0xFE,
    0x79, 0xF0, 0x1E, 0x07, 0x8F, 0xFF, 0x8F, 0xFE, 0x70, 0xE0, 0x1E, 0x07, 0x83, 0xE7, 0x03, 0xF8
};


/*
 * @brief Initialises display
 *
 * @return true if successful 
 */
void init_display()
{
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
}

/*
 * @brief Shows boot message, first Irnas Logo, then arribada pmp 
 *
 */
void boot_screen()
{
    display.clearDisplay();
    display.drawBitmap(32, 0, boot, 64, 64, 1);
    display.display();

    delay(2000); // Pause for 2 seconds

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("  ARRIBADA");
    display.println("    PMP");
    display.display();

    delay(2000); // Pause for 2 seconds

    display.clearDisplay();
    display.display();
}

void info_screen()
{
    specific_public_data_t print_data = s_PIRA->getter();
    time_t time_for_display = rtc_time_read();

#ifdef serial_debug
    serial_debug.println("Inside info screen:");
    serial_debug.println(ctime(&time_for_display));
#endif

    struct tm *timeinfo = localtime(&time_for_display);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    display.print("Time: ");
    display.print(timeinfo->tm_hour);
    display.print(":");

    if(timeinfo->tm_min < 10)
    {
      display.print("0");
      display.println(timeinfo->tm_min);
    }
    else
    {
      display.println(timeinfo->tm_min);
    }

    display.setCursor(0, 9);
    display.print("Date: ");
    display.print(timeinfo->tm_mday);
    display.print(".");
    display.print(1 + timeinfo->tm_mon);
    display.print(".");
    display.println(1900 + timeinfo->tm_year);

    display.drawFastHLine(0, 19, 128, 1);

    display.setCursor(0, 21);

    display.println("Photo count:");
    display.println(print_data.data_1);

    display.drawFastHLine(0, 38, 128, 1);
    display.setCursor(0, 40);

    display.println("Next Rpi wakeup:");
    display.print(print_data.data_2);
    display.print(" seconds");
    display.display();
    //RTC time
    //photo count
    //next wakeup pira
}
