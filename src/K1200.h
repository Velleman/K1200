/******************************************************************/
/*!
	@file     Velleman_K1200.h
	@author   B. Nuyttens (Velleman nv)
	@license  MIT (see license file)

	This is a library for the Velleman K1200 Arduino base watch
	-> http://www.velleman.eu/products/view/?id=438852

	v1.0  - Initial release
*/
/******************************************************************/
#define LIBCALL_ENABLEINTERRUPT
#include <Arduino.h>
#include <Wire.h>                                               //library to talk to PCF8523 Real Time Clock                                             
#include <RTClib.h>       
#define EI_ARDUINO_INTERRUPTED_PIN                                   //library for timekeeping on PCF8523 Real Time Clock 
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>                                       //library to attach a pininterrupt to watchbuttonpin so we 
#include <LowPower.h>                                           //library to configure the atmega328p so it uses almost no power in sleep, for sweet, sweet battery life
#include <avr/power.h>                                          //same as above                              
#include <avr/sleep.h>                                          //same as above




enum watchState {                                              
		SHOW_TIME,
		SET_TIME,
		DO_GAME,
		GO_TO_SLEEP
};
typedef void (*anim_ptr_t)( void );
typedef void (*game_ptr_t)( void );
class Velleman_K1200{
public:
	Velleman_K1200(void);
	void begin(void);
	void setState(watchState state);
	watchState getState();
	void executeState();
	DateTime getTime();
	void setTime(DateTime time);
	void showArray(int showtime);
	void showClock(int showtime);
	void clearArrays(void);
	void notShowingTime(void);
	void configureTime(void);
	void sleep(void);
	bool isButtonPressed(void);
	void setHand(int led, int brightness, bool minutehand, bool hourhand);
	void setAllLeds(int brightness,bool minutes,bool hours);
	//int addAnimation(anim_ptr_t fptr);
	void setBeginAnimation(anim_ptr_t fptr);
	void setEndAnimation(anim_ptr_t fptr);
	void playAnimation(int posAnimation);
	void addGame(game_ptr_t fptr);
	
private:
	const int step = 500;                                                
	const int ledcount  = 12;                                            
	const int longpress  = 40;                                           
	const int clocktime  = 200;                                          
	const int maxAnimations = 10;
	int animationAmount;
	game_ptr_t game;
	anim_ptr_t anim_ptr_array[2];
	RTC_PCF8523 rtc;
	watchState _state;                                         //"state" will be the variable that holds the aformentioned watchstates.

	int _ledpins[12];               //array of the pins that led's are attached to, minute and hour pins are attached to same pin but different current-sink pin
	int _setHourLeds[12];                                            //array to store brightness of hourleds 0-255 (0 = off), gets used by show() function
	int _setMinuteLeds[12];                                          //array to store brightness of minuteleds 0-255 (0 = off), gets used by show() function
	int _hourSinkPin;                                           
	int _minuteSinkPin;
	int _watchButtonPin;
	
	volatile int buttoncounter;                                 //variable to count longpresses

	bool specialhour;                                        //variable that remembers if the "special" hour indicator needs to be shown or not
	bool clockshow;                                         //variable that remembers if clockshow is in progress (is needed to break out of show routine to set the time on long press)	
	
	void checkReset(void);	
	void executeGame(void);
	void printDate(void);
};

static void wakeup()
{
	detachInterrupt(0);
}