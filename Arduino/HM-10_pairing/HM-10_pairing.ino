// HM-10 pairing source code
#include <SoftwareSerial.h>

SoftwareSerial BTSerial(2,3);

int txDelay(100);
int sleepDelay(5000);

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
}

void loop() {
  if(BTSerial.available()){
    int val = analogRead(0);
    
    BTSerial.write(val);
    while(BTSerial.available())
      Serial.write(BTSerial.read());
    Serial.println();

    delay(txDelay);
  }
  else{
    delay(sleepDelay);
  }
}
