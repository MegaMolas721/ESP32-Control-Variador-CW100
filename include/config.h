/**
 * @file config.h
 * @brief Configuración global del sistema PLC ESP32 - Control Variador CW100
 * @date 2025-12-23
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

// ============================================================
// CONFIGURACIÓN DE RED
// ============================================================

// Redes WiFi conocidas (el sistema elegirá la de mejor señal)
#define NUM_KNOWN_NETWORKS  3

struct WiFiNetwork {
    const char* ssid;
    const char* password;
};

// Las 3 redes conocidas
const WiFiNetwork KNOWN_NETWORKS[NUM_KNOWN_NETWORKS] = {
    {"CASA TIENDA", "casanueva321"},
    {"ADNIX MOLAS", "sentra223344"},
    {"SAITCOM", "SAITCOM2147"}
};

// Configuración de IP (se puede usar DHCP o estática)
#define USE_STATIC_IP       false               // false = DHCP, true = IP estática
#define ESP32_IP            192, 168, 86, 43
#define ESP32_GATEWAY       192, 168, 86, 1     // Ajustar según tu red
#define ESP32_SUBNET        255, 255, 255, 0
#define ESP32_DNS           8, 8, 8, 8

// Umbral RSSI para considerar una conexión aceptable
#define RSSI_THRESHOLD      -70                 // dBm (mejor señal = número más alto)

// Gateway Modbus TCP/RTU
#define GATEWAY_IP          192, 168, 86, 100
#define MODBUS_PORT         502

// Variador CW100
#define VFD_SLAVE_ID        5                   // ID del variador en red RTU

// ============================================================
// CONFIGURACIÓN PLC ESP32 - PINES DE I/O
// ============================================================

// 9 Entradas optoacopladas (12-24Vdc)
// Conectadas a: D17, D18, D19, D21, D22, D23, D27, D32, D33
// Nota: GPIO32 y GPIO33 funcionan correctamente como entradas digitales
#define NUM_INPUTS          9
extern const uint8_t INPUT_PINS[NUM_INPUTS];

// 7 Salidas a Relevador (5A 250Vac / 5A 30Vdc)
// Conectadas a: D2, D3, D4, D5, D13, D14, D16
// DESHABILITADOS (conflictos): D1 (TX0), D12 (strapping), D15 (strapping)
#define NUM_OUTPUTS         7
extern const uint8_t OUTPUT_PINS[NUM_OUTPUTS];

// ============================================================
// REGISTROS MODBUS DEL VARIADOR CW100
// ============================================================
// Valores hexadecimales SIN offset (la librería eModbus suma +1 automáticamente)
// Nota: LOGO Siemens requiere +1 manual, pero eModbus lo hace automáticamente

// REGISTROS DE ESCRITURA (Holding Registers - FC06/FC16)
#define REG_FREQUENCY_SETPOINT      0x1000  // = 4096 (eModbus suma +1 → escribe en 4097)
#define REG_COMMAND                 0x2000  // = 8192 (eModbus suma +1 → escribe en 8193)

// REGISTROS DE LECTURA (Holding Registers - FC03/FC04)
#define REG_FREQ_CURRENT            0x1001  // = 4097 (eModbus suma +1 → lee de 4098)
#define REG_VOLTAGE_OUTPUT          0x1003  // = 4099 (eModbus suma +1 → lee de 4100)
#define REG_CURRENT_OUTPUT          0x1004  // = 4100 (eModbus suma +1 → lee de 4101)
#define REG_SPEED_ACTUAL            0x1007  // = 4103 (eModbus suma +1 → lee de 4104)
#define REG_VFD_STATUS              0x3000  // = 12288 (eModbus suma +1 → lee de 12289)
#define REG_FAULT_CODE              0x8000  // = 32768 (eModbus suma +1 → lee de 32769)
#define REG_MAX_FREQUENCY           0xF00A  // = 61450 (eModbus suma +1 → lee de 61451)
#define REG_UPPER_LIMIT             0xF00C  // = 61452 (eModbus suma +1 → lee de 61453)

// ============================================================
// COMANDOS DEL VARIADOR CW100 (Registro 2000H)
// ============================================================
// Según especificación del dashboard:
// - RUN debe escribir 1 en registro 2000H
// - STOP debe escribir 6 en registro 2000H

#define CMD_RUN                     1       // 0x0001 - Arranque (RUN)
#define CMD_STOP                    6       // 0x0006 - Paro (STOP)
#define CMD_FAULT_RESET             0x0080  // 0x0080 - Reset de falla
// Comandos adicionales (no estaban definidos):
// 2 = RUN en sentido inverso (inverse RUN)
// 5 = Paro libre (free stop)
#define CMD_RUN_INVERSE             2       // 0x0002 - Arranque inverso
#define CMD_FREE_STOP               5       // 0x0005 - Paro libre

// ============================================================
// CONFIGURACIÓN DEL SISTEMA
// ============================================================

#define SERIAL_BAUDRATE             115200
#define MODBUS_TIMEOUT              2000    // Timeout en ms
#define MODBUS_IDLE_TIMEOUT         5000    // Timeout de inactividad en ms
#define STATUS_READ_INTERVAL        2000    // Intervalo de lectura de estado en ms
#define MODBUS_DELAY_BETWEEN_CMD    100     // Delay entre comandos Modbus en ms

// ============================================================
// RANGOS Y LÍMITES
// ============================================================

#define VFD_MIN_FREQUENCY           0.0f    // Frecuencia mínima en Hz
#define VFD_MAX_FREQUENCY           400.0f  // Frecuencia máxima en Hz
#define VFD_FREQ_MULTIPLIER         100     // Multiplicador para frecuencia (Hz × 100)

#endif // CONFIG_H
