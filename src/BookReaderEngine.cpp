#include "BookReaderEngine.h"

BookReaderEngine::BookReaderEngine() 
    : m_fileSize(0), m_currentPage(1), m_totalPages(1), m_screenWidth(264), m_screenHeight(176), m_fontSize(1) {
}

void BookReaderEngine::setFontSize(int size) {
    if (size < 0) size = 0;
    if (size > 2) size = 2;
    if (m_fontSize != size) {
        m_fontSize = size;
        Serial.printf("[BookReader] Font size changed to %s\n", size == 0 ? "Small" : size == 1 ? "Medium" : "Large");
        if (m_filePath.length() > 0) {
            int savedPage = m_currentPage;
            calculatePageOffsets();
            // Try to stay near the same reading position
            if (savedPage > m_totalPages) savedPage = m_totalPages;
            m_currentPage = savedPage;
        }
    }
}

int BookReaderEngine::getGlyphWidth() const {
    // All sizes use textSize(1) = 6px wide glyphs
    // Avoids crash with custom Hebrew bitmap rendering at textSize(2)
    return 6;
}

int BookReaderEngine::getLineHeight() const {
    switch (m_fontSize) {
        case 0: return 12;  // Small: compact, more text per page
        case 2: return 20;  // Large: spacious, easier to read
        default: return 16; // Medium: balanced default
    }
}

int BookReaderEngine::getTextSize() const {
    // Always textSize(1) — safe with custom Hebrew bitmap rendering
    return 1;
}

void BookReaderEngine::setDisplayDimensions(int width, int height) {
    if (m_screenWidth != width || m_screenHeight != height) {
        m_screenWidth = width;
        m_screenHeight = height;
        if (m_filePath.length() > 0) {
            calculatePageOffsets();
        }
    }
}

static int getGlyphCount(const String& str) {
    int count = 0;
    const uint8_t* p = (const uint8_t*)str.c_str();
    size_t len = str.length();
    for (size_t i = 0; i < len; i++) {
        if ((p[i] & 0xC0) != 0x80) { // Count leading UTF-8 bytes only
            count++;
        }
    }
    return count;
}

// ---------------------------------------------------------------------------
// UTF-8 & Hebrew Text Sanitizer
// Preserves standard ASCII and Hebrew UTF-8 range (U+05D0 .. U+05EA: 0xD7 0x90-0xAA)
// Replaces multi-byte UTF-8 smart quotes, dashes, ligatures with ASCII equivalents.
// ---------------------------------------------------------------------------
String BookReaderEngine::sanitizeText(const String& input) {
    String out = "";
    out.reserve(input.length());

    const uint8_t* p = (const uint8_t*)input.c_str();
    size_t len = input.length();

    for (size_t i = 0; i < len; i++) {
        uint8_t c = p[i];

        // Handle 2-byte UTF-8 sequences (Hebrew 0xD7 0x90 .. 0xD7 0xAA & Accents)
        if (c == 0xD7 && i + 1 < len) {
            uint8_t c2 = p[i + 1];
            if (c2 >= 0x90 && c2 <= 0xAA) { // Hebrew Alef to Tav
                out += (char)c;
                out += (char)c2;
                i++;
                continue;
            }
        }
        else if ((c & 0xE0) == 0xC0 && i + 1 < len) {
            uint8_t c2 = p[i + 1];
            if (c == 0xC2) {
                if (c2 == 0xA0) { out += ' '; i++; continue; }
                if (c2 == 0xAB || c2 == 0xBB) { out += '"'; i++; continue; }
            }
            if (c == 0xC3) {
                if (c2 >= 0x80 && c2 <= 0x85) { out += 'A'; i++; continue; }
                if (c2 >= 0xA0 && c2 <= 0xA5) { out += 'a'; i++; continue; }
                if (c2 >= 0x88 && c2 <= 0x8B) { out += 'E'; i++; continue; }
                if (c2 >= 0xA8 && c2 <= 0xAB) { out += 'e'; i++; continue; }
                if (c2 >= 0x8C && c2 <= 0x8F) { out += 'I'; i++; continue; }
                if (c2 >= 0xAC && c2 <= 0xAF) { out += 'i'; i++; continue; }
                if (c2 >= 0x92 && c2 <= 0x96) { out += 'O'; i++; continue; }
                if (c2 >= 0xB2 && c2 <= 0xB6) { out += 'o'; i++; continue; }
                if (c2 >= 0x99 && c2 <= 0x9C) { out += 'U'; i++; continue; }
                if (c2 >= 0xB9 && c2 <= 0xBC) { out += 'u'; i++; continue; }
            }
        }
        // Handle 3-byte UTF-8 sequences (Smart quotes / dashes / ligatures)
        else if (c == 0xE2 && i + 2 < len) {
            uint8_t c2 = p[i + 1];
            uint8_t c3 = p[i + 2];
            if (c2 == 0x80) {
                if (c3 == 0x98 || c3 == 0x99) { out += '\''; i += 2; continue; }
                if (c3 == 0x9C || c3 == 0x9D) { out += '"';  i += 2; continue; }
                if (c3 == 0x93 || c3 == 0x94) { out += '-';  i += 2; continue; }
                if (c3 == 0xA6)             { out += "..."; i += 2; continue; }
            }
        }

        if (c == '\r') continue;

        if (c >= 32 && c <= 126) {
            out += (char)c;
        } else if (c == '\n' || c == '\t') {
            out += (c == '\t' ? "  " : "\n");
        }
    }

    return out;
}

