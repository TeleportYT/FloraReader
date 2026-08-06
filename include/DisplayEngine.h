#ifndef DISPLAY_ENGINE_H
#define DISPLAY_ENGINE_H

#include <Arduino.h>
#include <vector>
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include "Config.h"
#include "FlowerTheme.h"

// GxEPD2 constructor for LilyGO T5S 2.7" panel
// CS=5, DC=17, RST=16, BUSY=4
typedef GxEPD2_BW<GxEPD2_270, GxEPD2_270::HEIGHT> DisplayType;

enum UIState {
    STATE_MAIN_MENU,
    STATE_LIBRARY,
    STATE_READING,
    STATE_WIFI_PORTAL,
    STATE_SETTINGS
};

class DisplayEngine {
public:
    DisplayEngine();
    void begin();
    
    // Core Display Refresh Control
    void clearScreen();
    void fullUpdate();
    void partialUpdate();
    
    // Cute Floral Frame & Header/Footer
    void drawFloralHeader(const char* title, int batteryPercent, bool wifiOn);
    void drawFloralFooter(int currentPage, int totalPages, float progressPercent);
    void drawFloralBorder();
    
    // Fast Page Renderers with Partial Refresh support
    void renderMainMenu(int selectedIndex, int batteryPct);
    void renderLibrary(const std::vector<String>& bookList, int selectedIndex, int topIndex, int totalBooks);
    void renderReadingPage(const char* bookTitle, const std::vector<String>& pageLines, int currentPage, int totalPages, float progressPercent, bool partial = true);
    void renderWiFiPortal(const char* ssid, const char* ipAddress, int fileCount);
    void renderSettingsMenu(int selectedIndex, int refreshInterval, int rotationMode);
    void renderNotification(const char* message, const char* subtext);

    DisplayType& getDisplay() { return m_display; }
    void resetPageTurnCounter() { m_pageTurnCount = 0; }

    void setRefreshInterval(int interval) { m_refreshInterval = interval; }
    int getRefreshInterval() const { return m_refreshInterval; }

    int getScreenWidth() const { return m_display.width(); }
    int getScreenHeight() const { return m_display.height(); }

    void setRotationMode(int rot);
    int getRotationMode() const { return m_rotationMode; }

    void drawStringWithHebrew(int x, int y, const String& text, uint16_t color = GxEPD_BLACK);
    void drawHebrewChar(int x, int y, uint8_t hIdx, uint16_t color = GxEPD_BLACK);

private:
    DisplayType m_display;
    int m_pageTurnCount;
    int m_refreshInterval;
    int m_rotationMode; // 1 = Landscape (264x176), 0 = Portrait (176x264)
};

#endif // DISPLAY_ENGINE_H
