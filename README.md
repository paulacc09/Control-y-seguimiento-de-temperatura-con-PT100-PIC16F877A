# Control y seguimiento de temperatura con PT100 — PIC16F877A

Firmware embebido para medir temperatura con sensor **PT100**, mostrar datos en **LCD 16×2 por I2C**, controlar un **ventilador con histéresis**, activar **alarma** por temperatura crítica y guardar **umbrales configurables** en EEPROM interna.

Proyecto de **Digitales 2 (UNI)**.

## Características

- Lectura de temperatura por ADC (AN0) con conversión lineal 25–60 °C
- Modos **AUTO**, **MANUAL** y **CONFIG** con botones en PORTB
- Histéresis del ventilador (Von/Voff) persistente en EEPROM
- Alarma a partir de 58 °C con indicación en LCD y salida RD1
- Vistas de temperatura máxima y mínima en modo AUTO
- Formateo manual en LCD (sin `printf`) para ahorrar RAM del PIC16F877A

## Hardware

| Componente | Detalle |
|------------|---------|
| Microcontrolador | PIC16F877A @ 4 MHz (XT) |
| Sensor | PT100 condicionado a voltaje (AN0) |
| Display | LCD I2C vía PCF8574 (RC3/RC4, dirección `0x4E`) |
| Ventilador | RD0 |
| Alarma | RD1 |
| Botones | RB0 MODE, RB1 UP, RB2 DOWN |

## Compilación

- **Compilador:** XC8 v3.10 (MPLAB X / CMake)
- **Archivos compilados:** `main.c`, `adc.c`, `botones.c`, `control.c`, `display.c`, `eeprom.c`, `I2C_LCD.c`
- **`lcd.c` no se compila** (driver paralelo legado; conflicto con botones en PORTB)
- **Salida:** `out/Digitales2_Proy_final/default.elf` (y `.hex`)

Abrir el proyecto en MPLAB X y ejecutar **Clean and Build**. En el log no debe aparecer `lcd.p1`.

## Documentación

Documentación completa (arquitectura, modos de operación, temporización, build, VS Code y correcciones aplicadas):

**[docs/README.md](docs/README.md)**
