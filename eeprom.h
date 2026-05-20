#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

uint8_t leer_EEPROM(uint8_t address);
void    escribir_EEPROM(uint8_t address, uint8_t data);
void    cargar_umbrales_EEPROM(void);
void    guardar_umbrales_EEPROM(void);

#endif