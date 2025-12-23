#ifndef VFD_CONTROLLER_H
#define VFD_CONTROLLER_H

#include <Arduino.h>
#include "ModbusManager.h"

// Registros Modbus del variador CW100 (actualizados)
#define REG_FREQ_SETPOINT       1000    // 0x03E8 (4097) - Frecuencia Setpoint (Escritura)
#define REG_COMMAND             2000    // 0x07D0 (8193) - Comando de ejecución (Escritura)
#define REG_VFD_STATUS          3000    // 0x0BB8 (12289) - Estatus del variador (Lectura)
#define REG_FAULT_CODE          8000    // 0x1F40 (32769) - Código de Falla (Lectura)
#define REG_FREQ_ACTUAL         1001    // 0x03E9 (4098) - Frecuencia actual (Lectura)
#define REG_VOLTAGE_OUT         1003    // 0x03EB (4100) - Voltaje de salida (Lectura)
#define REG_CURRENT_OUT         1004    // 0x03EC (4101) - Corriente de salida (Lectura)
#define REG_SPEED_ACTUAL        1007    // 0x03EF (4104) - Velocidad actual (Lectura)
#define REG_MAX_FREQUENCY       0xF00A  // (61451) - Frecuencia Máxima (Lectura)
#define REG_UPPER_LIMIT         0xF00C  // (61453) - Límite Superior (Lectura)

// Comandos del variador (Registro 2000)
#define CMD_RUN                 1       // RUN (arrancar)
#define CMD_STOP                6       // STOP (parar)
#define CMD_FAULT_RESET         0x0080  // Reset de falla
#define CMD_JOG_FORWARD         0x0010  // JOG adelante
#define CMD_JOG_REVERSE         0x0020  // JOG reversa

// Estados del variador (Registro 3000)
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
