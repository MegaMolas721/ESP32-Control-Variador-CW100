/**
 * @file main.cpp
 * @brief Programa principal - Control de Variador CW100 con PLC ESP32
 * @date 2025-12-23
 * 
 * Arquitectura del sistema:
 * - ESP32 PLC (192.168.86.43) - Maestro Modbus TCP
 * - Gateway TCP/RTU (192.168.86.100:502)
 * - Variador CW100 (Slave ID: 5) - Esclavo Modbus RTU
 * 
 * Módulos:
 * - WiFiManager: Gestión de conectividad WiFi
 * - ModbusManager: Comunicación Modbus TCP
 * - VFDController: Control del variador CW100
 * - IOManager: Gestión de I/O del PLC
 * - WebDashboard: Interfaz web de control
 */

#include <Arduino.h>
#include "WiFiManager.h"
#include "ModbusManager.h"
#include "VFDController.h"
#include "IOManager.h"
#include "WebDashboard.h"

// ============================================================
// CONFIGURACIÓN DEL SISTEMA
// ============================================================
// Gateway Modbus TCP/RTU
IPAddress GATEWAY_MODBUS_IP(192, 168, 86, 100); // IP del convertidor TCP/RTU
const uint16_t GATEWAY_MODBUS_PORT = 502;       // Puerto Modbus TCP
const uint8_t VARIADOR_SLAVE_ID = 5;            // ID del variador en red RTU

// ============================================================
// INSTANCIAS DE MÓDULOS
// ============================================================
WiFiManager wifiMgr;
ModbusManager modbusMgr;
IOManager ioMgr;
VFDController vfdController(modbusMgr);
WebDashboard webDashboard(vfdController, ioMgr, wifiMgr);

// ============================================================
// FUNCIONES AUXILIARES
// ============================================================

void printWelcomeMessage() {
    Serial.println("\n\n");
    Serial.println("===============================================");
    Serial.println("  PLC ESP32 - Control Variador CW100");
    Serial.println("  Modbus TCP/RTU Master + Web Dashboard");
    Serial.println("  Version 2.0 - Arquitectura Modular");
    Serial.println("===============================================\n");
}

void printCommandsHelp() {
    Serial.println("\n=== COMANDOS DISPONIBLES (Serial) ===");
    Serial.println("  s, start     - Arrancar motor");
    Serial.println("  p, stop      - Detener motor");
    Serial.println("  r, reset     - Resetear falla");
    Serial.println("  f[valor]     - Configurar frecuencia (ej: f50.0)");
    Serial.println("  i, info      - Leer todos los parámetros");
    Serial.println("  io           - Ver estado de I/O");
    Serial.println("  h, help      - Mostrar esta ayuda");
    Serial.println("===================================\n");
}

void processSerialCommands() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        cmd.toLowerCase();
        
        if (cmd == "s" || cmd == "start") {
            vfdController.start();
        }
        else if (cmd == "p" || cmd == "stop") {
            vfdController.stop();
        }
        else if (cmd == "r" || cmd == "reset") {
            vfdController.resetFault();
        }
        else if (cmd == "i" || cmd == "info") {
            vfdController.readAllParameters();
        }
        else if (cmd == "io") {
            ioMgr.printInputStates();
            ioMgr.printOutputStates();
        }
        else if (cmd.startsWith("f")) {
            float freq = cmd.substring(1).toFloat();
            if (freq >= 0 && freq <= 400) {
                vfdController.setFrequency(freq);
            } else {
                Serial.println("❌ Frecuencia fuera de rango (0-400 Hz)");
            }
        }
        else if (cmd == "h" || cmd == "help") {
            printCommandsHelp();
        }
        else if (cmd.length() > 0) {
            Serial.printf("⚠️ Comando desconocido: '%s'\n", cmd.c_str());
            Serial.println("Escribe 'h' para ver comandos disponibles");
        }
    }
}

// ============================================================
// SETUP
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    printWelcomeMessage();
    
    // 1. Inicializar I/O del PLC
    ioMgr.begin();
    
    // 2. Conectar WiFi con escaneo inteligente
    Serial.println("\n=== INICIANDO CONEXIÓN WIFI ===");
    Serial.println("Escaneando redes conocidas...");
    
    if (!wifiMgr.connectToBestNetwork()) {
        Serial.println("\n⚠️ ERROR: No se pudo conectar a ninguna red conocida");
        Serial.println("Verifica que las redes estén disponibles:");
        Serial.println("  - CASA TIENDA");
        Serial.println("  - ADNIX MOLAS");
        Serial.println("  - SAITCOM");
        Serial.println("\nSistema continuará sin WiFi. Reinicia para reintent ar.");
        // No detener el sistema, continuar sin WiFi
    }
    
    // 3. Inicializar Modbus TCP (solo si hay WiFi)
    if (wifiMgr.isConnected()) {
        modbusMgr.setGateway(GATEWAY_MODBUS_IP, GATEWAY_MODBUS_PORT);
        modbusMgr.setSlaveID(VARIADOR_SLAVE_ID);
        modbusMgr.begin();
        
        // 4. Inicializar controlador del variador
        vfdController.begin();
        
        // 5. Inicializar Web Dashboard
        webDashboard.begin(80);
        
        // Mostrar información del sistema
        Serial.println("\n=== SISTEMA LISTO ===");
        Serial.printf("Dashboard Web: http://%s\n", wifiMgr.getLocalIP().c_str());
        Serial.printf("Gateway Modbus: %s:%d\n", GATEWAY_MODBUS_IP.toString().c_str(), GATEWAY_MODBUS_PORT);
        Serial.printf("Variador ID: %d\n", VARIADOR_SLAVE_ID);
    } else {
        Serial.println("\n=== SISTEMA EN MODO SIN RED ===");
        Serial.println("Reinicia el ESP32 para intentar conectar WiFi");
    }
    
    printCommandsHelp();
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void loop() {
    // 1. Manejar reconexión WiFi automática (si se desconecta)
    wifiMgr.handleReconnection();
    
    // 2. Procesar comunicación Modbus (CRÍTICO: llamar frecuentemente)
    if (wifiMgr.isConnected()) {
        modbusMgr.process();
        vfdController.update();
    }
    
    // 3. Leer entradas del PLC
    ioMgr.readInputs();
    
    // 4. Procesar comandos desde Serial
    processSerialCommands();
    
    // 5. Control lógico basado en entradas (ejemplo)
    // Descomentar y adaptar según necesidad:
    /*
    static bool lastInput0 = false;
    bool input0 = ioMgr.getInput(0);
    
    if (input0 && !lastInput0) {
        Serial.println("Entrada 0 activada - Arrancando motor");
        vfdController.start();
    }
    lastInput0 = input0;
    */
    
    delay(10);  // Pequeña pausa para estabilidad
}
