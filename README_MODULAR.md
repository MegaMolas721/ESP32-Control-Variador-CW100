# Control de Variador CW100 con ESP32 PLC - Arquitectura Modular

## 📁 Estructura del Proyecto

```
ESP32_COntrolVariador/
├── platformio.ini          # Configuración del proyecto PlatformIO
├── include/                # Archivos de cabecera (.h)
│   ├── config.h           # ⚙️  Configuraciones globales
│   ├── wifi_manager.h     # 📡 Gestión de WiFi
│   ├── plc_io.h           # 🔌 Control de I/O del PLC
│   └── modbus_vfd.h       # 🔄 Comunicación Modbus VFD
├── src/                    # Código fuente (.cpp)
│   ├── main.cpp           # 🎯 Programa principal (limpio)
│   ├── config.cpp         # Definiciones de configuración
│   ├── wifi_manager.cpp   # Implementación WiFi
│   ├── plc_io.cpp         # Implementación I/O
│   └── modbus_vfd.cpp     # Implementación Modbus VFD
└── README_MODULAR.md      # Esta documentación
```

## 🏗️ Arquitectura Modular

### 1. **config.h / config.cpp** - Configuración Global
Contiene todas las constantes y configuraciones del sistema:
- Credenciales WiFi y configuración de red
- Definición de pines del PLC (entradas/salidas)
- Registros Modbus del variador CW100
- Comandos del variador
- Constantes del sistema

**📝 Para configurar:**
```cpp
// En config.h, modificar:
#define WIFI_SSID           "TU_SSID"
#define WIFI_PASSWORD       "TU_PASSWORD"
#define ESP32_GATEWAY       192, 168, 86, 1  // Ajustar según tu red
```

---

### 2. **wifi_manager.h / wifi_manager.cpp** - Gestión de WiFi
Clase `WiFiManager` para manejo de conectividad WiFi con IP estática.

**Métodos principales:**
- `begin()` - Inicializar y conectar WiFi
- `isConnected()` - Verificar conexión
- `getLocalIP()` - Obtener IP asignada
- `printNetworkInfo()` - Mostrar información de red

**Uso en main.cpp:**
```cpp
WiFiManager wifiMgr;
void setup() {
    if (!wifiMgr.begin()) {
        // Error de conexión
    }
}
```

---

### 3. **plc_io.h / plc_io.cpp** - Control de I/O del PLC
Clase `PlcIO` para gestión de las 9 entradas y 10 salidas del PLC.

**Métodos principales:**
- `begin()` - Inicializar pines
- `readInputs()` - Leer todas las entradas
- `getInput(n)` - Obtener estado de entrada n
- `setOutput(n, state)` - Establecer salida n
- `setOutputHigh(n)` / `setOutputLow(n)` - Activar/desactivar salida
- `toggleOutput(n)` - Alternar salida
- `allOutputsOff()` - Apagar todas las salidas
- `printInputStates()` / `printOutputStates()` - Mostrar estados

**Uso en main.cpp:**
```cpp
PlcIO plcIO;
void setup() {
    plcIO.begin();
}

void loop() {
    plcIO.readInputs();
    if (plcIO.getInput(0)) {
        plcIO.setOutputHigh(0);
    }
}
```

---

### 4. **modbus_vfd.h / modbus_vfd.cpp** - Control del Variador
Clase `ModbusVFD` para comunicación con el variador CW100 vía Modbus TCP/RTU.

**Métodos de control:**
- `start()` - Arrancar motor
- `stop()` - Detener motor
- `startReverse()` - Arrancar en reversa
- `resetFault()` - Resetear falla
- `setFrequency(Hz)` - Configurar frecuencia

**Métodos de lectura:**
- `readStatus()` - Leer estado del variador
- `readFrequency()` - Leer frecuencia actual
- `readCurrent()` - Leer corriente
- `readVoltage()` - Leer voltaje
- `readAllParameters()` - Leer todos los parámetros

**Métodos de acceso:**
- `isRunning()` - Verificar si está corriendo
- `hasFault()` - Verificar si hay falla
- `getFrequencyHz()` - Obtener frecuencia en Hz
- `printStatus()` - Imprimir estado detallado

**Uso en main.cpp:**
```cpp
ModbusVFD vfd;
void setup() {
    vfd.begin();
}

void loop() {
    vfd.process();  // CRÍTICO: llamar siempre
    
    vfd.setFrequency(50.0);
    vfd.start();
    
    if (vfd.hasFault()) {
        vfd.resetFault();
    }
}
```

---

### 5. **main.cpp** - Programa Principal
Archivo limpio que orquesta todos los módulos.

