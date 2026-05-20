/*
* File: lcd.h
* Author: paula
 *
 * Created on April 10, 2026, 11:30 AM
*
* Descripción:
* Este archivo de encabezado define las funciones y pines necesarios para controlar
* una pantalla LCD (en modo de 8 bits) con un microcontrolador PIC. Incluye funciones
* para inicializar la LCD, enviar caracteres, comandos, cadenas de texto y mover el
cursor.
*/

#ifndef LCD_H
#define	LCD_H

#include <xc.h>

/******************* CONSTANTES *******************/

const unsigned char CMDs[8] = {0x32, 0x28, 0x08, 0x01, 0x04, 0x0C};
const unsigned char POSy[4] = {0x80, 0xC0, 0x94, 0xD4};

#define TRISLCD TRISB
#define PORTLCD PORTB

#define RS PORTBbits.RB5 
#define E  PORTBbits.RB4

#define D4 PORTBbits.RB3
#define D5 PORTBbits.RB2
#define D6 PORTBbits.RB1
#define D7 PORTBbits.RB0

/******************* FUNCIONES *******************/

void lcd_init(void);
void lcd_setcursor(unsigned char y, unsigned char x);
void lcd_print(char *string);
void lcd_clear(void);

void cmd(unsigned char cmd);
void str(unsigned char cmd);

#endif
