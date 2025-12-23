#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <ESPAsyncWebServer.h>
#include "VFDController.h"
#include "IOManager.h"
#include "WiFiManager.h"

class WebDashboard {
public:
    WebDashboard(VFDController& vfd, IOManager& io, WiFiManager& wifi);
    
    // Inicialización
    void begin(uint16_t port = 80);
    
    // Estado
    bool isRunning();
    
private:
    VFDController& _vfd;
    IOManager& _io;
    WiFiManager& _wifi;
    AsyncWebServer _server;
    bool _running;
    
    // Handlers HTTP
    void setupRoutes();
    void handleRoot(AsyncWebServerRequest *request);
    void handleWiFiInfo(AsyncWebServerRequest *request);
    void handleWiFiDisconnect(AsyncWebServerRequest *request);
    void handleStatus(AsyncWebServerRequest *request);
    void handleCommand(AsyncWebServerRequest *request);
    void handleSetFrequency(AsyncWebServerRequest *request);
    
    // Generación de HTML
    String generateHTML();
    String generateStatusJSON();
    String generateWiFiInfoJSON();
};

#endif
