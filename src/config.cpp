/**
 * @file config.cpp
 * @brief Implementación de configuraciones globales
 */

#include "config.h"

// Definición de pines de entrada
const uint8_t INPUT_PINS[NUM_INPUTS] = {
    17, 18, 19, 21, 22, 23, 27, 32, 33
};

// Definición de pines de salida
// DESHABILITADOS: GPIO1 (TX0/UART), GPIO12 (strapping), GPIO15 (strapping)
const uint8_t OUTPUT_PINS[NUM_OUTPUTS] = {
    2, 3, 4, 5, 13, 14, 16
};
