# Hardware Pinout & Specs Guide: LilyGO T5S 2.7" E-Paper

This document serves as the hardware reference guide for the **LilyGO T5S 2.7-inch E-Paper** board used by FloraReader.

---

## Technical Specifications

| Feature | Specification |
| :--- | :--- |
| **Microcontroller** | ESP32-D0WDQ6 (Dual-core 240 MHz Xtensa LX6) |
| **Wireless** | 802.11 b/g/n Wi-Fi + Bluetooth 4.2 BLE |
| **Display** | 2.7" Monochrome E-Paper Display (264 x 176 pixels, SPI) |
| **Driver IC** | UC8176 / GDEW027W3 (GxEPD2 class `GxEPD2_270`) |
| **Storage** | MicroSD Card slot (FAT32, SPI mode) + ESP32 NVS |
| **Buttons** | 1 Reset Button + 3 Navigation Buttons (IO37, IO38, IO39) |
| **Battery Power** | TP4054 LiPo Charger + Battery ADC monitor on GPIO 35 |

---

## Pinout Mapping Table

### 1. E-Paper Display SPI Connection
* **EPD CS**: GPIO 5
* **EPD DC**: GPIO 17
* **EPD RST**: GPIO 16
* **EPD BUSY**: GPIO 4
* **SPI MOSI**: GPIO 23
* **SPI SCK**: GPIO 18

### 2. MicroSD Card Reader Pins
* **SD CS**: GPIO 13
* **SD SCK**: GPIO 14
* **SD MISO**: GPIO 2
* **SD MOSI**: GPIO 15

> [!WARNING]
> **Important Note regarding GPIO 2**: GPIO 2 is used for SD MISO and is also an ESP32 boot strapping pin. If you encounter errors while flashing firmware via USB-C, temporarily eject the MicroSD card until flashing completes.

### 3. Onboard User Buttons
* **Button 37**: Navigation (Previous Page / Up / Move Highlight Up)
* **Button 38**: Menu / Select (Open Selected Book / Select Menu / Exit WiFi Portal)
* **Button 39**: Navigation (Next Page / Down / Move Highlight Down)

### 4. Battery Monitoring
* **ADC Pin**: GPIO 35
* Voltage Divider Ratio: 100k / 100k (2.0x multiplier)
* Max Voltage: 4.20V (100%)
* Min Voltage: 3.20V (0%)
