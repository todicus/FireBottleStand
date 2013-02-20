#include <SoftPWM.h>
#include <Wtv020sd16p.h>

const int numModes = 2;
int flashMode = 2 ;  // flash animation style
// 0: circular
// 1: random
// 2: random pick 2

long seqTime = 60000;
int rampTime = 30000;

// LEDs:
  int maxBright = 255;
  int delayTime = 1000;  // msec btw LED flashes
  int fadeUpT   = 1200;
  int fadeDownT = 2000;
  
  const int numBottleLEDs = 9;
  uint8_t shotLEDs[numBottleLEDs] = {A5,0,1,2,3,4};
  const int numShotLEDs = 6;
  uint8_t bottleLEDs[numBottleLEDs] = {5,6,7,8,9,10,11,12,13};    // as ordered will go in circle
  int curLED   = 0;
  int oldLED   = 0;
  int oldLED2  = 0;
  int curShotLED = 0;
  int oldShotLED = 0;

// Sound:
  int curVol = 7;
  int numSongs = 5;
  int songNum = -1;
  int playLoops = 0;
  long playMsecs = 0;
  int resetPin  = A1;  // The pin number of the reset pin. 4
  int clockPin  = A2;  // The pin number of the clock pin. 5
  int dataPin   = A3;  // The pin number of the data pin.  3
  int busyPin   = A4;  // The pin number of the busy pin.  2
  Wtv020sd16p wtv020sd16p(resetPin, clockPin, dataPin, busyPin);
  boolean soundOn = false;

// Oher:
  boolean lightChange = false;
  boolean lightsOn    = false;
  long lastT = 0;
  
  int buttonState;
  int lastButtonState = HIGH;

void setup() {
  // Sound:
    wtv020sd16p.reset();
    wtv020sd16p.setVol(curVol);
    //Serial.println("play 0 async");
    //wtv020sd16p.asyncPlayVoice(songNum);
  
  // Get random seed for RNG from analog port
    randomSeed(analogRead(0));
  
  // Setup PWM:
    SoftPWMBegin();
    for (int i = 0; i < numBottleLEDs; i++) {
      SoftPWMSet(bottleLEDs[i], 0);
    }
    for (int i = 0; i < numShotLEDs; i++) {
      SoftPWMSet(shotLEDs[i], 0);
    }
    SoftPWMSetFadeTime(ALL, fadeUpT, fadeDownT);
  
  // Set LED pins to output
    for(int i=0; i<13; i++){
      pinMode(i, OUTPUT);
    }
    pinMode(A5, OUTPUT);
  
  // Set button pin to input
    pinMode(A0, INPUT);
    digitalWrite(A0, HIGH); // enable pullup resistor
  
  // Serial:
    Serial.begin(38400);
    Serial.println("setup");
}

void loop() {
  long dt = millis() - lastT;
  
  if(dt < rampTime) {
    lightsOn = true;
    maxBright = 255 - int(255.0*((float)dt/(float)seqTime));
    delayTime = int(dt / 30) + 30;
    fadeUpT   = int(dt / 10) + 200;
    fadeDownT = int(dt / 4) + 200;
  }
  else if(dt < seqTime) {
    lightsOn = true;
    //maxBright = 2*sqrt(255 - int(255.0*((float)dt/(float)seqTime)));
    //maxBright = 255;
    //Serial.print("max: ");
    //Serial.println(maxBright);
    delayTime = 200;
    fadeUpT   = 300;
    fadeDownT = 500;
  }
  else {
    if(lightsOn) {
      for (int i = 0; i < numBottleLEDs; i++) {
        SoftPWMSet(bottleLEDs[i], 0);
      }
    
      for (int i = 0; i < numShotLEDs; i++) {
        SoftPWMSet(shotLEDs[i], 0);
      }
      lightsOn = false;
      Serial.println("off");
    }
  }
  
  if(lightsOn){
    SoftPWMSetFadeTime(ALL, fadeUpT, fadeDownT);
    switch(flashMode) {
      case 0:
        SoftPWMSet(bottleLEDs[curLED], maxBright);
        SoftPWMSet(bottleLEDs[oldLED], 0);
        oldLED = curLED;
        curLED = (curLED + 1) % (numBottleLEDs);
        Serial.println(bottleLEDs[curLED]);
        delay(delayTime);
        break;
      case 1:
        SoftPWMSet(bottleLEDs[oldLED], 0);          // turn off old LED
        curLED = random(0, numBottleLEDs);
        SoftPWMSet(bottleLEDs[curLED], maxBright);  // turn on new LED
        oldLED = curLED;
        Serial.println(bottleLEDs[curLED]);
        delay(delayTime);
        break;
      case 2:
        SoftPWMSet(bottleLEDs[oldLED], 0);  // turn off old LED
        SoftPWMSet(bottleLEDs[oldLED2], 0); // turn off old LED
        curLED = random(0, numBottleLEDs);
        SoftPWMSet(bottleLEDs[curLED], maxBright);  // turn on new LED
        oldLED = curLED;
        curLED = random(0, numBottleLEDs);
        SoftPWMSet(bottleLEDs[curLED], maxBright);  // turn on new LED
        oldLED2 = curLED;
        //Serial.println(bottleLEDs[curLED]);
        
        // shot glass LEDs
        //curShotLED = curShotLED+1 % numShotLEDs;
        curShotLED = random(0, numShotLEDs);
        Serial.println(shotLEDs[curShotLED]);
        SoftPWMSet(shotLEDs[oldShotLED], 0); // turn off old LED
        SoftPWMSet(shotLEDs[curShotLED], maxBright);
        oldShotLED = curShotLED;
        
        delay(delayTime);
        break;
    }
  }
  
  /*// switch modes every so often
  if(millis() % 5000 < 10) {
    flashMode = (flashMode+1) % (numModes-1);
    Serial.println("5 sec!");
  }*/
  
  // Play sound
  if(soundOn) {
    if(playLoops == 0) {
      if(digitalRead(busyPin) == LOW) {
        Serial.print("playing #");
        Serial.println(songNum);
        wtv020sd16p.setVol(curVol);
        wtv020sd16p.asyncPlayVoice(songNum);
      }
      else {
        playLoops++;
        Serial.print(songNum);
        Serial.print(", ");
        Serial.println(playLoops);
        //delay(1000);
      }
    }
    else {
      if(digitalRead(busyPin) == HIGH) {
        playLoops++;
        //Serial.print("loops ");
        //Serial.println(playLoops);
        //delay(1000);
      }
      else {
        Serial.println("song finished");
        //songNum++;
        //Serial.print("playing song ");
        //Serial.println(songNum);
        playLoops = 0;
        soundOn = false;
      }
    }
  }
        
  // check button
  buttonState = digitalRead(A0);
  if(buttonState != lastButtonState) {
    delay(2);
    if(buttonState == digitalRead(A0)) {
      lastButtonState = buttonState;
      if(buttonState == LOW) {
        //ledState ^= 1;                // toggle
        Serial.println("undocked");     //
        lastT = millis();               // reset timer
        songNum = (songNum+1) % numSongs; // change sound
        soundOn = true;
      }
      // If button down
      else {
        Serial.println("docked");
        Serial.println(digitalRead(busyPin));
      }
    }
  }
  
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if(inChar == 'e') {
      //char eventNum = (char)Serial.read();
      //if(eventNum == '1') {
        lastT = millis();
        Serial.println("LDR");
      //}
    }
  }
}
