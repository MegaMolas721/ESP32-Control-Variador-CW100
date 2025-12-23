# Control Variador CW100 - ESP32 PLC
### Arquitectura Modular v2.0

## 📐 Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────┐
│                      ESP32 PLC (192.168.86.43)              │
│  ┌────────────┐  ┌──────────────┐  ┌─────────────────┐     │
│  │  9 Entradas│  │ 10 Salidas   │  │   Web Server    │     │
│  │  Digitales │  │  Relevador   │  │   Puerto 80     │     │
│  └────────────┘  └──────────────┘  └─────────────────┘     │
│         │                │                   │              │
│         └────────────────┴───────────────────┘              │
│                          │                                  │
│                     main.cpp                                │
│          ┌───────────────┼───────────────┐                  │
│          │               │               │                  │
│    WiFiManager    ModbusManager    IOManager                │
│          │               │               │                  │
│          └───────┬───────┴───────┬───────┘                  │
│                  │               │                          │
│           VFDController    WebDashboard                      │
└──────────────────┼───────────────┼──────────────────────────┘
                   │               │
                   │  Modbus TCP   │  HTTP
                   ▼               ▼
        ┌──────────────────┐  ┌────────────┐
        │  Gateway TCP/RTU │  │  Navegador │
        │  192.168.86.100  │  │    Web     │
        └────────┬─────────┘  └────────────┘
                 │ Modbus RTU
                 ▼
        ┌──────────────────┐
        │  Variador CW100  │
        │    Slave ID: 5   │
        └──────────────────┘
```

## 🗂️ Estructura de Archivos

```
ESP32_COntrolVariador/
├── platformio.ini                 # Configuración PlatformIO
├── README.md                      # Este archivo
├── README_VARIADOR.md            # Documentación técnica del variador
│
├── include/                       # Archivos de cabecera (.h)
│   ├── WiFiManager.h             # Gestión WiFi
│   ├── ModbusManager.h           # Comunicación Modbus TCP
│   ├── VFDController.h           # Control del variador
│   ├── IOManager.h               # Gestión de I/O
│   └── WebDashboard.h            # Servidor web
│
└── src/                          # Implementaciones (.cpp)
    ├── main.cpp                  # ⭐ Programa principal
    ├── WiFiManager.cpp           # Implementación WiFi
    ├── ModbusManager.cpp         # Implementación Modbus
    ├── VFDController.cpp         # Implementación VFD
    ├── IOManager.cpp             # Implementación I/O
    └── WebDashboard.cpp          # Implementación Web
