# Control y seguimiento de temperatura con PT100 — PIC16F877A

Sistema embebido para medir temperatura mediante un sensor PT100 (señal condicionada a voltaje), mostrar datos en un LCD 16×2 por I2C (expansor PCF8574), controlar un ventilador con histéresis, activar una alarma por temperatura crítica y guardar umbrales configurables en EEPROM interna.

| Parámetro | Valor |
|-----------|-------|
| Microcontrolador | PIC16F877A |
| Compilador | XC8 v3.10 (MPLAB X / CMake) |
| Oscilador | Cristal XT 4 MHz (`_XTAL_FREQ = 4000000UL`) |
| Entrada de temperatura | AN0 (RA0), ADC 10 bits |
| Display | I2C — `I2C_LCD.c` (RC3 SCL, RC4 SDA, dirección 0x4E) |

---

## Arquitectura del firmware

| Módulo | Archivos | Responsabilidad |
|--------|----------|-----------------|
| Principal | `main.c` | Configuración, ISR Timer0, bucle principal |
| Configuración | `config.h` | Constantes, pines, tipos, variables globales |
| ADC | `adc.c`, `adc.h` | Lectura AN0 y conversión V → °C |
| Control | `control.c`, `control.h` | Histéresis ventilador y alarma |
| Botones | `botones.c`, `botones.h` | Modos, debounce, pulsaciones |
| Pantalla | `display.c`, `display.h` | Textos LCD y animación de inicio |
| EEPROM | `eeprom.c`, `eeprom.h` | Persistencia Von / Voff |
| LCD I2C | `I2C_LCD.c`, `I2C_LCD.h` | Driver MSSP + PCF8574 |
| LCD paralelo (legado) | `lcd.c`, `lcd.h` | **No compilado** — ver sección Build |

---

## Hardware

### Sensor y conversión
- Mapeo lineal: 0 V → 25 °C, 5,0075 V → 60 °C.
- Saturación entre 25 °C y 60 °C.
- Lectura ADC justificada a la derecha: máscara `(ADRESH & 0x03)` en `leer_ADC()`.

### Salidas (PORTD)
| Pin | Función |
|-----|---------|
| RD0 | Ventilador |
| RD1 | Alarma |

### Botones (PORTB, activos en bajo, pull-ups internos)
| Pin | Función |
|-----|---------|
| RB0 | MODE — vistas, CONFIG, pulsación larga |
| RB1 | UP |
| RB2 | DOWN |

### EEPROM interna
| Dirección | Contenido |
|-----------|-----------|
| 0x00 | Byte mágico `0xA5` |
| 0x01 | Von (°C) |
| 0x02 | Voff (°C) |

**Por defecto:** Von = 50 °C, Voff = 35 °C (histéresis 15 °C).

---

## Temporización

- **Timer0:** Fosc/4, prescaler 1:256, TMR0 = 246 (~2,56 ms por desborde).
- Tras **50 desbordes** se activa `flag_timer` (~128 ms).
- En cada tick: ADC, temperatura, min/max, control ventilador, alarma, LCD.
- Bucle principal: `leer_botones()` + `__delay_ms(1)`.

`ticks_timer` solo se usa dentro de la ISR.

---

## Modos de operación

### AUTO
- Ventilador: ON si T > Von; OFF si T < Voff; entre ambos mantiene estado.
- **MODE (corta):** Normal → Máxima → Mínima → Normal.
- **UP + DOWN:** pasa a MANUAL.

### MANUAL
- **UP:** ventilador ON. **DOWN:** ventilador OFF.
- **MODE (corta):** vuelve a AUTO.

### CONFIG
- **MODE ~2 s:** entra en CONFIG (guarda modo anterior).
- Ajuste Von/Voff con UP/DOWN; campo activo con `*`.
- Restricciones: 26–59 °C, Von − Voff ≥ 5 °C.
- Valor inválido: mensaje ~2 s (`advertencia_count` es `uint16_t`, umbral 2000).
- **MODE ~2 s de nuevo:** guarda EEPROM y restaura modo.

---

## Alarma

| Umbral | Valor |
|--------|-------|
| Activación | T ≥ 58 °C |
| Desactivación | T < 56 °C |

- LCD alterna mensaje de alerta y pantalla en blanco.
- RD1 permanece en alto mientras la alarma está activa.

---

## Flujo de arranque

1. Puertos (A/B/C entradas; D salidas ventilador/alarma).
2. ADC canal 0.
3. I2C y LCD.
4. Animación «PT100 Sistema».
5. Carga umbrales EEPROM.
6. Timer0 e interrupciones.
7. Primera lectura (inicializa T, V, min, max).

---

## Compilación y build

### Archivos compilados
`main.c`, `adc.c`, `botones.c`, `control.c`, `display.c`, `eeprom.c`, `I2C_LCD.c`

### `lcd.c` excluido del build
Driver paralelo en PORTB (RB0–RB5) **no se enlaza** (conflicto con botones RB0–RB2). El archivo queda en disco como referencia.

Exclusión configurada en:
- `nbproject/configurations.xml` — fuera de `SourceFiles`; `excluded="true"`
- `nbproject/Makefile-default.mk`
- `cmake/Digitales2_Proy_final/default/user.cmake`
- `.vscode/Digitales2_Proy_final.mplab.json` — lista explícita sin `lcd.c`

### Salida
`out/Digitales2_Proy_final/default.elf` (y `.hex`)

---

## VS Code / análisis estático

| Archivo | Propósito |
|---------|-----------|
| `.vscode/c_cpp_properties.json` | Perfil XC8, `__XC8`, compiler xc8 v3.10 |
| `.vscode/settings.json` | clangd MPLAB; `--suppress-diag=main_returns_nonint` |
| `main.c` | `#undef main` para XC8 |

`void main(void)` es válido en PIC8 con XC8; `main_returns_nonint` es falso positivo de clangd.

**Simulador:** usar **4 MHz**, no 1 MHz.

---

## Correcciones aplicadas

| Problema | Solución |
|----------|----------|
| Flancos de botones no detectados | Variables `rb*_was` en `botones.c` |
| ADC con bits basura en ADRESH | Máscara `(ADRESH & 0x03)` en `adc.c` |
| Mensaje «VALOR INVALIDO» no expiraba | `advertencia_count` → `uint16_t` |
| `lcd.c` compilado sin uso | Exclusión en nbproject, CMake y mplab.json |
| Conflicto `void main` en IDE | `#undef main`, suppress clangd, `c_cpp_properties.json` |

---

## Estructura del repositorio

```
├── main.c, config.h
├── adc.c / adc.h
├── control.c / control.h
├── botones.c / botones.h
├── display.c / display.h
├── eeprom.c / eeprom.h
├── I2C_LCD.c / I2C_LCD.h
├── lcd.c / lcd.h          # Legado, no en build
├── nbproject/
├── cmake/
├── .vscode/
└── README.md
```

---

## Contexto

Proyecto **Digitales 2** (UNI): control y seguimiento de temperatura con PT100 en PIC16F877A.

---

## Pruebas recomendadas

1. AUTO: histéresis y vistas max/min (MODE corta).
2. MANUAL: UP/DOWN; salida con MODE corta.
3. CONFIG: MODE larga, Von/Voff, mensaje inválido ~2 s, EEPROM.
4. Alarma ≥ 58 °C.
5. Compilación sin `lcd.p1`.
