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
int statePin = 5;                     // Value for checking conntected state
int wakeupPin = 4;                    // Value for waking up module

int txDelay(50);                    // Delay after send data
int ibeaconDelay(10500);               // Delay after set minor value
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
    changedValue();
    if (nowState() == 2) {
        Serial.println("Module is connected");
        // If the module connected, send to sensor value
        BTSerial.print(val);
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
    /*
     * First 1 byte Left side :
     * Second 1 byte Right sid :
     *   ->  0-F-B-T
     *     
     */
    int lt = analogRead(0);
    int rt = analogRead(1);
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
    
    if(lt>=1000) lt = 3;
    else if(lt >= 600) lt = 2;
    else if(lt >= 200) lt = 2;
    else lt = 0;

    if(rt>=1000) rt = 3;
    else if(rt >= 600) rt = 2;
    else if(rt >= 200) rt = 2;
    else rt = 0;

    val = 0;

    val += lf << 4;
    val += lb << 2;
    val += lt;

    val <<= 8;

    val += rf << 4;
    val += rb << 2;
    val += rt;

    Serial.println("============================");
    Serial.print(lf);
    Serial.print(lb);
    Serial.print(lt);
    Serial.print(rf);
    Serial.print(rb);
    Serial.println(rt);

    Serial.print("Val : ");
    Serial.println(val,HEX);
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
    //Serial.println(sendStr);
    BTSerial.write(sendStr);
    BTSerial.flush();
    delay(100);
}
