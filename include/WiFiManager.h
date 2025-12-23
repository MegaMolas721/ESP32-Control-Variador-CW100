#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "Config.h"

struct WiFiConnectionInfo {
    String ssid;
    String localIP;
    String gateway;
    String subnet;
    String dns;
    int rssi;
    bool isConnected;
    bool usesDHCP;
};

class WiFiManager {
public:
    WiFiManager();
    
    // Conexión inteligente (escanea y elige la mejor red)
    bool connectToBestNetwork();
    
    // Reconexión automática si se desconecta
    void handleReconnection();
    
    // Desconexión manual
    void disconnect();
    
    // Estado
    bool isConnected();
    WiFiConnectionInfo getConnectionInfo();
    
    // Getters
    String getSSID();
    String getLocalIP();
    String getGateway();
    String getSubnet();
    String getDNS();
    int getRSSI();
    bool usesDHCP() { return !_useStaticIP; }
    
private:
    bool _useStaticIP;
    bool _isManuallyDisconnected;
    String _connectedSSID;
    unsigned long _lastReconnectAttempt;
    const unsigned long _reconnectInterval = 10000; // 10 segundos
    
    // Escaneo y conexión
    int scanNetworks();
    bool connectToNetwork(const char* ssid, const char* password);
    
    // Configuración IP
    void configureStaticIP();
};

#endif