String BookReaderEngine::getShortKey(const String& filePath) {
    uint32_t hash = 5381;
    for (size_t i = 0; i < filePath.length(); i++) {
        hash = ((hash << 5) + hash) + (uint8_t)filePath[i];
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "bk_%08x", hash);
    return String(buf);
}

bool BookReaderEngine::openBook(const String& filePath) {
    closeBook();
    m_filePath = filePath;
    
    int lastSlash = filePath.lastIndexOf('/');
    if (lastSlash >= 0) {
        m_bookTitle = filePath.substring(lastSlash + 1);
    } else {
        m_bookTitle = filePath;
    }
    if (m_bookTitle.endsWith(".txt") || m_bookTitle.endsWith(".TXT")) {
        m_bookTitle = m_bookTitle.substring(0, m_bookTitle.length() - 4);
    }
    m_bookTitle = sanitizeText(m_bookTitle);

    File file = SD.open(filePath, FILE_READ);
    if (!file) {
        Serial.printf("[BookReader] Failed to open file: %s\n", filePath.c_str());
        return false;
    }
    
    m_fileSize = file.size();
    file.close();
    
    Serial.printf("[BookReader] Fast indexing %s (%d bytes)...\n", m_bookTitle.c_str(), m_fileSize);
    calculatePageOffsets();
    
    if (!loadBookmark(filePath)) {
        m_currentPage = 1;
        saveBookmark();
    }
    
    return true;
}

void BookReaderEngine::closeBook() {
    if (m_filePath.length() > 0) {
        saveBookmark();
    }
    m_pageOffsets.clear();
    m_filePath = "";
    m_bookTitle = "";
    m_fileSize = 0;
    m_currentPage = 1;
    m_totalPages = 1;
}

