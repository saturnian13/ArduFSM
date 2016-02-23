#include "Arduino.h"

/*
BotRew10s_discreteTrial_101310a.pde
 TopRew_randAlt_020511b.pde
 
 09/10: updated this sketch ( headFixTime2VacToneStep_091409a -> TwoStimLever1_110209a -> discreteTrial) for the new headfix setup using normal Arduino and new Pololu steppers
 ALSO: using Tone library for cue, punishment noise, and reward tones, and using Stepper library to run whisker stim steppers
 
 101310: following up on sketch from 100510, put in check for holding down lever, plus made variables for intertrial interval and timeouts
 (NOTE: previous one had Top reward instead of bottom, so name is actually inaccurate)
 
 112410: now changing the program to trigger the trials when the lever is depressed and reward upon lifting
 
 020511: reduced sensitivity of lever by requiring certain time of continuous lifting for choice triggering.
 Also, started using random number generator library and am forcing alternation of trial type after certain number of consecutive trials.
 Also, broke out important experiment variables to beginning of sketch to allow easy manipulation of experimental parameters.
 ...and made special variables to specify whether top/bottom stim is rewarded.
 
 Aug 2011: modified the script for use in the 2p room
 
 Sep 2011: modified the script to include catch trials for Aniruddha's experiment
 
 Feb 2012: running steppers a little slower to try to address
 problem with stepper arm stopping/not moving sometimes
 Also, waiting a little longer after enable pins activated 
 before moving stim.
 
 010614: to alleviate problems with rewTime5 (based upon arduino time)
 I just made all times printed as millis() instead of based upon some
 previous time/trigger.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int programComplete = 0;

//////////declare motor variables
#include <Stepper.h>
int motorPin1 = 8;  // mapping for new boxes, Jan. 2011
int motorPin2 = 6;
//int motorPin3 = 7;  // mapping for new boxes, Jan. 2011
//int motorPin4 = 6;    
int enable1 = 7;
//int enable2 = 5;
int stepCCW = 20;
int stepCW = -20;

#define motorSteps 200  // 200steps/rot with Pololu steppers
Stepper upperStepper(motorSteps, motorPin1, motorPin2);
//Stepper lowerStepper(motorSteps, motorPin3, motorPin4);

//////////declare Hall effect sensor variables
int hallSensPin1 = 1;
int hallSensVal1 = 500;
//int hallSensPin2 = 0;  // analog pin for TOP Hall effect sensor (for stim arm placement)
//int hallSensVal2 = 500;   
int hallThresh = 50;

//////////declare speaker variables
#include <Tone.h>
Tone cueTones[1];
int speakerPin = 13;
//int toneFreq = 16000; will do white noise instead of a single tone
int toneDur = 2000;
int startTone = millis();
int currTime = millis();

//////////declare LED variables
int LED1pin = 12;
//int LED2pin = ;

//////////decalre reward variables
int solenoidPin = 4;

//////////declare timing variables
int timingPin = 2;
int iti = 10000;

//////////SETUP
void setup(){
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(enable1, OUTPUT);
  //pinMode(enable2, OUTPUT);
  pinMode(LED1pin, OUTPUT);
  //pinMode(LED2pin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
  pinMode(hallSensPin1, INPUT);
  pinMode(timingPin, OUTPUT);
  
  
  randomSeed(analogRead(4));
  
  //lowerStepper.setSpeed(80);
  upperStepper.setSpeed(80);
  
  cueTones[0].begin(speakerPin);
  
  //////////create stimulus array
  int numStimTypes = 4; //whisker only; whisker+tone; whisker+light; whisker+tone+light
  int prsPerStim = 100;
  int numTrials = numStimTypes * prsPerStim;
  int stimArray[numTrials];

  for (int i = 1; i < numStimTypes+1; i++){
    for (int j = 1; j < prsPerStim; j++){
      stimArray[((i-1) * prsPerStim) + j] = i;
    }
  }

  //shuffle stimulus array
  for (int sh = 1; sh < (numStimTypes * prsPerStim) + 1; sh++){
    int holding = stimArray[sh];
    int getRand = random(sh, numTrials + 1);
    int holding2 = stimArray[getRand];
    stimArray[sh] =  holding2;
    stimArray[getRand] = holding;
  }
  
  Serial.begin(9600);
  //Serial.println(stimArray);
  
  //execute the trials
  for (int trialNum = 1; trialNum < (numStimTypes * prsPerStim)+1; trialNum++){
      
      Serial.println("trialnum");
      Serial.println(trialNum);
      Serial.println(stimArray[trialNum]);  
    
      digitalWrite(timingPin, HIGH);
      delay(stimArray[trialNum]*10);
      digitalWrite(timingPin, LOW);
      
      //I'm having the cross-modal stimuli go on AFTER the whisker bar comes down because onset responses from other modalities might be stronger and have a bigger influence?
      
      topStimForward();
      
      //visual: if stimulus type is even, turn on LED pin 
      if(stimArray[trialNum] % 2 == 0){
        digitalWrite(LED1pin, HIGH);
      }
      
      //auditory: turn on white noise stimulus for appropriate stimulustypes
      if(stimArray[trialNum] > 2){
        
        //digitalWrite(solenoidPin, HIGH);
        //delay(20);
        //digitalWrite(solenoidPin, LOW);
        Serial.println("audio trial");
        startTone = millis();
        currTime = millis();
        int elapsedTime = currTime - startTone;
        
        while(elapsedTime < toneDur){ //this while loop generates white noise
          //Serial.println("probe");
          int freq = random(10000,20000);
          cueTones[0].play(freq);
          currTime = millis();
          elapsedTime = currTime - startTone;
        }
        cueTones[0].stop();
        
        //delay(toneDur);
      }
      else {
        Serial.println("no audio trial");
        //digitalWrite(solenoidPin, HIGH);
        //delay(10);
        //digitalWrite(solenoidPin, LOW);
        delay(toneDur);
      }
      
      digitalWrite(LED1pin, LOW);
      
      topStimBack();
      
      //delay(100); 
      
      ///*give reward? choose here which stim types are rewarded
      if (stimArray[trialNum] == 1 || stimArray[trialNum] == 4){
        digitalWrite(solenoidPin, HIGH);
        delay(100);
        digitalWrite(solenoidPin, LOW);
      }
      //*/
      
      iti = random(3000, 10000);
      delay(iti);
  }  
  
}

