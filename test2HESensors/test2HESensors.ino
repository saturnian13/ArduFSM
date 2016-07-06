//Check and print values of two hall effect sensors
int HEsensor1 = A0;
int HEval1=0;

int HEsensor2 = A1;
int HEval2=0;

void setup() {
  pinMode(HEsensor1, INPUT);
  pinMode(HEsensor2, INPUT);
  Serial.begin(115200);
}

void loop() {
  HEval1= analogRead(HEsensor1);
  Serial.print("HE Value 1 = ");
  Serial.println(HEval1);
  
  HEval2= analogRead(HEsensor2);
  Serial.print("HE Value 2 = ");
  Serial.println(HEval2);
  
  delay(500);

}
