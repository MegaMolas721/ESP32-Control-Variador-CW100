# Correcciones Aplicadas - Control Variador CW100

## 📋 Cambios Realizados (23/12/2025)

### ✅ 1. Corrección de Comandos de Control (Registro 2000H)

**Antes (Incorrecto):**
- `CMD_STOP_FORWARD = 0x0001` 
- `CMD_RUN_FORWARD = 0x0002`

**Ahora (Correcto):**
- `CMD_RUN = 1` (0x0001) - Arranque
- `CMD_STOP = 6` (0x0006) - Paro

**Archivos actualizados:**
- `include/Config.h` - Definiciones de comandos corregidas
- `src/VFDController.cpp` - Funciones start() y stop() actualizadas

---

### ✅ 2. Visualización de Comando de Ejecución 2000H

Se agregó un nuevo campo en el dashboard web que muestra el último comando enviado al registro 2000H.

**Características:**
- Muestra valor **1** cuando se envía comando RUN
- Muestra valor **6** cuando se envía comando STOP  
- Se inicializa en **6** (STOP) al arrancar el sistema
- Se actualiza automáticamente en tiempo real

**Archivos modificados:**
- `include/VFDController.h` - Agregado campo `lastCommand` a estructura `VFDData`
- `src/VFDController.cpp` - Funciones actualizadas para registrar comando enviado
- `src/WebDashboard.cpp` - HTML y JavaScript actualizados con visualización del comando

**Ubicación en Dashboard:**
- Posicionado entre los botones RUN/STOP y el control de frecuencia
- Estilo: Caja azul con valor grande
- Leyenda: "1 = RUN / 6 = STOP"

---

### ✅ 3. Verificación de Registros Modbus

Todos los registros han sido verificados y coinciden exactamente con la tabla proporcionada:

| Registro HEX | Decimal | Descripción | Tipo |
|--------------|---------|-------------|------|
| **1000** | 4097 | Frecuencia Setpoint | Escritura |
| **2000** | 8193 | Comando de ejecución | Escritura |
| **1001** | 4098 | Frecuencia actual | Lectura |
| **1003** | 4100 | Voltaje de salida | Lectura |
| **1004** | 4101 | Corriente de salida (Amperaje real) | Lectura |
| **1007** | 4104 | Velocidad actual | Lectura |
| **3000** | 12289 | Estado del variador | Lectura |
| **8000** | 32769 | Código de Falla | Lectura |
| **F00A** | 61451 | Frecuencia Máxima | Lectura |
| **F00C** | 61453 | Límite Superior | Lectura |

---

## 🎨 Dashboard Actualizado

El dashboard web ahora incluye:

1. **Barra de Porcentaje de Velocidad** (1000H) - Barra progresiva animada
2. **Frecuencia Actual** (1001H) - Display grande en Hz  
3. **Corriente Actual** (1004H) - Display en Amperios
4. **Status Variador** (3000H) - Estado: 1=Adelante, 2=Atrás, 3=Stop
5. **Botones RUN/STOP** - Controles de ejecución
6. **🆕 Visualización Comando 2000H** - Muestra 1 (RUN) o 6 (STOP)
7. **Control de Frecuencia** - Input para configurar Hz
8. **Código de Falla** (8000H) - Display con botón de RESET

---

## 🔧 Uso del Sistema

### Comandos por Serial (115200 baud):
```
s, start  - Arrancar motor (envía 1 a registro 2000H)
p, stop   - Detener motor (envía 6 a registro 2000H)
r, reset  - Resetear falla
f50.0     - Configurar frecuencia a 50 Hz
i, info   - Leer todos los parámetros
```

### Dashboard Web:
- Acceder desde: **http://192.168.86.43**
- Botón verde **RUN**: Envía comando 1
- Botón rojo **STOP**: Envía comando 6
- El campo "Visualización de comando de ejecución 2000H" muestra el último comando

---

## 📁 Estructura Modular

```
ESP32_COntrolVariador/
├── include/
│   ├── Config.h           ✅ Actualizado - Comandos corregidos
│   ├── WiFiManager.h
│   ├── ModbusManager.h
│   ├── VFDController.h    ✅ Actualizado - Campo lastCommand agregado
│   ├── IOManager.h
│   └── WebDashboard.h
├── src/
│   ├── main.cpp
│   ├── config.cpp
│   ├── WiFiManager.cpp
│   ├── ModbusManager.cpp
│   ├── VFDController.cpp  ✅ Actualizado - Registro de comandos
│   ├── IOManager.cpp
│   └── WebDashboard.cpp   ✅ Actualizado - Visualización comando 2000H
└── platformio.ini
```

---

## ⚠️ Importante

1. **RUN = 1** (no 2 como estaba antes)
2. **STOP = 6** (no 1 como estaba antes)
3. El campo **lastCommand** se sincroniza automáticamente con cada comando enviado
4. El dashboard se actualiza cada **1 segundo** mostrando el estado en tiempo real
5. El valor inicial es **6 (STOP)** al arrancar el ESP32

---

## 🔍 Verificación

Para verificar que todo funciona correctamente:

1. Compilar y cargar el código al ESP32
2. Abrir monitor serial a 115200 baud
3. Conectarse al dashboard: http://192.168.86.43
4. Presionar botón **RUN** → El campo 2000H debe mostrar **1**
5. Presionar botón **STOP** → El campo 2000H debe mostrar **6**
6. El estado del variador (3000H) debe reflejar el comando enviado

---

**Cambios aplicados por:** GitHub Copilot  
**Fecha:** 23 de Diciembre de 2025  
**Versión:** 2.1
