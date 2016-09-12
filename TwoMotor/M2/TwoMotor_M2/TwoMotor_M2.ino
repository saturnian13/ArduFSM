//This version is set for the modular behavior setups

// VARIABLES TO CHECK FIRST!
boolean stimResp = true; //determines whether responses are counted during the stim period
const int stimDur = 1500;
const int respDur = 500; //1500;
const int solTime = 30; //time soleniod valve is open for delivery of water; controls how much water comes out each time
const int punTime = 6000;//5500; //time in msec of timeout for unrewarded licks
const int rewType = 1; //Determines whether top stimulus is rewarded (= 1) or bottom stimulus is rewarded (= 2)
const int iti = 1000;//500; //3000;


// Initialize STEPPER library info
#include <Stepper.h>

String programName = "twomotor_M2_mpm03-3";
String notes = "second session today to fix";

//Trial variables
long trialstart = 0;
long respDur_start = 0; //response window time
long iti_start = 0;
long randNumber; //random number to determine trial type

int sessionStart = 0;
long currTime = 0;
boolean needToPunish = false;

//Stimulus variables
int stimType = 0; // Value changes depending on trial type (1 = rewarded, 2 = unrewarded, 3 = both stim, 4 = no stim)
const int rewStim = 25; //Percent rewarded stim only trials
const int unrewStim = 25; //Percent unrewarded stim only trials
const int bothStim = 25; //Percent trials with both stimuli present (rewarded)
const int noStim = 25;//Percent trials with no stimuli present (unrewarded)
long stimTimer = 0; //Time counter for the stimulus


//Motor-related variables
int enable_lower = 7;
int motorPin1_lower = 6;  // Digital output pins for one of the motors
int motorPin2_lower = 8;

int enable_upper = 10;
int motorPin1_upper = 9;  // Digital output pins for one of the motors (middle h-bridge)
int motorPin2_upper = 11;

const int motorSteps = 200; // defines the # steps in one revolution in the motor.
// defines a new stepper motor that is attached to the arduino board.
Stepper lowerStepper(motorSteps, motorPin1_lower, motorPin2_lower);
Stepper upperStepper(motorSteps, motorPin1_upper, motorPin2_upper);

//Stepper motor movement variables
const int stepNumber = 30; // step size (50 = 1/4 rotation of arm)
const int motorSpeed = 80; //speed at which motors move
int stepCount = 1;         // number of steps the motor has taken
int poleIn = 0;

//Hall effect sensor variables
int hallSensPinUpper = A1; //analog pin 0 on the board for box 1
int hallSensPinLower = A0; //analog pin 1 on the board
int hallSensValUpper = 500;
int hallSensValLower = 500;
int hallThreshUpper = 900; //Changing the thresholds can alter the exact position at which the motors stop
int hallThreshLower = 925;
int stepsUpper = stepNumber;
int stepsLower = stepNumber;
int upperMove = 0;
int lowerMove = 0;


//Lick-related variables
//int lickPin = 9; //Digital 9; sends TTL everytime animal licks //CHOOSE NEW PIN!
int lickDetectPin = A2; //for sparkfun lick detector input
int lickVal = 0; //for sparkfun lick detector
int lastLick = 0;
long lickTime = 0;
int rewPin = 5; //Digital 4; sends TTL whenever solenoid is opened/closed, powers solenoid
//int punPin = 10; //Digital 10; sends TTL when animal recevies timeout
int lickState = 0;
int rewState = 0; //Value changed to 1 when animal is reward or punished on each trial.
long respTime = 0; //Time in msec of reward/punish for each trial
long respTimer = 0; //Timer for counting out the response window
long punishResp = 0; //time that response that should be punished happened;
int lickthresh = 900;
int lowlickthresh = (lickthresh * -1);
//  // experimenter start button
//  int buttonPin = 8;    // define pin for exp lever
//  int buttonState  = 0;
//  int prevButtonState = 0;
int startProg = 0;
//  int speakerOut = 7; //speaker volume?


