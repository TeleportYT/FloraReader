#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "LibraryManager.h"

class WebServerManager {
public:
    WebServerManager(LibraryManager& libManager);
    void begin();
    void stop();
    void loop();
    
    bool isRunning() const { return m_running; }
    String getIPAddress() const { return m_ipAddress; }

private:
    void setupRoutes();
    
    LibraryManager& m_libManager;
    AsyncWebServer m_server;
    DNSServer m_dnsServer;
    bool m_running;
    bool m_routesSetup;
    String m_ipAddress;
};

#endif // WEB_SERVER_MANAGER_H
