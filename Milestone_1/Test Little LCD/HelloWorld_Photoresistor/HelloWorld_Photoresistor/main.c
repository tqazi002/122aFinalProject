
#include "timer.h"
#include "scheduler.h"
#include "SPI_LCD.c"


void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

enum States { Init, Read, Display } State;
unsigned char output = 0x00;
unsigned short readADC = 0x0000;
//unsigned short MAX = 0x0F7;
unsigned short MAX = 0x096;
unsigned char photoresistance = 000;

void Tick() {
	switch (State) {
		case Init:
		output = 0x00;
		State = Read;
		break;
		case Read:
		readADC = ADC;
		photoresistance = readADC;
		if (readADC >= (MAX / 2)) {
			State = Display;
		}
		else {
			State = Read;
			PORTC = 0x00;
			
			char *str = "It's dark outside   "; // text to display
			GotoXY(1,9); // position text cursor
			WriteString(str,BLACK); // display text 
			GotoXY(1,12);
			WriteInt(photoresistance);
		}
		break;
		case Display:
		PORTC = 0xFF;
		
		char *str = "Wear your sunscreen!"; // text to display
		GotoXY(1,9); // position text cursor
		WriteString(str,BLACK); // display text 
		if(photoresistance < 100){
			GotoXY(1,12);
		    WriteInt(photoresistance);
			char *str2 = "0";
			GotoXY(3,12);
			WriteString(str2, WHITE);}
		else{
			GotoXY(1,12);
			WriteInt(photoresistance);}
			
		State = Read;
		break;
	}
}

// ---------------------------------------------------------------------------
// MAIN PROGRAM
int main()
{
	 SetupPorts(); // use PortB for LCD interface
	 //FlashLED(1); // indicate program start
	 OpenSPI(); // start communication to TFT
	 InitDisplay(); // initialize TFT controller
	 //PortraitChars(); // show full screen of ASCII chars
	 //LineTest(); // paint background of lines
	 //FillEllipse(60,75,100,50,BLACK); // erase an oval in center
	 //Ellipse(60,75,100,50,LIME); // outline the oval in green
	 //SetOrientation(90);
	 ClearScreen();
	 FillRect(0, 0, XMAX, YMAX, WHITE);
	 SetOrientation(90);
	 //char *str = "Hello, World!"; // text to display
	 //GotoXY(4,9); // position text cursor
	 //WriteString(str,BLACK); // display text inside oval
	 //CloseSPI(); // close communication with TFT
	 //FlashLED(3); // indicate program end
	 
	 FillRect(1, 1, 20, 20, YELLOW);
	 char *str3 = "A";
	 GotoXY(1, 1);
	 WriteString(str3, BLACK);
	 
	DDRA = 0x00;
	PORTA = 0xFF;
	
	DDRC = 0xFF;
	PORTC = 0x00;
	
	State = Init;
 
 ADC_init();
 while(1)
 {
	 Tick();
 }
}