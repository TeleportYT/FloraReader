#include "DisplayEngine.h"
#include "HebrewFont.h"
#include <SD.h>

DisplayEngine::DisplayEngine() 
    : m_display(GxEPD2_270(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)), 
      m_pageTurnCount(0), 
      m_refreshInterval(10), 
      m_rotationMode(1) {
}

void DisplayEngine::begin() {
    SPI.begin(EPD_SCK, -1, EPD_MOSI, EPD_CS);
    m_display.init(115200, true, 2, false);
    m_display.setRotation(m_rotationMode);
    m_display.setTextColor(GxEPD_BLACK);
    m_display.setTextWrap(false);
}

void DisplayEngine::setRotationMode(int rot) {
    m_rotationMode = rot;
    m_display.setRotation(m_rotationMode);
}

void DisplayEngine::clearScreen() {
    m_display.setFullWindow();
    m_display.firstPage();
    do {
        m_display.fillScreen(GxEPD_WHITE);
    } while (m_display.nextPage());
}

void DisplayEngine::fullUpdate() {
    m_display.display(false);
}

void DisplayEngine::partialUpdate() {
    m_display.display(true);
}

void DisplayEngine::drawFloralBorder() {
    int w = getScreenWidth();
    int h = getScreenHeight();
    m_display.drawRect(2, 2, w - 4, h - 4, GxEPD_BLACK);
    m_display.drawRect(4, 4, w - 8, h - 8, GxEPD_BLACK);

    // Place 16x16 corner vines directly at (4,4) corners to prevent any text collisions
    m_display.drawBitmap(4, 4, epd_bitmap_corner_vine_16x16, 16, 16, GxEPD_BLACK);
    m_display.drawBitmap(w - 20, 4, epd_bitmap_corner_vine_16x16, 16, 16, GxEPD_BLACK);
    m_display.drawBitmap(4, h - 20, epd_bitmap_corner_vine_16x16, 16, 16, GxEPD_BLACK);
    m_display.drawBitmap(w - 20, h - 20, epd_bitmap_corner_vine_16x16, 16, 16, GxEPD_BLACK);
}

void DisplayEngine::drawFloralHeader(const char* title, int batteryPercent, bool wifiOn) {
    int w = getScreenWidth();
    m_display.fillRect(0, 0, w, 20, GxEPD_WHITE);
    m_display.drawFastHLine(0, 20, w, GxEPD_BLACK);
    
    m_display.drawBitmap(4, 4, epd_bitmap_daisy_12x12, 12, 12, GxEPD_BLACK);
    
    m_display.setTextSize(1);
    
    int rightSpace = wifiOn ? 55 : 35;
    int maxTitleChars = (w - 20 - rightSpace) / 6;
    if (maxTitleChars < 6) maxTitleChars = 6;
    if (maxTitleChars > 30) maxTitleChars = 30;

    char truncatedTitle[32];
    strncpy(truncatedTitle, title, maxTitleChars);
    truncatedTitle[maxTitleChars] = '\0';
    drawStringWithHebrew(20, 6, String(truncatedTitle), GxEPD_BLACK);

    if (wifiOn) {
        m_display.drawBitmap(w - 50, 4, epd_bitmap_wifi_12x12, 12, 12, GxEPD_BLACK);
    }
    
    int batX = w - 30;
    int batY = 5;
    m_display.drawBitmap(batX, batY, epd_bitmap_battery_frame_16x10, 16, 10, GxEPD_BLACK);
    
    int fillWidth = map(constrain(batteryPercent, 0, 100), 0, 100, 0, 10);
    if (fillWidth > 0) {
        m_display.fillRect(batX + 2, batY + 2, fillWidth, 6, GxEPD_BLACK);
    }
}

void DisplayEngine::drawFloralFooter(int currentPage, int totalPages, float progressPercent) {
    int w = getScreenWidth();
    int h = getScreenHeight();
    int footerY = h - 18;
    m_display.drawFastHLine(0, footerY, w, GxEPD_BLACK);
    
    m_display.setTextSize(1);
    m_display.setCursor(4, footerY + 5);
    m_display.printf("Pg %d/%d", currentPage, totalPages);
    
    m_display.setCursor(w - (w < 200 ? 45 : 55), footerY + 5);
    m_display.printf("%.0f%%", progressPercent);

    int barW = (w < 200) ? 40 : 80;
    int barX = (w - barW) / 2;
    int barY = footerY + 6;
    m_display.drawRect(barX, barY, barW, 6, GxEPD_BLACK);
    
    int fillW = (barW - 2) * (progressPercent / 100.0f);
    if (fillW > 0) {
        m_display.fillRect(barX + 1, barY + 1, fillW, 4, GxEPD_BLACK);
    }
}

