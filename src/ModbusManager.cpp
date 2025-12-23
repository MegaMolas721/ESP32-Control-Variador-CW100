#include "ModbusManager.h"

ModbusManager* ModbusManager::_instance = nullptr;

ModbusManager::ModbusManager() : 
    _client(nullptr),
    _gatewayPort(502),
    _slaveID(1),
    _initialized(false),
    _timeout(2000) {
    _instance = this;
}

void ModbusManager::setGateway(IPAddress ip, uint16_t port) {
    _gatewayIP = ip;
    _gatewayPort = port;
}

void ModbusManager::setSlaveID(uint8_t slaveID) {
    _slaveID = slaveID;
}

void ModbusManager::setTimeout(uint32_t timeout) {
    _timeout = timeout;
    if (_client) {
        _client->setTimeout(_timeout);
    }
}

void ModbusManager::onData(ModbusDataCallback callback) {
    _dataCallback = callback;
}

void ModbusManager::onError(ModbusErrorCallback callback) {
    _errorCallback = callback;
}

bool ModbusManager::begin() {
    Serial.println("\n=== INICIALIZANDO MODBUS TCP ===");
    Serial.printf("Gateway: %s:%d\n", _gatewayIP.toString().c_str(), _gatewayPort);
    Serial.printf("Slave ID: %d\n", _slaveID);
    
    // Crear cliente Modbus TCP con referencia a WiFiClient
    // IMPORTANTE: El constructor de ModbusClientTCP requiere un Client& como parámetro
    _client = new ModbusClientTCP(_wifiClient);
    
    // Configurar timeout
    _client->setTimeout(_timeout);
    
    // Registrar callbacks
    _client->onDataHandler(&ModbusManager::handleDataStatic);
    _client->onErrorHandler(&ModbusManager::handleErrorStatic);
    
    // Establecer el target (gateway IP y puerto)
    _client->setTarget(_gatewayIP, _gatewayPort);
    
    // Iniciar tarea de fondo del cliente Modbus
    _client->begin();
    
    _initialized = true;
    Serial.println("Modbus TCP inicializado correctamente");
    return true;
}

void ModbusManager::process() {
    // La librería eModbus maneja la comunicación en una tarea de fondo
    // No requiere llamadas periódicas en loop()
}

Error ModbusManager::readHoldingRegister(uint16_t address, uint16_t count, uint32_t token) {
    // addRequest con parámetros: token, serverID, function code, address, count
    return _client->addRequest(token, _slaveID, READ_HOLD_REGISTER, address, count);
}

Error ModbusManager::readInputRegister(uint16_t address, uint16_t count, uint32_t token) {
    return _client->addRequest(token, _slaveID, READ_INPUT_REGISTER, address, count);
}

Error ModbusManager::writeHoldingRegister(uint16_t address, uint16_t value, uint32_t token) {
    return _client->addRequest(token, _slaveID, WRITE_HOLD_REGISTER, address, value);
}

Error ModbusManager::writeMultipleRegisters(uint16_t address, uint16_t count, uint16_t* values, uint32_t token) {
    return _client->addRequest(token, _slaveID, WRITE_MULT_REGISTERS, address, count, count * 2, values);
}

bool ModbusManager::isReady() {
    return _initialized;
}

void ModbusManager::handleDataStatic(ModbusMessage response, uint32_t token) {
    if (_instance && _instance->_dataCallback) {
        _instance->_dataCallback(response, token);
    }
}

void ModbusManager::handleErrorStatic(Error error, uint32_t token) {
    if (_instance && _instance->_errorCallback) {
        _instance->_errorCallback(error, token);
    }
}