//////////////////////////////////////////
void setup() {

  pinMode(rewPin, OUTPUT);  // sets digital pin for turing solenoid valve ON
  //pinMode(lickPin, OUTPUT);
  pinMode(lickDetectPin, INPUT);
  //pinMode(trialStartPin, OUTPUT);
  //pinMode(punPin, OUTPUT);
  //    pinMode(buttonPin, INPUT);
  pinMode(enable_lower, OUTPUT);
  pinMode(enable_upper, OUTPUT);


  lowerStepper.setSpeed(motorSpeed); // sets the speed at which the motor will turn in rotations per minute
  upperStepper.setSpeed(motorSpeed);

  // initialize the serial port: (sets up communication with computer at a speed of 9600 bits per second
  Serial.begin(115200);
  //Wire.begin();

  randomSeed(analogRead(A4)); //make sure A4 is empty


  digitalWrite(rewPin, LOW);  // sets digital pin for turing solenoid valve ON
  //digitalWrite(lickDetectPin, LOW);
  //digitalWrite(trialStartPin, LOW);
  //digitalWrite(punPin, LOW);

  Serial.println("Program name:");
  Serial.println(programName);
   

  Serial.println("Rewarded stimulus (1=top, 2=bot)");
  Serial.println(rewType);
  Serial.println("Percent trials rewarded stim only = ");
  Serial.println(rewStim);
  Serial.println("Percent trials unrewarded stim only = ");
  Serial.println(unrewStim);
  Serial.println("Percent trials both stim = ");
  Serial.println(bothStim);
  Serial.println("Percent trials neither stim = ");
  Serial.println(noStim);
  Serial.println("reward amount/duration =");
  Serial.println(solTime);
  Serial.println("stimDur =");
  Serial.println(stimDur);
  Serial.println("respDur =");
  Serial.println(respDur);
  Serial.println("ITI =");
  Serial.println(iti);
  Serial.println("Timeout/punishment duration = ");
  Serial.println(punTime);
  Serial.println("Motor speed = ");
  Serial.println(motorSpeed);
  Serial.println("Number of steps for motor = ");
  Serial.println(stepNumber);
  Serial.println("Notes: ");
    Serial.println(notes);
  Serial.println("M2");
  Serial.print("Allow responses during stim? ");
  if (stimResp) {
    Serial.println("True"); //Print a blank line at the start of each trial
  }
  if (!stimResp) {
    Serial.println("False");
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()  { //START MAIN LOOP

  //Button to toggle program on and off


  if (sessionStart == 0) {

    //
    hallSensValLower = analogRead(hallSensPinLower);
    hallSensValUpper = analogRead(hallSensPinUpper);
    if (poleIn == 0) {
      moveSteppersForward(3, rewType); //Move upper stim forward to adjust postitions
      poleIn = 1;
      delay(20000);
      //delay(15000);
    }
  }

  //    buttonState = digitalRead(buttonPin);
  //    if (buttonState == HIGH) {
  if (startProg == 0) {
    startProg = 1;
    delay (1000);
    moveSteppersBack(3, rewType); //Move both stim back to begin session

    Serial.println("Starting session");
    sessionStart = 1;
    delay(iti);
  }
  //      else {
  //        startProg = 0;
  //      }
  //      delay(1000);
  // } //end if button state

  if (startProg == 1) {



    Serial.println(" "); //Print a blank line at the start of each trial
    //Set up trial type
    stimType = trialType(); // determine trial type at the start of each trial

    trialstart = millis(); // start time of each trial in msec
    needToPunish = false;
    rewState = 0; //Set to zero at the start of each trial

    Serial.print("Trial start time: ");
    Serial.println(trialstart);

    //Determine how to move the stimuli based on trial type and which stimulus is rewarded for this animal
    moveSteppersForward(stimType, rewType);
    delay(5);//additional delay to eliminate fake licks
    //Serial.print("Move stim forward");

    //Check for licks during the stimulus period
    currTime = millis();
    //Serial.println(currTime);

    while ((currTime - trialstart) <= stimDur) { //STIM WINDOW
      readTouchInputs(); //Checks the touch state from the lick detector; changes 'lickState' from 0 to 1 if touch occurred.

      if (stimResp) { //if allowing responses during the stim period

        if (lickState == 1) { //If animal licks
          if (rewState == 0) { // if animal hasn't already been rewarded/punished this trial
            respTime = millis();
            if (stimType == 1 || stimType == 3) { // Reward for rewarded stim only trials (== 1) or both stim trials (== 3)
              reward(respTime);
              rewState = 1;
            }
            else { // Punish for unrewarded stim trials (== 2) or neither stim trials (== 4)
              punishResp = respTime;
              Serial.print("Incorrect lick during stim period: ");
              Serial.println(punishResp);
              //  digitalWrite(punPin, HIGH);
              //  delay(1);
              //  digitalWrite(punPin, LOW);
              needToPunish = true;
              rewState = 1; //Set equal to 1 when reward/punishment given; zero otherwise. Ensures only 1 reward/punishment given per trial.
            }

          } // end rewState
        } // end lickState

      } //end if stimResp
      currTime = millis();
    } // End of stimulus duration

    //Move motors back => will this def happen at the right time?
    currTime = millis();
    if ((currTime - trialstart) >= stimDur) {
      moveSteppersBack(stimType, rewType);
      //Serial.print("Move stim back");
      //Serial.println(currTime);
    }

    respDur_start = millis(); //start response window
    Serial.print("Start Response Window:");
    Serial.println(respDur_start);

    //Check for licks during the response period after stimulus is gone
    respTimer = millis();
    while ((respTimer - respDur_start) <= respDur) { //RESPONSE WINDOW
      readTouchInputs();
      if (lickState == 1) { //If animal licks
        if (rewState == 0) { // if animal hasn't already been rewarded/punished this trial
          respTime = millis();
          if (stimType == 1 || stimType == 3) { // Reward for rewarded stim only trials (== 1) or both stim trials (== 3)
            reward(respTime);
          }
          else { // Punish for unrewarded stim trials (== 2) or neither stim trials (== 4)
            punish(respTime);
          }

        } // end rewState
      } // end lickState

      if (needToPunish) {
        respTime = millis();
        punish(respTime);
        needToPunish = false;
      }

      respTimer = millis();
    }
    Serial.print("End Response Window: ");
    Serial.println(millis());

    iti_start = millis();
    //Check for licks during the ITI
    while ((millis() - iti_start) <= iti) {
      readTouchInputs();
      rewState = 0; //reset rewState
      //lickState = 0; //reset lickState
    }

  } //end if startProg

} //END MAIN LOOP


/////////////////////////////////////////////////////////////////////////////////////////////////////
//START OF SUB-FUNCTIONS
int trialType() {

  long randNumber = random(0, 100);

  if (randNumber <= rewStim) {
    Serial.println("Rewarded stimulus only");
    stimType = 1;
  }
  else if (randNumber > rewStim && randNumber <= (rewStim + unrewStim)) {
    Serial.println("Unrewarded stimulus only");
    stimType = 2;
  }
  else if (randNumber > (rewStim + unrewStim) && randNumber <= (rewStim + unrewStim + bothStim)) {
    Serial.println("Both stimuli");
    stimType = 3;
  }
  else if (randNumber > (rewStim + unrewStim + bothStim)) {
    Serial.println("Neither stimuli");
    stimType = 4;
  }
  return stimType;
}

////////////////////////////////////////////
void moveSteppersForward(int stimtype, int rewtype) {

  if (rewtype == 2) { //Change rewarded/unrewarded motor movement depending on which stim is rewarded
    if (stimtype == 1) {
      stimtype = 2;
    }
    else if (stimtype == 2) {
      stimtype = 1;
    }
  }

  int moveUpper = 0;
  int moveLower = 0;
  int s = 0;
  int sL = 0;
  int sU = 0;

  digitalWrite(enable_upper, HIGH);
  digitalWrite(enable_lower, HIGH);
  delay(20);

  switch (stimtype)
  {
    case 1:  //Upper stim moved into whisker field

      moveUpper = -1; //move into whiskers based on HE sensor
      moveLower = -1; //move away from whiskers based on number of steps

      stepsUpper = 0;
      stepsLower = stepsLower; //still using value from last trial
      //while (moveUpper != 0) {
      for (int i = 0; i <= 80; i++) { //Will loop 80 times; therefore that's the max # steps the motor can make

        if (s < (stepsLower)) {  //moving until number of steps is achieved
          s = s + 1;
        }
        else {
          moveLower = 0;
        }

        hallSensValUpper = analogRead(hallSensPinUpper);

        if (hallSensValUpper < hallThreshUpper) { // && moveUpper != 0) { //moving until HE sensor crossed
          stepsUpper = stepsUpper + 1;
        }
        else {
          moveUpper = 0;
        }
        lowerStepper.step(moveLower);
        delay(1);  // slight delay after movement to ensure proper step before next
        upperStepper.step(moveUpper);
        delay(1);

        hallSensValUpper = analogRead(hallSensPinUpper);
      }
      stepsLower = s;
      break;

    case 2:

      moveUpper = 1; //move away from whiskers based on number of steps
      moveLower = 1; //move into whiskers based on HE sensor

      stepsUpper = stepsUpper; //using value from last trial
      stepsLower = 0;
      //while (moveLower !=0) {
      for (int i = 0; i <= 80; i++) { //Will loop 80 times; therefore that's the max # steps the motor can make

        if (s < (stepsUpper)) {  //moving until number of steps is achieved
          s = s + 1;
        }
        else {
          moveUpper = 0;
        }

        hallSensValLower = analogRead(hallSensPinLower);

        if (hallSensValLower < hallThreshLower && moveLower != 0) { //moving until HE sensor crossed
          stepsLower = stepsLower + 1;
        }
        else {
          moveLower = 0;
        }
        lowerStepper.step(moveLower);
        delay(1);  // slight delay after movement to ensure proper step before next
        upperStepper.step(moveUpper);
        delay(1);

        hallSensValLower = analogRead(hallSensPinLower);
      }
      stepsUpper = s;
      break;

    case 3:
      moveUpper = -1; //move into whiskers
      moveLower = 1; //move into whiskers

      stepsUpper = 0;
      stepsLower = 0;

      //while (moveUpper != 0 && moveLower != 0) {
      for (int i = 0; i <= 80; i++) { //Will loop 80 times; therefore that's the max # steps the motor can make

        hallSensValUpper = analogRead(hallSensPinUpper);
//        Serial.println("Upper");
//        Serial.println(hallSensValUpper);

        if (hallSensValUpper < hallThreshUpper) { // && moveUpper != 0) { //moving until HE sensor crossed
          stepsUpper = stepsUpper + 1;
        }
        else {
          moveUpper = 0;
        }

        hallSensValLower = analogRead(hallSensPinLower);
//        Serial.println("Lower");
//        Serial.println(hallSensValLower);

        if (hallSensValLower < hallThreshLower) { // && moveLower != 0) { //moving until HE sensor crossed
          stepsLower = stepsLower + 1;
        }
        else {
          moveLower = 0;
        }
        lowerStepper.step(moveLower);
        delay(1);  // slight delay after movement to ensure proper step before next
        upperStepper.step(moveUpper);
        delay(1);
      }

      break;

    case 4:
      moveUpper = 1; //move away
      moveLower = -1; //move away

      stepsLower = stepsLower; //using value from last trial
      stepsUpper = stepsUpper; //using value from last trial

      //while (moveLower !=0 || moveUpper != 0) {
      for (int i = 0; i <= 80; i++) { //Will loop 80 times; therefore that's the max # steps the motor can make
        if (sU < (stepsUpper)) {  //moving until number of steps is achieved
          sU = sU + 1;
        }
        else {
          moveUpper = 0;
        }
        if (sL < (stepsLower)) {  //moving until number of steps is achieved
          sL = sL + 1;
        }
        else {
          moveLower = 0;
        }
        lowerStepper.step(moveLower);
        delay(1);  // slight delay after movement to ensure proper step before next
        upperStepper.step(moveUpper);
        delay(1);
      }
      stepsUpper = sU;
      stepsLower = sL;

      break;
  }
  digitalWrite(enable_upper, LOW);
  digitalWrite(enable_lower, LOW);

  delay(20); //need this to eliminate false licks with sparkfun lick detector

}

void moveSteppersBack(int stimtype, int rewtype) {
  if (rewtype == 2) { //Change rewarded/unrewarded motor movement depending on which stim is rewarded
    if (stimtype == 1) {
      stimtype = 2;
    }
    else if (stimtype == 2) {
      stimtype = 1;
    }
  }

  int sU = 0;
  int sL = 0;
  int returnUpper = 0;
  int returnLower = 0;
  stepsUpper = stepNumber;
  stepsLower = stepNumber;

  digitalWrite(enable_lower, HIGH);
  digitalWrite(enable_upper, HIGH);
  //delay(20);
  switch (stimtype)
  {
    case 1:
      returnUpper = 1;
      returnLower = 1;
      break;
    case 2:
      returnUpper = -1;
      returnLower = -1;
      break;
    case 3:
      returnUpper = 1;
      returnLower = -1;
      break;
    case 4:
      returnUpper = -1;
      returnLower = 1;
      break;
  }
  //while (returnLower !=0 || returnUpper != 0) {
  for (int i = 0; i <= 80; i++) { //Will loop 80 times; therefore that's the max # steps the motor can make

    sU = sU + 1;
    sL = sL + 1;
    if (sU >= (stepsUpper)) {  //moving until number of steps is achieved
      returnUpper = 0;
    }
    if (sL >= (stepsLower)) {  //moving until number of steps is achieved
      returnLower = 0;
    }
    lowerStepper.step(returnLower);
    delay(1);  // slight delay after movement to ensure proper step before next
    upperStepper.step(returnUpper);
    delay(1);
  }
  digitalWrite(enable_lower, LOW);
  digitalWrite(enable_upper, LOW);
  delay(20); //need this to eliminate false licks with sparkfun lick detector
}

void reward(int resptime) {
  Serial.print("Reward!: ");
  Serial.println(millis());
  digitalWrite(rewPin, HIGH); //Open soleniod valve
  delay(solTime); //keep this delay statement
  digitalWrite(rewPin, LOW); //Close solenoid valve
  rewState = 1; //Set equal to 1 when reward/punishment given; zero otherwise. Ensures only 1 reward/punishment given per trial.

}

void punish(int resptime) {
  Serial.print("Unrewarded lick, timeout: ");
  Serial.println(respTime);
  //  digitalWrite(punPin, HIGH);
  //  delay(1);
  //  digitalWrite(punPin, LOW);
  int t = millis();
  while ((t - resptime) <= punTime) {
    readTouchInputs();
    t = millis();
  }
  //delay(punTime);
  rewState = 1; //Set equal to 1 when reward/punishment given; zero otherwise. Ensures only 1 reward/punishment given per trial.
}

///////////
// Checks for touch inputs to the lick detector

void readTouchInputs() {

  lickVal = analogRead(lickDetectPin); //read touch state from sparkfun lick detector
  //Serial.println(lickVal - lastLick);

  if (lickState == 0) { //if not already licking
    if (lickVal - lastLick > lickthresh) {
      Serial.print("Lick!: ");
      lickTime = millis();
      Serial.println(lickTime);
      lickState = 1;
    }
  }

  if (lickState == 1) { //if already licking
    if (lickVal - lastLick < lowlickthresh) {
      lickState = 0;
    }
  }

  lastLick = lickVal;

} //END ReadTouchInputs