void DisplayEngine::drawHebrewChar(int x, int y, uint8_t hIdx, uint16_t color) {
    if (hIdx >= 27) return;
    const uint8_t* fontGlyph = hebrew_font_5x7[hIdx];
    for (int col = 0; col < 5; col++) {
        uint8_t colData = pgm_read_byte(&fontGlyph[col]);
        for (int row = 0; row < 8; row++) {
            if (colData & (1 << row)) {
                m_display.drawPixel(x + col, y + row, color);
            }
        }
    }
}

void DisplayEngine::drawStringWithHebrew(int x, int y, const String& text, uint16_t color) {
    int w = getScreenWidth();
    bool hasHebrew = false;
    const uint8_t* p = (const uint8_t*)text.c_str();
    size_t len = text.length();

    for (size_t i = 0; i < len; i++) {
        if (p[i] == 0xD7 && i + 1 < len && p[i + 1] >= 0x90 && p[i + 1] <= 0xAA) {
            hasHebrew = true;
            break;
        }
    }

    if (!hasHebrew) {
        m_display.setTextColor(color);
        m_display.setCursor(x, y);
        m_display.print(text);
        return;
    }

    uint16_t bgColor = (color == GxEPD_WHITE) ? GxEPD_BLACK : GxEPD_WHITE;

    struct GlyphToken {
        bool isHebrew;
        uint8_t hIdx;
        char asciiChar;
    };
    std::vector<GlyphToken> tokens;

    size_t i = 0;
    while (i < len) {
        if (p[i] == 0xD7 && i + 1 < len && p[i + 1] >= 0x90 && p[i + 1] <= 0xAA) {
            GlyphToken t;
            t.isHebrew = true;
            t.hIdx = p[i + 1] - 0x90;
            tokens.push_back(t);
            i += 2;
        } else {
            GlyphToken t;
            t.isHebrew = false;
            t.asciiChar = (char)p[i];
            tokens.push_back(t);
            i++;
        }
    }

    int totalWidth = tokens.size() * 6;
    int curX = (x > 20) ? (x + totalWidth) : (w - 20);
    if (curX > w - 12) curX = w - 12;

    for (const auto& tok : tokens) {
        if (tok.isHebrew) {
            drawHebrewChar(curX - 6, y, tok.hIdx, color);
        } else {
            m_display.drawChar(curX - 6, y, tok.asciiChar, color, bgColor, 1);
        }
        curX -= 6;
        if (curX < 8) break;
    }
}

