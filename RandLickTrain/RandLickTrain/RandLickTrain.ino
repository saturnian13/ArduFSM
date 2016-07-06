//Lick train program
const int solTime = 1040;
int lickDetectPin = A2;
int rewPin = 5; 
int lickVal = 0;
long randNumber = 0;
long waittime = 0;

void setup() {
  
  pinMode(lickDetectPin, INPUT);
  pinMode(rewPin, OUTPUT);
  digitalWrite(rewPin, LOW);
  
  Serial.begin(115200);
  randomSeed(analogRead(A4));
  
  delay(2000);
  
}

void loop() { //gives water on random interval between 1 and 6 seconds
  
  //lickVal = analogRead(lickDetectPin);
  randNumber = random(3, 7);
  waittime = randNumber*1000;
  Serial.println("wait time = ");
  Serial.print(waittime);
  delay(waittime);
  
  digitalWrite(rewPin, HIGH);
  Serial.print("Solenoid open");
  delay(solTime);  
  digitalWrite(rewPin, LOW);
  
}
