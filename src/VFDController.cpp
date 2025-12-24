#include "VFDController.h"

VFDController::VFDController(ModbusManager& modbus) : 
    _modbus(modbus),
    _lastAutoUpdate(0) {
    memset(&_data, 0, sizeof(VFDData));
    _data.lastCommand = CMD_STOP;  // Inicializar en STOP (6) por defecto
    _modbusLocked = false;
    _maxReadRetries = 2;
    _lastSetpointRequested = 0;
    _lastSetpointRead = 0;
}

void VFDController::begin() {
    Serial.println("\n=== INICIALIZANDO CONTROLADOR VFD ===");
    
    // Configurar callbacks para respuestas Modbus
    _modbus.onData([this](ModbusMessage response, uint32_t token) {
        this->handleModbusData(response, token);
    });
    
    _modbus.onError([this](Error error, uint32_t token) {
        this->handleModbusError(error, token);
    });
    
    Serial.println("Controlador VFD inicializado");
}

// ============================================================
// COMANDOS DE CONTROL
// ============================================================

void VFDController::start() {
    Serial.println("▶️ Arrancando motor...");
    Serial.printf("   Escribiendo: Registro=%d (0x%04X), Valor=%d\n", REG_COMMAND, REG_COMMAND, CMD_RUN);
    if (_modbusLocked) {
        unsigned long waitStart = millis();
        while (_modbusLocked && (millis() - waitStart) < 500) delay(10);
        if (_modbusLocked) {
            Serial.println("[WARN] Modbus ocupado, abortando arranque");
            return;
        }
    }
    _modbusLocked = true;
    _modbus.writeHoldingRegister(REG_COMMAND, CMD_RUN, REG_COMMAND);
    _data.lastCommand = CMD_RUN;  // Registrar comando enviado (1)
}

void VFDController::stop() {
    Serial.println("⏹️ Deteniendo motor...");
    Serial.printf("   Escribiendo: Registro=%d (0x%04X), Valor=%d\n", REG_COMMAND, REG_COMMAND, CMD_STOP);
    if (_modbusLocked) {
        unsigned long waitStart = millis();
        while (_modbusLocked && (millis() - waitStart) < 500) delay(10);
        if (_modbusLocked) {
            Serial.println("[WARN] Modbus ocupado, abortando paro");
            return;
        }
    }
    _modbusLocked = true;
    _modbus.writeHoldingRegister(REG_COMMAND, CMD_STOP, REG_COMMAND);
    _data.lastCommand = CMD_STOP;  // Registrar comando enviado (6)
}

void VFDController::resetFault() {
    Serial.println("🔄 Reseteando falla...");
    _modbus.writeHoldingRegister(REG_COMMAND, CMD_FAULT_RESET, REG_COMMAND);
    delay(100);
    _modbus.writeHoldingRegister(REG_COMMAND, CMD_STOP, REG_COMMAND);
    _data.lastCommand = CMD_STOP;  // Después del reset, queda en STOP (6)
}

void VFDController::setFrequency(float hz) {
    // Calcular porcentaje asumiendo 60Hz = 100%
    float percent = (hz / 60.0f) * 100.0f;
    uint16_t percentValue = (uint16_t)(percent * 100);  // Porcentaje x 100 (ej: 50% = 5000)
    Serial.printf("Configurando velocidad: %.2f Hz = %.2f%% (0x%04X)\n", hz, percent, percentValue);
    Serial.printf("   Escribiendo: Registro=%d (0x%04X), Valor=%d\n", REG_FREQUENCY_SETPOINT, REG_FREQUENCY_SETPOINT, percentValue);
    if (_modbusLocked) {
        unsigned long waitStart = millis();
        while (_modbusLocked && (millis() - waitStart) < 500) delay(10);
        if (_modbusLocked) {
            Serial.println("[WARN] Modbus ocupado, abortando escritura de setpoint");
            return;
        }
    }
    _modbusLocked = true;
    _lastSetpointRequested = percentValue;
    _modbus.writeHoldingRegister(REG_FREQUENCY_SETPOINT, percentValue, REG_FREQUENCY_SETPOINT);
    // Verificación: leer el holding register inmediatamente para confirmar escritura
    uint32_t verifyToken = REG_FREQUENCY_SETPOINT | 0x80000000u;
    delay(50);
    _modbus.readHoldingRegister(REG_FREQUENCY_SETPOINT, 1, verifyToken);
}

