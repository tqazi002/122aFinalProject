#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

Adafruit_AM2320 am2320 = Adafruit_AM2320();

void setup() {
  Serial.begin(9600);
  pinMode(0,INPUT);//RXD pin is set for INPUT
  pinMode(1,OUTPUT);
  while (!Serial) {
    delay(10); // hang out until serial port opens
  }

  Serial.println("Adafruit AM2320 Basic Test");
  am2320.begin();
}

byte rx_byte = 0;        // stores received byte

void loop() {
  //Serial.print("Temp: "); Serial.println(am2320.readTemperature());
  //Serial.print("Hum: "); 
  //Serial.println(am2320.readHumidity());
  Serial.write((byte)am2320.readHumidity());

  //delay(2000);
}