// ---------------------------------------------------------------------------
// High-Speed 1KB Chunked Page Indexing
// ---------------------------------------------------------------------------
void BookReaderEngine::calculatePageOffsets() {
    m_pageOffsets.clear();
    m_pageOffsets.push_back(0); // Page 1 starts at byte 0
    
    File file = SD.open(m_filePath, FILE_READ);
    if (!file) return;

    int glyphW = getGlyphWidth();
    int lineH = getLineHeight();
    int maxCharsPerLine = (m_screenWidth - 16) / glyphW;
    int linesPerPage = (m_screenHeight - 44) / lineH;
    if (maxCharsPerLine < 10) maxCharsPerLine = 10;
    if (linesPerPage < 3) linesPerPage = 3;
    int currentLineCount = 0;

    char lineBuf[512];
    size_t linePos = 0;
    size_t filePos = 0;
    
    uint8_t chunk[1024];

    while (file.available()) {
        size_t bytesRead = file.read(chunk, sizeof(chunk));
        for (size_t b = 0; b < bytesRead; b++) {
            uint8_t c = chunk[b];
            filePos++;

            if (c == '\n' || linePos >= sizeof(lineBuf) - 1) {
                lineBuf[linePos] = '\0';
                String line = sanitizeText(String(lineBuf));
                linePos = 0;

                // Check for explicit Page/Day Breaks (\f or line starting with "Day " or "[Day")
                bool isDayHeader = (line.startsWith("Day ") || line.startsWith("Day\t") || 
                                    line.startsWith("[Day ") || line.startsWith("# Day "));
                bool isFormFeed = (line.indexOf('\f') >= 0);

                if ((isDayHeader || isFormFeed) && currentLineCount > 0) {
                    if (filePos < m_fileSize && m_pageOffsets.back() != (filePos - line.length() - 1)) {
                        m_pageOffsets.push_back(filePos - line.length() - 1);
                        currentLineCount = 0;
                    }
                }

                std::vector<String> wrappedLines = wrapTextToLines(line, maxCharsPerLine);
                if (wrappedLines.empty()) {
                    currentLineCount++;
                } else {
                    for (size_t i = 0; i < wrappedLines.size(); i++) {
                        currentLineCount++;
                        if (currentLineCount >= linesPerPage) {
                            if (filePos < m_fileSize && m_pageOffsets.back() != filePos) {
                                m_pageOffsets.push_back(filePos);
                                currentLineCount = 0;
                            }
                        }
                    }
                }
            } else if (c != '\r') {
                lineBuf[linePos++] = (char)c;
            }
        }
    }

    if (linePos > 0) {
        lineBuf[linePos] = '\0';
        String line = sanitizeText(String(lineBuf));
        std::vector<String> wrappedLines = wrapTextToLines(line, maxCharsPerLine);
        for (size_t i = 0; i < wrappedLines.size(); i++) {
            currentLineCount++;
            if (currentLineCount >= linesPerPage && filePos < m_fileSize) {
                m_pageOffsets.push_back(filePos);
                currentLineCount = 0;
            }
        }
    }

    file.close();
    m_totalPages = m_pageOffsets.size();
    if (m_totalPages == 0) m_totalPages = 1;

    Serial.printf("[BookReader] Fast Indexing complete: %d total pages.\n", m_totalPages);
}

std::vector<String> BookReaderEngine::wrapTextToLines(const String& text, int maxCharsPerLine) {
    std::vector<String> result;
    if (text.length() == 0) return result;

    int totalGlyphs = getGlyphCount(text);
    if (totalGlyphs <= maxCharsPerLine) {
        result.push_back(text);
        return result;
    }

    int start = 0;
    int len = text.length();

    while (start < len) {
        // Find split index by counting glyphs
        int glyphsInSub = 0;
        int end = start;
        while (end < len && glyphsInSub < maxCharsPerLine) {
            if ((text[end] & 0xC0) != 0x80) {
                glyphsInSub++;
            }
            end++;
        }

        if (end >= len) {
            result.push_back(text.substring(start));
            break;
        }

        // Search backwards for space
        int spaceIdx = text.lastIndexOf(' ', end);
        if (spaceIdx > start) {
            result.push_back(text.substring(start, spaceIdx));
            start = spaceIdx + 1;
        } else {
            result.push_back(text.substring(start, end));
            start = end;
        }
    }

    return result;
}