```

## 📦 Módulos del Sistema

### 1️⃣ **WiFiManager** (`WiFiManager.h` / `.cpp`)
**Responsabilidad:** Gestión de conectividad WiFi

**Funcionalidades:**
- Configuración de credenciales WiFi
- IP estática o DHCP
- Reconexión automática
- Estado de conexión

**Uso:**
```cpp
WiFiManager wifiMgr;
wifiMgr.setCredentials("SSID", "PASSWORD");
wifiMgr.setStaticIP(IP, Gateway, Subnet);
wifiMgr.connect();
```

---

### 2️⃣ **ModbusManager** (`ModbusManager.h` / `.cpp`)
**Responsabilidad:** Comunicación Modbus TCP

**Funcionalidades:**
- Cliente Modbus TCP
- Lectura/escritura de registros
- Callbacks para respuestas y errores
- Cola de peticiones

**Uso:**
```cpp
ModbusManager modbus;
modbus.setGateway(IP, 502);
modbus.setSlaveID(5);
modbus.begin();
modbus.process();  // Llamar en loop
```

---

### 3️⃣ **VFDController** (`VFDController.h` / `.cpp`)
**Responsabilidad:** Control del variador CW100

**Funcionalidades:**
- Start/Stop del motor
- Configuración de frecuencia
- Lectura de parámetros (frecuencia, corriente, voltaje)
- Reset de fallas
- Cálculo de porcentaje de velocidad

**Registros Modbus implementados:**
- `1000` - Frecuencia Setpoint (Escritura)
- `2000` - Comando (1=RUN, 6=STOP)
- `3000` - Estado del variador (1=Adelante, 2=Atrás, 3=Stop)
- `1001` - Frecuencia actual (Lectura)
- `1004` - Corriente de salida (Lectura)
- `8000` - Código de falla (Lectura)

**Uso:**
```cpp
VFDController vfd(modbus);
vfd.begin();
vfd.setFrequency(50.0);  // 50 Hz
vfd.start();
vfd.readStatus();
float freq = vfd.getFrequencyHz();
```

---

### 4️⃣ **IOManager** (`IOManager.h` / `.cpp`)
**Responsabilidad:** Gestión de I/O del PLC

**Hardware:**
- **9 Entradas:** GPIOs 17, 18, 19, 21, 22, 23, 27, 32, 33
- **10 Salidas:** GPIOs 1, 2, 3, 4, 5, 12, 13, 14, 15, 16

**Uso:**
```cpp
IOManager io;
io.begin();
io.readInputs();
bool input0 = io.getInput(0);
io.setOutput(0, HIGH);
```

---

### 5️⃣ **WebDashboard** (`WebDashboard.h` / `.cpp`)
**Responsabilidad:** Interfaz web de control

**Características:**
- Dashboard en tiempo real
- Botones RUN/STOP
- Control de frecuencia
- Visualización de:
  - Frecuencia actual
  - Corriente de salida
  - Estado del variador
  - Porcentaje de velocidad
  - Código de falla
- Actualización automática cada 1 segundo

**API REST:**
- `GET /` - Página principal
- `GET /api/status` - Estado en JSON
- `POST /api/command` - Enviar comando (start/stop/reset)
- `POST /api/frequency` - Configurar frecuencia

**Uso:**
```cpp
WebDashboard dashboard(vfd, io);
dashboard.begin(80);
// Acceder desde: http://192.168.86.43
```

---

### 6️⃣ **main.cpp** (Raíz del sistema)
**Responsabilidad:** Coordinación de todos los módulos

**Flujo de ejecución:**
```cpp
setup() {
    1. Inicializar Serial
    2. Inicializar IOManager
    3. Conectar WiFi
    4. Inicializar ModbusManager
    5. Inicializar VFDController
    6. Inicializar WebDashboard
}

loop() {
    1. Procesar Modbus (process)
    2. Leer entradas del PLC
    3. Actualizar datos del variador
    4. Procesar comandos Serial
    5. Lógica de control personalizada
}
```

## ⚙️ Configuración del Sistema

### Editar `src/main.cpp`

```cpp
// Credenciales WiFi
const char* WIFI_SSID = "TU_RED_WIFI";      // ⚠️ CAMBIAR
const char* WIFI_PASSWORD = "TU_PASSWORD";   // ⚠️ CAMBIAR

// IP Estática del ESP32
IPAddress ESP32_IP(192, 168, 86, 43);

// Gateway Modbus TCP/RTU
IPAddress GATEWAY_MODBUS(192, 168, 86, 100);
const uint8_t VFD_SLAVE_ID = 5;
```

## 🚀 Compilación y Carga

### Usando PlatformIO:
```bash
# Compilar
pio run

# Cargar al ESP32
pio run --target upload

# Monitor serial
pio device monitor
```

### Usando VS Code:
1. Abrir carpeta del proyecto
2. Click en botón **PlatformIO: Upload** (→)
3. Click en **PlatformIO: Serial Monitor** para ver salida

## 💻 Comandos Serial (115200 baud)

| Comando | Descripción |
|---------|-------------|
| `s` o `start` | Arrancar motor |
| `p` o `stop` | Detener motor |
| `r` o `reset` | Resetear falla |
| `f50.0` | Configurar frecuencia a 50 Hz |
| `i` o `info` | Leer todos los parámetros |
| `io` | Ver estado de I/O |
| `h` o `help` | Mostrar ayuda |

## 🌐 Acceso al Dashboard Web

Una vez conectado, acceder desde cualquier navegador en la misma red:

```
http://192.168.86.43
```

El dashboard muestra en tiempo real:
- ✅ Porcentaje de velocidad nominal (barra de progreso)
- ✅ Frecuencia actual en Hz
- ✅ Corriente de salida en Amperes
- ✅ Estado del variador (Adelante/Atrás/Stop)
- ✅ Código de falla
- ✅ Botones RUN y STOP
- ✅ Input para configurar frecuencia
- ✅ Botón de reset de falla

## 📊 Diagrama de Flujo de Datos

```
Usuario → Web/Serial → main.cpp
                          ↓
                    VFDController
                          ↓
                   ModbusManager
                          ↓
                    Gateway TCP/RTU
                          ↓
                    Variador CW100
                          ↓
                      (Respuesta)
                          ↓
                   ModbusManager
                          ↓
                    VFDController
                          ↓
                   WebDashboard/Serial
                          ↓
                       Usuario
