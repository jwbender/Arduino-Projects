
//Display
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=blue*/ 13, /* data=black*/ 11, /* cs=yellow*/ 10, /* dc=green*/ 9, /* reset=red*/ 8);  

//Scale
#include "HX711.h"
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
float weightloss;
float initialweight;
float finalweight;
int scalemode = 0;
HX711 scale;

//Thermocouple
#include <max6675.h>
int thermoDO = 44;
int thermoCS = 46;
int thermoCLK = 48;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
float etemp=0;
char etemptext[5];
float btemp =0;
char btemptext[5];
float cracktemp;
char cracktemptext[5];

//buttons
#include <PinButton.h>
PinButton tareButton(5);  //pin where one leg of button is wire to, other leg to ground
PinButton weightButton(6);
PinButton timeButton(7);

//timer
#include <TimeLib.h>
unsigned long starttime;
unsigned long roasttime=0;
char roasttimetext[5];
unsigned long cracktime;
long cracktime2;
char cracktimetext [5];
unsigned long finaltime;
int timemode=0;

//Bluetooth
struct Packet {
  float c;
} pkt; // Instantiate a Packet struct

void setup() {
  // put your setup code here, to run once:
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  u8g2.begin();
  Serial.begin(9600);
  Serial2.begin(9600); // open the bluetooth serial port
}
void loop() {
      u8g2.clearBuffer();					// clear the internal memory
tareButton.update();  
weightButton.update();
timeButton.update();
  if (tareButton.isSingleClick() || tareButton.isLongClick()) {
    scale.tare();
  }
  if (weightButton.isSingleClick()|| weightButton.isLongClick()) {
    if(scalemode==0 ){//initial weight
      initialweight = scale.get_units(10);
      scalemode=1;
    }
    else {//final weight
      finalweight = scale.get_units(10);
      weightloss= (initialweight-finalweight); 
      weightloss=weightloss/initialweight;   
      weightloss=weightloss *100;
      scalemode=0;
    }
  }
  if (timeButton.isSingleClick()|| timeButton.isLongClick()) {
    if(timemode==0||timemode==3){//start
      starttime = now();
      Serial.println("srt time");
      roasttime=0;
      cracktime=0;
      dtostrf(cracktime,5,0,cracktimetext);
      timemode=1;
    }
    else if(timemode==1){//crack
      cracktime = now()-starttime; 
      Serial.println("crack");
      float cracktemp = thermocouple.readFahrenheit();  
      dtostrf(cracktemp,4,0,cracktemptext);
      cracktime2=cracktime;
      dtostrf(cracktime,5,0,cracktimetext);
      cracktime=cracktime2;
      timemode=2;
    }
    else{//final
      finaltime = now()-starttime;
      roasttime=finaltime;
      Serial.println("finished");
      timemode=3;

    }
  }

  float etemp = thermocouple.readFahrenheit(); 
  dtostrf(etemp,4,0,etemptext);
//Receive btemp
  bluetooth_receive();

    if(timemode==1 || timemode==2){
      roasttime=now()-starttime;
    }
    else{};
   dtostrf(roasttime,5,0,roasttimetext);
   cracktime2=cracktime;
   dtostrf(cracktime,5,0,cracktimetext);
   cracktime=cracktime2;
//stage text
  u8g2.setFont(u8g2_font_courR08_tf);
  if(timemode==0) u8g2.drawStr(35,7,"START");	
  else if(timemode==1) u8g2.drawStr(35,7,"PRE CRACK");
  else if(timemode==2) u8g2.drawStr(35,7,"POST CRACK");	
  else u8g2.drawStr(35,7,"FINISHED");	

  u8g2.setFont(u8g2_font_unifont_tr );	
  u8g2.drawStr(0,18,"B Temp:    f");	
  u8g2.drawStr(56,18,"999");	  
  u8g2.drawStr(0,30,"E Temp:    f");
  u8g2.drawStr(56,30,"999");	
  u8g2.setFont(u8g2_font_courR08_tf);
  u8g2.drawStr(0,38,"Roast Time:   s");	
  u8g2.drawStr(52,38,roasttimetext);	
  u8g2.drawStr(0,46,"Crack Time:   s");	
  u8g2.drawStr(52,46,cracktimetext);	 
  u8g2.drawStr(0,55,"W1:      g");	
  u8g2.drawStr(47,55,"   W2:      g");		
  u8g2.drawStr(0,64,"Weight Loss:");	
  u8g2.drawStr(90,64,"%");	
  u8g2.sendBuffer();
}

void bluetooth_receive() {
  // Check the software serial buffer for data to read
  if(Serial2.available() >= sizeof(Packet)) {
    // Read in the appropriate number of bytes to fit our Packet
    Serial2.readBytes((byte *) & pkt,sizeof(Packet));
  }   
    dtostrf(pkt.c,4,0,btemptext);
}
