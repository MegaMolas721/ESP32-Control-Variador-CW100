---
name: Bug report
about: Report a reproducible bug or unexpected behavior in firmware/UI
---

**Título breve**
<!-- Ej: Slider se reinicia a 0 al hacer RUN -->

**Descripción**
<!-- Descripción concisa del problema observado -->

**Pasos para reproducir**
1. Cargar firmware (commit / versión X) en ESP32
2. Abrir Dashboard en http://<ip>
3. Ajustar slider a un valor distinto de 0 (p.ej. 30%)
4. Pulsar RUN
5. Observar que el slider se reinicia a 0 temporalmente

**Resultado esperado**
- El slider conserva el valor del setpoint (1000H) independientemente de RUN/STOP.

**Salida del monitor serial**
- Adjuntar líneas relevantes del serial, p. ej.:
  - `Configurando velocidad: 30.00 Hz = 50.00% (0x...)`
  - `Escribiendo: Registro=4096 (0x1000), Valor=5000`
  - `[VERIFY] Lectura verificación para registro 0x1000: valor=5000 (0x1388)`
  - Cualquier `[ALERT]` o `Error Modbus` mostrado

**Archivos relacionados / cambios recientes**
- `src/VFDController.cpp` (locks Modbus, verificación, detección sobrescritura)
- `src/WebDashboard.cpp` (UI: usa setpoint efectivo)
- `include/VFDController.h` (nuevo getter `getEffectiveSetpoint`)

**Contexto adicional**
- Gateway Modbus: 192.168.86.100:502
- Variador CW100 Slave ID: 5
- eModbus v1.7.4

**Posible causa**
- Otro maestro Modbus (LOGO) sobrescribiendo registros o configuración del variador que impide exponer la frecuencia actual en `0x1001`.

**Prioridad**: P3 (verificación/UX)

**Adjuntos**: logs, capturas de pantalla, capturas serial


<!-- Mantener este template y completar antes de crear el issue -->