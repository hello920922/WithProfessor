#include <SoftwareSerial.h>         // Include library

SoftwareSerial BTSerial(2, 3);      // Value for using bluetooth

void setup() {
    Serial.begin(9600);
    BTSerial.begin(9600);             // Start communication with bluetooth module
}

void loop() {
    if(Serial.available()){
      while(Serial.available()){
        BTSerial.write(Serial.read());
      }
    }
    if(BTSerial.available()){
      while(BTSerial.available()){
        Serial.write(BTSerial.read());
      }
    }
}
