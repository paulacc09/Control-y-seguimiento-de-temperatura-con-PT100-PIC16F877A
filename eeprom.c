#include "eeprom.h"
#include "config.h"

uint8_t leer_EEPROM(uint8_t address) {
    EEADR  = address;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD    = 1;
    return EEDATA;
}

void escribir_EEPROM(uint8_t address, uint8_t data) {
    uint8_t gi_state = GIE;
    EEADR  = address;
    EEDATA = data;
    EECON1bits.EEPGD = 0;
    EECON1bits.WREN  = 1;
    GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    while (EECON1bits.WR);
    EECON1bits.WREN = 0;
    GIE = gi_state;
}

void cargar_umbrales_EEPROM(void) {
    uint8_t magic = leer_EEPROM(EEPROM_MAGIC);
    if (magic == EEPROM_MAGIC_VAL) {
        t_von  = (int8_t)leer_EEPROM(EEPROM_VON);
        t_voff = (int8_t)leer_EEPROM(EEPROM_VOFF);
        if ((t_von  < T_CONFIG_MIN) || (t_von  > T_CONFIG_MAX)) t_von  = T_ON_DEFAULT;
        if ((t_voff < T_CONFIG_MIN) || (t_voff > T_CONFIG_MAX)) t_voff = T_OFF_DEFAULT;
        if ((t_von - t_voff) < DIFF_MIN) {
            t_von  = T_ON_DEFAULT;
            t_voff = T_OFF_DEFAULT;
        }
    } else {
        t_von  = T_ON_DEFAULT;
        t_voff = T_OFF_DEFAULT;
        guardar_umbrales_EEPROM();
    }
}

void guardar_umbrales_EEPROM(void) {
    escribir_EEPROM(EEPROM_MAGIC, EEPROM_MAGIC_VAL);
    escribir_EEPROM(EEPROM_VON,  (uint8_t)t_von);
    escribir_EEPROM(EEPROM_VOFF, (uint8_t)t_voff);
}