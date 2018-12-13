/*
 * HumidSensor.c
 *
 * Created: 11/23/2018 9:10:41 PM
 * Author : Tasmiyah
 */ 

/*
#include <avr/io.h>
#include <util/twi.h>	
#include <util/delay.h>
#include "SPI_LCD.c"

#define F_CPU 8000000UL


int main(void)
{
	//DDRA = 0xFF;
	//PORTA = 0x00;
	
	DDRC = 0xFF;
	PORTC = 0x00;
    //PORTA = 1;
    //while (1) {}
	
		
	TWBR = ((F_CPU/100000) - 16) / 2;
	 //PORTA = 1;

	// set the start condition
	TWCR = ((1 << TWEN) | (1 << TWINT) | (1 << TWSTA));
	while ( !(TWCR & (1 << TWINT)) );
	 //PORTA = 1;

	// send the address
	TWDR = 0x5C + 0;            // SLA+W, address + write bit
	TWCR = ((1 << TWEN) | (1 << TWINT)); // trigger I2C action
	while ( !(TWCR & (1 << TWINT)) );
	 //PORTA = 1;

	// specify the register
	TWDR = 0x00;                        // register value in the data register
	TWCR = ((1 << TWEN) | (1 << TWINT)); // trigger I2C action
	while ( !(TWCR & (1 << TWINT)) );
	// PORTA = 1;

	// switch master to read (receiver) mode and slave to transmitter
	TWCR = ((1 << TWEN) | (1 << TWINT) | (1 << TWSTA)); // set another start condition
	while ( !(TWCR & (1 << TWINT)) );
	TWDR = 0x5C + 1;                             // SLA+R, switch to reading
	TWCR = ((1 << TWEN) | (1 << TWINT));                 // trigger I2C action
	while ( !(TWCR & (1 << TWINT)) );
	// PORTA = 1;

	uint16_t register_value = 0;
	// perform first read to get the MSB
	 //PORTA = 1;
	 
	TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWEA)); // with ACK set
	 //PORTA = 1;
	while ( !(TWCR & (1 << TWINT)) );
	 PORTA = 1;
	
	// the received byte is now in the TWDR data register
	register_value = (TWDR << 8);                     // put value in top half of variable
	// second read to get LSB
	TWCR = ((1 << TWINT) | (1 << TWEN));              // no acknowledge bit set, NOT ACK
	 //PORTA = 1;
	while ( !(TWCR & (1 << TWINT)) );
	 //PORTA = 1;
	// the second byte is now in TWDR
	register_value += TWDR;
	*/
	
	
	/*
	SetupPorts(); // use PortB for LCD interface
	OpenSPI(); // start communication to TFT
	InitDisplay(); // initialize TFT controller
	ClearScreen();
	FillRect(0, 0, XMAX, YMAX, WHITE);
	SetOrientation(90);
	
	char *str = "This is Humidity"; // text to display
	GotoXY(1,9); // position text cursor
	WriteString(str,BLACK); // display text
	GotoXY(1,12);
	WriteInt(register_value);
	*/
	
	// set stop condition
	//TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTO));
//}

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include "SPI_LCD.c"

#define F_CPU 16000000L // CPU clock speed 16 MHz
#define F_SCL 9600L // I2C clock speed 100 kHz#define TW_START 0xA4 // send start condition (TWINT,TWSTA,TWEN)
#define TW_READY (TWCR & 0x80) // ready when TWINT returns to logic 1.
#define TW_STATUS (TWSR & 0xF8) // returns value of status register#define DS1307 0x5C // I2C bus address of DS1307 RTC
#define TW_SEND 0x84 // send data (TWINT,TWEN)#define TW_STOP 0x94 // send stop condition (TWINT,TWSTO,TWEN)
#define I2C_Stop() TWCR = TW_STOP // inline macro for stop condition#define TW_NACK 0x84 // read data with NACK (last byte)
#define READ 1#define TW_ACK 0xC4void I2C_Init()
// at 16 MHz, the SCL frequency will be 16/(16+2(TWBR)), assuming prescalar of 0.
// so for 100KHz SCL, TWBR = ((F_CPU/F_SCL)-16)/2 = ((16/0.1)-16)/2 = 144/2 = 72.
{
	TWSR = 0; // set prescalar to zero
	TWBR = ((F_CPU/F_SCL)-16)/2; // set SCL frequency in TWI bit register
}byte I2C_Start()
// generate a TW start condition
{
	TWCR = TW_START; // send start condition
	while (!TW_READY); // wait
	return (TW_STATUS==0x08); // return 1 if found; 0 otherwise
}byte I2C_SendAddr(addr)
// send bus address of slave
{
	TWDR = addr; // load device's bus address
	TWCR = TW_SEND; // and send it
	while (!TW_READY); // wait
	return (TW_STATUS==0x18); // return 1 if found; 0 otherwise
}

byte I2C_Write (byte data) // sends a data byte to slave
{
	TWDR = data; // load data to be sent
	TWCR = TW_SEND; // and send it
	while (!TW_READY); // wait
	return (TW_STATUS!=0x28); // return 1 if found; 0 otherwise
}

byte I2C_ReadNACK () // reads a data byte from slave
{
	TWCR = TW_NACK; // nack = not reading more data
	while (!TW_READY); // wait
	return TWDR;
}byte I2C_ReadACK () // reads a data byte from slave
{
	TWCR = TW_ACK; // ack = will read more data
	while (!TW_READY); // wait
	return TWDR;
	//return (TW_STATUS!=0x28);
}int main(void){	SetupPorts(); // use PortB for LCD interface
	OpenSPI(); // start communication to TFT
	InitDisplay(); // initialize TFT controller
	ClearScreen();
	SetOrientation(270);
	/*
	char *str = "This is Humidity"; // text to display
	GotoXY(1,9); // position text cursor
	WriteString(str,MAGENTA); // display text
	GotoXY(1,12);
	WriteInt(register_value);	*/	DDRC = 0x00;	int data = 0;		I2C_Init();	I2C_Start();	I2C_SendAddr(0x5C);	//I2C_Write (0x03);	I2C_Write (0x00);	I2C_Stop(); 	_delay_ms(10);		I2C_Start();	I2C_SendAddr(0x5C);	I2C_Write (0x03);	I2C_Write (0x00);	//I2C_Write (2);
	I2C_Stop();	_delay_ms(2);		I2C_Start();	I2C_SendAddr(0x5C+READ);	GotoXY(10,12);
	WriteInt(8);	data = I2C_ReadACK();
	data = data + I2C_ReadNACK();	I2C_Stop();		GotoXY(10,12);
	WriteInt(data);		_delay_ms(4000);}