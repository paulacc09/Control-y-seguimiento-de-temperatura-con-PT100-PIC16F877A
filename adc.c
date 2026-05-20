#include "adc.h"
#include "config.h"

uint16_t leer_ADC(void) {
    ADCON0bits.GO_DONE = 1;
    while (ADCON0bits.GO_DONE);
    return ((uint16_t)(ADRESH << 8) | ADRESL);
}

float convertir_temperatura(uint16_t lectura, float *voltaje_out) {
    float vout = (lectura * VCC_ADC) / ADC_RESOL;
    if (voltaje_out) *voltaje_out = vout;
    float temp = T_MIN + vout * (RANGO_T / VOUT_MAX);
    if (temp < T_MIN)             temp = T_MIN;
    if (temp > (T_MIN + RANGO_T)) temp = T_MIN + RANGO_T;
    return temp;
}