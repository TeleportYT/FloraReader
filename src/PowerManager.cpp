#include "PowerManager.h"

PowerManager::PowerManager() {
}

void PowerManager::begin() {
    pinMode(BATTERY_ADC_PIN, INPUT);
    analogReadResolution(12); // 12-bit ADC (0 - 4095)
}

float PowerManager::getBatteryVoltage() {
    // Read raw ADC value from GPIO 35
    uint32_t rawSum = 0;
    for (int i = 0; i < 10; i++) {
        rawSum += analogRead(BATTERY_ADC_PIN);
        delay(2);
    }
    float rawAvg = (float)rawSum / 10.0f;
    
    // LilyGO T5S voltage divider ratio is 2.0 (100k / 100k)
    // 3.3V reference / 4095.0 * 2.0
    float voltage = (rawAvg / 4095.0f) * 3.3f * 2.0f;
    return voltage;
}

int PowerManager::getBatteryPercentage() {
    float v = getBatteryVoltage();
    if (v >= BATT_V_MAX) return 100;
    if (v <= BATT_V_MIN) return 0;
    
    int pct = (int)(((v - BATT_V_MIN) / (BATT_V_MAX - BATT_V_MIN)) * 100.0f);
    return constrain(pct, 0, 100);
}

void PowerManager::enterDeepSleep() {
    Serial.println("[Power] Entering deep sleep mode. Press Button 38 to wake up...");
    
    // Configure wake-up trigger on Button 38 (LOW level)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_MENU, 0);
    esp_deep_sleep_start();
}