bool DisplayEngine::drawBmpImage(const String& path, int x, int y) {
    File bmpFile = SD.open(path, FILE_READ);
    if (!bmpFile) return false;

    uint16_t type = bmpFile.read() | (bmpFile.read() << 8);
    if (type != 0x4D42) {
        bmpFile.close();
        return false;
    }

    bmpFile.seek(10);
    uint32_t offset = bmpFile.read() | (bmpFile.read() << 8) | (bmpFile.read() << 16) | (bmpFile.read() << 24);

    bmpFile.seek(18);
    int32_t bmpWidth = bmpFile.read() | (bmpFile.read() << 8) | (bmpFile.read() << 16) | (bmpFile.read() << 24);
    int32_t bmpHeight = bmpFile.read() | (bmpFile.read() << 8) | (bmpFile.read() << 16) | (bmpFile.read() << 24);
    
    bmpFile.seek(28);
    uint16_t depth = bmpFile.read() | (bmpFile.read() << 8);
    
    bmpFile.seek(30);
    uint32_t compression = bmpFile.read() | (bmpFile.read() << 8) | (bmpFile.read() << 16) | (bmpFile.read() << 24);

    if (compression != 0) {
        bmpFile.close();
        return false;
    }

    bool flip = true;
    if (bmpHeight < 0) {
        bmpHeight = -bmpHeight;
        flip = false;
    }

    int screenW = getScreenWidth();
    int screenH = getScreenHeight();

    if (x == 0 && y == 0) {
        x = (screenW - bmpWidth) / 2;
        y = (screenH - bmpHeight) / 2;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
    }

    if (depth == 24) {
        uint32_t rowSize = ((bmpWidth * 3 + 3) / 4) * 4;
        uint8_t rowBuf[3 * 300];

        for (int row = 0; row < bmpHeight; row++) {
            int currentY = flip ? (y + bmpHeight - 1 - row) : (y + row);
            if (currentY < 0 || currentY >= screenH) continue;

            bmpFile.seek(offset + row * rowSize);
            size_t bytesToRead = min((size_t)rowSize, sizeof(rowBuf));
            bmpFile.read(rowBuf, bytesToRead);

            for (int col = 0; col < bmpWidth && (col * 3 + 2) < (int)bytesToRead; col++) {
                int currentX = x + col;
                if (currentX < 0 || currentX >= screenW) continue;

                int b = rowBuf[col * 3];
                int g = rowBuf[col * 3 + 1];
                int r = rowBuf[col * 3 + 2];

                uint8_t lum = (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);
                uint16_t color = (lum < 128) ? GxEPD_BLACK : GxEPD_WHITE;
                m_display.drawPixel(currentX, currentY, color);
            }
        }
        bmpFile.close();
        return true;
    } else if (depth == 1) {
        uint32_t rowSize = ((bmpWidth + 31) / 32) * 4;
        uint8_t rowBuf[64];

        for (int row = 0; row < bmpHeight; row++) {
            int currentY = flip ? (y + bmpHeight - 1 - row) : (y + row);
            if (currentY < 0 || currentY >= screenH) continue;

            bmpFile.seek(offset + row * rowSize);
            bmpFile.read(rowBuf, min((size_t)rowSize, sizeof(rowBuf)));

            for (int col = 0; col < bmpWidth; col++) {
                int currentX = x + col;
                if (currentX < 0 || currentX >= screenW) continue;

                uint8_t byteVal = rowBuf[col / 8];
                bool isBitSet = (byteVal & (0x80 >> (col % 8))) != 0;
                uint16_t color = isBitSet ? GxEPD_WHITE : GxEPD_BLACK;
                m_display.drawPixel(currentX, currentY, color);
            }
        }
        bmpFile.close();
        return true;
    }

    bmpFile.close();
    return false;
}

void DisplayEngine::renderMainMenu(int selectedIndex, int batteryPct) {
    int w = getScreenWidth();
    int h = getScreenHeight();
    m_display.setPartialWindow(0, 0, w, h);
    m_display.firstPage();
    do {
        m_display.fillScreen(GxEPD_WHITE);
        drawFloralBorder();
        
        m_display.setTextSize(2);
        int titleW = 11 * 12; // "FloraReader" length (132px)
        int titleX = (w - titleW) / 2;
        if (titleX < 22) titleX = 22;
        m_display.setCursor(titleX, 20);
        m_display.print("FloraReader");
        
        m_display.setTextSize(1);
        int subW = 16 * 6; // "~ Cute E-Paper ~" length (96px)
        m_display.setCursor((w - subW) / 2, 38);
        m_display.print("~ Cute E-Paper ~");

        const char* menuItems[] = {
            "Read Current Book",
            "Book Library",
            "WiFi Upload Portal",
            "Settings & Info"
        };
        
        int startY = 52;
        int itemH = 18;
        int spacing = 22;
        int itemX = 22;
        int itemW = w - 44;
        
        for (int i = 0; i < 4; i++) {
            int itemY = startY + (i * spacing);
            uint16_t bg = (i == selectedIndex) ? GxEPD_BLACK : GxEPD_WHITE;
            uint16_t fg = (i == selectedIndex) ? GxEPD_WHITE : GxEPD_BLACK;

            if (i == selectedIndex) {
                m_display.fillRect(itemX, itemY - 2, itemW, itemH, GxEPD_BLACK);
            } else {
                m_display.drawRect(itemX, itemY - 2, itemW, itemH, GxEPD_BLACK);
            }
            m_display.setTextColor(fg);

            if (i == 0) {
                m_display.drawBitmap(itemX + 6, itemY + 3, epd_bitmap_daisy_12x12, 12, 12, fg);
            } else if (i == 1) {
                m_display.drawBitmap(itemX + 6, itemY + 3, epd_bitmap_book_12x12, 12, 12, fg);
            } else if (i == 2) {
                m_display.drawBitmap(itemX + 6, itemY + 3, epd_bitmap_wifi_12x12, 12, 12, fg);
            } else {
                m_display.drawBitmap(itemX + 4, itemY + 1, epd_bitmap_sakura_16x16, 16, 16, fg);
            }

            m_display.setCursor(itemX + 24, itemY + 3);
            
            String itemText = String(menuItems[i]);
            int maxChars = (itemW - 28) / 6;
            if ((int)itemText.length() > maxChars && maxChars > 5) {
                itemText = itemText.substring(0, maxChars - 2) + "..";
            }
            m_display.print(itemText);
        }
        
        m_display.setTextColor(GxEPD_BLACK);
        m_display.setCursor(22, h - 14);
        if (w < 200) {
            m_display.printf("Bat:%d%%|[37][38][39]", batteryPct);
        } else {
            m_display.printf("Bat: %d%% | [37]Up [38]Ok [39]Down", batteryPct);
        }

    } while (m_display.nextPage());
}

