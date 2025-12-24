# CHANGELOG

Todas las modificaciones relevantes y notas de versiones para el proyecto.

## [Unreleased] - 2025-12-23
### Fixed
- Evitar que el slider de "Porcentaje de Velocidad Nominal" se reinicie a 0 al cambiar RUN/STOP.
- Corrección de lectura del registro de frecuencia (`0x1001`) para usar Holding Registers y sincronización adecuada.
- Protección de operaciones Modbus mediante un lock simple para evitar solapamientos y respuestas duplicadas.
- Añadida verificación de escritura para `REG_FREQUENCY_SETPOINT` (0x1000) y logging de verificación (`[VERIFY]`).
- Detección de sobrescritura externa: se registra `[ALERT]` si el setpoint escrito no coincide con la lectura posterior (posible otro maestro escribiendo).
- UI: `WebDashboard` ahora prefiere mostrar el `setpoint` efectivo (valor leído o último solicitado) para el slider.

### Added
- `getEffectiveSetpoint()` en `VFDController` para exponer el setpoint efectivo al UI.
- Registro detallado en serial para lecturas/escrituras Modbus (registro y valor en decimal/hex).
- Verificación inmediata tras escritura (lectura de confirmación).
- Archivo `README.md` actualizado con sección de diagnóstico y resolución.

### Notes
- Se recomienda deshabilitar otros maestros Modbus (ej. LOGO) durante las pruebas para evitar sobrescrituras.
- Si se desea, puede añadirse reintentos automáticos en caso de verificación fallida en una futura iteración.


---

*Generado automáticamente el 2025-12-23*