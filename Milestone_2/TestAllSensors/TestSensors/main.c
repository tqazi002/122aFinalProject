
#include <avr/io.h>
#include "SPI_LCD.c"

// ADC init
void ADC_init() {
	ADMUX = (1<<REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

// Read ADC from different ports
uint16_t ReadADC(uint8_t data)
{
	data &= 0x07;
	ADMUX = (ADMUX & 0xF8) | data;

	ADCSRA |= (1 << ADSC);

	while(ADCSRA & (1 << ADSC));

	ADCSRA |= (1<<ADIF);
	return (ADC);
}

// GLOBAL VARIABLES
int global_earthquake = 0;
unsigned short readAD = 0x0000;
unsigned short readADC1 = 0x0000;
unsigned short MAX = 0x096;
int count_secs = 0;
int x = 1;
int y = 1;
int press = 0;

// Temperature
enum States { Init, Read} State;
void Tick() {
	switch (State) {
		case Init:
		State = Read;
		break;
		
		case Read:
		readAD = ReadADC(4) / 136.4;/// 2.2;
		//_delay_ms(20);
		State = Read;
		break;
	}
}

// Humidity
enum States4 { Init4, Read4} State4;
	unsigned short readAD4 = 0x0000;
void Tick4() {
	switch (State4) {
		case Init4:
		State4 = Read4;
		break;
		
		case Read4:
		readAD4 = ReadADC(6) / 753.5;//* 0.086;
		State4 = Read4;
		break;
	}
}

//Light
enum States1 { Init1, Read1 } State1;
void Tick1() {
	switch (State1) {
		case Init1:
		State1 = Read1;
		break;
		
		case Read1:
		readADC1 = ReadADC(5) / 61.2;
		State1 = Read1;
		break;
	}
}

// Display and Touch
enum States2 {Normal, Emergency, Em2} State2;
void Tick2() {
	switch (State2) {
		case Normal:
		if(global_earthquake == 0){
			//Temperature
			GotoXY(40,5);
			WriteInt(readAD);
			char *str = "F"; 
			GotoXY(38,5); // position text cursor
			WriteString(str,GREEN); // display text
			
			if(readAD > 83){
				char *str1 = "Are you staying hydrated?    "; 
				GotoXY(36,5); // position text cursor
				WriteString(str1,BLUE); // display text
			}
			else if((readAD <= 83) && (readAD >= 72)){
				char *str2 = "It's nice outside            "; // text to display
				GotoXY(36,5); // position text cursor
				WriteString(str2,GREEN); // display text
			}
			else {
				char *str3 = "Did you wear something warm? "; // text to display
				GotoXY(36,5); // position text cursor
				WriteString(str3,BLUE); // display text
			}
			
			//Light
			if (readADC1 >= (MAX / 2)) {
				char *str4 = "Did you wear your sunscreen?"; // text to display
				GotoXY(40,13); // position text cursor
				WriteString(str4,BLUE); // display text
			}
			else {
				char *str5 = "It's cloudy/dark outside    "; // text to display
				GotoXY(40,13); // position text cursor
				WriteString(str5,GREEN); // display text
			}
			
			// Humidity
			GotoXY(40,21);
			WriteInt(readAD4);
			char *str10 = "%";
			GotoXY(38,21); // position text cursor
			WriteString(str10,GREEN); // display text
			if(readAD4 > 80){
				char *str11 = "Did you take your umbrella?";
				GotoXY(36,21); // position text cursor
				WriteString(str11,BLUE); // display text
			}
			else{
				char *str12 = "No need for umbrella       "; // text to display
				GotoXY(36,21); // position text cursor
				WriteString(str12,GREEN); // display text
			}
			
			// Touch
			//FillCircle (x, y, 10, MAGENTA);
			
			State2 = Normal;
		}
		else{
			/*
			char *str8 = " "; // text to display
			GotoXY(40,4); // position text cursor
			WriteString(str8,BLACK); // display text
			char *str9 = " "; // text to display
			GotoXY(40,10); // position text cursor
			WriteString(str9,BLACK); // display text
			*/
			ClearScreen();
			State2 = Emergency;
		}
		break;
		
		case Emergency:
		count_secs = count_secs + 1;
		if(count_secs < 10){
			char *str6 = "EVACUATE BUILDING!"; // text to display
			GotoXY(36,12); // position text cursor
			WriteString(str6,BLUE); // display text
			PORTC = 0xFF;
			_delay_ms(1000);
			State2 = Em2;
		}
		else{
			global_earthquake = 0;
			count_secs = 0;
			State2 = Normal;
		}
		break;
		
		case Em2:
		if(count_secs < 10){
			char *str7 = "                  "; // text to display
			GotoXY(36,12); // position text cursor
			WriteString(str7,BLUE); // display text
			PORTC = 0x00;
			_delay_ms(1000);
			State2 = Emergency;
		}
		else{
			count_secs = 0;
			global_earthquake = 0;
			State2 = Normal;
		}
		break;
		
		default:
		State2 = Normal;
		break;
	}
}


enum States3 { Init3 } State3;
unsigned char vibration = 0x00; 
void Tick3() {
	vibration = ~PIND & 0x01;
	switch (State3) {
		case Init3:
		if (vibration) {
			global_earthquake = 1;
		}
		State3 = Init3;
		break;
		
	}
}

unsigned int readTouchX(void) {

	unsigned int x_val = 0;

	DDRA &= ~((1<<PINA1) | (1<<PINA2)); //Sets X+ and X- to inputs

	PORTA &= ~((1<<PINA1) | (1<<PINA2)); //Sets X+,X- to ground
	

	DDRA |= (1<<PINA0) | (1<<PINA3); //Sets Y+ and Y- as outputs

	PORTA &= ~(1<<PINA3); //Set Y- to ground

	PORTA |= (1<<PINA0); //Sets Y+ high



	ADCSRA |= (1<<ADEN)|(1<<ADSC) + 7;

	ADMUX = (1<<ADLAR)|((1<<REFS0) + 1); //Read X-


	//WAIT FOR DONE BIT

	_delay_us(200);

	x_val = ADCL; //Read high first
	x_val = x_val + (ADCH<<8); //Then read low
	
	if(x_val!=0)
	{
		//convert x_val to 0-480 range
		x_val=(x_val-8000)/150; // 4000/117
	}
	

	return x_val; //Returns raw X position
}


unsigned int readTouchY(void) {

	unsigned int y_val=0;

	DDRA &= ~((1<<PINA0) | (1<<PINA3)); //Set Y+ and Y- to inputs

	PORTA &= ~((1<<PINA0) | (1<<PINA3)); //Set Y+,Y-to ground
	

	DDRA |= ((1<<PINA1) | (1<<PINA2)); //Set X+ and X- to outputs

	PORTA &= ~(1<<PINA1); // Set X- to Ground

	PORTA |= (1<<PINA2);	//Set X+ high
	

	ADCSRA = (1<<ADEN) | (1<<ADSC) + 7;

	ADMUX = (1<<ADLAR) | (1<<REFS0);  //Read Y+

	
	//enable ADC and set prescaler to 1/128*16MHz=125,000
	//and clear interupt enable
	//and start a conversion
	// while(ADSC != 0){};

	_delay_us(200); //wait on done bit

	y_val = ADCL;	//Read high first
	y_val = y_val + (ADCH<<8); //Then read low
	
	if(y_val!=0)
	{
		//convert y_val to 0-320 range
		y_val=(y_val-8000)/180;

	}
	

	return y_val; //Return raw Y position
}


uint16_t pressureX(void) {
	// Set X+ output to ground
	DDRA |= (1<<PINA2);
	PORTA &= ~(1<<PINA2);
	
	// Set Y- output to VCC
	DDRA |= (1<<PINA3);
	PORTA |= (1<<PINA3);
	
	// Set X- and Y+ as inputs and to ground
	DDRA &= ~((1<<PINA0) | (1<<PINA1));
	PORTA &= ~((1<<PINA0) | (1<<PINA1));
	
	ADMUX = (1<<REFS0) + 1; //Read the value of X-
	ADCSRA |= (1<<ADSC);
	_delay_us(200);
	int z1 = ADCL;	//Read the high
	z1 = z1 + (ADCH<<8); //Read the low
	ADMUX = (1<<REFS0); //Read the value of Y+
	ADCSRA |= (1<<ADSC);
	_delay_us(200);
	int z2 = ADCL; //Read the high
	z2 = z2 + (ADCH<<8); //Read the low

	// now read the x
	float rtouch;
	rtouch = z2;
	rtouch /= z1;
	rtouch -= 1;
	//rtouch *= readTouchY();
	rtouch *= readTouchX();
	rtouch *= 330;
	rtouch /= 1024;
	
	return rtouch; //Return the pressure value
}

uint16_t pressureY(void) {
	// Set X+ output to ground
	DDRA |= (1<<PINA2);
	PORTA &= ~(1<<PINA2);
	
	// Set Y- output to VCC
	DDRA |= (1<<PINA3);
	PORTA |= (1<<PINA3);
	
	// Set X- and Y+ as inputs and to ground
	DDRA &= ~((1<<PINA0) | (1<<PINA1));
	PORTA &= ~((1<<PINA0) | (1<<PINA1));
	
	ADMUX = (1<<REFS0) + 1; //Read the value of X-
	ADCSRA |= (1<<ADSC);
	_delay_us(200);
	int z1 = ADCL;	//Read the high
	z1 = z1 + (ADCH<<8); //Read the low
	ADMUX = (1<<REFS0); //Read the value of Y+
	ADCSRA |= (1<<ADSC);
	_delay_us(200);
	int z2 = ADCL; //Read the high
	z2 = z2 + (ADCH<<8); //Read the low

	// now read the x
	float rtouch;
	rtouch = z2;
	rtouch /= z1;
	rtouch -= 1;
	//rtouch *= readTouchY();
	rtouch *= readTouchY();
	rtouch *= 330;
	rtouch /= 1024;
	
	return rtouch; //Return the pressure value
}

enum States5 { Init5 } State5;

void Tick5() {
	switch (State5) {
		case Init5:
		press = pressureX();
		if((press < 10) || (press > 1000)){
			
		}
		else{
			press = pressureX();
			x = readTouchX(); 
			press = pressureY();
			y = readTouchY(); 
			
		}
		State5 = Init5;
		break;
	}
}

int main(void)
{
	SetupPorts(); // use PortB for LCD interface
	OpenSPI(); // start communication to TFT
	InitDisplay(); // initialize TFT controller
	ClearScreen();
	SetOrientation(270);
	
	//char *str1 = "HALLOOOOO"; // text to display
	//GotoXY(36,12); // position text cursor
	//WriteString(str1,BLUE); // display text
	
	DDRD = 0x00; PORTD = 0xFF; // Configure PORTD as input, initialize to 1s
	DDRC = 0xFF; PORTC = 0x00; // Configure PORTC as outputs, initialize to 0s
	
	DDRA = 0x00;
	PORTA = 0xFF;
	
	State = Init;
	State1=Init1;
	
	ADC_init();
    while (1) 
    {
		Tick();
		Tick1();
		Tick2();
		Tick3();
		Tick4();
		Tick5();
		/*
		press = pressureX();
		 if((press < 10) || (press > 1000)){
		 
		 }
		 else{
			 press = pressureX();
			 x = readTouchX(); // ReadADC(2);
			 //_delay_ms(200);
			 press = pressureY(); 
			 y = readTouchY(); // ReadADC(0);
		 
			 FillCircle (x, y, 10, MAGENTA);
		 
		 }
		 */
		 //_delay_ms(200);
    }
}

