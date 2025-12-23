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
#define NUM_INPUTS          9
extern const uint8_t INPUT_PINS[NUM_INPUTS];

// 10 Salidas a Relevador (5A 250Vac / 5A 30Vdc)
// Conectadas a: D1, D2, D3, D4, D5, D12, D13, D14, D15, D16
#define NUM_OUTPUTS         10
extern const uint8_t OUTPUT_PINS[NUM_OUTPUTS];

// ============================================================
// REGISTROS MODBUS DEL VARIADOR CW100
// ============================================================
// Valores decimales con offset +1 ya integrado (probados y funcionales)

// REGISTROS DE ESCRITURA (Holding Registers - FC06/FC16)
#define REG_FREQUENCY_SETPOINT      4097    // 0x1000 Hex → 4097 Decimal - Frecuencia Setpoint (Hz × 100)
#define REG_COMMAND                 8193    // 0x2000 Hex → 8193 Decimal - Comando de ejecución

// REGISTROS DE LECTURA (Holding Registers - FC03/FC04)
#define REG_FREQ_CURRENT            4098    // 0x1001 Hex → 4098 Decimal - Frecuencia actual (Lectura)
#define REG_VOLTAGE_OUTPUT          4100    // 0x1003 Hex → 4100 Decimal - Voltaje de salida (Lectura)
#define REG_CURRENT_OUTPUT          4101    // 0x1004 Hex → 4101 Decimal - Corriente de salida (Lectura)
#define REG_SPEED_ACTUAL            4104    // 0x1007 Hex → 4104 Decimal - Velocidad actual (Lectura)
#define REG_VFD_STATUS              12289   // 0x3000 Hex → 12289 Decimal - Estado del variador (Lectura)
#define REG_FAULT_CODE              32769   // 0x8000 Hex → 32769 Decimal - Código de Falla (Lectura)
#define REG_MAX_FREQUENCY           61451   // 0xF00A Hex → 61451 Decimal - Frecuencia Máxima (Lectura)
#define REG_UPPER_LIMIT             61453   // 0xF00C Hex → 61453 Decimal - Límite Superior (Lectura)

// ============================================================
// COMANDOS DEL VARIADOR CW100 (Registro 2000H)
// ============================================================
// Según especificación del dashboard:
// - RUN debe escribir 1 en registro 2000H
// - STOP debe escribir 6 en registro 2000H

#define CMD_RUN                     1       // 0x0001 - Arranque (RUN)
#define CMD_STOP                    6       // 0x0006 - Paro (STOP)
#define CMD_FAULT_RESET             0x0080  // 0x0080 - Reset de falla

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
