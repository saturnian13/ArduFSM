#include "Arduino.h"
int testPin=7;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(testPin,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //digitalWrite(testPin,HIGH);
  //delay(3000);
  //digitalWrite(testPin,LOW);
  //delay(1000);
}
