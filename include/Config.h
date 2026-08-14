#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// LilyGO T5S 2.7" E-Paper Hardware Pins
// ==========================================

// Display Pins (GxEPD2 Driver for 2.7" 264x176 display)
#define EPD_CS          5
#define EPD_DC          17
#define EPD_RST         16
#define EPD_BUSY        4
#define EPD_MOSI        23
#define EPD_SCK         18

// Screen Dimensions
#define SCREEN_WIDTH    264
#define SCREEN_HEIGHT   176

// SD Card SPI Pins
#define SD_CS           13
#define SD_SCK          14
#define SD_MISO         2
#define SD_MOSI         15

// Navigation Buttons (LilyGO T5S Onboard Buttons)
#define BUTTON_PREV     37  // Left / Previous Page / Up
#define BUTTON_MENU     38  // Select / Open Menu / Mode Switch
#define BUTTON_NEXT     39  // Right / Next Page / Down

// Battery ADC Monitoring
#define BATTERY_ADC_PIN 35
#define BATT_V_MAX      4.20
#define BATT_V_MIN      3.20

// WiFi Access Point Settings (For iPhone Book Uploads)
#define WIFI_AP_SSID    "FloraReader-WiFi"
#define WIFI_AP_PASS    "flower123"  // Minimum 8 characters or empty for open AP
#define WEB_PORT        80

// NVS Storage Keys for Bookmarks & Preferences
#define NVS_NAMESPACE   "florareader"
#define KEY_LAST_BOOK   "last_book"
#define KEY_LAST_OFFSET "last_offset"
#define KEY_FONT_SIZE   "font_size"

// Reader Engine Defaults
#define MAX_LINE_LEN    48
#define LINES_PER_PAGE  8
#define BOOKS_DIR       "/books"

// Auto Page Turn Defaults
#define DEFAULT_AUTO_PAGE_SEC 15

#endif // CONFIG_H
