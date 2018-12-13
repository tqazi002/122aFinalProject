/*
 * Humid2.c
 *
 * Created: 11/27/2018 11:30:51 PM
 * Author : Tasmiyah
 */ 

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include "SPI_LCD.c"

#define F_CPU 16000000UL
#define F_SCL 9600UL // SCL frequency
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

void i2c_init(void)
{
	TWBR = (uint8_t)TWBR_val;
}

uint8_t i2c_start(uint8_t address)
{
	
	// reset TWI control register
	TWCR = 0;
	// transmit START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// check if the start condition was successfully transmitted
	if((TWSR & 0xF8) != TW_START){ return 1; }
	// load slave address into data register
	TWDR = address;
	// start transmission of address
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW_STATUS & 0xF8;
	if ( twst != TW_MT_SLA_ACK ) return 1;
	return 0;
}

uint8_t i2c_write(uint8_t data)
{
	// load data into data register
	TWDR = data;
	// start transmission of data
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	if( (TWSR & 0xF8) != TW_MT_DATA_ACK ){ return 1; }
	return 0;
}

uint8_t i2c_read_ack(void)
{
	
	// start TWI module and acknowledge data after reception
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	//_delay_ms(2000);
	// return received data from TWDR
	return TWDR;
}

uint8_t i2c_read_nack(void)
{
	
	// start receiving without acknowledging reception
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	//_delay_ms(2000);
	// return received data from TWDR
	return TWDR;
}
/*
uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length)
{
	if (i2c_start(address | 0)) return 1;
	
	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) return 1;
	}
	
	i2c_stop();
	
	return 0;
}

uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length)
{
	if (i2c_start(address | 1)) return 1;
	
	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();
	
	i2c_stop();
	
	return 0;
}

uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	if (i2c_start(devaddr | 0x00)) return 1;

	i2c_write(regaddr);

	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) return 1;
	}

	i2c_stop();

	return 0;
}

uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	if (i2c_start(devaddr)) return 1;

	i2c_write(regaddr);

	if (i2c_start(devaddr | 0x01)) return 1;

	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();

	i2c_stop();

	return 0;
}
*/
void i2c_stop(void)
{
	// transmit STOP condition
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

int main(void)
{
	SetupPorts(); // use PortB for LCD interface
	OpenSPI(); // start communication to TFT
	InitDisplay(); // initialize TFT controller
	ClearScreen();
	SetOrientation(270);
	
	char *str = "This is Humidity"; // text to display
	GotoXY(20,9); // position text cursor
	WriteString(str,MAGENTA); // display text
	
	
	DDRC = 0x00;
	PORTC = 0xFF;
	i2c_init();
	int data = 0;
	
    
    while (1) 
    {
		i2c_start(0x5C);
		i2c_write(0x00);
		i2c_stop();
		_delay_ms(10);
		
		 i2c_start(0x5C);
		 i2c_write(0x03);
		 i2c_write(0x00);
		 i2c_stop();
		 _delay_ms(10);
		 
		 i2c_start(0x5C+1);
		 data = i2c_read_ack();
		 //GotoXY(10,12);
		 //WriteInt(8);
		 data = data + i2c_read_nack();
		 i2c_stop();
		 
		 GotoXY(10,12);
		 WriteInt(data);
		 _delay_ms(2000);
    }
	
}

