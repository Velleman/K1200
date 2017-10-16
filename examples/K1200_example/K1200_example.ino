//------------------------------------------------------------------
/*
    Velleman K1200 Watch Firmware
    Version 1.0
    Written by: JGE,BN
    20-03-2017

    Written in Arduino 1.8.1

	Board: LilyPad Arduino 
	Processor: ATmega328
*/
//------------------------------------------------------------------
#define EI_ARDUINO_INTERRUPTED_PIN
#define EI_NOTEXTERNAL
#include "EnableInterrupt.h"
#include "LowPower.h"                                           //library to configure the atmega328p so it uses almost no power in sleep, for sweet, sweet battery life
#include <avr/power.h>                                          //same as above                              
#include <avr/sleep.h>   
#include "K1200.h"

#define GAMEWINS 10 

enum gameState{
  none,
  click,
  sleep
};

Velleman_K1200 watch = Velleman_K1200();
bool isShownHourArray[12];
bool isShownMinuteArray[12];
//------------------------------------------------------------------
/*
   Function:  setup
   --------------------
    Runs once on startup and on every reset.
    When the watch resets due to esd spike it will check if the stored year is between 2017 <-> 2019 it will set the "state" to "SHOW_TIME" to display time.
    As the year value is not used on the watch it can be used to "detect" if the time has been set, if it would not be set it would be a random value and then the "state" will be set to "SET_TIME" and the watch will go to the "set the time" routine after initializing the time to "0"

    This way if the watch experiences a reset of the microcontroller it is not needed to re-enter the time. As the RTC will still have the correct time. Only on battery entry it will default to the "set the time" routine

    returns: nothing
*/
//------------------------------------------------------------------
void setup() {
  watch.setBeginAnimation(anim_pop_out);
  watch.setEndAnimation(anim_pop_in);
  watch.addGame(game);
  watch.begin();  
}
//------------------------------------------------------------------
/*
   Function:  loop
   --------------------
   Execute the current state, you can get the current state by calling the function getState.

    returns: nothing
*/
//------------------------------------------------------------------
void loop() {
  watch.executeState();
}
//------------------------------------------------------------------
/*
   Function:  game
   --------------------
    When called you can only exit by either long press or by 10 consecutive loses
    Try to press the button when moving paddle reaches 12 hour mark.

    Horizontal animation -> lose
    Vertical animation -> win

    10 consecutive wins -> fireworks!

    returns: nothing
*/
//------------------------------------------------------------------
void game() {

  bool gamewin = false;
  bool endgame = false;
  int gamecounter = 0;
  int inactiveCounter = 0;
  int wincounter = 0;
  int gametimeout = 100;
  int maxInactiveAttempts = 5;
  int randnumber = 0;
  int seednumber = 0;
  int btnCounter = 0;
  
  watch.notShowingTime();

  anim_horizontal();
  anim_vertical();

  DateTime now = watch.getTime();
  seednumber = now.second();
  randomSeed(seednumber);

  delay(100);

  watch.setHand(11, 255, true, true);
  watch.showArray(50);

  while (endgame == false) {
    randnumber = random(1, 6);
    inactiveCounter++;

    if (inactiveCounter > maxInactiveAttempts) {
      inactiveCounter = 0;
      endgame = true;
    }

    if (endgame == true) {
      anim_pop_in();
      break;
    }
    
    gameState state = showGameAnim(randnumber,true);
    switch(state)
    {
      case none: //nothing happend, continue
        break;
      case click: //the button was clicked, before the hand reached the middle, it's a failed attempt , the user reacted so reset the fail counter
        inactiveCounter = 0; //user reacted
        gamecounter = gametimeout; //failed attempt set the the gamecounter equal to the timeout value
        break;
      case sleep:
        endgame =true;
        break;
    }
    
    while (1) {
      gamecounter++;
      delay(1);
      if (gamecounter >= gametimeout) {
        gamewin = false;
        gamecounter = 0;
        break;
      }
      if (watch.isButtonPressed()) {
        inactiveCounter = 0;
        gamewin = true;
        gamecounter = 0;
        break;
      }
    }
    if ((gamewin == true) && (endgame == false)) {
      anim_vertical();
      wincounter++;
    }
    else if ((gamewin == false) && (endgame == false)) {
      anim_horizontal();
      wincounter = 0;
    }

    if (wincounter >= GAMEWINS) {
      anim_fireworks();
      wincounter = 0;
      delay(500);
    }

    if (endgame == true) {
      break;
    }
    state = showGameAnim(randnumber,true);
    switch(state)
    {
      case none:
        break;
      case click:
        inactiveCounter = 0;
        gamecounter = gametimeout;
        break;
      case sleep:
        endgame =true;
        break;
    }
      
    while (1) {
      gamecounter++;
      delay(1);
      if (gamecounter >= gametimeout) {
        gamewin = false;
        gamecounter = 0;
        break;
      }
      if (watch.isButtonPressed()) {
        inactiveCounter = 0;
        gamewin = true;
        gamecounter = 0;
        break;
      }
    }
    if ((gamewin == true) && (endgame == false)) {
      anim_vertical();
      wincounter++;
    }
    else if ((gamewin == false) && (endgame == false)) {
      anim_horizontal();
      wincounter = 0;
    }

    if (wincounter >= GAMEWINS) {
      anim_fireworks();
      wincounter = 0;
      delay(500);
    }
  }
  btnCounter = 0;
  watch.setState(GO_TO_SLEEP);
}

