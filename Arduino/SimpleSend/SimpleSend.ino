/*
  SendDemo - sends RF codes.
*/

#include <RCSwitch.h> // https://github.com/sui77/rc-switch
RCSwitch mySwitch = RCSwitch();

void setup() {
 
  // VCC  : 3.3V
  // GND  : GND
  // Data : D7
  // ANT  : in between the two headers!
 
  // Data Pin
  pinMode(7, OUTPUT);  // Either way, we'll use pin D7 to drive the data pin of the transmitter.

  Serial.begin(115200);
 
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(7);
  Serial.println("mySwich transmitte start");

}

void loop() {
  static long value=0;   

//  mySwitch.send(value, 24);
  mySwitch.send(value, 32);
  value++;
  delay(1000);
}