```

## 🔧 Agregar Nueva Funcionalidad

### Ejemplo: Leer un nuevo registro del variador

1. **En `VFDController.h`**, agregar definición:
```cpp
#define REG_NEW_PARAMETER 1005
```

2. **En `VFDController.h`**, agregar en struct VFDData:
```cpp
uint16_t newParameter;
```

3. **En `VFDController.h`**, declarar función:
```cpp
void readNewParameter();
```

4. **En `VFDController.cpp`**, implementar:
```cpp
void VFDController::readNewParameter() {
    _modbus.readHoldingRegister(REG_NEW_PARAMETER, 1, REG_NEW_PARAMETER);
}
```

5. **En `VFDController.cpp`**, procesar en `handleModbusData()`:
```cpp
case REG_NEW_PARAMETER:
    _data.newParameter = value;
    Serial.printf("Nuevo parámetro: %d\n", value);
    break;
```

6. **En `main.cpp`**, usar:
```cpp
vfdController.readNewParameter();
```

## 🛠️ Solución de Problemas

### No compila
- Verificar que todas las librerías estén en `platformio.ini`
- Limpiar proyecto: `pio run --target clean`

### No conecta WiFi
- Verificar SSID y contraseña
- Verificar que la IP esté disponible en la red

### No hay comunicación Modbus
- Verificar IP del gateway (192.168.86.100)
- Verificar que el Slave ID sea 5
- Ver salida serial para errores Modbus

### Dashboard no carga
- Verificar que el ESP32 esté conectado a WiFi
- Verificar IP en monitor serial
- Probar `ping 192.168.86.43`

## 📚 Dependencias

```ini
lib_deps = 
    eModbus/eModbus@^1.7.0          # Modbus TCP/RTU
    ESP Async WebServer@^1.2.3       # Servidor web asíncrono
    AsyncTCP@^1.1.1                  # TCP asíncrono para ESP32
```

## 🎯 Ventajas de la Arquitectura Modular

✅ **Código limpio y organizado** - Cada módulo tiene una responsabilidad única  
✅ **Fácil mantenimiento** - Cambios localizados en módulos específicos  
✅ **Reutilizable** - Los módulos se pueden usar en otros proyectos  
✅ **Testeable** - Cada módulo se puede probar independientemente  
✅ **Escalable** - Agregar nuevas funcionalidades sin afectar el sistema  
✅ **Legible** - main.cpp minimalista y fácil de entender  

## � Correcciones y Configuración Final de Compilación

### Problemas Resueltos Durante el Desarrollo

#### 1️⃣ **Incompatibilidad con la API de eModbus**

**Problema inicial:** Se intentaba usar la API de eModbus de forma incorrecta:
```cpp
// ❌ INCORRECTO - No existe constructor por defecto
_client = new ModbusClientTCP();
_client->loopHandler();  // Método inexistente
```

**Solución implementada:**
```cpp
// ✅ CORRECTO - Constructor requiere referencia a WiFiClient
WiFiClient _wifiClient;
_client = new ModbusClientTCP(_wifiClient);

// La librería maneja la comunicación automáticamente en segundo plano
// No requiere llamadas a loopHandler()
```

**Cambios en ModbusManager.cpp:**
- Constructor requiere `WiFiClient&` como parámetro
- Uso de `setTarget()` para configurar IP/puerto del gateway
- Llamada a `begin()` para iniciar tarea de fondo
- Eliminación de `loopHandler()` - la comunicación es automática

---

#### 2️⃣ **Firma incorrecta de addRequest()**

**Problema:** Se intentaba pasar IP y puerto en cada llamada a `addRequest()`

**Antes (incorrecto):**
```cpp
return _client->addRequest(
    token,
    _gatewayIP,      // ❌ No acepta estos parámetros
    _gatewayPort,    // ❌ No acepta estos parámetros
    _slaveID,
    READ_HOLD_REGISTER,
    address,
    count
);
```

**Después (correcto):**
```cpp
// Configurar target una sola vez en begin()
_client->setTarget(_gatewayIP, _gatewayPort);
_client->begin();