//////////LOOP
void loop(){
}

//////////various stimulus functions
/*
void botStimForward() {
  Serial.println("botStimForward");
  digitalWrite(enable2, HIGH);
  //digitalWrite(airpuffPin, HIGH);
  delay(100);
  //        upperStepper.step(stepCW); // 042110: now using Stepper library for stim control
  hallSensVal2 = analogRead(hallSensPin2);
  //Serial.println("hallSensVal2init:");
  //Serial.println(hallSensVal2);
  while (hallSensVal2 >hallThresh) {
    lowerStepper.step(1);
    delay(1);  // slight delay after movement to ensure proper step before next
    hallSensVal2 = analogRead(hallSensPin2);
    //Serial.println("hallSensVal2:");
    //Serial.println(hallSensVal2);
    
  }
}
*/

/*
void botStimBack() {
  Serial.println("botStimBack");
  lowerStepper.step(stepCW); // 042110: now using Stepper library for stim control
  delay(200);  
  digitalWrite(enable2, LOW);
}
*/

void topStimForward() {
  digitalWrite(enable1, HIGH);
  //digitalWrite(airpuffPin, HIGH);
  delay(100);
  //        upperStepper.step(stepCW); // 042110: now using Stepper library for stim control
  hallSensVal1 = analogRead(hallSensPin1);
  while (hallSensVal1 >hallThresh) {
    upperStepper.step(-1);
    delay(1);  // slight delay after movement to ensure proper step before next
    hallSensVal1 = analogRead(hallSensPin1);
    //Serial.println(hallSensVal1);
  }
}

void topStimBack() {
  upperStepper.step(stepCCW); // 042110: now using Stepper library for stim control
  delay(200);  
  digitalWrite(enable1, LOW);
}
