**Estructura:**
```cpp
#include "config.h"
#include "wifi_manager.h"
#include "plc_io.h"
#include "modbus_vfd.h"

WiFiManager wifiMgr;
PlcIO plcIO;
ModbusVFD vfd;

void setup() {
    // 1. Inicializar I/O
    plcIO.begin();
    
    // 2. Conectar WiFi
    wifiMgr.begin();
    
    // 3. Inicializar Modbus
    vfd.begin();
}

void loop() {
    plcIO.readInputs();    // Leer entradas
    vfd.process();         // Procesar Modbus
    
    // Tu lógica aquí
}
```

---

## 🎯 Flujo de Trabajo

### Al Iniciar (setup):
1. ✅ Inicializar I/O del PLC
2. ✅ Conectar WiFi con IP estática
3. ✅ Configurar Modbus TCP
4. ✅ Mostrar menú de comandos

### En el Loop:
1. 🔄 Leer entradas físicas del PLC
2. 🔄 Procesar cola Modbus (`vfd.process()`)
3. 🔄 Leer estado del variador periódicamente
4. 🔄 Procesar comandos serial
5. 🔄 Ejecutar lógica personalizada

---

## 💻 Comandos Serial Disponibles

| Comando | Descripción | Ejemplo |
|---------|-------------|---------|
| `s` | Arrancar motor | `s` |
| `p` | Parar motor | `p` |
| `r` | Reset de falla | `r` |
| `f[Hz]` | Configurar frecuencia | `f50.0` |
| `i` | Leer información completa | `i` |
| `status` | Estado detallado | `status` |
| `io` | Estado de I/O del PLC | `io` |
| `h` | Ayuda | `h` |

---

## 🔧 Configuración Inicial

### Paso 1: Configurar Red
Editar **include/config.h**:
```cpp
#define WIFI_SSID           "MiRedWiFi"
#define WIFI_PASSWORD       "MiPassword"
#define ESP32_GATEWAY       192, 168, 1, 1  // Tu gateway
```

### Paso 2: Compilar y Cargar
```bash
pio run -t upload
pio device monitor
```

### Paso 3: Probar Comunicación
En el monitor serial (115200 baud):
```
i          # Leer información del variador
f50.0      # Configurar a 50 Hz
s          # Arrancar
```

---

## 📊 Ejemplo de Uso - Lógica Personalizada

### Control basado en entradas digitales
```cpp
void loop() {
    plcIO.readInputs();
    vfd.process();
    
    // Botón START en entrada 0
    static bool lastBtn0 = false;
    if (plcIO.getInput(0) && !lastBtn0) {
        vfd.setFrequency(50.0);
        vfd.start();
        plcIO.setOutputHigh(0);  // LED indicador
    }
    lastBtn0 = plcIO.getInput(0);
    
    // Botón STOP en entrada 1
    static bool lastBtn1 = false;
    if (plcIO.getInput(1) && !lastBtn1) {
        vfd.stop();
        plcIO.setOutputLow(0);
    }
    lastBtn1 = plcIO.getInput(1);
    
    // Indicador de falla en salida 1
    plcIO.setOutput(1, vfd.hasFault());
}
```

### Control automático de velocidad
```cpp
void loop() {
    plcIO.readInputs();
    vfd.process();
    
    // Selector de velocidad en entradas 2-3
    bool vel1 = plcIO.getInput(2);
    bool vel2 = plcIO.getInput(3);
    
    if (vel1 && !vel2) {
        vfd.setFrequency(30.0);  // Velocidad baja
    } else if (!vel1 && vel2) {
        vfd.setFrequency(50.0);  // Velocidad media
    } else if (vel1 && vel2) {
        vfd.setFrequency(60.0);  // Velocidad alta
    }
}
```

---

## 🚀 Ventajas de la Arquitectura Modular

✅ **Código limpio y organizado** - main.cpp fácil de leer
✅ **Reutilizable** - Módulos independientes
✅ **Mantenible** - Cambios aislados en cada módulo
✅ **Escalable** - Fácil agregar nuevos módulos
✅ **Profesional** - Siguiendo buenas prácticas de programación

---

## 📚 Referencias Técnicas

- **Variador:** CW100 (Compatible con múltiples marcas)
- **Protocolo:** Modbus TCP → Gateway → Modbus RTU
- **Librería:** eModbus 1.7.0
- **Plataforma:** ESP32 (ESP32-WROOM-32)
- **Framework:** Arduino

---

## 🛠️ Solución de Problemas

### No compila
- Verificar que todos los archivos estén en sus carpetas correctas
- Ejecutar `pio run -t clean` y luego `pio run`

### Error de conexión WiFi
- Verificar credenciales en `config.h`
- Verificar que la IP 192.168.86.43 esté disponible

### Sin respuesta Modbus
- Verificar gateway TCP/RTU en 192.168.86.100:502
- Verificar Slave ID = 5 en el variador
- Revisar cableado RS485 en el gateway

---

**Desarrollado para PLC ESP32 30 pines - Jelelectrónica**
**Variador CW100 - Control Modbus TCP/RTU**
