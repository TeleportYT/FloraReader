#ifndef BOOK_READER_ENGINE_H
#define BOOK_READER_ENGINE_H

#include <Arduino.h>
#include <SD.h>
#include <Preferences.h>
#include <vector>
#include "Config.h"

class BookReaderEngine {
public:
    BookReaderEngine();
    
    // Book Operations
    bool openBook(const String& filePath);
    void closeBook();
    
    // Page Navigation
    bool nextPage();
    bool prevPage();
    bool jumpToPage(int pageNum);
    
    // Font Size (0=Small, 1=Medium, 2=Large)
    void setFontSize(int size);
    int getFontSize() const { return m_fontSize; }
    int getGlyphWidth() const;
    int getLineHeight() const;
    int getTextSize() const;
    
    // Getters for Display Engine
    std::vector<String> getCurrentPageLines();
    int getCurrentPage() const { return m_currentPage; }
    int getTotalPages() const { return m_totalPages; }
    float getProgressPercent() const;
    String getBookTitle() const { return m_bookTitle; }
    
    // Bookmark Management (ESP32 NVS Storage)
    void saveBookmark();
    bool loadBookmark(const String& filePath);
    bool restoreLastReadBook();

    // Text Sanitizer & Helper
    static String sanitizeText(const String& input);
    void setDisplayDimensions(int width, int height);

private:
    void calculatePageOffsets();
    std::vector<String> wrapTextToLines(const String& text, int maxCharsPerLine);
    String getShortKey(const String& filePath);

    String m_filePath;
    String m_bookTitle;
    size_t m_fileSize;
    
    int m_currentPage;
    int m_totalPages;
    int m_screenWidth;
    int m_screenHeight;
    int m_fontSize;  // 0=Small, 1=Medium(default), 2=Large
    
    // Byte offset index of each page start position
    std::vector<size_t> m_pageOffsets;
    
    Preferences m_prefs;
};

#endif // BOOK_READER_ENGINE_H
