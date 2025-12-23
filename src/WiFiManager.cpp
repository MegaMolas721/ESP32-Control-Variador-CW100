#include "WiFiManager.h"

WiFiManager::WiFiManager() : 
    _useStaticIP(USE_STATIC_IP),
    _isManuallyDisconnected(false),
    _lastReconnectAttempt(0) {
}

bool WiFiManager::connectToBestNetwork() {
    Serial.println("\n=== ESCANEANDO REDES WIFI ===");
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Escanear redes disponibles
    int numNetworks = WiFi.scanNetworks();
    Serial.printf("Redes encontradas: %d\n", numNetworks);
    
    if (numNetworks == 0) {
        Serial.println("No se encontraron redes");
        return false;
    }
    
    // Buscar cuál de las redes conocidas tiene mejor señal
    int bestRSSI = -100;
    int bestNetworkIndex = -1;
    String bestSSID = "";
    String bestPassword = "";
    
    for (int i = 0; i < NUM_KNOWN_NETWORKS; i++) {
        String knownSSID = String(KNOWN_NETWORKS[i].ssid);
        
        // Buscar esta red en las escaneadas
        for (int j = 0; j < numNetworks; j++) {
            String scannedSSID = WiFi.SSID(j);
            int rssi = WiFi.RSSI(j);
            
            if (scannedSSID == knownSSID) {
                Serial.printf("  ✓ Red conocida encontrada: %s (RSSI: %d dBm)\n", 
                             scannedSSID.c_str(), rssi);
                
                // Si esta red tiene mejor señal, elegirla
                if (rssi > bestRSSI) {
                    bestRSSI = rssi;
                    bestNetworkIndex = i;
                    bestSSID = knownSSID;
                    bestPassword = String(KNOWN_NETWORKS[i].password);
                }
            }
        }
    }
    
    // Si no se encontró ninguna red conocida
    if (bestNetworkIndex == -1) {
        Serial.println("❌ No se encontró ninguna red conocida");
        return false;
    }
    
    // Conectar a la mejor red encontrada
    Serial.printf("\n▶ Conectando a la mejor red: %s (RSSI: %d dBm)\n", 
                  bestSSID.c_str(), bestRSSI);
    
    bool connected = connectToNetwork(bestSSID.c_str(), bestPassword.c_str());
    
    if (connected) {
        _connectedSSID = bestSSID;
        _isManuallyDisconnected = false;
        
        Serial.println("\n✓ ¡CONEXIÓN EXITOSA!");
        Serial.printf("  SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("  Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("  Subnet: %s\n", WiFi.subnetMask().toString().c_str());
        Serial.printf("  DNS: %s\n", WiFi.dnsIP().toString().c_str());
        Serial.printf("  RSSI: %d dBm\n", WiFi.RSSI());
        Serial.printf("  Tipo: %s\n", _useStaticIP ? "IP Estática" : "DHCP");
    }
    
    return connected;
}

bool WiFiManager::connectToNetwork(const char* ssid, const char* password) {
    // Configurar IP estática si está habilitada
    if (_useStaticIP) {
        IPAddress localIP(ESP32_IP);
        IPAddress gateway(ESP32_GATEWAY);
        IPAddress subnet(ESP32_SUBNET);
        IPAddress dns(ESP32_DNS);
        
        if (!WiFi.config(localIP, gateway, subnet, dns)) {
            Serial.println("⚠ Error configurando IP estática, usando DHCP");
            _useStaticIP = false;
        } else {
            Serial.printf("✓ IP estática configurada: %s\n", localIP.toString().c_str());
        }
    }
    
    WiFi.begin(ssid, password);
    
    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    return (WiFi.status() == WL_CONNECTED);
}

void WiFiManager::handleReconnection() {
    // Si se desconectó manualmente, no reconectar
    if (_isManuallyDisconnected) {
        return;
    }
    
    // Si ya está conectado, no hacer nada
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }
    
    // Limitar intentos de reconexión
    unsigned long now = millis();
    if (now - _lastReconnectAttempt < _reconnectInterval) {
        return;
    }
    
    _lastReconnectAttempt = now;
    
    Serial.println("\n⚠ Conexión WiFi perdida. Intentando reconectar...");
    connectToBestNetwork();
}

void WiFiManager::disconnect() {
    Serial.println("\n=== DESCONECTANDO WIFI ===");
    _isManuallyDisconnected = true;
    WiFi.disconnect(true);
    delay(100);
    Serial.println("✓ WiFi desconectado manualmente");
    Serial.println("Para reconectar, llama a connectToBestNetwork()");
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

WiFiConnectionInfo WiFiManager::getConnectionInfo() {
    WiFiConnectionInfo info;
    
    info.isConnected = isConnected();
    
    if (info.isConnected) {
        info.ssid = WiFi.SSID();
        info.localIP = WiFi.localIP().toString();
        info.gateway = WiFi.gatewayIP().toString();
        info.subnet = WiFi.subnetMask().toString();
        info.dns = WiFi.dnsIP().toString();
        info.rssi = WiFi.RSSI();
        info.usesDHCP = !_useStaticIP;
    } else {
        info.ssid = "No conectado";
        info.localIP = "0.0.0.0";
        info.gateway = "0.0.0.0";
        info.subnet = "0.0.0.0";
        info.dns = "0.0.0.0";
        info.rssi = 0;
        info.usesDHCP = true;
    }
    
    return info;
}

String WiFiManager::getSSID() {
    return WiFi.SSID();
}

String WiFiManager::getLocalIP() {
    return WiFi.localIP().toString();
}

String WiFiManager::getGateway() {
    return WiFi.gatewayIP().toString();
}

String WiFiManager::getSubnet() {
    return WiFi.subnetMask().toString();
}

String WiFiManager::getDNS() {
    return WiFi.dnsIP().toString();
}

int WiFiManager::getRSSI() {
    return WiFi.RSSI();
}
