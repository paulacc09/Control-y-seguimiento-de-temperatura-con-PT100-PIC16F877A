#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>
#include <stdint.h>

// Frecuencia del oscilador
#define _XTAL_FREQ      4000000UL
#define LCD_ADDR        0x4E

// Conversión ADC
#define VCC_ADC         5.0f
#define ADC_RESOL       1023.0f
#define T_MIN           25.0f
#define RANGO_T         35.0f
#define VOUT_MAX        5.0075f

// Umbrales por defecto
#define T_ON_DEFAULT    50
#define T_OFF_DEFAULT   35
#define DIFF_MIN        5
#define T_CONFIG_MAX    59
#define T_CONFIG_MIN    26

// Alarma
#define T_ALARMA_ON     58.0f
#define T_ALARMA_OFF    56.0f

// Pines ventilador y alarma
#define FAN_TRIS        TRISDbits.TRISD0
#define FAN_PIN         PORTDbits.RD0
#define ALARM_TRIS      TRISDbits.TRISD1
#define ALARM_PIN       PORTDbits.RD1

// Botones
#define BTN_MODE        PORTBbits.RB0
#define BTN_UP          PORTBbits.RB1
#define BTN_DOWN        PORTBbits.RB2

// EEPROM
#define EEPROM_MAGIC    0x00
#define EEPROM_VON      0x01
#define EEPROM_VOFF     0x02
#define EEPROM_MAGIC_VAL 0xA5

// ── Modos y vistas ──────────────────────────────
typedef enum { MODO_AUTO = 0, MODO_MANUAL, MODO_CONFIG } Modo_t;
typedef enum { VISTA_NORMAL = 0, VISTA_MAX, VISTA_MIN }  Vista_t;
typedef enum { EDIT_VON = 0, EDIT_VOFF }                 EditCampo_t;

// ── Variables globales compartidas (extern) ──────
// Timer
extern volatile uint8_t  flag_timer;
extern volatile uint16_t ticks_timer;

// Modo / vista
extern Modo_t      modo_actual;
extern Modo_t      modo_anterior;
extern Vista_t     vista_actual;
extern EditCampo_t campo_edit;

// Ventilador
extern uint8_t fan_on;

// Umbrales
extern int8_t t_von;
extern int8_t t_voff;

// Historial
extern float temp_max;
extern float temp_min;

// Alarma
extern uint8_t alarma_activa;
extern uint8_t alarma_parpadeo;

// Temperatura/voltaje actuales
extern float voltaje_actual;
extern float temperatura_actual;

// Advertencia CONFIG
extern uint8_t advertencia_activa;
extern uint8_t advertencia_count;

#endif