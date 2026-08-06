#ifndef LIBRARY_MANAGER_H
#define LIBRARY_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <vector>
#include "Config.h"

class LibraryManager {
public:
    LibraryManager();
    bool begin();
    
    // Directory Scanning & File Access
    void scanBooksDirectory();
    const std::vector<String>& getBookList() const { return m_bookList; }
    int getBookCount() const { return m_bookList.size(); }
    String getBookPath(int index) const;
    
    // File Management
    bool saveBookFile(const char* filename, const uint8_t* data, size_t length);
    bool deleteBookFile(const char* filename);
    bool isSDReady() const { return m_sdInitialized; }

private:
    bool m_sdInitialized;
    SPIClass m_spi;
    std::vector<String> m_bookList;
};

#endif // LIBRARY_MANAGER_H
