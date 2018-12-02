// Help from http://w8bh.net/avr/AvrTFT.pdf (modified)

//-----------------------------------------------------------------------------
// TFT: Experiments interfacing ATmega328 to an ST7735 1.8" LCD TFT display
//
// Author : Bruce E. Hall <bhall66@gmail.com>
// Website : w8bh.net
// Version : 1.0
// Date : 04 May 2014
// Target : ATmega328P microcontroller
// Language : C, using AVR studio 6
// Size : 3622 bytes
//
// Fuse settings: 8 MHz osc with 65 ms Delay, SPI enable; *NO* clock/8
//
// Connections from LCD to DC Boarduino:
//
// TFT pin 1 (backlight) +5V
// TFT pin 2 (MISO) n/c
// TFT pin 3 (SCK) digital13, PB5(SCK)
// TFT pin 4 (MOSI) digital11, PB3(MOSI)
// TFT pin 5 (TFT_Select) gnd
// TFT pin 7 (DC) digital9, PB1
// TFT pin 8 (Reset) digital8, PB0
// TFT pin 9 (Vcc) +5V
// TFT pin 10 (gnd) gnd
//
// ---------------------------------------------------------------------------
// GLOBAL DEFINES
#define F_CPU 16000000L // run CPU at 16 MHz
#define LED 5 // Boarduino LED on PB5
#define ClearBit(x,y) x &= ~_BV(y) // equivalent to cbi(x,y)
#define SetBit(x,y) x |= _BV(y) // equivalent to sbi(x,y)
// ---------------------------------------------------------------------------
// INCLUDES
#include <avr/io.h> // deal with port registers
#include <avr/interrupt.h> // deal with interrupt calls
#include <avr/pgmspace.h> // put character data into progmem
#include <util/delay.h> // used for _delay_ms function
#include <string.h> // string manipulation routines
#include <avr/sleep.h> // used for sleep functions
#include <stdlib.h>
// ---------------------------------------------------------------------------
// TYPEDEFS
typedef uint8_t byte; // I just like byte & sbyte better
typedef int8_t sbyte;
// ---------------------------------------------------------------------------
// GLOBAL VARIABLES
const byte FONT_CHARS[96][5] PROGMEM =
{
 { 0x00, 0x00, 0x00, 0x00, 0x00 }, // (space)
 { 0x00, 0x00, 0x5F, 0x00, 0x00 }, // !
 { 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
 { 0x14, 0x7F, 0x14, 0x7F, 0x14 }, // #
 { 0x12, 0x2A, 0x7F, 0x2A, 0x24 }, // $
 { 0x62, 0x64, 0x08, 0x13, 0x23 }, // %
 { 0x50, 0x22, 0x55, 0x49, 0x36 }, // &
 { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
 { 0x00, 0x41, 0x22, 0x1C, 0x00 }, // (
 { 0x00, 0x1C, 0x22, 0x41, 0x00 }, // )
 { 0x08, 0x2A, 0x1C, 0x2A, 0x08 }, // *
 { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // +
 { 0x00, 0x00, 0x30, 0x50, 0x00 }, // ,
 { 0x08, 0x08, 0x08, 0x08, 0x08 }, // -
 { 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
 { 0x02, 0x04, 0x08, 0x10, 0x20 }, // /
 { 0x3E, 0x45, 0x49, 0x51, 0x3E }, // 0
 { 0x00, 0x40, 0x7F, 0x42, 0x00 }, // 1
 { 0x46, 0x49, 0x51, 0x61, 0x42 }, // 2
 { 0x31, 0x4B, 0x45, 0x41, 0x21 }, // 3
 { 0x10, 0x7F, 0x12, 0x14, 0x18 }, // 4
 { 0x39, 0x45, 0x45, 0x45, 0x27 }, // 5
 { 0x30, 0x49, 0x49, 0x4A, 0x3C }, // 6
 { 0x03, 0x05, 0x09, 0x71, 0x01 }, // 7
 { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
 { 0x1E, 0x29, 0x49, 0x49, 0x06 }, // 9
 { 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
 { 0x00, 0x00, 0x36, 0x56, 0x00 }, // ;
 { 0x41, 0x22, 0x14, 0x08, 0x00 }, // <
 { 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
 { 0x00, 0x08, 0x14, 0x22, 0x41 }, // >
 { 0x06, 0x09, 0x51, 0x01, 0x02 }, // ?
 { 0x3E, 0x41, 0x79, 0x49, 0x32 }, // @
 { 0x7E, 0x11, 0x11, 0x11, 0x7E }, // A
 { 0x36, 0x49, 0x49, 0x49, 0x7F }, // B
 { 0x22, 0x41, 0x41, 0x41, 0x3E }, // C
 { 0x1C, 0x22, 0x41, 0x41, 0x7F }, // D
 { 0x41, 0x49, 0x49, 0x49, 0x7F }, // E
 { 0x01, 0x01, 0x09, 0x09, 0x7F }, // F
 { 0x32, 0x51, 0x41, 0x41, 0x3E }, // G
 { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // H
 { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // I
 { 0x01, 0x3F, 0x41, 0x40, 0x20 }, // J
 { 0x41, 0x22, 0x14, 0x08, 0x7F }, // K
 { 0x40, 0x40, 0x40, 0x40, 0x7F }, // L
 { 0x7F, 0x02, 0x04, 0x02, 0x7F }, // M
 { 0x7F, 0x10, 0x08, 0x04, 0x7F }, // N
 { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // O
 { 0x06, 0x09, 0x09, 0x09, 0x7F }, // P
 { 0x5E, 0x21, 0x51, 0x41, 0x3E }, // Q
 { 0x46, 0x29, 0x19, 0x09, 0x7F }, // R
 { 0x31, 0x49, 0x49, 0x49, 0x46 }, // S
 { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // T
 { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // U
 { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // V
 { 0x7F, 0x20, 0x18, 0x20, 0x7F }, // W
 { 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
 { 0x03, 0x04, 0x78, 0x04, 0x03 }, // Y
 { 0x43, 0x45, 0x49, 0x51, 0x61 }, // Z
 { 0x41, 0x41, 0x7F, 0x00, 0x00 }, // [
 { 0x20, 0x10, 0x08, 0x04, 0x02 }, // "\"
 { 0x00, 0x00, 0x7F, 0x41, 0x41 }, // ]
 { 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
 { 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
 { 0x00, 0x04, 0x02, 0x01, 0x00 }, // `
 { 0x78, 0x54, 0x54, 0x54, 0x20 }, // a
 { 0x38, 0x44, 0x44, 0x48, 0x7F }, // b
 { 0x20, 0x44, 0x44, 0x44, 0x38 }, // c
 { 0x7F, 0x44, 0x44, 0x48, 0x38 }, // d
 { 0x18, 0x54, 0x54, 0x54, 0x38 }, // e
 { 0x02, 0x01, 0x09, 0x7E, 0x08 }, // f
 { 0x3C, 0x54, 0x54, 0x14, 0x08 }, // g
 { 0x78, 0x04, 0x04, 0x08, 0x7F }, // h
 { 0x00, 0x40, 0x7D, 0x44, 0x00 }, // i
 { 0x00, 0x3D, 0x44, 0x40, 0x20 }, // j
 { 0x44, 0x28, 0x10, 0x7F, 0x00 }, // k
 { 0x00, 0x40, 0x7F, 0x41, 0x00 }, // l
 { 0x78, 0x04, 0x18, 0x04, 0x7C }, // m
 { 0x78, 0x04, 0x04, 0x08, 0x7C }, // n
 { 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
 { 0x08, 0x14, 0x14, 0x14, 0x7C }, // p
 { 0x7C, 0x18, 0x14, 0x14, 0x08 }, // q
 { 0x08, 0x04, 0x04, 0x08, 0x7C }, // r { 0x7C, 0x08, 0x04, 0x04, 0x08 },
 { 0x20, 0x54, 0x54, 0x54, 0x48 }, // s
 { 0x20, 0x40, 0x44, 0x3F, 0x04 }, // t
 { 0x7C, 0x20, 0x40, 0x40, 0x3C }, // u
 { 0x1C, 0x20, 0x40, 0x20, 0x1C }, // v
 { 0x3C, 0x40, 0x30, 0x40, 0x3C }, // w
 { 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
 { 0x3C, 0x50, 0x50, 0x50, 0x0C }, // y
 { 0x44, 0x4C, 0x54, 0x64, 0x44 }, // z
 { 0x00, 0x41, 0x36, 0x08, 0x00 }, // {
 { 0x00, 0x00, 0x7F, 0x00, 0x00 }, // |
 { 0x00, 0x08, 0x36, 0x41, 0x00 }, // }
 { 0x08, 0x1C, 0x2A, 0x08, 0x08 }, // ->
 { 0x08, 0x08, 0x2A, 0x1C, 0x08 }, // <-
};
// ---------------------------------------------------------------------------
// MISC ROUTINES
void SetupPorts()
{
 DDRB = 0xFF; // 0010.1111; set B0-B3, B5 as outputs
 DDRC = 0x00; // 0000.0000; set PORTC as inputs
 SetBit(PORTB,0); // start with TFT reset line inactive high
}
void msDelay(int delay) // put into a routine
{ // to remove code inlining
 for (int i=0;i<delay;i++) // at cost of timing accuracy
 _delay_ms(1);
}
void FlashLED(byte count)
// flash the on-board LED at ~ 3 Hz
{
 for (;count>0;count--)
 {
 SetBit(PORTB,LED); // turn LED on
 msDelay(150); // wait
 ClearBit(PORTB,LED); // turn LED off
 msDelay(150); // wait
 }
}
unsigned long intsqrt(unsigned long val)
// calculate integer value of square root
{
 unsigned long mulMask = 0x0008000;
 unsigned long retVal = 0;
 if (val > 0)
 {
 while (mulMask != 0)
 {
 retVal |= mulMask;
 if ((retVal*retVal) > val)
 retVal &= ~ mulMask;
 mulMask >>= 1;
 }
 }
 return retVal;
}
/*
char* itoa(int i, char b[]){
 char const digit[] = "0123456789";
 char* p = b;
 if(i<0){
 *p++ = '-';
 i *= -1;
 }
 int shifter = i;
 do{ //Move to where representation ends
 ++p;
 shifter = shifter/10;
 }while(shifter);
 *p = '\0';
 do{ //Move back, inserting digits as u go
 *--p = digit[i%10];
 i = i/10;
 }while(i);
 return b;
}
*/
// ---------------------------------------------------------------------------
// SPI ROUTINES
//
// b7 b6 b5 b4 b3 b2 b1 b0
// SPCR: SPIE SPE DORD MSTR CPOL CPHA SPR1 SPR0
// 0 1 0 1 . 0 0 0 1
//
// SPIE - enable SPI interrupt
// SPE - enable SPI
// DORD - 0=MSB first, 1=LSB first
// MSTR - 0=slave, 1=master
// CPOL - 0=clock starts low, 1=clock starts high
// CPHA - 0=read on rising-edge, 1=read on falling-edge
// SPRx - 00=osc/4, 01=osc/16, 10=osc/64, 11=osc/128
//
// SPCR = 0x50: SPI enabled as Master, mode 0, at 16/4 = 4 MHz
void OpenSPI()
{
 SPCR = 0x50; // SPI enabled as Master, Mode0 at 4 MHz
 SetBit(SPSR,SPI2X); // double the SPI rate: 4-->8 MHz
}
void CloseSPI()
{
 SPCR = 0x00; // clear SPI enable bit
}
byte Xfer(byte data)
{
 SPDR = data; // initiate transfer
 while (!(SPSR & 0x80)); // wait for transfer to complete
 return SPDR;
}
// ---------------------------------------------------------------------------
// ST7735 ROUTINES
//#define SWRESET 0x01 // software reset
//#define SLPOUT 0x11 // sleep out
//#define DISPOFF 0x28 // display off
//#define DISPON 0x29 // display on
//#define CASET 0x2A // column address set
//#define RASET 0x2B // row address set
//#define RAMWR 0x2C // RAM write
//#define MADCTL 0x36 // axis control
//#define COLMOD 0x3A // color mode

#define MADCTL_MY  0x80  ///< Bottom to top
#define MADCTL_MX  0x40  ///< Right to left
#define MADCTL_MV  0x20  ///< Reverse Mode
#define MADCTL_ML  0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00  ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08  ///< Blue-Green-Red pixel order
#define MADCTL_MH  0x04  ///< LCD refresh right to left


#define ILI9341_NOP        0x00     ///< No-op register
#define SWRESET    0x01     ///< Software reset register
#define ILI9341_RDDID      0x04     ///< Read display identification information
#define ILI9341_RDDST      0x09     ///< Read Display Status

#define ILI9341_SLPIN      0x10     ///< Enter Sleep Mode
#define SLPOUT     0x11     ///< Sleep Out
#define ILI9341_PTLON      0x12     ///< Partial Mode ON
#define ILI9341_NORON      0x13     ///< Normal Display Mode ON

#define ILI9341_RDMODE     0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL   0x0B     ///< Read Display MADCTL
#define ILI9341_RDPIXFMT   0x0C     ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT   0x0D     ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F     ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF     0x20     ///< Display Inversion OFF
#define ILI9341_INVON      0x21     ///< Display Inversion ON
#define ILI9341_GAMMASET   0x26     ///< Gamma Set
#define DISPOFF    0x28     ///< Display OFF
#define DISPON     0x29     ///< Display ON

#define CASET      0x2A     ///< Column Address Set
#define RASET      0x2B     ///< Page Address Set
#define RAMWR      0x2C     ///< Memory Write
#define ILI9341_RAMRD      0x2E     ///< Memory Read

#define ILI9341_PTLAR      0x30     ///< Partial Area
#define MADCTL     0x36     ///< Memory Access Control
#define ILI9341_VSCRSADD   0x37     ///< Vertical Scrolling Start Address
#define COLMOD     0x3A     ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1    0xB1     ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2    0xB2     ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3    0xB3     ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR     0xB4     ///< Display Inversion Control
#define ILI9341_DFUNCTR    0xB6     ///< Display Function Control

#define ILI9341_PWCTR1     0xC0     ///< Power Control 1
#define ILI9341_PWCTR2     0xC1     ///< Power Control 2
#define ILI9341_PWCTR3     0xC2     ///< Power Control 3
#define ILI9341_PWCTR4     0xC3     ///< Power Control 4
#define ILI9341_PWCTR5     0xC4     ///< Power Control 5
#define ILI9341_VMCTR1     0xC5     ///< VCOM Control 1
#define ILI9341_VMCTR2     0xC7     ///< VCOM Control 2

#define ILI9341_RDID1      0xDA     ///< Read ID 1
#define ILI9341_RDID2      0xDB     ///< Read ID 2
#define ILI9341_RDID3      0xDC     ///< Read ID 3
#define ILI9341_RDID4      0xDD     ///< Read ID 4

#define ILI9341_GMCTRP1    0xE0     ///< Positive Gamma Correction
#define ILI9341_GMCTRN1    0xE1     ///< Negative Gamma Correction
// 1.8" TFT display constants
#define XSIZE 240 //150 // 128 or 240
#define YSIZE 320 // 200 // 160 or 320
#define XMAX XSIZE-1
#define YMAX YSIZE-1
// Color constants

#define BLACK       0x0000  ///<   0,   0,   0
#define NAVY        0x000F  ///<   0,   0, 123
#define DARKGREEN   0x03E0  ///<   0, 125,   0
#define DARKCYAN    0x03EF  ///<   0, 125, 123
#define MAROON      0x7800  ///< 123,   0,   0
#define PURPLE      0x780F  ///< 123,   0, 123
#define OLIVE       0x7BE0  ///< 123, 125,   0
#define LIGHTGREY   0xC618  ///< 198, 195, 198
#define DARKGREY    0x7BEF  ///< 123, 125, 123
#define BLUE        0x001F  ///<   0,   0, 255
#define GREEN       0x07E0  ///<   0, 255,   0
#define CYAN        0x07FF  ///<   0, 255, 255
#define RED         0xF800  ///< 255,   0,   0
#define MAGENTA     0xF81F  ///< 255,   0, 255
#define YELLOW      0xFFE0  ///< 255, 255,   0
#define WHITE       0xFFFF  ///< 255, 255, 255
#define ORANGE      0xFD20  ///< 255, 165,   0
#define GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define PINK        0xFC18  ///< 255, 130, 198
void WriteCmd (byte cmd)
{
 ClearBit(PORTB,1); // B1=DC; 0=command, 1=data
 Xfer(cmd);
 SetBit(PORTB,1); // return DC high
}
void WriteByte (byte b)
{
 Xfer(b);
}
void WriteWord (int w)
{
 Xfer(w >> 8); // write upper 8 bits
 Xfer(w & 0xFF); // write lower 8 bits
}
void Write888 (long data, int count)
{
 byte red = data>>16; // red = upper 8 bits
 byte green = (data>>8) & 0xFF; // green = middle 8 bits
 byte blue = data & 0xFF; // blue = lower 8 bits
 for (;count>0;count--)
 {
 WriteByte(red);
 WriteByte(green);
 WriteByte(blue);
 }
}
void Write565 (int data, unsigned int count)
// send 16-bit pixel data to the controller
// note: inlined spi xfer for optimization
{
 WriteCmd(RAMWR);
 for (;count>0;count--)
 {
 SPDR = (data >> 8); // write hi byte
 while (!(SPSR & 0x80)); // wait for transfer to complete
 SPDR = (data & 0xFF); // write lo byte
 while (!(SPSR & 0x80)); // wait for transfer to complete
 }
}
void HardwareReset()
{
 ClearBit(PORTB,0); // pull PB0 (digital 8) low
 msDelay(1); // 1mS is enough
 SetBit(PORTB,0); // return PB0 high
 msDelay(150); // wait 150mS for reset to finish
}
void InitDisplay()
{
 HardwareReset(); // initialize display controller
 WriteCmd(SLPOUT); // take display out of sleep mode
 msDelay(150); // wait 150mS for TFT driver circuits
 WriteCmd(COLMOD); // select color mode:
 WriteByte(0x05); // mode 5 = 16bit pixels (RGB565)
 WriteCmd(DISPON); // turn display on!
}

void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
 WriteCmd(CASET); // set column range (x0,x1)
 WriteWord(x0);
 WriteWord(x1);
 WriteCmd(RASET); // set row range (y0,y1)
 WriteWord(y0);
 WriteWord(y1);
}
/*
void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
	uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
	WriteCmd(CASET); // Column addr set
	//SPI_WRITE32(xa);
	WriteCmd(RASET); // Row addr set
	//SPI_WRITE32(ya);
	WriteCmd(RAMWR); // write to RAM
}
*/
void ClearScreen()
{
 SetAddrWindow(0,0,XMAX,YMAX); // set window to entire display
 WriteCmd(RAMWR);
 
 for (unsigned long int i=153600;i>0;--i) // byte count = 128*160*2
 {
	 SPDR = 0; // initiate transfer of 0x00
	 while (!(SPSR & 0x80)); // wait for xfer to finish
 }
}
 
 /*
 for (unsigned int i=60000;i>0;--i) // byte count = 128*160*2=40960 or 240*320*2=153600
 {
 SPDR = 0; // initiate transfer of 0x00
 while (!(SPSR & 0x80)); // wait for xfer to finish
 }
 
}
*/
// ---------------------------------------------------------------------------
// SIMPLE GRAPHICS ROUTINES
//
// note: many routines have byte parameters, to save space,
// but these can easily be changed to int params for larger displays.
void DrawPixel (int x, int y, int color)
{
 SetAddrWindow(x,y,x,y);
 Write565(color,1);
}
void HLine (int x0, int x1, int y, int color)
// draws a horizontal line in given color
{
 int width = x1-x0+1;
 SetAddrWindow(x0,y,x1,y);
 Write565(color,width);
}
void VLine (int x, int y0, int y1, int color)
// draws a vertical line in given color
{
 int height = y1-y0+1;
 SetAddrWindow(x,y0,x,y1);
 Write565(color,height);
}
void Line (int x0, int y0, int x1, int y1, int color)
// an elegant implementation of the Bresenham algorithm
{
 int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
 int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
 int err = (dx>dy ? dx : -dy)/2, e2;
 for(;;)
 {
 DrawPixel(x0,y0,color);
 if (x0==x1 && y0==y1) break;
 e2 = err;
 if (e2 >-dx) { err -= dy; x0 += sx; }
 if (e2 < dy) { err += dx; y0 += sy; }
 }
}
void DrawRect (int x0, int y0, int x1, int y1, int color)
// draws a rectangle in given color
{
 HLine(x0,x1,y0,color);
 HLine(x0,x1,y1,color);
 VLine(x0,y0,y1,color);
 VLine(x1,y0,y1,color);
}
void FillRect (int x0, int y0, int x1, int y1, int color)
{
 int width = x1-x0+1;
 int height = y1-y0+1;
 SetAddrWindow(x0,y0,x1,y1);
 Write565(color,width*height);
}
void CircleQuadrant (int xPos, int yPos, int radius, int quad, int color)
// draws circle quadrant(s) centered at x,y with given radius & color
// quad is a bit-encoded representation of which cartesian quadrant(s) to draw.
// Remember that the y axis on our display is 'upside down':
// bit 0: draw quadrant I (lower right)
// bit 1: draw quadrant IV (upper right)
// bit 2: draw quadrant II (lower left)
// bit 3: draw quadrant III (upper left)
{
 int x, xEnd = (707*radius)/1000 + 1;
 for (x=0; x<xEnd; x++)
 {
 int y = intsqrt(radius*radius - x*x);
 if (quad & 0x01)
 {
 DrawPixel(xPos+x,yPos+y,color); // lower right
 DrawPixel(xPos+y,yPos+x,color);
 }
 if (quad & 0x02)
 {
 DrawPixel(xPos+x,yPos-y,color); // upper right
 DrawPixel(xPos+y,yPos-x,color);
 }
 if (quad & 0x04)
 {
 DrawPixel(xPos-x,yPos+y,color); // lower left
 DrawPixel(xPos-y,yPos+x,color);
 }
 if (quad & 0x08)
 {
 DrawPixel(xPos-x,yPos-y,color); // upper left
 DrawPixel(xPos-y,yPos-x,color);
 }
 }
}
void Circle (int xPos, int yPos, int radius, int color)
// draws circle at x,y with given radius & color
{
 CircleQuadrant(xPos,yPos,radius,0x0F,color); // do all 4 quadrants
}
void RoundRect (int x0, int y0, int x1, int y1, int r, int color) // draws a rounded rectangle with corner radius r. coordinates: top left = x0,y0; bottom right = x1,y1
{
 HLine(x0+r,x1-r,y0,color); // top side
 HLine(x0+r,x1-r,y1,color); // bottom side
 VLine(x0,y0+r,y1-r,color); // left side
 VLine(x1,y0+r,y1-r,color); // right side
 CircleQuadrant(x0+r,y0+r,r,8,color); // upper left corner
 CircleQuadrant(x1-r,y0+r,r,2,color); // upper right corner
 CircleQuadrant(x0+r,y1-r,r,4,color); // lower left corner
 CircleQuadrant(x1-r,y1-r,r,1,color); // lower right corner
}
void FillCircle (int xPos, int yPos, int radius, int color)
{
 long r2 = radius * radius;
 for (int x=0; x<=radius; x++)
 {
 byte y = intsqrt(r2-x*x);
 byte y0 = yPos-y;
 byte y1 = yPos+y;
 VLine(xPos+x,y0,y1,color);
 VLine(xPos-x,y0,y1,color);
 }
}
void Ellipse (int x0, int y0, int width, int height, int color)
{
 int a=width/2, b=height/2;
 int x = 0, y = b;
 long a2 = (long)a*a*2;
 long b2 = (long)b*b*2;
 long error = (long)a*a*b;
 long stopY=0, stopX = a2*b;
 while (stopY <= stopX)
 {
 DrawPixel(x0+x,y0+y,color);
 DrawPixel(x0+x,y0-y,color);
 DrawPixel(x0-x,y0+y,color);
 DrawPixel(x0-x,y0-y,color);
 x++;
 error -= b2*(x-1);
 stopY += b2;
 if (error < 0)
 {
 error += a2*(y-1);
 y--;
 stopX -= a2;
 }
 }
 x=a; y=0; error = b*b*a;
 stopY = a*b2; stopX = 0;
 while (stopY >= stopX)
 {
 DrawPixel(x0+x,y0+y,color);
 DrawPixel(x0+x,y0-y,color);
 DrawPixel(x0-x,y0+y,color);
 DrawPixel(x0-x,y0-y,color);
 y++;
 error -= a2*(y-1);
 stopX += a2;
 if (error < 0)
 {
 error += b2*(x-1);
 x--;
 stopY -= b2;
 }
 }
}
void FillEllipse(int xPos,int yPos,int width,int height, int color)
{
 int a=width/2, b=height/2; // get x & y radii
 int x1, x0 = a, y = 1, dx = 0;
 long a2 = a*a, b2 = b*b; // need longs: big numbers!
 long a2b2 = a2 * b2;
 HLine(xPos-a,xPos+a,yPos,color); // draw centerline
 while (y<=b) // draw horizontal lines...
 {
 for (x1= x0-(dx-1); x1>0; x1--)
 if (b2*x1*x1 + a2*y*y <= a2b2)
 break;
 dx = x0-x1;
 x0 = x1;
 HLine(xPos-x0,xPos+x0,yPos+y,color);
 HLine(xPos-x0,xPos+x0,yPos-y,color);
 y += 1;
 }
}
// ---------------------------------------------------------------------------
// TEXT ROUTINES
//
// Each ASCII character is 5x7, with one pixel space between characters
// So, character width = 6 pixels & character height = 8 pixels.
//
// In portrait mode:
// Display width = 128 pixels, so there are 21 chars/row (21x6 = 126). or 40x6=240
// Display height = 160 pixels, so there are 20 rows (20x8 = 160). or 40x8=320
// Total number of characters in portait mode = 21 x 20 = 420. or 40x40=1600
//
// In landscape mode:
// Display width is 160, so there are 26 chars/row (26x6 = 156). or 53x6=318
// Display height is 128, so there are 16 rows (16x8 = 128). or 30x8=240
// Total number of characters in landscape mode = 26x16 = 416. or 53x30=1590

int curX,curY; // current x & y cursor position
void GotoXY (int x,int y)
// position cursor on character x,y grid, where 0<x<20, 0<y<19.
{
 curX = x;
 curY = y;
}
void GotoLine(int y)
// position character cursor to start of line y, where 0<y<19.
{
 curX = 0;
 curY = y;
}
void AdvanceCursor()
// moves character cursor to next position, assuming portrait orientation
{
 curX--; // advance x position
 if (curX>39) // beyond right margin?
 {
 curY++; // go to next line
 curX = 0; // at left margin
 }
 if (curY>39) // beyond bottom margin?
 curY = 0; // start at top again
}
void SetOrientation(int degrees)
// Set the display orientation to 0,90,180,or 270 degrees
{
 byte arg;
 switch (degrees)
 {
 case 90: arg = 0x60; break;
 case 180: arg = 0xC0; break;
 case 270: arg = 0xA0; break;
 default: arg = 0x00; break;
 }
 WriteCmd(MADCTL);
 WriteByte(arg);
}
void PutCh (char ch, int x, int y, int color)
// write ch to display X,Y coordinates using ASCII 5x7 font
{
 int pixel;
 int row, col, bit, data, mask = 0x01;
 SetAddrWindow(x,y,x+4,y+6);
 WriteCmd(RAMWR);
 for (row=0; row<7;row++)
 {
 for (col=0; col<5;col++)
 {
 data = pgm_read_byte(&(FONT_CHARS[ch-32][col]));
 bit = data & mask;
 if (bit==0) pixel=BLACK;
 else pixel=color;
 WriteWord(pixel);
 }
 mask <<= 1;
 }
}
void WriteChar(char ch, int color)
// writes character to display at current cursor position.
{
 PutCh(ch,curX*6, curY*8, color);
 AdvanceCursor();
}
void WriteString(char *text, int color)
// writes string to display at current cursor position.
{
 for (;*text;text++) // for all non-nul chars
 WriteChar(*text,color); // write the char
}
void WriteInt(int i)
// writes integer i at current cursor position
{
 char str[8]; // buffer for string result
 itoa(i,str,10); // convert to string, base 10
 WriteString(str,WHITE);
}
void WriteHex(int i)
// writes hexadecimal value of integer i at current cursor position
{
 char str[8]; // buffer for string result
 itoa(i,str,16); // convert to base 16 (hex)
 WriteString(str,WHITE);
}
// ---------------------------------------------------------------------------
// TEST ROUTINES
void PixelTest()
// draws 4000 pixels on the screen
{
 for (int i=4000; i>0; i--) // do a whole bunch:
 {
 int x = rand() % XMAX; // random x coordinate
 int y = rand() % YMAX; // random y coordinate
 DrawPixel(x,y,YELLOW); // draw pixel at x,y
 }
}
void LineTest()
{
 ClearScreen();
 int x,y,x0=64,y0=80;
 for (x=0;x<XMAX;x+=2) Line(x0,y0,x,0,YELLOW);
 for (y=0;y<YMAX;y+=2) Line(x0,y0,XMAX,y,CYAN);
 for (x=XMAX;x>0;x-=2) Line(x0,y0,x,YMAX,YELLOW);
 for (y=YMAX;y>0;y-=2) Line(x0,y0,0,y,CYAN);
 msDelay(2000);
}
void CircleTest()
// draw series of concentric circles
{
 for(int radius=6;radius<60;radius+=2)
 Circle(60,80,radius,YELLOW);
}
void PortraitChars()
// Writes 420 characters (5x7) to screen in portrait mode
{
 ClearScreen();
 for (int i=1600;i>0;i--)
 {
 int x= i % 40;
 int y= i / 40;
 char ascii = (i % 96)+32;
 PutCh(ascii,x*6,y*8,CYAN);
 }
 msDelay(2000);
}
// ---------------------------------------------------------------------------
// MAIN PROGRAM
//int main()
//{
 //SetupPorts(); // use PortB for LCD interface
 //FlashLED(1); // indicate program start
 //OpenSPI(); // start communication to TFT
 //InitDisplay(); // initialize TFT controller
 //ClearScreen();
 //SetOrientation(270);
 //PortraitChars(); // show full screen of ASCII chars
 //LineTest(); // paint background of lines
 //FillEllipse(60,75,100,50,BLACK); // erase an oval in center
 //Ellipse(60,75,100,50,LIME); // outline the oval in green
 //char *str = "!dlroW ,olleH"; // text to display
 //GotoXY(22,22); // position text cursor
 //WriteString(str,MAGENTA); // display text inside oval
 //CloseSPI(); // close communication with TFT
 //FlashLED(3); // indicate program end
//}