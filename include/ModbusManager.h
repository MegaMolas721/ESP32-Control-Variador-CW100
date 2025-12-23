#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <WiFi.h>
#include <ModbusClientTCP.h>

// Callback types
typedef std::function<void(ModbusMessage response, uint32_t token)> ModbusDataCallback;
typedef std::function<void(Error error, uint32_t token)> ModbusErrorCallback;

class ModbusManager {
public:
    ModbusManager();
    
    // Configuración
    void setGateway(IPAddress ip, uint16_t port);
    void setSlaveID(uint8_t slaveID);
    void setTimeout(uint32_t timeout);
    
    // Callbacks
    void onData(ModbusDataCallback callback);
    void onError(ModbusErrorCallback callback);
    
    // Inicialización
    bool begin();
    
    // Procesamiento (llamar en loop)
    void process();
    
    // Lectura de registros
    Error readHoldingRegister(uint16_t address, uint16_t count, uint32_t token);
    Error readInputRegister(uint16_t address, uint16_t count, uint32_t token);
    
    // Escritura de registros
    Error writeHoldingRegister(uint16_t address, uint16_t value, uint32_t token);
    Error writeMultipleRegisters(uint16_t address, uint16_t count, uint16_t* values, uint32_t token);
    
    // Estado
    bool isReady();
    
private:
    WiFiClient _wifiClient;
    ModbusClientTCP* _client;
    IPAddress _gatewayIP;
    uint16_t _gatewayPort;
    uint8_t _slaveID;
    uint32_t _timeout;
    bool _initialized;
    
    ModbusDataCallback _dataCallback;
    ModbusErrorCallback _errorCallback;
    
    // Callbacks estáticos para eModbus
    static ModbusManager* _instance;
    static void handleDataStatic(ModbusMessage response, uint32_t token);
    static void handleErrorStatic(Error error, uint32_t token);
};

#endif
