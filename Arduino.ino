#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  int val = analogRead(0);  //read sensor
  sendToBeacon(val);        //call function : send to beacon
  
  delay(2000);
}

void sendToBeacon(int val) {
  //make command header
  String str = "AT+MARJ0x";
  
  //make string of sensor value
  String valStr;
  if (val > 0)
    valStr = String(val, HEX);
  else
    valStr = "0001";
  valStr.toUpperCase();

  //complete command
  for (int i = 0; i < 4 - valStr.length(); i++) {
    str += "0";
  }
  str += valStr;

  //to character array
  char sendStr[14];
  str.toCharArray(sendStr, 14);

  //print log
  Serial.print("sensor value : ");
  Serial.println(val);

  Serial.print("send to beacon : ");
  Serial.println(sendStr);

  //send to beacon
  mySerial.write(sendStr);

  //print response
  delay(50); //wait a moment
  if (mySerial.available()) {
    while (mySerial.available()) {
      Serial.write(mySerial.read());
    }
    Serial.println("\n"); //two newlines
  }
}
