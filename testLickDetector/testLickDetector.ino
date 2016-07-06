
//Read values from the sparkfun lick detector
int lickDetectPin=A2;
int lickVal = 0; //for sparkfun lick detector
int lastLick = 0;
long lickTime = 0;
int lickState = 0;

void setup() {

pinMode(lickDetectPin, INPUT);

Serial.begin(115200);

}

void loop() {

lickVal = analogRead(lickDetectPin);
Serial.println(lickVal);
delay(500);

}