// ============================================================
// LECTURA DE PARÁMETROS
// ============================================================

void VFDController::readStatus() {
    _modbus.readHoldingRegister(REG_VFD_STATUS, 1, REG_VFD_STATUS);
}

void VFDController::readFrequency() {
    if (_modbusLocked) return; // Evitar solapamiento de lecturas
    _modbusLocked = true;
    Serial.printf("[DEBUG] Leyendo frecuencia desde registro %d (0x%04X) [holding register]\n", REG_FREQ_CURRENT, REG_FREQ_CURRENT);
    _modbus.readHoldingRegister(REG_FREQ_CURRENT, 1, REG_FREQ_CURRENT);
}

void VFDController::readCurrent() {
    _modbus.readHoldingRegister(REG_CURRENT_OUTPUT, 1, REG_CURRENT_OUTPUT);
}

void VFDController::readVoltage() {
    _modbus.readHoldingRegister(REG_VOLTAGE_OUTPUT, 1, REG_VOLTAGE_OUTPUT);
}

void VFDController::readFaultCode() {
    _modbus.readHoldingRegister(REG_FAULT_CODE, 1, REG_FAULT_CODE);
}

void VFDController::readAllParameters() {
    Serial.println("\n--- Leyendo parámetros del variador ---");
    readStatus();
    delay(50);
    readFrequency();
    delay(50);
    readCurrent();
    delay(50);
    readVoltage();
    delay(50);
    readFaultCode();
    delay(50);
    // Leer también el setpoint (holding register 1000H) para mantener sincronizado el UI
    _modbus.readHoldingRegister(REG_FREQUENCY_SETPOINT, 1, REG_FREQUENCY_SETPOINT);
}

// ============================================================
// ACTUALIZACIÓN PERIÓDICA
// ============================================================

void VFDController::update() {
    unsigned long now = millis();
    
    // Actualizar automáticamente cada cierto tiempo
    if (now - _lastAutoUpdate >= _autoUpdateInterval) {
        _lastAutoUpdate = now;
        readStatus();
        delay(50);
        readFrequency();
        delay(50);
        readCurrent();
    }
}

// ============================================================
// PROCESAMIENTO DE RESPUESTAS MODBUS
// ============================================================

