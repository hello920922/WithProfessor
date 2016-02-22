/*
 * Title : HM-10 i-beacon & pairing source code
 * Since : 2016-02-10
 * Auther : Mingyu Park
 * Last Modify : 2016-02-10
 */


/*
 * Recommends using this source code after following commands
 * 1. AT+RENEW (Initialize bluetooth module)
 * 2. AT+PWRM1 (Set mode to don't auto sleep)
 * 3. AT+IBEA1 (Turn on the iBeacon switch)
 * 4. AT+MINO0x0001 (Set minor value to the default value)
 * 5. AT+MARJ[PARA] (Set service major number)
 * 6. AT+IBE0 ~ AT+IBE3[00000001 ~ FFFFFFFE] (Set service iBeacon uuid)
 * 7. AT+SAVE1 (Set save mode which does not save connected device's address)
 * 8. AT+ADVI9 (Set advertising interval to the maximum level)
 *                (The max level of AT+ADVI command is F(7000ms) but, IOS system recommends level 9 (1285ms)
 * 9. AT+NAME[PARA] (rename to [PARA])
 * 10. AT+RESET (Apply setting)
 */


#include <SoftwareSerial.h>         // Include library

SoftwareSerial BTSerial(2, 3);      // Value for using bluetooth
int statePin = 4;                     // Value for checking conntected state
int wakeupPin = 5;                    // Value for waking up module

int txDelay(1500);                    // Delay after send data
int ibeaconDelay(1000);               // Delay after set minor value
int sleepDelay(5000);                 // Delay when the module is sleeping
int beforeVal;                        // Value for checking change
int offCount;                         // Value for invalid value count
int limitCount = 10;                  // Value for turning off module

int val;

void setup() {
    Serial.begin(9600);
    BTSerial.begin(9600);             // Start communication with bluetooth module
    pinMode(statePin, INPUT);         // Setting for read connected state
    pinMode(wakeupPin, OUTPUT);       // Setting for waking up module
    digitalWrite(wakeupPin, HIGH);    // Initialize wakeupPin
    offCount = 0;                     // Initialize off count
    beforeVal = 0;                    // Initialize default value

    val = 0;
}

void loop() {
    if (nowState() == 2) {
        // If the module connected, send to sensor value
        BTSerial.print(analogRead(0));
        BTSerial.flush();

        // Wait during tx delay period after sending date
        delay(txDelay);
    }
    else if (nowState() == 1) {
        Serial.println("Module is working");
        if (changedValue()) {
            Serial.println("Value is changed");
            offCount = 0;
            setValue();
            delay(ibeaconDelay);
        }
        else {
            Serial.println("Value is not changed");
            if ((++offCount) >= limitCount) {
                Serial.println("Go to Sleep");
                offCount = 0;
                if(beforeVal == 0)
                  setInterval(false);
                modeSleep();
                delay(sleepDelay);
            }
            else {
                delay(ibeaconDelay);
            }
        }
    }
    else {
        Serial.println("Module is sleeping");
        if (changedValue()) {
            Serial.println("Value is enough");
            modeWakeup();
            setInterval(true);
            setValue();
            delay(ibeaconDelay);
        }
        else
            Serial.println("Value is not enough");
        delay(sleepDelay);
    }
}

int nowState() {
    if (digitalRead(statePin) == HIGH) {
        delay(500);
        if (digitalRead(statePin) == HIGH)
            return 2;     // If state pin is high during 1 sec, the module is connected with phone.
        else
            return 1;     // If state pin is high during 0.5 sec, the module is woken up
    }
    else {
        delay(500);
        if (digitalRead(statePin) == HIGH)
            return 1;     // If state pin is high during 0.5 sec, the module is woken up
        else
            return 0;     // If state pin is low during 1 sec, the module is sleeping
    }
}

boolean changedValue() {
    int tilt1 = analogRead(0);
    int tilt2 = analogRead(1);
    
    if(tilt1>=1000) val = 0x300;
    else if(tilt1 >= 600) val = 0x200;
    else if(tilt1 >= 200) val = 0x100;
    else val = 0x000;

    if(tilt2>=1000) val += 3;
    else if(tilt2 >= 600) val += 2;
    else if(tilt2 >= 200) val += 1;
    else val += 0;
    
    Serial.println(val);
    if(val == beforeVal)
      return false;
    beforeVal = val;
    return true;
}

void modeSleep() {
    delay(1000);
    // Send sleep command to bluetooth module
    BTSerial.write("AT+SLEEP");
    BTSerial.flush();
    digitalWrite(wakeupPin, HIGH);   // Deactivate system key
    delay(100);
}

void modeWakeup() {
    digitalWrite(wakeupPin, LOW);    // Activate system key
    while (nowState() == 0) {}           // wait until module is woken up
    digitalWrite(wakeupPin, HIGH);   // Deactivate system key
}

void setInterval(boolean flag) {
    if (flag)
        BTSerial.write("AT+ADVI0");
    else
        BTSerial.write("AT+ADVI9");
    BTSerial.flush();
    delay(100);
    BTSerial.write("AT+RESET");
    BTSerial.flush();
    delay(100);
}

void setValue() {
    //Make command header
    String str("AT+MINO0x");

    //Make string of sensor value
    if (val == 0)
        val = 1;
    else if (val == 1023)
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