//------------------------------------------------------------------
/*
   Function:  anim_pop_out
   --------------------
    "Pop out of controller" animation

    returns: nothing
*/
//------------------------------------------------------------------
void anim_pop_out() {
  watch.clearArrays();
  for (int i = 0; i <= 255; i = i + 20) {
    watch.setAllLeds(i, false, true);
    watch.showArray(0);
  }
  for (int i = 255; i > 0; i = i - 20) {
    watch.setAllLeds(i, false, true);
    watch.showArray(0);
  }
  watch.clearArrays();
  for (int i = 0; i <= 255; i = i + 20) {
    watch.setAllLeds(i, true, false);
    watch.showArray(0);
  }
  for (int i = 255; i > 0; i = i - 20) {
    watch.setAllLeds(i, true, false);
    watch.showArray(0);
  }
  watch.clearArrays();
}
//------------------------------------------------------------------
/*
   Function:  anim_pop_in
   --------------------
    "Pop into controller" animation

    returns: nothing
*/
//------------------------------------------------------------------
void anim_pop_in() {
  watch.clearArrays();
  for (int i = 0; i <= 255; i = i + 20) {
    watch.setAllLeds(i, true, false);
    watch.showArray(0);
  }
  for (int i = 255; i > 0; i = i - 20) {
    watch.setAllLeds(i, true, false);
    watch.showArray(0);
  }
  watch.clearArrays();

  for (int i = 0; i <= 255; i = i + 20) {
    watch.setAllLeds(i, false, true);
    watch.showArray(0);
  }
  for (int i = 255; i > 0; i = i - 20) {
    watch.setAllLeds(i, false, true);
    watch.showArray(0);
  }
  watch.clearArrays();
}
//------------------------------------------------------------------
/*
   Function:  anim_vertical
   --------------------
    "Vertical" animation

    returns: nothing
*/
//------------------------------------------------------------------
void anim_vertical() {
  watch.clearArrays();
  for (int i = 0; i <= 255; i = i + 10) {
    watch.setHand(11, i, true, true);
    watch.setHand(5, i, true, true);
    watch.showArray(0);
  }
  for (int i = 255; i > 0; i = i - 10) {
    watch.setHand(11, i, true, true);
    watch.setHand(5, i, true, true);
    watch.showArray(0);
  }
  watch.clearArrays();
}
//------------------------------------------------------------------
/*
   Function:  anim_horizontal
   --------------------
    "Horizontal" animation

    returns: nothing
*/
//------------------------------------------------------------------
void anim_horizontal() {
  watch.clearArrays();
  for (int i = 0; i <= 255; i = i + 10) {
    watch.setHand(2, i, true, true);
    watch.setHand(8, i, true, true);
    watch.showArray(0);
  }
  for (int i = 255; i > 0; i = i - 10) {
    watch.setHand(2, i, true, true);
    watch.setHand(8, i, true, true);
    watch.showArray(0);
  }
  watch.clearArrays();
}
//------------------------------------------------------------------
/*
   Function:  anim_fireworks
   --------------------
    "Firework" animation

    returns: nothing
*/
//------------------------------------------------------------------
void anim_fireworks() {
  watch.clearArrays();

  DateTime now = watch.getTime();
  int seed = now.second();
  randomSeed(seed);

  for (int i = 0; i <= 75; i++) {
    switch (random(0, 2)) {
      case 0:
        watch.setHand(random(0, 11), 255, true, false);
        watch.showArray(10);
        watch.clearArrays();
        delay(75);
        break;
      case 1:
        watch.setHand(random(0, 11), 255, false, true);
        watch.showArray(10);
        watch.clearArrays();
        delay(75);
        break;
    }
  }
  watch.clearArrays();
}

gameState showGameAnim(int speedAnim,bool clockwise)
{
  int btnCounter=0;
  for (int i = 0; i < 12; i++) {
      if (watch.isButtonPressed()) {
        while (watch.isButtonPressed()) {
          btnCounter++;
          delay(1);
          if (btnCounter >= 500) {
            btnCounter = 0;
            anim_pop_in();
            watch.setState(GO_TO_SLEEP);
            return sleep;
          }
        }
        btnCounter = 0;
        return click;
      }
      if(clockwise)
        watch.setHand(i, 255, true, true);
      else
        watch.setHand(11-i,255,true,true);
      watch.setHand(11, 255, true, true);
      watch.showArray(speedAnim * 20);
      watch.clearArrays();
    }
    return none;
}

