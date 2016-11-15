


#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <DMD.h>        //
#include <TimerOne.h>   //
#include <EEPROM.h>
#include "SystemFont5x7.h"
#include "Arial_black_16.h"
#include "Arial14.h"
#include "Droid_Sans_24.h"
#include "Droid_Sans_16.h"
#include "My32x15Font.h"
#include <TrueRandom.h>


///define display
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 2
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
////

void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}


byte mac[6] = { 0x90, 0xA2, 0xDA, 0x00, 0x00, 0x00 };
char macstr[18];
//IPAddress ip(192, 168, 0, 177);

unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[128];
char  ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

String DisplayAddress(IPAddress address)
{
 return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);
}

void setup() {
   //buffer to hold incoming packet,
    ///
    if (EEPROM.read(1) == '#') {
    for (int i = 3; i < 6; i++) {
      mac[i] = EEPROM.read(i);
    }
  } else {
    for (int i = 3; i < 6; i++) {
      mac[i] = TrueRandom.randomByte();
      EEPROM.write(i, mac[i]);
    }
    EEPROM.write(1, '#');
  }
  snprintf(macstr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Ethernet.begin(mac);
  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)

  // start the Ethernet and UDP:
  
  Udp.begin(localPort);
  Serial.begin(9600);
  Serial.println(Ethernet.localIP());
  dmd.selectFont(SystemFont5x7);
  String message=DisplayAddress(Ethernet.localIP());
  Serial.println(message);
  char  showIP[15];
  message.toCharArray(showIP, 15) ;
  dmd.drawString( 0,2, showIP, 15, GRAPHICS_NORMAL );
  Serial.println(showIP);
  delay(10000);
}

void loop() {
   
    ////
  dmd.clearScreen( true );
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }

    // read the packet into packetBufffer
    Udp.read(packetBuffer, 128);
    Serial.println("Contents:");
    Serial.println(packetBuffer);
    ///reply
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
    ///
    show_to_plc(packetBuffer);
    memset(packetBuffer, '\0', 128);
    // send a reply to the IP address and port that sent us the packet we received
    
  }
  delay(10);
}

void show_to_plc(char* text){

  String str(text);

  delay(10);
  Serial.println("str");
  int index=str.indexOf(';',0);
  String message=str.substring(0,index);
  str=str.substring(message.length()+1,str.length());
  int messageLength=message.length();
  ////
  index=str.indexOf(';',0);
  String font = str.substring(0,index);
  str=str.substring(font.length()+1,str.length());
  ///
  index=str.indexOf(';',0);
  String effect = str.substring(0,index);
  str=str.substring(effect.length()+1,str.length());
  ////
  index=str.indexOf(';',0);
  String x = str.substring(0,index);
  str=str.substring(x.length()+1,str.length());
  ///
  index=str.indexOf(';',0);
  String y = str.substring(0,index);
  str=str.substring(y.length()+1,str.length());
  ///
  ///
  index=str.indexOf(';',0);
  String showtime = str.substring(0,index);
  str=str.substring(showtime.length()+1,str.length());
  ///

  ///
  delay(10);
  dmd.clearScreen( true );
  //dmd.selectFont(Verdana24);
  set_font(font);
  char  showMessage[128];
  message.toCharArray(showMessage, 128) ;
  int delayy= showtime.toInt() * 1000;
  ///constant text
  if (effect.toInt() == 0){
     dmd.drawString( x.toInt(),y.toInt(), showMessage, messageLength, GRAPHICS_NORMAL );
     Serial.println(delayy);
     delay(delayy);
     
  }
  ////scrolling text
  else
  {
   Serial.println(showMessage);
   for(int i=0;i < showtime.toInt();i++){
   dmd.drawMarquee(showMessage,messageLength,(32*DISPLAYS_ACROSS)-1,y.toInt());
   long start=millis();
   long timer=start;
   boolean ret=false;
   while(!ret){
     if ( (timer + 70) < millis()) {
       ret=dmd.stepMarquee(-1,0);
       timer=millis();
     }
    }
   }
 }
 ////end scrolling
}
  


void set_font(String  font)
{

  if(font.equals("Arial16")){
    dmd.selectFont(Arial_Black_16);
  }
  if(font=="5*7"){
    dmd.selectFont(SystemFont5x7);
  }
  if(font.equals("Arial14")){
    dmd.selectFont(Arial_14);
  }
  if(font.equals("Droid_Sans_24")){
    dmd.selectFont(Droid_Sans_24);
  }
  if(font.equals("Droid_Sans_16")){
    dmd.selectFont(Droid_Sans_16);
  }
  if(font.equals("Font32*15")){
    dmd.selectFont(My32x15Font);
  }
}












