#include "IOManager.h"

IOManager::IOManager() {
    // Inicializar estados a false
    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
        _inputStates[i] = false;
    }
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
        _outputStates[i] = false;
    }
}

void IOManager::begin() {
    Serial.println("\n=== INICIALIZANDO I/O DEL PLC ===");
    
    // Configurar entradas
    Serial.println("Configurando entradas:");
    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
        pinMode(_inputPins[i], INPUT);
        Serial.printf("  Entrada %d: GPIO %d\n", i, _inputPins[i]);
    }
    
    // Configurar salidas
    Serial.println("Configurando salidas:");
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
        pinMode(_outputPins[i], OUTPUT);
        digitalWrite(_outputPins[i], LOW);
        Serial.printf("  Salida %d: GPIO %d\n", i, _outputPins[i]);
    }
    
    Serial.println("I/O inicializado");
}

// ============================================================
// ENTRADAS
// ============================================================

void IOManager::readInputs() {
    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
        _inputStates[i] = digitalRead(_inputPins[i]);
    }
}

bool IOManager::getInput(uint8_t index) {
    if (index < NUM_INPUTS) {
        return _inputStates[index];
    }
    return false;
}

bool IOManager::getInputState(uint8_t gpioPin) {
    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
        if (_inputPins[i] == gpioPin) {
            return _inputStates[i];
        }
    }
    return false;
}

void IOManager::printInputStates() {
    Serial.print("Entradas: ");
    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
        Serial.printf("%d", _inputStates[i] ? 1 : 0);
        if (i < NUM_INPUTS - 1) Serial.print(" ");
    }
    Serial.println();
}

// ============================================================
// SALIDAS
// ============================================================

void IOManager::setOutput(uint8_t index, bool state) {
    if (index < NUM_OUTPUTS) {
        _outputStates[index] = state;
        digitalWrite(_outputPins[index], state ? HIGH : LOW);
    }
}

void IOManager::setOutputByGPIO(uint8_t gpioPin, bool state) {
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
        if (_outputPins[i] == gpioPin) {
            setOutput(i, state);
            return;
        }
    }
}

bool IOManager::getOutput(uint8_t index) {
    if (index < NUM_OUTPUTS) {
        return _outputStates[index];
    }
    return false;
}

void IOManager::toggleOutput(uint8_t index) {
    if (index < NUM_OUTPUTS) {
        setOutput(index, !_outputStates[index]);
    }
}

void IOManager::allOutputsOff() {
    Serial.println("Apagando todas las salidas");
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
        setOutput(i, false);
    }
}

void IOManager::allOutputsOn() {
    Serial.println("Encendiendo todas las salidas");
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
        setOutput(i, true);
    }
}

void IOManager::printOutputStates() {
    Serial.print("Salidas: ");
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
        Serial.printf("%d", _outputStates[i] ? 1 : 0);
        if (i < NUM_OUTPUTS - 1) Serial.print(" ");
    }
    Serial.println();
}

// ============================================================
// INFORMACIÓN
// ============================================================

uint8_t IOManager::getInputPin(uint8_t index) {
    if (index < NUM_INPUTS) {
        return _inputPins[index];
    }
    return 0;
}

uint8_t IOManager::getOutputPin(uint8_t index) {
    if (index < NUM_OUTPUTS) {
        return _outputPins[index];
    }
    return 0;
}
