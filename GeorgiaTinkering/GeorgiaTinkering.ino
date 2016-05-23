/* Test the lick detector
Will also test using the python connection and saving files in new boxes
*/

#include "chat.h"
#include "hwconstants.h"

int lickDetectPin=A2;
int lickVal = 0; //for sparkfun lick detector
int lastLick = 0;
long lickTime = 0;
int lickState = 0;

void setup() {
  // put your setup code here, to run once:

pinMode(lickDetectPin, INPUT);

Serial.begin(9600);

}

void loop() {

lickVal = analogRead(lickDetectPin);
Serial.println(lickVal);
delay(500);

}