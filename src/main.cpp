#include <Arduino.h>
#include "Config.h"
#include "DisplayEngine.h"
#include "LibraryManager.h"
#include "BookReaderEngine.h"
#include "WebServerManager.h"
#include "BLEManager.h"
#include "PowerManager.h"

// Global Subsystem Instances
DisplayEngine    display;
LibraryManager   library;
BookReaderEngine reader;
WebServerManager webServer(library);
BLEManager       bleManager;
PowerManager     power;

// Application State Variables
UIState currentState = STATE_MAIN_MENU;
int menuSelection = 0;
int librarySelection = 0;
int libraryTopIndex = 0;
int settingsSelection = 0;

Preferences mainPrefs;

// Button & Idle Activity Timers
unsigned long lastBtnCheck = 0;
unsigned long lastActivityTime = 0;
const unsigned long IDLE_TIMEOUT_MS = 180000; // 3 Minutes (180,000 ms)
UIState stateBeforeSleep = STATE_MAIN_MENU;

void handleInput();
void updateUI();

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n==========================================");
    Serial.println("   FloraReader - LilyGO T5S 2.7\" E-Paper");
    Serial.println("==========================================");

    power.begin();
    
    pinMode(BUTTON_PREV, INPUT_PULLUP);
    pinMode(BUTTON_MENU, INPUT_PULLUP);
    pinMode(BUTTON_NEXT, INPUT_PULLUP);

    display.begin();

    mainPrefs.begin(NVS_NAMESPACE, true);
    int savedRot = mainPrefs.getInt("rotation", 1);
    int savedRefresh = mainPrefs.getInt("refresh_int", 10);
    mainPrefs.end();

    display.setRotationMode(savedRot);
    display.setRefreshInterval(savedRefresh);
    reader.setDisplayDimensions(display.getScreenWidth(), display.getScreenHeight());

    display.renderNotification("FloraReader", "Starting up...");

    if (!library.begin()) {
        display.renderNotification("SD Error!", "Please insert MicroSD card.");
        delay(2000);
    } else {
        reader.restoreLastReadBook();
    }

    bleManager.begin();

    lastActivityTime = millis();
    updateUI();
}

void loop() {
    if (currentState == STATE_WIFI_PORTAL) {
        webServer.loop();
    }

    // Trigger Idle Screen Saver after 3 minutes of inactivity (except in WiFi portal)
    if (currentState != STATE_SLEEP && currentState != STATE_WIFI_PORTAL) {
        if (millis() - lastActivityTime > IDLE_TIMEOUT_MS) {
            stateBeforeSleep = currentState;
            currentState = STATE_SLEEP;
            updateUI();
        }
    }

    handleInput();
    delay(20);
}