void DisplayEngine::renderLibrary(const std::vector<String>& bookList, int selectedIndex, int topIndex, int totalBooks) {
    int w = getScreenWidth();
    int h = getScreenHeight();
    m_display.setPartialWindow(0, 0, w, h);
    m_display.firstPage();
    do {
        m_display.fillScreen(GxEPD_WHITE);
        drawFloralHeader("Book Library", 90, false);
        
        if (totalBooks == 0) {
            m_display.setTextSize(1);
            m_display.setCursor(16, 60);
            m_display.print("No books on SD Card!");
            m_display.setCursor(16, 80);
            m_display.print("Upload .txt files in /books/");
            m_display.setCursor(16, 100);
            m_display.print("via WiFi AP mode.");
        } else {
            int itemsPerPage = (h - 50) / 24;
            if (itemsPerPage < 3) itemsPerPage = 3;
            int startY = 28;
            
            for (int i = 0; i < itemsPerPage && (topIndex + i) < totalBooks; i++) {
                int idx = topIndex + i;
                int itemY = startY + (i * 24);
                
                if (idx == selectedIndex) {
                    m_display.fillRect(8, itemY - 2, w - 16, 20, GxEPD_BLACK);
                    m_display.setTextColor(GxEPD_WHITE);
                } else {
                    m_display.drawRect(8, itemY - 2, w - 16, 20, GxEPD_BLACK);
                    m_display.setTextColor(GxEPD_BLACK);
                }
                
                m_display.drawBitmap(12, itemY + 2, epd_bitmap_book_12x12, 12, 12, (idx == selectedIndex) ? GxEPD_WHITE : GxEPD_BLACK);
                
                String displayName = bookList[idx];
                int maxNameChars = (w - 36) / 6;
                if ((int)displayName.length() > maxNameChars) {
                    displayName = displayName.substring(0, maxNameChars - 3) + "...";
                }
                drawStringWithHebrew(28, itemY + 4, displayName, (idx == selectedIndex) ? GxEPD_WHITE : GxEPD_BLACK);
            }
        }
        
        m_display.setTextColor(GxEPD_BLACK);
        m_display.drawFastHLine(0, h - 18, w, GxEPD_BLACK);
        m_display.setCursor(8, h - 12);
        if (w < 200) {
            m_display.printf("%d Books | [38] Select", totalBooks);
        } else {
            m_display.printf("Total: %d Books | [38] Select", totalBooks);
        }

    } while (m_display.nextPage());
}

void DisplayEngine::renderReadingPage(const char* bookTitle, const std::vector<String>& pageLines, int currentPage, int totalPages, float progressPercent, bool autoPageOn, bool partial) {
    int w = getScreenWidth();
    int h = getScreenHeight();
    m_pageTurnCount++;

    bool usePartial = partial && (m_pageTurnCount % m_refreshInterval != 0);

    if (usePartial) {
        m_display.setPartialWindow(0, 0, w, h);
    } else {
        m_display.setFullWindow();
    }

    m_display.firstPage();
    do {
        m_display.fillScreen(GxEPD_WHITE);
        drawFloralHeader(bookTitle, 85, false);
        
        m_display.setTextSize(1);
        m_display.setTextColor(GxEPD_BLACK);
        
        int lineY = 28;
        int lineSpacing = 16;
        
        for (const auto& line : pageLines) {
            bool isDayHeader = (line.startsWith("Day ") || line.startsWith("[Day ") || line.startsWith("# Day "));
            if (isDayHeader) {
                m_display.drawFastHLine(8, lineY - 2, w - 16, GxEPD_BLACK);
                m_display.drawBitmap(8, lineY, epd_bitmap_sakura_16x16, 16, 16, GxEPD_BLACK);
                drawStringWithHebrew(28, lineY + 4, line);
                m_display.drawBitmap(w - 24, lineY, epd_bitmap_sakura_16x16, 16, 16, GxEPD_BLACK);
                lineY += 22;
            } else {
                drawStringWithHebrew(8, lineY, line);
                lineY += lineSpacing;
            }
            if (lineY > h - 24) break;
        }
        
        drawFloralFooter(currentPage, totalPages, progressPercent);

        // Auto page turn indicator
        if (autoPageOn) {
            int footerY = h - 18;
            m_display.setCursor(w - (w < 200 ? 88 : 100), footerY + 5);
            m_display.setTextColor(GxEPD_BLACK);
            m_display.print("\x10Auto");  // ▶ Auto
        }

    } while (m_display.nextPage());
}

