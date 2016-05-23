
int solDur = 50;
int solenoid = 5;
unsigned long currtime = 0;

void setup() {
pinMode(solenoid, OUTPUT);
Serial.begin(9600);

}

void loop() {
  
  currtime=millis();
 Serial.print("open: ");
 Serial.println(currtime);
 
  digitalWrite(solenoid,HIGH);
 delay(solDur);
 digitalWrite(solenoid,LOW);
 
 currtime=millis();
 Serial.print("closed: ");
 Serial.println(currtime);
 
 delay(1000);
 
}
