# 🌸 FloraReader - Cute E-Reader for LilyGO T5S 2.7" E-Paper

**FloraReader** is a feature-rich, high-contrast, cute flower-themed e-reader software built for the **LilyGO T5S 2.7" E-Paper (ESP32)** board.

![FloraReader Floral Aesthetic](https://img.shields.io/badge/Board-LilyGO%20T5S%202.7%22-e88ca5?style=for-the-badge)
![Display](https://img.shields.io/badge/Display-264x176%20E--Paper-95d5b2?style=for-the-badge)

---

## Key Features

- **🌸 Cute Floral Theme**: Custom pixel-art corner vines, flower icons (daisy, rose, sakura), decorative dividers, and floral progress bar.
- **📖 Text & Markdown Book Reader**: Dynamic word wrapping, clean high-readability margins, line spacing, page indexing, and smooth navigation.
- **📲 iPhone WiFi Upload Portal**: Connect your iPhone to `FloraReader-WiFi` and upload `.txt` / `.md` books directly to the MicroSD card via `http://192.168.4.1` without pulling out the SD card.
- **🔖 Automatic Bookmarking**: Saves your exact reading position (page number and byte offset) in ESP32 Non-Volatile Storage (NVS).
- **🔋 Battery & Power Optimization**: Battery voltage reading via GPIO 35 ADC and deep sleep support for extended reading battery life.
- **🔘 Physical Button Navigation**: Intuitive controls using onboard buttons 37, 38, and 39.

---

## Codebase Architecture

```
nifty-borg/
├── platformio.ini               # PlatformIO project configuration & dependencies
├── README.md                    # Project overview & flashing instructions
├── docs/
│   ├── hardware_pinout_guide.md # Complete LilyGO T5S 2.7" pinout guide
│   └── user_guide.md            # Operating guide & WiFi upload instructions
├── include/
│   ├── Config.h                 # Hardware pins, screen dimensions, WiFi AP config
│   ├── FlowerTheme.h            # Bitmap graphics for cute flower icons & frames
│   ├── DisplayEngine.h          # GxEPD2 E-Paper UI rendering engine
│   ├── LibraryManager.h         # SD Card FATFS file system manager
│   ├── BookReaderEngine.h       # Text parsing, word wrap, pagination & bookmarks
│   ├── WebServerManager.h       # Async Web Server & WiFi AP portal
│   ├── BLEManager.h             # Bluetooth BLE service
│   └── PowerManager.h           # Battery ADC monitoring & deep sleep
├── src/
│   ├── main.cpp                 # Main application state machine & loop
│   ├── DisplayEngine.cpp        # UI rendering implementation
│   ├── LibraryManager.cpp       # Storage implementation
│   ├── BookReaderEngine.cpp     # Pagination implementation
│   ├── WebServerManager.cpp     # WiFi upload portal implementation
│   ├── BLEManager.cpp           # Bluetooth service implementation
│   └── PowerManager.cpp         # Battery monitoring implementation
└── web/
    └── index.html               # Mobile-optimized cute Web Upload interface
```

---

## How to Build & Flash Firmware

### Option A: Using PlatformIO (Recommended)
1. Install [PlatformIO IDE extension](https://platformio.org/) in VS Code or CLI.
2. Connect your LilyGO T5S board to your computer via USB-C.
3. *(If upload fails, temporarily remove the MicroSD card as GPIO 2 is a boot strapping pin).*
4. Run the upload command:
   ```bash
   pio run --target upload
   ```

### Option B: Using Arduino IDE
1. Open Arduino IDE, go to **Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module**.
2. Set **PSRAM** to **Enabled**.
3. Install required libraries via Library Manager:
   - `GxEPD2` by ZinggJM
   - `Adafruit GFX Library`
   - `ArduinoJson`
   - `ESPAsyncWebServer` & `AsyncTCP`
4. Compile and flash `src/main.cpp`.

---

## Documentation Links
* 📌 [Hardware Pinout & Specs Guide](docs/hardware_pinout_guide.md)
* 📌 [User & Operating Guide](docs/user_guide.md)
