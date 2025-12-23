# Control de Variador CW100 con ESP32 PLC

## 🔧 Configuración del Sistema

### Hardware
- **PLC ESP32 30 pines** (Jelelectrónica)
  - 9 Entradas optoacopladas: GPIOs 17, 18, 19, 21, 22, 23, 27, 32, 33
  - 10 Salidas a relevador: GPIOs 1, 2, 3, 4, 5, 12, 13, 14, 15, 16

### Red
- **ESP32 (Maestro Modbus TCP)**
  - IP: `192.168.86.43`
  - Puerto: `502`

- **Gateway Modbus TCP/RTU**
  - IP: `192.168.86.100`
  - Puerto: `502`

- **Variador CW100 (Esclavo Modbus RTU)**
  - Slave ID: `5`

## 📋 Registros Modbus del Variador CW100

### Registros de Escritura (Holding Registers)
| Registro | Hex    | Decimal | Descripción                    | Rango/Formato |
|----------|--------|---------|--------------------------------|---------------|
| 1000     | 0x03E8 | 4097    | Frecuencia Setpoint            | Hz × 100      |
| 2000     | 0x07D0 | 8193    | Comando de ejecución           | Ver comandos  |

### Registros de Lectura (Holding Registers)
| Registro | Hex    | Decimal | Descripción                    |
|----------|--------|---------|--------------------------------|
| 3000     | 0x0BB8 | 12289   | Estado del variador            |
| 3001     | 0x0BB9 | 12290   | Frecuencia Meta                |
| 3002     | 0x0BBA | 12291   | Frecuencia Real                |
| 3003     | 0x0BBB | 12292   | Amperaje real                  |
| 8000     | 0x1F40 | 32769   | Código de Falla                |
| 1001     | 0x03E9 | 4098    | Frecuencia actual              |
| 1003     | 0x03EB | 4100    | Voltaje de salida              |
| 1004     | 0x03EC | 4101    | Corriente de salida            |
| 1007     | 0x03EF | 4104    | Velocidad actual               |
| F00A     | 0xF00A | 61451   | Frecuencia Máxima              |
| F00C     | 0xF00C | 61453   | Límite Superior                |

## 🎮 Comandos de Control (Registro 2000)

| Comando | Valor  | Descripción             |
|---------|--------|-------------------------|
| STOP    | 0x0001 | Paro en adelante        |
| RUN FWD | 0x0002 | Arranque en adelante    |
| RUN REV | 0x0004 | Arranque en reversa     |
| STOP REV| 0x0008 | Paro en reversa         |
| JOG FWD | 0x0010 | JOG adelante            |
| JOG REV | 0x0020 | JOG reversa             |
| RESET   | 0x0080 | Reset de falla          |

## 💻 Uso del Sistema

### Configuración Inicial
1. Editar `src/main.cpp` líneas 22-23:
```cpp
const char* WIFI_SSID = "TU_SSID";
const char* WIFI_PASSWORD = "TU_PASSWORD";
```

2. Ajustar gateway de red si es necesario (línea 26):
```cpp
IPAddress gateway(192, 168, 86, 1);  // Cambiar según tu red
```

### Comandos Serial

Una vez cargado el código, abrir el monitor serial a **115200 baudios**:

| Comando | Acción                              | Ejemplo  |
|---------|-------------------------------------|----------|
| `s`     | Arrancar motor                      | `s`      |
| `p`     | Detener motor                       | `p`      |
| `r`     | Resetear falla                      | `r`      |
| `f[Hz]` | Configurar frecuencia               | `f50.0`  |
| `i`     | Leer información completa           | `i`      |
| `h`     | Mostrar ayuda                       | `h`      |

### Funciones Disponibles en el Código

#### Control Básico
```cpp
vfd_Start();              // Arrancar motor
vfd_Stop();               // Detener motor
vfd_StartReverse();       // Arrancar en reversa
vfd_ResetFault();         // Resetear falla
```

#### Configuración
```cpp
vfd_SetFrequency(50.0);   // Configurar a 50 Hz
vfd_SetFrequency(30.5);   // Configurar a 30.5 Hz
```

#### Lectura de Parámetros
```cpp
vfd_ReadStatus();         // Leer estado
vfd_ReadFrequency();      // Leer frecuencia actual
vfd_ReadCurrent();        // Leer corriente
vfd_ReadVoltage();        // Leer voltaje
vfd_ReadFaultCode();      // Leer código de falla
vfd_ReadAllParameters();  // Leer todos los parámetros
```

#### Control de I/O Local
```cpp
readInputs();             // Leer todas las entradas
writeOutput(0, HIGH);     // Activar salida 0
writeOutput(1, LOW);      // Desactivar salida 1
```

## 📊 Interpretación de Datos

### Frecuencia
Los valores de frecuencia están multiplicados por 100:
- Valor Modbus `5000` = `50.00 Hz`
- Valor Modbus `3050` = `30.50 Hz`

### Corriente
Los valores de corriente están multiplicados por 100:
- Valor Modbus `1500` = `15.00 A`

### Estado del Variador (Registro 3000)
- **Bit 0**: Motor en marcha (1 = corriendo)
- **Bit 2**: Falla activa (1 = hay falla)

## 🔍 Ejemplo de Secuencia Típica

```cpp
void setup() {
  // ... inicialización ...
  
  // Después de conectar WiFi y Modbus:
  delay(2000);  // Esperar estabilización
  
  vfd_SetFrequency(50.0);    // Configurar a 50 Hz
  delay(500);
  
  vfd_Start();               // Arrancar motor
}

void loop() {
  MBclient.loopHandler();    // SIEMPRE llamar esto
  
  // Leer estado cada 2 segundos
  static unsigned long lastRead = 0;
  if (millis() - lastRead >= 2000) {
    lastRead = millis();
    vfd_ReadStatus();
    vfd_ReadFrequency();
  }
  
  delay(10);
}
```

## ⚠️ Consideraciones Importantes

1. **Siempre llamar `MBclient.loopHandler()`** en el loop principal
2. **No saturar** el bus Modbus con peticiones muy rápidas (mínimo 50-100ms entre comandos)
3. **Verificar estado** antes de enviar comandos
4. **Resetear fallas** antes de intentar arrancar después de una falla
5. Los **pines del PLC** están fijos y no se pueden remapear

## 🛠️ Solución de Problemas

### No se conecta al WiFi
- Verificar SSID y contraseña
- Verificar que la IP 192.168.86.43 esté disponible en la red

### No hay comunicación Modbus
- Verificar que el gateway TCP/RTU esté en 192.168.86.100
- Verificar que el variador tenga Slave ID = 5
- Revisar baudrate y parámetros del RTU en el gateway

### El motor no arranca
- Leer código de falla con `vfd_ReadFaultCode()`
- Intentar `vfd_ResetFault()`
- Verificar que la frecuencia setpoint sea válida

## 📚 Referencias

- Manual del variador CW100 (incluido en el proyecto)
- Biblioteca eModbus: https://github.com/eModbus/eModbus
- PlatformIO ESP32: https://docs.platformio.org/en/latest/platforms/espressif32.html
