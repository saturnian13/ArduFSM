#include "Arduino.h"

int sleep=4; //set HIGH to enable driver
int stepPin=7; //set HIGH to increment step
int dir=2; //set direction of rotation
int M0=13; //use this pin with M1 to select step size
int M1=12; //use this pin with M0 to select step size

void setup() {
  // put your setup code here, to run once:
  pinMode(sleep,OUTPUT);
  pinMode(stepPin,OUTPUT);
  pinMode(dir,OUTPUT);
  pinMode(M0,OUTPUT);
  pinMode(M1,OUTPUT);

  digitalWrite(sleep,HIGH); //Enable driver
  digitalWrite(stepPin,LOW);

  //Select step mode (M0,M1); set to full-step mode (without clocking STEP input) to measure current through motor coil
  // full: LOW, LOW
  // 1/2: HIGH, LOW
  // 1/4: FLOAT, LOW
  // 1/8: LOW, HIGH
  // 1/16: HIGH, HIGH
  // 1/32: FLOAT, HIGH  

  digitalWrite(M0,LOW);
  digitalWrite(M1,LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  //Remember not to clock STEP input when testing current through coils, so comment out following lines
  digitalWrite(stepPin,HIGH);
  digitalWrite(stepPin,LOW);
  delay(50);
}
