void setup() {
  Serial.begin(115200);  //initial the Serial
}
 
void loop() {
  if (Serial.available())  {
    String data = "ack : ";
    while(Serial.available()){
      data += (char)Serial.read();
      delay(50);
    }
    Serial.println(data);
  }
}
