#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "Config.h"

class PowerManager {
public:
    PowerManager();
    void begin();
    
    float getBatteryVoltage();
    int getBatteryPercentage();
    
    void enterDeepSleep();
};

#endif // POWER_MANAGER_H
