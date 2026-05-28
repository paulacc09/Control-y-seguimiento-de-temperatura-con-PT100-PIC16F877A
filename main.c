// Bits de configuración
#pragma config FOSC=XT, WDTE=OFF, PWRTE=OFF, BOREN=ON
#pragma config LVP=OFF, CPD=OFF, WRT=OFF, CP=OFF

#include <xc.h>
#include <stdint.h>
#include "config.h"
#include "adc.h"
#include "eeprom.h"
#include "control.h"
#include "botones.h"
#include "display.h"
#include "I2C_LCD.h"

// XC8: suprimir redeclaración de main para PIC8
#undef main

// ── Definición de variables globales (declaradas extern en config.h) ──────────
volatile uint8_t  flag_timer   = 0;
volatile uint16_t ticks_timer  = 0;
Modo_t      modo_actual    = MODO_AUTO;
Modo_t      modo_anterior  = MODO_AUTO;
Vista_t     vista_actual   = VISTA_NORMAL;
EditCampo_t campo_edit     = EDIT_VON;
uint8_t     fan_on         = 0;
int8_t      t_von          = T_ON_DEFAULT;
int8_t      t_voff         = T_OFF_DEFAULT;
float       temp_max       = -999.0f;
float       temp_min       =  999.0f;
uint8_t     alarma_activa  = 0;
uint8_t     alarma_parpadeo= 0;
float       voltaje_actual     = 0.0f;
float       temperatura_actual = 0.0f;
uint8_t     advertencia_activa = 0;
uint16_t    advertencia_count  = 0;
uint8_t     lcd_necesita_update = 1;

// ── ISR ───────────────────────────────────────────────────────────────────────
void __interrupt() isr(void) {
    if (TMR0IF) {
        TMR0IF = 0;
        TMR0   = 250;
        ticks_timer++;
        if (ticks_timer >= 50) {
            ticks_timer = 0;
            flag_timer  = 1;
        }
    }
}

// ── Timer0 ────────────────────────────────────────────────────────────────────
void configurar_timer0(void) {
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA  = 0;
    OPTION_REGbits.PS   = 0b111;
    TMR0   = 250;
    TMR0IF = 0;
    TMR0IE = 1;
    GIE    = 1;
    PEIE   = 1;
}

// ── Main ──────────────────────────────────────────────────────────────────────
void main(void) {
    ADCON1 = 0b10000000;

    TRISA = 0xFF; TRISB = 0xFF; TRISC = 0xFF;
    TRISD = 0x00; PORTD = 0x00;
    FAN_TRIS = 0; ALARM_TRIS = 0;
    FAN_PIN = FAN_OFF_VAL;

    OPTION_REGbits.nRBPU = 0;

    ADCON0 = 0b01000001;
    __delay_ms(10);

    I2C_Master_Init();
    __delay_ms(50);
    LCD_Init(LCD_ADDR);
    __delay_ms(20);

    animacion_inicio();
    cargar_umbrales_EEPROM();
    configurar_timer0();

    {
        uint16_t adc_init = leer_ADC();
        float    volt_init = 0.0f;
        float    temp_init = convertir_temperatura(adc_init, &volt_init);
        temp_max = temp_min = temp_init;
        voltaje_actual = volt_init;
        temperatura_actual = temp_init;
    }

    while (1) {
        leer_botones();

        if (flag_timer) {
            flag_timer = 0;
            uint16_t adc_val = leer_ADC();
            temperatura_actual = convertir_temperatura(adc_val, &voltaje_actual);
            if (temperatura_actual > temp_max) temp_max = temperatura_actual;
            if (temperatura_actual < temp_min) temp_min = temperatura_actual;
            control_histeresis();
            control_alarma();
            lcd_necesita_update = 1;
        }

        if (lcd_necesita_update) {
            actualizar_LCD();
        }

        leer_botones();
        __delay_ms(1);
    }
}