// Luego usar addRequest() sin IP/puerto
return _client->addRequest(token, _slaveID, READ_HOLD_REGISTER, address, count);
```

---

#### 3️⃣ **Enum Error::NOT_CONNECTED inexistente**

**Problema:** Se intentaba retornar un código de error que no existe en eModbus

**Antes:**
```cpp
if (!_initialized) {
    return Error::NOT_CONNECTED;  // ❌ No existe en eModbus
}
```

**Solución:** Eliminar las verificaciones - la librería eModbus maneja los errores internamente y retorna códigos apropiados como `TIMEOUT`, `IP_CONNECTION_FAILED`, etc.

---

#### 4️⃣ **Incompatibilidad de librerías AsyncTCP**

**Problema:** Las librerías esphome de AsyncTCP no son compatibles con el framework Arduino ESP32 actual

**Configuración inicial (fallaba):**
```ini
lib_deps = 
    emodbus@^1.7.0
    esphome/AsyncTCP-esphome@^2.0.1          # ❌ Errores de compilación
    ottowinter/ESPAsyncWebServer-esphome@^3.1.0  # ❌ Incompatible
```

**Configuración final (funciona):**
```ini
lib_deps = 
    https://github.com/eModbus/eModbus.git   # ✅ Versión desde GitHub
    mathieucarbou/AsyncTCP@^3.0.0           # ✅ Compatible con ESP32
    mathieucarbou/ESP Async WebServer@^3.0.0 # ✅ Versión actualizada
```

---

#### 5️⃣ **Código duplicado en ModbusManager.h**

**Problema:** El archivo header tenía declaraciones duplicadas y dos `#endif`

**Solución:** Limpiar el archivo eliminando duplicaciones:
```cpp
// ✅ Estructura correcta
class ModbusManager {
    // ... código ...
    static ModbusManager* _instance;
    static void handleDataStatic(ModbusMessage response, uint32_t token);
    static void handleErrorStatic(Error error, uint32_t token);
};

#endif  // Solo un #endif al final
```

---

### 📋 Configuración Final de ModbusManager

#### **ModbusManager.h (Estructura final)**
```cpp
class ModbusManager {
public:
    ModbusManager();
    
    // Configuración
    void setGateway(IPAddress ip, uint16_t port);
    void setSlaveID(uint8_t slaveID);
    void setTimeout(uint32_t timeout);
    
    // Callbacks
    void onData(ModbusDataCallback callback);
    void onError(ModbusErrorCallback callback);
    
    // Inicialización
    bool begin();
    void process();  // No hace nada - comunicación automática
    
    // Operaciones Modbus
    Error readHoldingRegister(uint16_t address, uint16_t count, uint32_t token);
    Error readInputRegister(uint16_t address, uint16_t count, uint32_t token);
    Error writeHoldingRegister(uint16_t address, uint16_t value, uint32_t token);
    Error writeMultipleRegisters(uint16_t address, uint16_t count, uint16_t* values, uint32_t token);
    
    bool isReady();
    
private:
    WiFiClient _wifiClient;              // Cliente WiFi para TCP
    ModbusClientTCP* _client;            // Cliente Modbus TCP
    IPAddress _gatewayIP;
    uint16_t _gatewayPort;
    uint8_t _slaveID;
    uint32_t _timeout;
    bool _initialized;
    
    ModbusDataCallback _dataCallback;
    ModbusErrorCallback _errorCallback;
    
    // Callbacks estáticos para eModbus
    static ModbusManager* _instance;
    static void handleDataStatic(ModbusMessage response, uint32_t token);
    static void handleErrorStatic(Error error, uint32_t token);
};
```

