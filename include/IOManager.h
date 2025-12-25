#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Configuración de pines del PLC ESP32 30 pines
// NUM_INPUTS y NUM_OUTPUTS se definen en config.h
// 9 entradas, 7 salidas (3 deshabilitadas por conflictos)

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
    
    // UI toggle states (controlados por botones o entradas físicas)
    void setReverseToggle(bool state);
    bool getReverseToggle() const;
    void setFreeToggle(bool state);
    bool getFreeToggle() const;
    // Control source: true = controlled by HW (GPIO), false = controlled by UI
    void setReverseControlByHW(bool state);
    bool getReverseControlByHW() const;
    void setFreeControlByHW(bool state);
    bool getFreeControlByHW() const;
    // Control flags for RUN/STOP (when true, UI commands are ignored)
    void setRunControlByHW(bool state);
    bool getRunControlByHW() const;
    void setStopControlByHW(bool state);
    bool getStopControlByHW() const;
    
private:
    // Pines de entradas (9 entradas optoacopladas)
    const uint8_t _inputPins[NUM_INPUTS] = {17, 18, 19, 21, 22, 23, 27, 32, 33};
    
    // Pines de salidas (7 salidas a relevador - GPIO1, GPIO12, GPIO15 deshabilitados)
    const uint8_t _outputPins[NUM_OUTPUTS] = {2, 3, 4, 5, 13, 14, 16};
    
    // Estados actuales
    bool _inputStates[NUM_INPUTS];
    bool _outputStates[NUM_OUTPUTS];
    // Estados de toggles vinculados a UI
    bool _reverseToggle;
    bool _freeToggle;
    // Control source flags
    bool _reverseControlByHW;
    bool _freeControlByHW;
    bool _runControlByHW;
    bool _stopControlByHW;
};

#endif
