#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <Arduino.h>

// Configuración de pines del PLC ESP32 30 pines
#define NUM_INPUTS  9
#define NUM_OUTPUTS 10

class IOManager {
public:
    IOManager();
    
    // Inicialización
    void begin();
    
    // Entradas
    void readInputs();
    bool getInput(uint8_t index);
    bool getInputState(uint8_t gpioPin);
    void printInputStates();
    
    // Salidas
    void setOutput(uint8_t index, bool state);
    void setOutputByGPIO(uint8_t gpioPin, bool state);
    bool getOutput(uint8_t index);
    void toggleOutput(uint8_t index);
    void allOutputsOff();
    void allOutputsOn();
    void printOutputStates();
    
    // Información
    uint8_t getInputPin(uint8_t index);
    uint8_t getOutputPin(uint8_t index);
    
private:
    // Pines de entradas (9 entradas optoacopladas)
    const uint8_t _inputPins[NUM_INPUTS] = {17, 18, 19, 21, 22, 23, 27, 32, 33};
    
    // Pines de salidas (10 salidas a relevador)
    const uint8_t _outputPins[NUM_OUTPUTS] = {1, 2, 3, 4, 5, 12, 13, 14, 15, 16};
    
    // Estados actuales
    bool _inputStates[NUM_INPUTS];
    bool _outputStates[NUM_OUTPUTS];
};

#endif