#### **ModbusManager.cpp (Método begin() final)**
```cpp
bool ModbusManager::begin() {
    Serial.println("\n=== INICIALIZANDO MODBUS TCP ===");
    Serial.printf("Gateway: %s:%d\n", _gatewayIP.toString().c_str(), _gatewayPort);
    Serial.printf("Slave ID: %d\n", _slaveID);
    
    // 1. Crear cliente con referencia a WiFiClient
    _client = new ModbusClientTCP(_wifiClient);
    
    // 2. Configurar timeout
    _client->setTimeout(_timeout);
    
    // 3. Registrar callbacks estáticos
    _client->onDataHandler(&ModbusManager::handleDataStatic);
    _client->onErrorHandler(&ModbusManager::handleErrorStatic);
    
    // 4. Establecer target (gateway IP y puerto)
    _client->setTarget(_gatewayIP, _gatewayPort);
    
    // 5. Iniciar tarea de fondo
    _client->begin();
    
    _initialized = true;
    Serial.println("Modbus TCP inicializado correctamente");
    return true;
}
```

---

### 🎯 Flujo de Comunicación Modbus Final

```
┌─────────────────────────────────────────────────────────────┐
│  ESP32 - ModbusManager                                      │
│                                                             │
│  1. begin()                                                 │
│     ├─ Crea ModbusClientTCP con WiFiClient                 │
│     ├─ Configura target (192.168.86.100:502)               │
│     ├─ Registra callbacks                                  │
│     └─ Inicia tarea de fondo automática                    │
│                                                             │
│  2. writeHoldingRegister(2000, CMD_RUN, token)             │
│     └─ addRequest(token, slaveID=5, WRITE, 2000, 1)       │
│                                                             │
│  3. Tarea de fondo (automática en eModbus)                 │
│     ├─ Encola petición                                     │
│     ├─ Conecta al gateway vía TCP                          │
│     ├─ Envía trama Modbus TCP                              │
│     ├─ Espera respuesta                                    │
│     └─ Llama a handleDataStatic() o handleErrorStatic()   │
│                                                             │
│  4. handleDataStatic(response, token)                      │
│     └─ Ejecuta callback del usuario (_dataCallback)        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
                          ↓ TCP
┌─────────────────────────────────────────────────────────────┐
│  Gateway Modbus TCP/RTU (192.168.86.100:502)               │
│     - Convierte TCP → RTU                                  │
│     - Reenvía a bus RS485                                  │
└─────────────────────────────────────────────────────────────┘
                          ↓ RS485
┌─────────────────────────────────────────────────────────────┐
│  Variador CW100 (Slave ID: 5)                              │
│     - Recibe comando en registro 2000H                      │
│     - Ejecuta acción (RUN=1, STOP=6)                        │
│     - Responde con confirmación                             │
└─────────────────────────────────────────────────────────────┘
```

---

### ✅ Resultado Final

**Estado de compilación:**
```
RAM:   [=         ]  13.9% (used 45428 bytes from 327680 bytes)
Flash: [=======   ]  65.5% (used 858169 bytes from 1310720 bytes)
============================== [SUCCESS] ==============================
```

**Librerías utilizadas:**
- `eModbus @ 1.7.4` - Comunicación Modbus TCP con gestión automática de cola y conexión
- `AsyncTCP @ 3.3.2` - TCP asíncrono optimizado para ESP32
- `ESP Async WebServer @ 3.0.6` - Servidor web con soporte completo para JSON y SSE

**Arquitectura Modbus implementada:**
- ✅ Cliente Modbus TCP con tarea de fondo automática
- ✅ Callbacks para procesar respuestas y errores de forma asíncrona
- ✅ Cola interna de peticiones (no bloquea loop principal)
- ✅ Reconexión automática al gateway
- ✅ Timeout configurable (2000ms por defecto)
- ✅ Sistema de tokens para identificar respuestas

---

### 📝 Notas Importantes

1. **No es necesario llamar a `process()` repetidamente** - La librería eModbus gestiona todo en segundo plano
2. **Las respuestas llegan vía callbacks** - El código no bloquea esperando respuestas
3. **Los tokens identifican cada petición** - Usar valores únicos (millis(), contador, etc.)
4. **La conexión TCP se gestiona automáticamente** - No requiere gestión manual
5. **Los errores se reportan vía callback de error** - Incluye TIMEOUT, IP_CONNECTION_FAILED, etc.

## �📝 Licencia

Proyecto educativo - Libre uso y modificación

## 👨‍💻 Autor

Control de Variador CW100 - ESP32 PLC  
Versión 2.0 - Arquitectura Modular  
Diciembre 2025
