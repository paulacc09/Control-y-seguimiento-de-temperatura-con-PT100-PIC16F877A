
#include "I2C_LCD.h"
#include "botones.h"
#include "config.h"
#include "eeprom.h"

// Variables privadas de anti-rebote (no necesitan ser globales)
static uint8_t rb0_prev = 1, rb1_prev = 1, rb2_prev = 1;
static uint16_t rb0_hold_count = 0;
static uint8_t  rb0_hold_fired = 0;
static uint8_t  rb0_debounce   = 0;
static uint8_t  rb1_debounce   = 0;
static uint8_t  rb2_debounce   = 0;
static uint8_t  rb12_fired     = 0;
static uint8_t  rb0_stable     = 0;
static uint8_t  rb1_stable     = 0;
static uint8_t  rb2_stable     = 0;

void leer_botones(void) {
    uint8_t rb0 = (BTN_MODE == 0);
    uint8_t rb1 = (BTN_UP == 0);
    uint8_t rb2 = (BTN_DOWN == 0);
    uint8_t combo_fired_this_tick = 0;

    // a) Debounce: estable tras 5 ms consecutivos en el mismo estado
    if (rb0 != rb0_prev) rb0_debounce = 0;
    if (rb1 != rb1_prev) rb1_debounce = 0;
    if (rb2 != rb2_prev) rb2_debounce = 0;
    if (rb0_debounce < 3) rb0_debounce++;
    if (rb1_debounce < 3) rb1_debounce++;
    if (rb2_debounce < 3) rb2_debounce++;
    rb0_prev = rb0;
    rb1_prev = rb1;
    rb2_prev = rb2;

    // b) Estado estable del ciclo anterior (antes de actualizar stable)
    uint8_t rb0_was_stable = rb0_stable;
    uint8_t rb1_was_stable = rb1_stable;
    uint8_t rb2_was_stable = rb2_stable;

    // c) Actualizar estado estable confirmado
    if (rb0_debounce == 3) rb0_stable = rb0;
    if (rb1_debounce == 3) rb1_stable = rb1;
    if (rb2_debounce == 3) rb2_stable = rb2;

    // MODE (RB0): pulsación larga mientras estable presionado
    if (rb0_stable) {
        rb0_hold_count++;
        if (!rb0_hold_fired && rb0_hold_count >= 80) {
            rb0_hold_fired = 1;
            if (modo_actual != MODO_CONFIG) {
                modo_anterior = modo_actual;
                modo_actual = MODO_CONFIG;
                campo_edit = EDIT_VON;
                lcd_necesita_update = 1;
                LCD_Clear();
            } else {
                guardar_umbrales_EEPROM();
                modo_actual = modo_anterior;
                lcd_necesita_update = 1;
                LCD_Clear();
            }
        }
    }

    // MODE (RB0): flanco descendente estable = pulsación corta si no hubo hold
    if (!rb0_stable && rb0_was_stable) {
        if (!rb0_hold_fired) {
            if (modo_actual == MODO_CONFIG) {
                campo_edit = (campo_edit == EDIT_VON) ? EDIT_VOFF : EDIT_VON;
                lcd_necesita_update = 1;
            } else if (modo_actual == MODO_AUTO) {
                if (vista_actual == VISTA_NORMAL) vista_actual = VISTA_MAX;
                else if (vista_actual == VISTA_MAX) vista_actual = VISTA_MIN;
                else vista_actual = VISTA_NORMAL;
                lcd_necesita_update = 1;
                LCD_Clear();
            } else if (modo_actual == MODO_MANUAL) {
                modo_actual = MODO_AUTO;
                vista_actual = VISTA_NORMAL;
                lcd_necesita_update = 1;
                LCD_Clear();
            }
        }
        rb0_hold_count = 0;
        rb0_hold_fired = 0;
    }

    // RB1 + RB2 en MODO_AUTO: una vez por combo
    if (modo_actual == MODO_AUTO && rb1_stable && rb2_stable) {
        if (!rb12_fired) {
            rb12_fired = 1;
            modo_actual = MODO_MANUAL;
            combo_fired_this_tick = 1;
            lcd_necesita_update = 1;
            LCD_Clear();
        }
    } else if (!rb1_stable || !rb2_stable) {
        rb12_fired = 0;
    }

    // d) UP (RB1): flanco ascendente estable
    if (!combo_fired_this_tick && rb1_stable && !rb1_was_stable) {
        if (modo_actual == MODO_MANUAL) {
            fan_on = 1;
            FAN_PIN = FAN_ON_VAL;
            lcd_necesita_update = 1;
        } else if (modo_actual == MODO_CONFIG) {
            if (campo_edit == EDIT_VON) {
                int8_t next = t_von + 1;
                if (next <= T_CONFIG_MAX && (next - t_voff) >= DIFF_MIN) {
                    t_von = next;
                    advertencia_activa = 0;
                    lcd_necesita_update = 1;
                } else {
                    advertencia_activa = 1;
                    advertencia_count = 0;
                }
            } else if (campo_edit == EDIT_VOFF) {
                int8_t next = t_voff + 1;
                if (next <= T_CONFIG_MAX && (t_von - next) >= DIFF_MIN) {
                    t_voff = next;
                    advertencia_activa = 0;
                    lcd_necesita_update = 1;
                } else {
                    advertencia_activa = 1;
                    advertencia_count = 0;
                }
            }
        }
    }

    // d) DOWN (RB2): flanco ascendente estable
    if (!combo_fired_this_tick && rb2_stable && !rb2_was_stable) {
        if (modo_actual == MODO_MANUAL) {
            fan_on = 0;
            FAN_PIN = FAN_OFF_VAL;
            lcd_necesita_update = 1;
        } else if (modo_actual == MODO_CONFIG) {
            if (campo_edit == EDIT_VON) {
                int8_t next = t_von - 1;
                if (next >= T_CONFIG_MIN && (next - t_voff) >= DIFF_MIN) {
                    t_von = next;
                    advertencia_activa = 0;
                    lcd_necesita_update = 1;
                } else {
                    advertencia_activa = 1;
                    advertencia_count = 0;
                }
            } else if (campo_edit == EDIT_VOFF) {
                int8_t next = t_voff - 1;
                if (next >= T_CONFIG_MIN && (t_von - next) >= DIFF_MIN) {
                    t_voff = next;
                    advertencia_activa = 0;
                    lcd_necesita_update = 1;
                } else {
                    advertencia_activa = 1;
                    advertencia_count = 0;
                }
            }
        }
    }

    // Warning timer
    if (advertencia_activa) {
        advertencia_count++;
        if (advertencia_count >= 2000) {
            advertencia_activa = 0;
            advertencia_count = 0;
        }
    }
}
