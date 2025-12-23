#include "VFDController.h"

VFDController::VFDController(ModbusManager& modbus) : 
    _modbus(modbus),
    _lastAutoUpdate(0) {
    memset(&_data, 0, sizeof(VFDData));
    _data.lastCommand = CMD_STOP;  // Inicializar en STOP (6) por defecto
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
    _modbus.writeHoldingRegister(REG_COMMAND, CMD_RUN, REG_COMMAND);
    _data.lastCommand = CMD_RUN;  // Registrar comando enviado (1)
}

void VFDController::stop() {
    Serial.println("⏹️ Deteniendo motor...");
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
    uint16_t freqValue = (uint16_t)(hz * 100);  // Convertir Hz a valor Modbus (Hz x 100)
    Serial.printf("Configurando frecuencia: %.2f Hz (0x%04X)\n", hz, freqValue);
    _modbus.writeHoldingRegister(REG_FREQ_SETPOINT, freqValue, REG_FREQ_SETPOINT);
}

// ============================================================
// LECTURA DE PARÁMETROS
// ============================================================

void VFDController::readStatus() {
    _modbus.readHoldingRegister(REG_VFD_STATUS, 1, REG_VFD_STATUS);
}

void VFDController::readFrequency() {
    _modbus.readHoldingRegister(REG_FREQ_ACTUAL, 1, REG_FREQ_ACTUAL);
}

void VFDController::readCurrent() {
    _modbus.readHoldingRegister(REG_CURRENT_OUT, 1, REG_CURRENT_OUT);
}

void VFDController::readVoltage() {
    _modbus.readHoldingRegister(REG_VOLTAGE_OUT, 1, REG_VOLTAGE_OUT);
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
    uint16_t value = (response[3] << 8) | response[4];
    
    // Actualizar timestamp
    _data.lastUpdate = millis();
    
    // Procesar según el registro (token)
    switch(token) {
        case REG_VFD_STATUS:
            _data.status = value;
            _data.isRunning = (value == STATUS_FORWARD || value == STATUS_REVERSE);
            Serial.printf("Estado VFD: %d (%s)\n", value, getStatusText().c_str());
            break;
            
        case REG_FREQ_ACTUAL:
            _data.frequencyActual = value;
            calculateSpeedPercent();
            Serial.printf("Frecuencia actual: %.2f Hz (%.2f%%)\n", 
                         value / 100.0, _data.speedPercent);
            break;
            
        case REG_CURRENT_OUT:
            _data.currentOut = value;
            Serial.printf("Corriente: %.2f A\n", value / 100.0);
            break;
            
        case REG_VOLTAGE_OUT:
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
            
        case REG_FREQ_SETPOINT:
            _data.frequencySetpoint = value;
            Serial.println("✓ Frecuencia configurada");
            break;
            
        case REG_COMMAND:
            Serial.println("✓ Comando enviado");
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

String VFDController::getStatusText() const {
    switch(_data.status) {
        case STATUS_FORWARD:  return "Adelante";
        case STATUS_REVERSE:  return "Atrás";
        case STATUS_STOP:     return "Stop";
        default:              return "Desconocido";
    }
}
