
#include "display.h"
#include "config.h"
#include "I2C_LCD.h"
#include <xc.h>

static uint8_t u8_to_str2(uint8_t n, char *buf, uint8_t pos) {
    buf[pos++] = (char)('0' + (n / 10));
    buf[pos++] = (char)('0' + (n % 10));
    return pos;
}

static uint8_t float_to_str_1dec(float val, char *buf, uint8_t pos) {
    int entero = (int)val;
    int decimal = (int)((val - (float)entero) * 10.0f + 0.5f);
    if (decimal >= 10) {
        decimal = 0;
        entero++;
    }
    if (entero >= 100) {
        buf[pos++] = (char)('0' + (entero / 100));
    }
    if (entero >= 10) {
        buf[pos++] = (char)('0' + ((entero / 10) % 10));
    }
    buf[pos++] = (char)('0' + (entero % 10));
    buf[pos++] = '.';
    buf[pos++] = (char)('0' + decimal);
    return pos;
}

static uint8_t float_to_str_3dec(float val, char *buf, uint8_t pos) {
    int entero = (int)val;
    int decimal = (int)((val - (float)entero) * 1000.0f + 0.5f);
    if (decimal >= 1000) {
        decimal = 0;
        entero++;
    }
    if (entero >= 10) {
        buf[pos++] = (char)('0' + (entero / 10));
    }
    buf[pos++] = (char)('0' + (entero % 10));
    buf[pos++] = '.';
    buf[pos++] = (char)('0' + (decimal / 100));
    buf[pos++] = (char)('0' + ((decimal / 10) % 10));
    buf[pos++] = (char)('0' + (decimal % 10));
    return pos;
}

static void fmt_config_line(char *buf, uint8_t von_star) {
    uint8_t i = 0;

    buf[i++] = von_star ? '*' : ' ';
    buf[i++] = 'V';
    buf[i++] = 'o';
    buf[i++] = 'n';
    buf[i++] = ':';
    i = u8_to_str2((uint8_t)t_von, buf, i);
    buf[i++] = ' ';
    buf[i++] = ' ';
    buf[i++] = von_star ? ' ' : '*';
    buf[i++] = 'V';
    buf[i++] = 'o';
    buf[i++] = 'f';
    buf[i++] = 'f';
    buf[i++] = ':';
    i = u8_to_str2((uint8_t)t_voff, buf, i);
    buf[i] = '\0';
}

static void fmt_manual_temp(char *buf) {
    uint8_t i = 0;

    buf[i++] = 'T';
    buf[i++] = ':';
    i = float_to_str_1dec(temperatura_actual, buf, i);
    buf[i++] = 'C';
    buf[i++] = ' ';
    buf[i++] = ' ';
    buf[i++] = 'M';
    buf[i++] = 'A';
    buf[i++] = 'N';
    buf[i++] = 'U';
    buf[i++] = 'A';
    buf[i++] = 'L';
    buf[i++] = ' ';
    buf[i++] = ' ';
    buf[i] = '\0';
}

static void fmt_auto_volt_temp(char *buf) {
    uint8_t i = 0;

    buf[i++] = 'V';
    buf[i++] = ':';
    i = float_to_str_3dec(voltaje_actual, buf, i);
    buf[i++] = 'V';
    buf[i++] = ' ';
    buf[i++] = 'T';
    buf[i++] = ':';
    i = float_to_str_1dec(temperatura_actual, buf, i);
    buf[i++] = 'C';
    buf[i] = '\0';
}

static void fmt_temp_stat_line(char *buf, float temp) {
    char num[8];
    uint8_t nlen = float_to_str_1dec(temp, num, 0);
    uint8_t pad = (uint8_t)((nlen < 4u) ? (4u - nlen) : 0u);
    uint8_t i = 0;
    uint8_t k;

    for (k = 0; k < 4u; k++) {
        buf[i++] = ' ';
    }
    for (k = 0; k < pad; k++) {
        buf[i++] = ' ';
    }
    for (k = 0; k < nlen; k++) {
        buf[i++] = num[k];
    }
    buf[i++] = ' ';
    buf[i++] = 'C';
    for (k = 0; k < 6u; k++) {
        buf[i++] = ' ';
    }
    buf[i] = '\0';
}

void actualizar_LCD(void) {
    char buf1[17], buf2[17];

    if (alarma_activa) {
        if (alarma_parpadeo) {
            LCD_Set_Cursor(1,1);
            LCD_Write_String("  ALERTA TEMP!  ");
            LCD_Set_Cursor(2,1);
            LCD_Write_String("   !! CRITICO!! ");
        } else {
            LCD_Clear();
        }
        lcd_necesita_update = 0;
        return;
    }

    if (modo_actual == MODO_CONFIG) {
        if (advertencia_activa) {
            LCD_Set_Cursor(1,1);
            LCD_Write_String("  VALOR INVALIDO");
            LCD_Set_Cursor(2,1);
            LCD_Write_String("Von-Voff >= 5C! ");
            lcd_necesita_update = 0;
            return;
        }
        fmt_config_line(buf1, (uint8_t)(campo_edit == EDIT_VON));
        LCD_Set_Cursor(1,1); LCD_Write_String(buf1);
        LCD_Set_Cursor(2,1); LCD_Write_String("UP/DN ajusta    ");
        lcd_necesita_update = 0;
        return;
    }

    if (modo_actual == MODO_MANUAL) {
        fmt_manual_temp(buf1);
        LCD_Set_Cursor(1,1); LCD_Write_String(buf1);
        if (fan_on) {
            LCD_Set_Cursor(2,1); LCD_Write_String("FAN: ON         ");
        } else {
            LCD_Set_Cursor(2,1); LCD_Write_String("FAN: OFF        ");
        }
        lcd_necesita_update = 0;
        return;
    }

    if (modo_actual == MODO_AUTO) {
        if (vista_actual == VISTA_NORMAL) {
            fmt_auto_volt_temp(buf1);
            LCD_Set_Cursor(1,1); LCD_Write_String(buf1);
            if (fan_on) {
                LCD_Set_Cursor(2,1); LCD_Write_String("FAN:ON  AUTO    ");
            } else {
                LCD_Set_Cursor(2,1); LCD_Write_String("FAN:OFF AUTO    ");
            }
        } else if (vista_actual == VISTA_MAX) {
            LCD_Set_Cursor(1,1); LCD_Write_String("  Temp. MAXIMA  ");
            fmt_temp_stat_line(buf2, temp_max);
            LCD_Set_Cursor(2,1); LCD_Write_String(buf2);
        } else if (vista_actual == VISTA_MIN) {
            LCD_Set_Cursor(1,1); LCD_Write_String("  Temp. MINIMA  ");
            fmt_temp_stat_line(buf2, temp_min);
            LCD_Set_Cursor(2,1); LCD_Write_String(buf2);
        }
        lcd_necesita_update = 0;
        return;
    }
}

void animacion_inicio(void) {
    LCD_Clear();
    LCD_Set_Cursor(1, 1); LCD_Write_String("  PT100 Sistema ");
    LCD_Set_Cursor(2, 1);
    for (uint8_t i = 0; i < 16; i++) {
        LCD_Set_Cursor(2, i + 1);
        LCD_Write_Char('#');
        __delay_ms(100);
    }
    __delay_ms(500);
    LCD_Clear();
}