void anim_test()
{
    watch.clearArrays();
    watch.setAllLeds(255,true,true);
  for (int i = 0; i <= 10; i++) {
    watch.setHand(i,0, true, false);
    watch.setHand(10 - i,0, false, true);
    watch.showArray(83);
  }
  watch.setHand(11,0, true, true);
  watch.showArray(83);
  watch.clearArrays();
}

void anim_test2()
{
    watch.clearArrays();

  for (int i = 10; i >= 0; i--) {
    watch.setHand(i,255, true, false);
    watch.setHand(10 - i,255, false, true);
    watch.showArray(83);
  }
  watch.setHand(11,255,true,true);
  watch.showArray(83);
  watch.clearArrays();
}

void anim_test3(){
  watch.clearArrays();
  for(int i = 2; i<=8;i++)
  {
    watch.setHand(i,255,true,true);
    int otherSide = i+6;
    if(otherSide>11)
    {
      otherSide-=12;
    }
    watch.setHand(otherSide,255,true,true);
    watch.showArray(100);
  }
  watch.clearArrays();
}

void anim_test4()
{
  watch.clearArrays();
  watch.setHand(0,5,true,true);
  watch.showArray(100);
  for(int i=0; i <= 11;i++)
  {
      watch.setHand(i,i*10,true,true);
      watch.showArray(100);
  }
  watch.showArray(1000);
  watch.clearArrays();
}


void anim_test5(){
  watch.clearArrays();
  for(int i = 0; i<6;i++)
  {
    watch.setHand(i,255,true,true);
    int otherSide = i+6;
    if(otherSide>11)
    {
      otherSide-=12;
    }
    watch.setHand(otherSide,255,true,true);
    watch.showArray(83);
    watch.clearArrays();
  }
  watch.clearArrays();
}

void anim_test6(){
  watch.clearArrays();
  watch.setAllLeds(255,true,true);
  watch.showArray(83);
  for(int i=0;i<11;i++)
  {
    watch.setHand(i,0,true,true);
    watch.showArray(83);
  }
  delay(200);
  watch.clearArrays();
}

void anim_test7(){
  //watch.clearArrays();
  watch.setHand(11,255,true,true);
  watch.showArray(83);
  for(int i=0;i<11;i++)
  {
    watch.setHand(i,255,true,true);
    watch.showArray(83);
  }
  delay(200);
  watch.clearArrays();
}

void smiley(){
  smallSmile();
  bigSmile();
  smallSmile();
}

void bigSmile(){
  watch.clearArrays();
  watch.setHand(1,255,true,false);
  watch.setHand(9,255,true,false);
  watch.setHand(3,255,true,false);
  watch.setHand(4,255,true,false);
  watch.setHand(5,255,true,false);
  watch.setHand(6,255,true,false);
  watch.setHand(7,255,true,false);
  watch.showArray(1000);
  watch.clearArrays();
}

void smallSmile(){
  watch.clearArrays();
  watch.setHand(0,255,false,true);
  watch.setHand(10,255,false,true);
  watch.setHand(3,255,false,true);
  watch.setHand(4,255,false,true);
  watch.setHand(5,255,false,true);
  watch.setHand(6,255,false,true);
  watch.setHand(7,255,false,true);
  watch.showArray(1000);
  watch.clearArrays();
}

void randomFill(){
  long rand1 =0;
  for(int i =0;i<12;i++)
  {
    isShownHourArray[i] = false;
    isShownMinuteArray[i] = false;
  }
  while(!isFilled()){
    rand1 = random(24);
    if(rand1 > 11)
    {
      if(!isShown(rand1,false))
      {
        watch.setHand(rand1 - 12,255,true,false);
        
      }
    }
    else{
      if(!isShown(rand1,true))
      {
        watch.setHand(rand1,255,false,true);
        
      }
    }
    watch.showArray(100);
  }
  for(int i =0;i<12;i++)
  {
    isShownHourArray[i] = false;
    isShownMinuteArray[i] = false;
  }
}

bool isShown(long number,bool isHour)
{
  if(isHour)
  {
    if(isShownHourArray[number])
    {
      return true;
    }
    else{
      isShownHourArray[number] = true;
      return false;
    }
  }
  else{
    number -= 12;
    if(isShownMinuteArray[number])
    {
      return true;
    }
    else{
      isShownMinuteArray[number] = true;
      return false;
    }
  }
}

bool isFilled(){
  for(int i=0; i<12;i++){
    if(!isShownHourArray[i] || !isShownMinuteArray[i])
     {
        return false;
     }
  }
  return true;
}

