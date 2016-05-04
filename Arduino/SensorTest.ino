void setup() {
  Serial.begin(9600);
}

void loop() {
  changedValue();
  delay(1000);
}

void changedValue(){
  int lf = analogRead(2);
  int lb = analogRead(3);
  int rf = analogRead(4);
  int rb = analogRead(5);

  
  if(lf >= 270) lf = 3;
  else if(lf >= 150) lf = 2;
  else if(lf >= 100) lf = 1;
  else lf = 0;

  if(lb >= 200) lb = 3;
  else if(lb >= 130) lb = 2;
  else if(lb >= 70) lb = 1;
  else lb = 0;

  if(rf >= 320) rf = 3;
  else if(rf >= 250) rf = 2;
  else if(rf >= 100) rf = 1;
  else rf = 0;

  if(rb >= 240) rb = 3;
  else if(rb >= 150) rb = 2;
  else if(rb >= 100) rb = 1;
  else rb = 0; 
  

  Serial.println("==========================");
  Serial.print("lf : ");
  Serial.println(lf);
  Serial.print("lb : ");
  Serial.println(lb);
  Serial.print("rf : ");
  Serial.println(rf);
  Serial.print("rb : ");
  Serial.println(rb);
  Serial.println("==========================");
}
