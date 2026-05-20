#include "botones.h"
#include "config.h"
#include "eeprom.h"
#include "I2C_LCD.h"

// Variables privadas de anti-rebote (no necesitan ser globales)
static uint8_t rb0_prev = 1, rb1_prev = 1, rb2_prev = 1;
static uint16_t rb0_hold_count = 0;
static uint8_t  rb0_hold_fired = 0;
static uint8_t  rb0_debounce   = 0;
static uint8_t  rb1_debounce   = 0;
static uint8_t  rb2_debounce   = 0;

void leer_botones(void) {
    uint8_t rb0 = BTN_MODE  ? 0 : 1;
    uint8_t rb1 = BTN_UP    ? 0 : 1;
    uint8_t rb2 = BTN_DOWN  ? 0 : 1;

    // ── RB0 MODE ──────────────────────────────────────────────────────────────
    if (rb0) {
        if (rb0_debounce < 20) rb0_debounce++;
        if (rb0_debounce >= 20) {
            rb0_hold_count++;
            if ((rb0_hold_count >= 2000) && !rb0_hold_fired) {
                rb0_hold_fired = 1;
                if (modo_actual != MODO_CONFIG) {
                    modo_anterior = modo_actual;
                    modo_actual   = MODO_CONFIG;
                    campo_edit    = EDIT_VON;
                    LCD_Clear();
                } else {
                    guardar_umbrales_EEPROM();
                    modo_actual = modo_anterior;
                    LCD_Clear();
                }
            }
        }
    } else {
        if ((rb0_debounce >= 20) && !rb0_hold_fired) {
            if (modo_actual == MODO_CONFIG) {
                campo_edit = (campo_edit == EDIT_VON) ? EDIT_VOFF : EDIT_VON;
            } else if (modo_actual == MODO_AUTO) {
                vista_actual = (Vista_t)((vista_actual + 1) % 3);
                LCD_Clear();
            } else if (modo_actual == MODO_MANUAL) {
                modo_actual  = MODO_AUTO;
                vista_actual = VISTA_NORMAL;
                LCD_Clear();
            }
        }
        rb0_debounce   = 0;
        rb0_hold_count = 0;
        rb0_hold_fired = 0;
    }
    rb0_prev = rb0;

    // ── RB1 UP ────────────────────────────────────────────────────────────────
    if (rb1 && !rb1_prev) rb1_debounce = 0;
    if (rb1) {
        if (rb1_debounce < 20) rb1_debounce++;
        if (rb1_debounce == 20) {
            if (modo_actual == MODO_MANUAL) {
                fan_on = 1; FAN_PIN = 1;
                rb1_debounce = 21;
            } else if (modo_actual == MODO_CONFIG) {
                if (campo_edit == EDIT_VON) {
                    int8_t nuevo_von = t_von + 1;
                    if (nuevo_von <= T_CONFIG_MAX && (nuevo_von - t_voff) >= DIFF_MIN) {
                        t_von = nuevo_von; advertencia_activa = 0;
                    } else { advertencia_activa = 1; advertencia_count = 0; }
                } else {
                    int8_t nuevo_voff = t_voff + 1;
                    if (nuevo_voff <= T_CONFIG_MAX && (t_von - nuevo_voff) >= DIFF_MIN) {
                        t_voff = nuevo_voff; advertencia_activa = 0;
                    } else { advertencia_activa = 1; advertencia_count = 0; }
                }
                rb1_debounce = 21;
            }
        }
    } else { rb1_debounce = 0; }
    rb1_prev = rb1;

    // ── RB2 DOWN ──────────────────────────────────────────────────────────────
    if (rb2 && !rb2_prev) rb2_debounce = 0;
    if (rb2) {
        if (rb2_debounce < 20) rb2_debounce++;
        if (rb2_debounce == 20) {
            if (modo_actual == MODO_MANUAL) {
                fan_on = 0; FAN_PIN = 0;
                rb2_debounce = 21;
            } else if (modo_actual == MODO_CONFIG) {
                if (campo_edit == EDIT_VON) {
                    int8_t nuevo_von = t_von - 1;
                    if (nuevo_von >= T_CONFIG_MIN && (nuevo_von - t_voff) >= DIFF_MIN) {
                        t_von = nuevo_von; advertencia_activa = 0;
                    } else { advertencia_activa = 1; advertencia_count = 0; }
                } else {
                    int8_t nuevo_voff = t_voff - 1;
                    if (nuevo_voff >= T_CONFIG_MIN && (t_von - nuevo_voff) >= DIFF_MIN) {
                        t_voff = nuevo_voff; advertencia_activa = 0;
                    } else { advertencia_activa = 1; advertencia_count = 0; }
                }
                rb2_debounce = 21;
            }
        }
    } else { rb2_debounce = 0; }
    rb2_prev = rb2;

    // ── RB1+RB2 simultáneo → MANUAL ───────────────────────────────────────────
    if (rb1 && rb2 && (modo_actual == MODO_AUTO)) {
        modo_actual = MODO_MANUAL;
        LCD_Clear();
    }

    // ── Temporizador advertencia CONFIG ───────────────────────────────────────
    if (advertencia_activa) {
        advertencia_count++;
        if (advertencia_count >= 2000) {
            advertencia_activa = 0;
            advertencia_count  = 0;
        }
    }
}