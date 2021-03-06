#include <SoftwareSerial.h>

SoftwareSerial BTSerial(2, 3);  // RX, TX
int statePin = 8;                 // Value for checking connected state
int systemKey = 9;                // Value for waking up bluetooth device

String ACK;
int offCount;

int checkVal = 30;      // Value for checking on state
int offDelay = 5000;   // 5 sec
int onDelay = 500;     // 500 ms
int offInterval = 9;    // Max interval parameter is F (7000 ms) But IOS system recommends 9 (1285 ms)
int onInterval = 0;     // Min interval parameter is 0 (100 ms)
int limitCount;         // Value for checking off count

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);

  pinMode(statePin, INPUT);
  pinMode(systemKey, OUTPUT);
  
  offCount = 0;
}

void loop() {
  int val = analogRead(0);    //read sensor

  if(!isWakeup()){
    if(val < checkVal){
      Serial.println("Sleeping...");
      delay(offDelay);
    }
    else{
      modeWakeup();
      sendToBeacon(val);
      delay(onDelay);
    }
  }
  else{
    if(val < checkVal){
      offCount = 0;
      sendToBeacon(val);
      delay(onDelay);
    }
    else{
      if(offCount > limitCount){
        modeSleep();
        offCount = 0;
        delay(offDelay);
      }
      else{
        offCount ++;
        sendToBeacon(val);
        delay(onDelay);
      }
    }
  }
}

boolean isWakeup() {
  ACK = "";
  BTSerial.flush();
  BTSerial.write("AT");
  delay(80);
  while (BTSerial.available()) {
    ACK += (char)BTSerial.read();
    delay(50);
  }
  Serial.println(ACK);
  return ACK.indexOf("OK") > -1;
}

void modeSleep(){
  ACK = "";

  // Make interval setting command
  String str("AT+ADVI"+offInterval);
  char sendStr[9];
  str.toCharArray(sendStr,9);
  
  //Send to beacon
  BTSerial.flush();
  BTSerial.write(sendStr);
  delay(100);

  ACK = "";
  //print response
  while (BTSerial.available()) {
    ACK += (char)BTSerial.read();
    delay(50);
  }
  Serial.println(ACK);
  Serial.println();

  // Go to sleep
  BTSerial.flush();
  BTSerial.write("AT+SLEEP");
  delay(80);
  while(BTSerial.available()){
    ACK += (char)BTSerial.read();
    delay(50);
  }
  Serial.println(ACK);
}

void modeWakeup() {
  ACK = "";
  BTSerial.flush();

  digitalWrite(systemKey, HIGH);
  while(1){             //Until the module be woken up, send characters
    while (BTSerial.available()) {
      ACK += (char)BTSerial.read();
      delay(50);
    }
    //Serial.println(ACK);
    if (!ACK.compareTo("OK+WAKE"))    //If ack message is "OK+WAKE" escape loop
      break;
  }
  digitalWrite(systemKey, LOW);

  // Make interval setting command
  String str("AT+ADVI"+onInterval);
  char sendStr[9];
  str.toCharArray(sendStr,9);
  
  //Send to beacon
  BTSerial.flush();
  BTSerial.write(sendStr);
  delay(100);

  ACK = "";
  //print response
  while (BTSerial.available()) {
    ACK += (char)BTSerial.read();
    delay(50);
  }
  Serial.println(ACK);
  Serial.println();
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
  BTSerial.flush();
  BTSerial.write(sendStr);
  delay(100);

  //print response
  while (BTSerial.available()) {
    ACK += (char)BTSerial.read();
    delay(50);
  }
  Serial.println(ACK);
  Serial.println();
}
