int state;

void setup() {
  state = 0;
  Serial.begin(9600);
}

void loop() {
  int val = map(analogRead(0), 0, 1023, 0, 5);
  if(state != val){
    state = val;
    Serial.print("Change State : ");
    Serial.println(val);
  }
  delay(1000);
}
