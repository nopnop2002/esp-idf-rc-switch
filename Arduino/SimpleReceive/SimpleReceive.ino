/*
  RF_Sniffer - receive RF codes.
*/

#include <RCSwitch.h> // https://github.com/sui77/rc-switch
RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(115200);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is D2
  Serial.println("mySwich receive start");
}

void loop() {
 
  if (mySwitch.available()) {
    
    int value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
   
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
    }
    
    mySwitch.resetAvailable();
    
  }

}
