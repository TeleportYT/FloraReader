#include "LibraryManager.h"

LibraryManager::LibraryManager() 
    : m_sdInitialized(false), m_spi(HSPI) {
}

bool LibraryManager::begin() {
    m_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    
    if (!SD.begin(SD_CS, m_spi, 20000000)) { // 20 MHz SPI speed for reliability
        Serial.println("[LibraryManager] SD Card mount failed!");
        m_sdInitialized = false;
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[LibraryManager] No SD Card attached!");
        m_sdInitialized = false;
        return false;
    }

    Serial.println("[LibraryManager] SD Card mounted successfully.");
    m_sdInitialized = true;

    // Ensure /books directory exists
    if (!SD.exists(BOOKS_DIR)) {
        SD.mkdir(BOOKS_DIR);
        Serial.println("[LibraryManager] Created /books directory.");
    }

    scanBooksDirectory();
    return true;
}

void LibraryManager::scanBooksDirectory() {
    m_bookList.clear();
    
    if (!m_sdInitialized) return;

    File root = SD.open(BOOKS_DIR);
    if (!root || !root.isDirectory()) {
        Serial.println("[LibraryManager] Failed to open /books directory!");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String name = String(file.name());
            if (name.endsWith(".txt") || name.endsWith(".TXT") || name.endsWith(".md") || name.endsWith(".MD")) {
                m_bookList.push_back(name);
                Serial.printf("[LibraryManager] Found book: %s (%d bytes)\n", name.c_str(), file.size());
            }
        }
        file = root.openNextFile();
    }
}

String LibraryManager::getBookPath(int index) const {
    if (index >= 0 && index < (int)m_bookList.size()) {
        return String(BOOKS_DIR) + "/" + m_bookList[index];
    }
    return "";
}

bool LibraryManager::saveBookFile(const char* filename, const uint8_t* data, size_t length) {
    if (!m_sdInitialized) return false;
    
    String fullPath = String(BOOKS_DIR) + "/" + String(filename);
    File file = SD.open(fullPath, FILE_WRITE);
    if (!file) {
        Serial.printf("[LibraryManager] Failed to open %s for writing\n", fullPath.c_str());
        return false;
    }
    
    size_t written = file.write(data, length);
    file.close();
    
    scanBooksDirectory();
    return written == length;
}

bool LibraryManager::deleteBookFile(const char* filename) {
    if (!m_sdInitialized) return false;
    
    String fullPath = String(BOOKS_DIR) + "/" + String(filename);
    if (SD.exists(fullPath)) {
        bool res = SD.remove(fullPath);
        scanBooksDirectory();
        return res;
    }
    return false;
}