std::vector<String> BookReaderEngine::getCurrentPageLines() {
    std::vector<String> pageLines;
    if (m_filePath.length() == 0 || m_currentPage < 1 || m_currentPage > (int)m_pageOffsets.size()) {
        return pageLines;
    }

    File file = SD.open(m_filePath, FILE_READ);
    if (!file) return pageLines;

    size_t startByte = m_pageOffsets[m_currentPage - 1];
    file.seek(startByte);

    int glyphW = getGlyphWidth();
    int lineH = getLineHeight();
    int maxCharsPerLine = (m_screenWidth - 16) / glyphW;
    int linesInPage = (m_screenHeight - 44) / lineH;
    if (maxCharsPerLine < 10) maxCharsPerLine = 10;
    if (linesInPage < 3) linesInPage = 3;

    while (file.available() && (int)pageLines.size() < linesInPage) {
        size_t currentPos = file.position();
        if (m_currentPage < (int)m_pageOffsets.size() && currentPos >= m_pageOffsets[m_currentPage]) {
            break;
        }

        String rawLine = file.readStringUntil('\n');
        String line = sanitizeText(rawLine);

        std::vector<String> wrapped = wrapTextToLines(line, maxCharsPerLine);
        if (wrapped.empty()) {
            pageLines.push_back("");
        } else {
            for (const auto& w : wrapped) {
                pageLines.push_back(w);
                if ((int)pageLines.size() >= linesInPage) break;
            }
        }
    }

    file.close();
    return pageLines;
}

bool BookReaderEngine::nextPage() {
    if (m_currentPage < m_totalPages) {
        m_currentPage++;
        saveBookmark();
        return true;
    }
    return false;
}

bool BookReaderEngine::prevPage() {
    if (m_currentPage > 1) {
        m_currentPage--;
        saveBookmark();
        return true;
    }
    return false;
}

bool BookReaderEngine::jumpToPage(int pageNum) {
    if (pageNum >= 1 && pageNum <= m_totalPages) {
        m_currentPage = pageNum;
        saveBookmark();
        return true;
    }
    return false;
}

float BookReaderEngine::getProgressPercent() const {
    if (m_totalPages <= 1) return 100.0f;
    return ((float)m_currentPage / (float)m_totalPages) * 100.0f;
}

void BookReaderEngine::saveBookmark() {
    if (m_filePath.length() == 0) return;
    
    String shortKey = getShortKey(m_filePath);

    m_prefs.begin(NVS_NAMESPACE, false);
    m_prefs.putString(KEY_LAST_BOOK, m_filePath);
    m_prefs.putInt(shortKey.c_str(), m_currentPage);
    m_prefs.end();
    
    Serial.printf("[BookReader] Saved bookmark: %s -> Page %d/%d\n", m_filePath.c_str(), m_currentPage, m_totalPages);
}

bool BookReaderEngine::loadBookmark(const String& filePath) {
    String shortKey = getShortKey(filePath);

    m_prefs.begin(NVS_NAMESPACE, true);
    int savedPage = m_prefs.getInt(shortKey.c_str(), 0);
    m_prefs.end();

    if (savedPage >= 1 && savedPage <= m_totalPages) {
        m_currentPage = savedPage;
        Serial.printf("[BookReader] Loaded bookmark: %s -> Page %d/%d\n", filePath.c_str(), m_currentPage, m_totalPages);
        return true;
    }
    return false;
}

bool BookReaderEngine::restoreLastReadBook() {
    m_prefs.begin(NVS_NAMESPACE, true);
    String lastBook = m_prefs.getString(KEY_LAST_BOOK, "");
    m_prefs.end();

    if (lastBook.length() > 0 && SD.exists(lastBook)) {
        Serial.printf("[BookReader] Restoring last read book from NVS: %s\n", lastBook.c_str());
        return openBook(lastBook);
    }
    return false;
}

