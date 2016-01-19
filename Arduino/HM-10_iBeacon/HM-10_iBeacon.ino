/*
 * Title : HM-10 iBeacon source code
 * Since : 2016-01-19
 * Auther : Mingyu Park
 * Last Modify : 2016-01-19
 */


/*
 * Recommends using this source code after applying following commands
 * 1. AT+RENEW (Initialize bluetooth module)
 * 2. AT+ADTY3 (Set advertising type to only allowing advertising)
 * 3. AT+IBEA1 (Turn on the iBeacon switch)
 * 4. AT+MARJ0x0001 (Set Major value to the default value)
 * 5. AT+NAME[PARA] (rename to [PARA])
 * 6. AT+IBE0 ~ AT+IBE3[00000001 ~ FFFFFFFE] (Set iBeacon UUID)
 * 7. AT+PWRM1 (Set mode to don't auto sleep)
 * 8. AT+POWE0 (Set power to the minimum)
 * 10. AT+ADVI9 (Set advertising interval to the maximum level)
 *     (The max level of AT+ADVI command is F(7000ms) but, IOS system recommends level 9 (1285ms)
 * 11. AT+RESET (Apply setting)
 */


#include <SoftwareSerial.h>     // Include library

SoftwareSerial BTSerial(2, 3);  // Value for using bluetooth
int statePin = 8;                 // Value for checking sleeping state
int wakeupPin = 9;                // Value for waking up module

int txDelay = 200;                // Delay after send data
int sleepDelay = 5000;            // Delay when the module is sleeping
int offCount;                     // Value for invalid value count
int limitCount = 10;              // Value for turning off module

int val;

void setup() {
    Serial.begin(9600);
    BTSerial.begin(9600);           // Start communication with bluetooth module
    pinMode(statePin, INPUT);       // Setting for reading connected state
    pinMode(wakeupPin, OUTPUT);     // Setting for waking up module
    digitalWrite(wakeupPin, HIGH);  // Initialize wakeupPin
    offCount = 0;                   // Initialize off count

    val = 0;
}

void loop() {
    if(!isWakeup()){
        Serial.println("Module is sleeping");
        if(enoughValue()){
            Serial.println("Value is enough");
            modeWakeup();
            setInterval(true);
            setValue();
            delay(txDelay);
        }
        else
            Serial.println("Value is not enough");
            delay(sleepDelay);
    }
    else{
        Serial.println("Module is working");
        if(enoughValue()){
            Serial.println("Value is enough");
            offCount = 0;
            setValue();
            delay(txDelay);
        }
        else{
            Serial.println("Value is not enough");
            if((++offCount) >= limitCount){
                Serial.println("Go to Sleep");
                offCount = 0;
                setInterval(false);
                modeSleep();
                delay(sleepDelay);
            }
            else{
                setValue();
                delay(txDelay);
            }
        }
    }
}

boolean isWakeup(){
    // If current led is turned on, bluetooth module is woken up
    if(digitalRead(statePin) == HIGH)
      return true;
  
    // If current led is turned off, should recheck after 500 ms
    delay(500);
    if(digitalRead(statePin) == HIGH)
      return true;
  
    // If current led is turned off after 500ms, bluetooth module is sleeping
    return false;
}

boolean enoughValue(){
    val = analogRead(0);
    Serial.println(val);
    return val > 30;
}

void modeSleep(){
    delay(1000);
    // Send sleep command to bluetooth module
    BTSerial.write("AT+SLEEP");
    BTSerial.flush();
    digitalWrite(wakeupPin, HIGH);  // Deactivate system key
    delay(100);
}

void modeWakeup(){
    digitalWrite(wakeupPin, LOW);   // Activate system key
    while(!isWakeup()){}            // wait until module is woken up
    digitalWrite(wakeupPin, HIGH);  // Deactivate system key
}

void setInterval(boolean flag){
    if(flag)
        BTSerial.write("AT+ADVI0");
    else
        BTSerial.write("AT+ADVI9");
    BTSerial.flush();
    delay(100);
    BTSerial.write("AT+RESET");
    BTSerial.flush();
    delay(100);
}

void setValue(){
    //Make command header
    String str("AT+MARJ0x");
  
    //Make string of sensor value
    if(val == 0)
        val = 1;
    else if(val == 1023)
        val = 1022;
    String valStr(val, HEX);
    valStr.toUpperCase();
  
    //Complete command
    for (int i = 0; i < 4 - valStr.length(); i++)
        str += "0";
    str += valStr;
  
    //Transform to character array
    char sendStr[14];
    str.toCharArray(sendStr, 14);
  
    //Send to beacon
    Serial.println(sendStr);
    BTSerial.write(sendStr);
    BTSerial.flush();
    delay(100);
}

