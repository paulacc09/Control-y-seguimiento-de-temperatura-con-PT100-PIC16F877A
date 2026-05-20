#include "display.h"
#include "config.h"
#include "I2C_LCD.h"
#include <stdio.h>
#include <xc.h>

void actualizar_LCD(void) {
    char buf1[17], buf2[17];

    if (alarma_activa) {
        if (alarma_parpadeo) {
            LCD_Set_Cursor(1, 1); LCD_Write_String("  ALERTA TEMP!  ");
            LCD_Set_Cursor(2, 1); LCD_Write_String("   !! CRITICO!! ");
        } else { LCD_Clear(); }
        return;
    }

    if (modo_actual == MODO_CONFIG) {
        if (advertencia_activa) {
            LCD_Set_Cursor(1, 1); LCD_Write_String("  VALOR INVALIDO");
            LCD_Set_Cursor(2, 1); LCD_Write_String("Von-Voff >= 5C! ");
            return;
        }
        LCD_Set_Cursor(1, 1);
        if (campo_edit == EDIT_VON)
            sprintf(buf1, "*Von:%2d  Voff:%2d", t_von, t_voff);
        else
            sprintf(buf1, " Von:%2d *Voff:%2d", t_von, t_voff);
        LCD_Write_String(buf1);
        LCD_Set_Cursor(2, 1); LCD_Write_String("UP/DN ajusta    ");
        return;
    }

    if (modo_actual == MODO_MANUAL) {
        LCD_Set_Cursor(1, 1);
        sprintf(buf1, "T:%.1fC  MANUAL  ", temperatura_actual);
        LCD_Write_String(buf1);
        LCD_Set_Cursor(2, 1);
        if (fan_on) LCD_Write_String("FAN:ON  U=ON D=OF");
        else        LCD_Write_String("FAN:OFF U=ON D=OF");
        return;
    }

    switch (vista_actual) {
        case VISTA_NORMAL:
        default:
            sprintf(buf1, "V:%.3fV T:%.1fC", voltaje_actual, temperatura_actual);
            LCD_Set_Cursor(1, 1); LCD_Write_String(buf1);
            LCD_Set_Cursor(2, 1);
            sprintf(buf2, "FAN:%s  AUTO    ", fan_on ? "ON " : "OFF");
            LCD_Write_String(buf2);
            break;
        case VISTA_MAX:
            LCD_Set_Cursor(1, 1); LCD_Write_String("  Temp. MAXIMA  ");
            LCD_Set_Cursor(2, 1);
            sprintf(buf2, "    %.1f C       ", temp_max > -999.0f ? temp_max : 0.0f);
            LCD_Write_String(buf2);
            break;
        case VISTA_MIN:
            LCD_Set_Cursor(1, 1); LCD_Write_String("  Temp. MINIMA  ");
            LCD_Set_Cursor(2, 1);
            sprintf(buf2, "    %.1f C       ", temp_min < 999.0f ? temp_min : 0.0f);
            LCD_Write_String(buf2);
            break;
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