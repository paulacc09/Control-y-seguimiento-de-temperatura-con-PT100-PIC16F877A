#include "control.h"
#include "config.h"

void control_histeresis(void) {
    if (modo_actual == MODO_AUTO) {
        if (temperatura_actual > (float)t_von) {
            fan_on  = 1;
            FAN_PIN = 1;
        } else if (temperatura_actual < (float)t_voff) {
            fan_on  = 0;
            FAN_PIN = 0;
        }
    }
}

void control_alarma(void) {
    if (!alarma_activa) {
        if (temperatura_actual >= T_ALARMA_ON) {
            alarma_activa = 1;
            ALARM_PIN     = 1;
        }
    } else {
        if (temperatura_actual < T_ALARMA_OFF) {
            alarma_activa   = 0;
            ALARM_PIN       = 0;
            alarma_parpadeo = 0;
        }
        alarma_parpadeo = !alarma_parpadeo;
    }
}