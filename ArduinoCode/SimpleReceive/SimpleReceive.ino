/*
  RF_Sniffer - receive RF codes.
  
  https://github.com/sui77/rc-switch/
*/

#include <RCSwitch.h> // https://github.com/sui77/rc-switch

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(115200);

  // VCC  : 3.3V
  // GND  : GND
  // Data : D2
  // ANT  : in between the two headers!

  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is INT0
  Serial.println("mySwich receive start");
}

void loop() {
 
  if (mySwitch.available()) {
    
    int value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
   
      Serial.print("Received 0x");
      Serial.print( mySwitch.getReceivedValue(), HEX );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
    }
    
    mySwitch.resetAvailable();
    
  }

}
