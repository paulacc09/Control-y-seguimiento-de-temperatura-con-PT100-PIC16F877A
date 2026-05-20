#ifndef ADC_H
#define ADC_H

#include <stdint.h>

uint16_t leer_ADC(void);
float    convertir_temperatura(uint16_t lectura, float *voltaje_out);

#endif