void VFDController::handleModbusData(ModbusMessage response, uint32_t token) {
    uint8_t fc = response.getFunctionCode();
    
    // Verificar que hay suficientes datos
    if (response.size() < 5) return;
    
    // Extraer valor (los registros están en bytes 3 y 4)
    uint16_t value = 0;
    if (response.size() >= 5) {
        value = (response[3] << 8) | response[4];
    }
    
    // Actualizar timestamp
    _data.lastUpdate = millis();
    
    // Procesar según el registro (token)
    bool isVerify = (token & 0x80000000u) != 0;
    uint32_t bareToken = token & 0x7FFFFFFFu;
    if (isVerify) {
        Serial.printf("[VERIFY] Lectura verificación para registro 0x%04X: valor=%d (0x%04X)\n", bareToken, value, value);
        // Actualizar lectura cache
        if (bareToken == REG_FREQUENCY_SETPOINT) {
            _lastSetpointRead = value;
            if (value == 0 && _lastSetpointRequested != 0) {
                Serial.println("[ALERT] El setpoint 1000H fue escrito pero ahora lee 0: posible sobreescritura externa");
            }
        }
        // Liberar lock si era la verificación
        _modbusLocked = false;
        return;
    }

    switch(bareToken) {
        case REG_VFD_STATUS:
            _data.status = value;
            _data.isRunning = (value == STATUS_FORWARD || value == STATUS_REVERSE);
            Serial.printf("Estado VFD: %d (%s)\n", value, getStatusText().c_str());
            break;
            
        case REG_FREQ_CURRENT:
            _data.frequencyActual = value;
            calculateSpeedPercent();
            Serial.printf("Frecuencia actual: %.2f Hz (%.2f%%)\\n", 
                         value / 100.0, _data.speedPercent);
            // Liberar bloqueo si estaba leyendo frecuencia
            _modbusLocked = false;
            break;
            
        case REG_CURRENT_OUTPUT:
            _data.currentOut = value;
            Serial.printf("Corriente: %.2f A\n", value / 100.0);
            break;
            
        case REG_VOLTAGE_OUTPUT:
            _data.voltageOut = value;
            Serial.printf("Voltaje salida: %d V\n", value);
            break;
            
        case REG_FAULT_CODE:
            _data.faultCode = value;
            _data.hasFault = (value != 0);
            if (value != 0) {
                Serial.printf("⚠️ CÓDIGO DE FALLA: 0x%04X (%d)\n", value, value);
            }
            break;
            
        case REG_SPEED_ACTUAL:
            _data.speedActual = value;
            Serial.printf("Velocidad: %d\n", value);
            break;
            
        case REG_MAX_FREQUENCY:
            _data.maxFrequency = value;
            Serial.printf("Frecuencia máxima: %.2f Hz\n", value / 100.0);
            break;
            
        case REG_FREQUENCY_SETPOINT:
            _data.frequencySetpoint = value;
            _lastSetpointRead = value;
            Serial.println("✓ Frecuencia configurada");
            // Al recibir un valor 0 inesperado, avisar
            if (value == 0 && _lastSetpointRequested != 0) {
                Serial.println("[ALERT] Registro 1000H tiene valor 0 pero se solicitó diferente: posible sobreescritura externa");
            }
            // Liberar bloqueo si fue una respuesta a la escritura de setpoint
            _modbusLocked = false;
            break;
            
        case REG_COMMAND:
            Serial.println("✓ Comando enviado");
            // Liberar bloqueo si fue respuesta al comando
            _modbusLocked = false;
            break;
            
        default:
            Serial.printf("Respuesta reg 0x%04X: %d (0x%04X)\n", token, value, value);
            break;
    }
}

void VFDController::handleModbusError(Error error, uint32_t token) {
    ModbusError me(error);
    Serial.printf("Error Modbus [Reg 0x%04X]: %02X - %s\n", 
                  token, (int)error, (const char *)me);
    // Liberar bloqueo si hubo error en cualquiera de las operaciones que usamos con lock
    if (token == REG_FREQ_CURRENT || token == REG_COMMAND || token == REG_FREQUENCY_SETPOINT) {
        _modbusLocked = false;
    }
}

// ============================================================
// FUNCIONES AUXILIARES
// ============================================================

void VFDController::calculateSpeedPercent() {
    // Calcular porcentaje de velocidad nominal
    // Asumiendo frecuencia nominal de 60Hz = 100%
    const float nominalFreq = 60.0;
    _data.speedPercent = (_data.frequencyActual / 100.0) / nominalFreq * 100.0;
    if (_data.speedPercent > 100.0) _data.speedPercent = 100.0;
}

uint16_t VFDController::getEffectiveSetpoint() const {
    if (_lastSetpointRead != 0) return _lastSetpointRead;
    return _lastSetpointRequested;
}

String VFDController::getStatusText() const {
    switch(_data.status) {
        case STATUS_FORWARD:  return "Adelante";
        case STATUS_REVERSE:  return "Atrás";
        case STATUS_STOP:     return "Stop";
        default:              return "Desconocido";
    }
}
