#ifndef VFD_CONTROLLER_H
#define VFD_CONTROLLER_H

#include <Arduino.h>
#include "ModbusManager.h"
#include "config.h"  // Incluir config.h para usar las definiciones de registros

// Estados del variador (Registro REG_VFD_STATUS)
#define STATUS_FORWARD          1       // 1 = Adelante
#define STATUS_REVERSE          2       // 2 = Atrás
#define STATUS_STOP             3       // 3 = Stop

// Estructura de datos del variador
struct VFDData {
    uint16_t status;            // Estado actual (1=Adelante, 2=Atrás, 3=Stop)
    uint16_t frequencySetpoint; // Frecuencia setpoint (Hz x 100)
    uint16_t frequencyActual;   // Frecuencia actual (Hz x 100)
    uint16_t currentOut;        // Corriente de salida (A x 100)
    uint16_t voltageOut;        // Voltaje de salida (V)
    uint16_t speedActual;       // Velocidad actual
    uint16_t faultCode;         // Código de falla
    uint16_t maxFrequency;      // Frecuencia máxima
    uint16_t upperLimit;        // Límite superior
    uint16_t lastCommand;       // Último comando enviado al registro 2000H (1=RUN, 6=STOP)
    float speedPercent;         // Porcentaje de velocidad nominal (0-100%)
    bool isRunning;             // Motor corriendo
    bool hasFault;              // Hay falla activa
    unsigned long lastUpdate;   // Timestamp última actualización
};

class VFDController {
public:
    VFDController(ModbusManager& modbus);
    
    // Inicialización
    void begin();
    
    // Comandos de control
    void start();                       // Arrancar motor
    void stop();                        // Detener motor
    void resetFault();                  // Resetear falla
    void setFrequency(float hz);        // Configurar frecuencia en Hz
    
    // Lectura de parámetros
    void readStatus();                  // Leer estado
    void readFrequency();               // Leer frecuencia actual
    void readCurrent();                 // Leer corriente
    void readVoltage();                 // Leer voltaje
    void readFaultCode();               // Leer código de falla
    void readAllParameters();           // Leer todos los parámetros
    
    // Obtener datos
    const VFDData& getData() const { return _data; }
    float getFrequencyHz() const { return _data.frequencyActual / 100.0; }
    float getCurrentAmps() const { return _data.currentOut / 100.0; }
    float getSpeedPercent() const { return _data.speedPercent; }
    bool isRunning() const { return _data.isRunning; }
    bool hasFault() const { return _data.hasFault; }
    uint16_t getStatus() const { return _data.status; }
    String getStatusText() const;
    
    // Actualización periódica
    void update();                      // Llamar en loop para actualizar datos
    
private:
    ModbusManager& _modbus;
    VFDData _data;
    unsigned long _lastAutoUpdate;
    const unsigned long _autoUpdateInterval = 2000; // Actualizar cada 2s
    
    // Procesamiento de respuestas Modbus
    void handleModbusData(ModbusMessage response, uint32_t token);
    void handleModbusError(Error error, uint32_t token);
    
    // Cálculos
    void calculateSpeedPercent();
};

#endif
