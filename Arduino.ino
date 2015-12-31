#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX
String ACK;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  int val = analogRead(0);    //read sensor

  if (val < 30) { //Check sensor value
    Serial.println("Go to sleep mode");
    modeSleep();              //Put module into sleep
    delay(2000);
  }

  else if (!isWakeup()) {     //Check AT mode whether wakeup or sleep
    Serial.println("AT is sleep mode");
    modeWakeup();             //Wakeup module
  }

  else {
    Serial.println("AT is wakeup mode");
    sendToBeacon(val);        //Send BT Command
    delay(2000);
  }
}

boolean isWakeup() {
  ACK = "";
  mySerial.write("AT");
  delay(80);
  while (mySerial.available()) {
    ACK += (char)mySerial.read();
    delay(50);
  }
  Serial.println(ACK);
  return ACK.indexOf("OK") > -1;
}

void modeSleep(){
  ACK = "";
  mySerial.write("AT+SLEEP");
  delay(80);
  while(mySerial.available()){
    ACK += (char)mySerial.read();
    delay(50);
  }
  Serial.println(ACK);
}

void modeWakeup() {
  ACK = "";
  while(1){             //Until the module be woken up, send characters
    Serial.println("Send 80 characters");
    mySerial.write("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    delay(2000);
    while (mySerial.available()) {
      ACK += (char)mySerial.read();
      delay(50);
    }
    Serial.println(ACK);
    if (!ACK.compareTo("OK+WAKE"))    //If ack message is "OK+WAKE" escape loop
      break;
  }
}

void sendToBeacon(int val) {
  ACK = "";
  //Make command header
  String str("AT+MARJ0x");

  //Make string of sensor value
  String valStr(val, HEX);
  valStr.toUpperCase();

  //Complete command
  for (int i = 0; i < 4 - valStr.length(); i++)
    str += "0";
  str += valStr;

  //Transform to character array
  char sendStr[14];
  str.toCharArray(sendStr, 14);

  //print log
  Serial.print("sensor value : ");
  Serial.println(val);
  Serial.print("send to beacon : ");
  Serial.println(sendStr);

  //Send to beacon
  mySerial.write(sendStr);
  delay(100);

  //print response
  while (mySerial.available()) {
    ACK += (char)mySerial.read();
    delay(50);
  }
  Serial.println(ACK);
  Serial.println();
}
