/*
 * Title : HM-10 pairing source code
 * Since : 2016-01-18
 * Auther : Mingyu Park
 * Last Modify : 2016-01-19
 */


/*
 * Recommends using this source code after following commands
 * 1. AT+RENEW (Initialize bluetooth module)
 * 2. AT+PWRM0 (Set mode to auto sleep)
 * 3. AT+SAVE1 (Set save mode which does not save connected device's address
 * 4. AT+ADVI9 (Set advertising interval to the maximum level)
 *    (The max level of AT+ADVI command is F(7000ms) but, IOS system recommends level 9 (1285ms)
 * 5. AT+NAME[PARA] (rename to [PARA])
 * 6. AT+RESET (Apply setting)
 */


#include <SoftwareSerial.h>     // Include library

SoftwareSerial BTSerial(2, 3);  // Value for using bluetooth
int statePin = 8;                 // Value for checking conntected state
int wakeupPin = 9;                // Value for waking up module

int txDelay(2000);                // Delay after send data
int sleepDelay(5000);             // Delay when the module is sleeping

void setup() {
    BTSerial.begin(9600);           // Start communication with bluetooth module
    pinMode(statePin, INPUT);       // Setting for read connected state
    pinMode(wakeupPin, OUTPUT);     // Setting for waking up module
    digitalWrite(wakeupPin, HIGH);  // Initialize wakeupPin
}

void loop() {
    if (digitalRead(statePin) == HIGH) {
        // If the module connected, send to sensor value
        BTSerial.print(analogRead(0));
        BTSerial.flush();
    
        // Wait during tx delay period after sending date
        delay(txDelay);
    }
    else {
        // If the module disconnected, just wait during sleep delay period
        delay(sleepDelay);
    }
}