void DisplayEngine::renderWiFiPortal(const char* ssid, const char* ipAddress, int fileCount) {
    int w = getScreenWidth();
    int h = getScreenHeight();
    m_display.setFullWindow();
    m_display.firstPage();
    do {
        m_display.fillScreen(GxEPD_WHITE);
        drawFloralHeader("WiFi Book Loader", 95, true);
        drawFloralBorder();
        
        int boxX = 22;
        int boxW = w - 44;

        m_display.setTextSize(1);
        m_display.setCursor(boxX, 28);
        m_display.print("Connect to WiFi AP:");
        
        m_display.fillRect(boxX, 42, boxW, 20, GxEPD_BLACK);
        m_display.setTextColor(GxEPD_WHITE);
        m_display.setCursor(boxX + 6, 48);
        m_display.printf("SSID: %s", ssid);
        
        m_display.setTextColor(GxEPD_BLACK);
        m_display.setCursor(boxX, 70);
        m_display.print("Open web browser to:");
        
        int ipLen = strlen(ipAddress);
        int ipFontScale = (ipLen * 12 > boxW) ? 1 : 2;
        m_display.setTextSize(ipFontScale);
        int ipW = ipLen * (ipFontScale == 2 ? 12 : 6);
        m_display.setCursor((w - ipW) / 2, 86);
        m_display.print(ipAddress);
        
        m_display.setTextSize(1);
        m_display.setCursor(boxX, 114);
        m_display.print("Upload .txt .md .bmp or .bin!");
        m_display.setCursor(boxX, 128);
        m_display.printf("SD Books: %d | OTA Ready", fileCount);
        
        m_display.setCursor(boxX, h - 14);
        m_display.print("Press [38] to Exit");

    } while (m_display.nextPage());
}

void DisplayEngine::renderSettingsMenu(int selectedIndex, int refreshInterval, int rotationMode, int autoPageInterval) {
    int w = getScreenWidth();
    int h = getScreenHeight();
    m_display.setPartialWindow(0, 0, w, h);
    m_display.firstPage();
    do {
        m_display.fillScreen(GxEPD_WHITE);
        drawFloralHeader("Settings", 95, false);
        drawFloralBorder();

        const char* rotStr = (rotationMode == 1) ? "Landscape" : "Portrait";
        char refreshStr[32];
        snprintf(refreshStr, sizeof(refreshStr), "Every %d Pages", refreshInterval);
        char autoPageStr[32];
        snprintf(autoPageStr, sizeof(autoPageStr), "Every %d Seconds", autoPageInterval);

        const char* labels[] = {
            "Orientation",
            "Full Refresh Rate",
            "Auto Page Speed",
            "Back to Main Menu"
        };
        const char* values[] = {
            rotStr,
            refreshStr,
            autoPageStr,
            ""
        };

        int totalItems = 4;
        int startY = 30;
        int itemX = 22;
        int itemW = w - 44;
        int itemH = (h < 200) ? 26 : 30;
        int itemSpacing = (h < 200) ? 30 : 36;

        for (int i = 0; i < totalItems; i++) {
            int itemY = startY + (i * itemSpacing);
            if (i == selectedIndex) {
                m_display.fillRect(itemX, itemY - 2, itemW, itemH, GxEPD_BLACK);
                m_display.setTextColor(GxEPD_WHITE);
            } else {
                m_display.drawRect(itemX, itemY - 2, itemW, itemH, GxEPD_BLACK);
                m_display.setTextColor(GxEPD_BLACK);
            }

            m_display.setTextSize(1);
            m_display.setCursor(itemX + 6, itemY + 3);
            m_display.print(labels[i]);

            if (values[i][0] != '\0') {
                m_display.setCursor(itemX + 6, itemY + 14);
                m_display.print(values[i]);
            }
        }

        m_display.setTextColor(GxEPD_BLACK);
        m_display.setCursor(22, h - 14);
        if (w < 200) {
            m_display.print("[37] [38]Select [39]");
        } else {
            m_display.print("[37]Up [38]Change/Ok [39]Down");
        }

    } while (m_display.nextPage());
}