void handleInput() {
    if (millis() - lastBtnCheck < 250) return;

    bool btn37 = (digitalRead(BUTTON_PREV) == LOW);
    bool btn38 = (digitalRead(BUTTON_MENU) == LOW);
    bool btn39 = (digitalRead(BUTTON_NEXT) == LOW);

    if (!btn37 && !btn38 && !btn39) return;

    lastBtnCheck = millis();
    lastActivityTime = millis(); // Reset idle activity timer on any button press

    // Wake up from Idle Screen Saver on any button press
    if (currentState == STATE_SLEEP) {
        currentState = stateBeforeSleep;
        updateUI();
        return;
    }

    switch (currentState) {
        case STATE_MAIN_MENU:
            if (btn37) {
                menuSelection = (menuSelection - 1 + 4) % 4;
                updateUI();
            } else if (btn39) {
                menuSelection = (menuSelection + 1) % 4;
                updateUI();
            } else if (btn38) {
                if (menuSelection == 0) {
                    if (reader.getCurrentPage() > 0 && reader.getTotalPages() > 0) {
                        currentState = STATE_READING;
                    } else if (library.getBookCount() > 0) {
                        reader.openBook(library.getBookPath(0));
                        currentState = STATE_READING;
                    } else {
                        currentState = STATE_LIBRARY;
                    }
                } else if (menuSelection == 1) {
                    library.scanBooksDirectory();
                    librarySelection = 0;
                    libraryTopIndex = 0;
                    currentState = STATE_LIBRARY;
                } else if (menuSelection == 2) {
                    bleManager.stop();
                    delay(100);
                    webServer.begin();
                    currentState = STATE_WIFI_PORTAL;
                } else if (menuSelection == 3) {
                    settingsSelection = 0;
                    currentState = STATE_SETTINGS;
                }
                updateUI();
            }
            break;

        case STATE_LIBRARY:
            if (btn37) {
                if (librarySelection > 0) {
                    librarySelection--;
                    if (librarySelection < libraryTopIndex) libraryTopIndex = librarySelection;
                    updateUI();
                }
            } else if (btn39) {
                if (librarySelection < library.getBookCount() - 1) {
                    librarySelection++;
                    if (librarySelection >= libraryTopIndex + 5) libraryTopIndex++;
                    updateUI();
                }
            } else if (btn38) {
                if (library.getBookCount() > 0) {
                    String path = library.getBookPath(librarySelection);
                    if (reader.openBook(path)) {
                        currentState = STATE_READING;
                        updateUI();
                    }
                } else {
                    currentState = STATE_MAIN_MENU;
                    updateUI();
                }
            }
            break;

        case STATE_READING:
            if (btn37) {
                if (reader.prevPage()) {
                    updateUI();
                }
            } else if (btn39) {
                if (reader.nextPage()) {
                    updateUI();
                }
            } else if (btn38) {
                reader.saveBookmark();
                currentState = STATE_MAIN_MENU;
                updateUI();
            }
            break;

        case STATE_WIFI_PORTAL:
            if (btn38) {
                webServer.stop();
                delay(100);
                // Transition UI first so the screen updates even if BLE has issues
                currentState = STATE_MAIN_MENU;
                display.clearScreen();
                updateUI();
                lastBtnCheck = millis() + 500;
                // Non-critical tasks after UI is already showing main menu
                library.scanBooksDirectory();
                bleManager.begin();
            }
            break;

        case STATE_SETTINGS:
            if (btn37) {
                settingsSelection = (settingsSelection - 1 + 3) % 3;
                updateUI();
            } else if (btn39) {
                settingsSelection = (settingsSelection + 1) % 3;
                updateUI();
            } else if (btn38) {
                if (settingsSelection == 0) {
                    int newRot = (display.getRotationMode() == 1) ? 0 : 1;
                    display.setRotationMode(newRot);
                    reader.setDisplayDimensions(display.getScreenWidth(), display.getScreenHeight());
                    display.clearScreen();
                    mainPrefs.begin(NVS_NAMESPACE, false);
                    mainPrefs.putInt("rotation", newRot);
                    mainPrefs.end();
                    updateUI();
                } else if (settingsSelection == 1) {
                    int curRef = display.getRefreshInterval();
                    int newRef = (curRef == 5) ? 10 : (curRef == 10) ? 15 : (curRef == 15) ? 20 : 5;
                    display.setRefreshInterval(newRef);
                    mainPrefs.begin(NVS_NAMESPACE, false);
                    mainPrefs.putInt("refresh_int", newRef);
                    mainPrefs.end();
                    updateUI();
                } else if (settingsSelection == 2) {
                    currentState = STATE_MAIN_MENU;
                    updateUI();
                }
            }
            break;

        case STATE_SLEEP:
            currentState = stateBeforeSleep;
            updateUI();
            break;
    }
}

void updateUI() {
    int batPct = power.getBatteryPercentage();

    switch (currentState) {
        case STATE_MAIN_MENU:
            display.renderMainMenu(menuSelection, batPct);
            break;

        case STATE_LIBRARY:
            display.renderLibrary(library.getBookList(), librarySelection, libraryTopIndex, library.getBookCount());
            break;

        case STATE_READING:
            display.renderReadingPage(
                reader.getBookTitle().c_str(),
                reader.getCurrentPageLines(),
                reader.getCurrentPage(),
                reader.getTotalPages(),
                reader.getProgressPercent()
            );
            break;

        case STATE_WIFI_PORTAL:
            display.renderWiFiPortal(WIFI_AP_SSID, webServer.getIPAddress().c_str(), library.getBookCount());
            break;

        case STATE_SETTINGS:
            display.renderSettingsMenu(settingsSelection, display.getRefreshInterval(), display.getRotationMode());
            break;

        case STATE_SLEEP:
            display.renderSleepScreen(batPct);
            break;
    }
}

