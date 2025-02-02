#include <max6675.h>
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
float btemp=0;

//Bluetooth
#include <SoftwareSerial.h>
SoftwareSerial mySerial(3,2);
// Struct to hold the data we want to transmit
struct Packet {
  float c;
} pkt; // Instantiate a Packet struct

void setup() {
  // put your setup code here, to run once:
 mySerial.begin(9600); // open the bluetooth serial port

}

void loop() {
  // put your main code here, to run repeatedly:
  float btemp = thermocouple.readFahrenheit(); 
  pkt.c = btemp;
  mySerial.write((byte *) & pkt,sizeof(Packet));
  delay(500);
}