void DisplayEngine::renderNotification(const char* message, const char* subtext) {
    int w = getScreenWidth();
    m_display.setFullWindow();
    m_display.firstPage();
    do {
        m_display.fillScreen(GxEPD_WHITE);
        drawFloralBorder();
        
        m_display.drawBitmap((w - 16) / 2, 22, epd_bitmap_sakura_16x16, 16, 16, GxEPD_BLACK);
        
        m_display.setTextSize(2);
        int msgLen = strlen(message) * 12;
        int msgX = (w - msgLen) / 2;
        if (msgX < 22) msgX = 22;
        m_display.setCursor(msgX, 50);
        m_display.print(message);
        
        m_display.setTextSize(1);
        int subLen = strlen(subtext) * 6;
        int subX = (w - subLen) / 2;
        if (subX < 22) subX = 22;
        m_display.setCursor(subX, 86);
        m_display.print(subtext);

    } while (m_display.nextPage());
}

void DisplayEngine::renderSleepScreen(int batteryPercent) {
    int w = getScreenWidth();
    int h = getScreenHeight();
    
    m_display.setFullWindow();
    m_display.firstPage();
    do {
        m_display.fillScreen(GxEPD_WHITE);
        
        // Search SD card for custom screen saver BMP image
        bool loadedBmp = false;
        const char* bmpPaths[] = {
            "/books/screensaver.bmp",
            "/images/screensaver.bmp",
            "/screensaver.bmp",
            "/books/idle.bmp",
            "/images/idle.bmp"
        };
        
        for (int i = 0; i < 5; i++) {
            if (SD.exists(bmpPaths[i])) {
                if (drawBmpImage(bmpPaths[i], 0, 0)) {
                    loadedBmp = true;
                    break;
                }
            }
        }

        if (!loadedBmp) {
            File booksDir = SD.open("/books");
            if (booksDir && booksDir.isDirectory()) {
                File f = booksDir.openNextFile();
                while (f) {
                    String fn = String(f.name());
                    if (fn.endsWith(".bmp") || fn.endsWith(".BMP")) {
                        String fullPath = String("/books/") + fn;
                        if (drawBmpImage(fullPath, 0, 0)) {
                            loadedBmp = true;
                            f.close();
                            break;
                        }
                    }
                    f = booksDir.openNextFile();
                }
                booksDir.close();
            }
        }

        if (!loadedBmp) {
            // Fallback: Render cute e-ink floral artwork with quote
            drawFloralBorder();
            
            int midX = w / 2;
            int midY = h / 2 - 10;
            
            m_display.drawBitmap(midX - 8, midY - 32, epd_bitmap_sakura_16x16, 16, 16, GxEPD_BLACK);
            m_display.drawBitmap(midX - 36, midY - 14, epd_bitmap_daisy_12x12, 12, 12, GxEPD_BLACK);
            m_display.drawBitmap(midX + 24, midY - 14, epd_bitmap_daisy_12x12, 12, 12, GxEPD_BLACK);
            
            m_display.setTextSize(2);
            int t1W = 11 * 12; // "FloraReader"
            m_display.setCursor((w - t1W) / 2, midY - 12);
            m_display.print("FloraReader");
            
            m_display.setTextSize(1);
            const char* q = "~ Read & Blossom ~";
            int qW = strlen(q) * 6;
            m_display.setCursor((w - qW) / 2, midY + 12);
            m_display.print(q);

            m_display.drawBitmap(midX - 8, midY + 28, epd_bitmap_sakura_16x16, 16, 16, GxEPD_BLACK);
            
            m_display.drawFastHLine(24, h - 26, w - 48, GxEPD_BLACK);
            
            m_display.setCursor(24, h - 18);
            m_display.printf("Bat:%d%% | Press button to wake", batteryPercent);
        } else {
            // Overlay status bar on custom image
            m_display.fillRect(0, h - 16, w, 16, GxEPD_WHITE);
            m_display.drawFastHLine(0, h - 16, w, GxEPD_BLACK);
            m_display.setTextColor(GxEPD_BLACK);
            m_display.setTextSize(1);
            m_display.setCursor(8, h - 12);
            m_display.printf("Bat:%d%% | Press button to wake", batteryPercent);
        }

    } while (m_display.nextPage());
}


