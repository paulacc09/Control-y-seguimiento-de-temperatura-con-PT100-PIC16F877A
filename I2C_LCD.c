#include "config.h"
#include <xc.h>
#include "I2C_LCD.h"
 
unsigned char RS, i2c_add, BackLight_State = LCD_BACKLIGHT;
#define I2C_BaudRate 100000
#define SCL_D TRISC3
#define SDA_D TRISC4
#define I2C_WAIT_TIMEOUT  5000u

static void I2C_Bus_Recovery(void) {
    uint8_t i;

    SSPCON = 0x00;
    __delay_us(10);

    TRISC3 = 0;
    TRISC4 = 0;
    RC3 = 1;
    RC4 = 1;

    for (i = 0; i < 9u; i++) {
        RC3 = 0;
        __delay_us(10);
        RC3 = 1;
        __delay_us(10);
    }

    RC4 = 0;
    __delay_us(10);
    RC3 = 1;
    __delay_us(10);
    RC4 = 1;
    __delay_us(10);

    __delay_us(50);
    I2C_Master_Init();
}

static uint8_t I2C_Wait_SSPIF(void) {
    uint16_t timeout = I2C_WAIT_TIMEOUT;

    while (!SSPIF) {
        if (--timeout == 0u) {
            I2C_Bus_Recovery();
            return 0;
        }
    }
    SSPIF = 0;
    return 1;
}

//---------------[ I2C Routines ]-------------------
//--------------------------------------------------
void I2C_Master_Init()
{
  SSPCON = 0x28;
  SSPCON2 = 0x00;
  SSPSTAT = 0x00;
  SSPADD = ((_XTAL_FREQ/4)/I2C_BaudRate) - 1;
  SCL_D = 1;
  SDA_D = 1;
  __delay_ms(10);
}
 
void I2C_Master_Wait()
{
  uint16_t timeout = I2C_WAIT_TIMEOUT;

  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)) {
    if (--timeout == 0u) {
      I2C_Bus_Recovery();
      return;
    }
  }
}
 
void I2C_Master_Start()
{
  I2C_Master_Wait();
  SEN = 1;
}
 
void I2C_Master_RepeatedStart()
{
  I2C_Master_Wait();
  RSEN = 1;
}
 
void I2C_Master_Stop()
{
  I2C_Master_Wait();
  PEN = 1;
}
 
void I2C_ACK(void)
{
  ACKDT = 0; // 0 -> ACK
  I2C_Master_Wait();
  ACKEN = 1; // Send ACK
}
 
void I2C_NACK(void)
{
  ACKDT = 1; // 1 -> NACK
  I2C_Master_Wait();
  ACKEN = 1; // Send NACK
}
 
unsigned char I2C_Master_Write(unsigned char data)
{
  I2C_Master_Wait();
  SSPBUF = data;
  if (!I2C_Wait_SSPIF()) {
    return 1;
  }
  return ACKSTAT;
}
 
unsigned char I2C_Read_Byte(void)
{
  //---[ Receive & Return A Byte ]---
  I2C_Master_Wait();
  RCEN = 1;
  if (!I2C_Wait_SSPIF()) {
    return 0;
  }
  I2C_Master_Wait();
  return SSPBUF;
}
//======================================================
 
//---------------[ LCD Routines ]----------------
//-----------------------------------------------
 
void LCD_Init(unsigned char I2C_Add)
{
  i2c_add = I2C_Add;
  IO_Expander_Write(0x00);
  __delay_ms(30);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(LCD_RETURN_HOME);
  __delay_ms(5);
  LCD_CMD(0x20 | (LCD_TYPE << 2));
  __delay_ms(5);
  LCD_CMD(LCD_TURN_ON);
  __delay_ms(5);
  LCD_CMD(LCD_CLEAR);
  __delay_ms(5);
  LCD_CMD(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  __delay_ms(5);
}
 
void IO_Expander_Write(unsigned char Data)
{
  I2C_Master_Start();
  I2C_Master_Write(i2c_add);
  I2C_Master_Write(Data | BackLight_State);
  I2C_Master_Stop();
}
 
void LCD_Write_4Bit(unsigned char Nibble)
{
  // Get The RS Value To LSB OF Data
  Nibble |= RS;
  IO_Expander_Write(Nibble | 0x04);
  IO_Expander_Write(Nibble & 0xFB);
  __delay_us(50);
}
 
void LCD_CMD(unsigned char CMD)
{
  RS = 0; // Command Register Select
  LCD_Write_4Bit(CMD & 0xF0);
  LCD_Write_4Bit((CMD << 4) & 0xF0);
}
 
void LCD_Write_Char(char Data)
{
  RS = 1; // Data Register Select
  LCD_Write_4Bit(Data & 0xF0);
  LCD_Write_4Bit((Data << 4) & 0xF0);
}
 
void LCD_Write_String(char* Str)
{
  for(int i=0; Str[i]!='\0'; i++)
    LCD_Write_Char(Str[i]);
}
 
void LCD_Set_Cursor(unsigned char ROW, unsigned char COL)
{
  switch(ROW) 
  {
    case 2:
      LCD_CMD(0xC0 + COL-1);
      break;
    case 3:
      LCD_CMD(0x94 + COL-1);
      break;
    case 4:
      LCD_CMD(0xD4 + COL-1);
      break;
    // Case 1
    default:
      LCD_CMD(0x80 + COL-1);
  }
}
 
void Backlight()
{
  BackLight_State = LCD_BACKLIGHT;
  IO_Expander_Write(0);
}
 
void noBacklight()
{
  BackLight_State = LCD_NOBACKLIGHT;
  IO_Expander_Write(0);
}
 
void LCD_SL()
{
  LCD_CMD(0x18);
  __delay_us(40);
}
 
void LCD_SR()
{
  LCD_CMD(0x1C);
  __delay_us(40);
}
 
void LCD_Clear()
{
  LCD_CMD(0x01);
  __delay_ms(2);
}