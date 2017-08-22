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
#include "K1200.h"
#define DEBUG
Velleman_K1200::Velleman_K1200(){
	_ledpins[0] = A1;
	_ledpins[1] = A2;
	_ledpins[2] = A3;
	_ledpins[3] = 4;
	_ledpins[4] = 5;
	_ledpins[5] = 6;
	_ledpins[6] = 7;
	_ledpins[7] = 8;
	_ledpins[8] = 9;
	_ledpins[9] = A0;
	_ledpins[10] = 2;
	_ledpins[11] = 13;
	
	
	_hourSinkPin = 10;                                           
	_minuteSinkPin = 3;
	_watchButtonPin = 12;
	buttoncounter = 0;                                 //variable to count LONGPRESSes
	specialhour = true;                                        //variable that remembers if the "special" hour indicator needs to be shown or not
	clockshow = false;                                         //variable that remembers if clockshow is in progress (is needed to break out of show routine to set the time on long press)	
}

void Velleman_K1200::begin(){
	
	#ifdef DEBUG
    Serial.begin(57600);
  #endif

  for (int i=0; i < ledcount; i++){
    pinMode(_ledpins[i], OUTPUT);
  } 
  pinMode(_hourSinkPin, OUTPUT);
  pinMode(_minuteSinkPin, OUTPUT);
  pinMode(_watchButtonPin, INPUT);
  
  
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (! rtc.begin()) {
    #ifdef DEBUG
      Serial.println("Couldn't find RTC, check connections and assembly");
    #endif
    while (!isButtonPressed()){
      //anim_vertical();
	  playAnimation(3);
    }
  }

  checkReset();
}

//------------------------------------------------------------------
/*
 * Function:  checkreset 
 * --------------------
 *  Checks if the stored year in the RTC chip is either 2017, 2018 or 2019. If this is the case, state will be set to "SHOW_TIME"
 *  This means the year is "sensible" and the RTC should have the correct data. (Battery can last no longer than 2 years...)
 *  
 *  If the stored year is any other year we assume the date in the chip is incorrect (like after a batteryswap) and the rtc time will be reset to "2017, 0, 0, 0, 0, 0" (Start year will always be 2017 - "set time" routine also uses 2017)
 *  
 *  This routine should be called in setup to detect if an erroneous reset (esd spike) has occured. When the RTC chip still has the correct time but the µ-controller just had a reset the watch will just do a show time instead of asking the user to re-enter the time.
 * 
 *  returns: nothing
 */
//------------------------------------------------------------------
void Velleman_K1200::checkReset(){
	 DateTime now = rtc.now();
  if ((now.year() >= 2017) && (now.year() <= 2019))
  {
    setState(SHOW_TIME);
  }
  else
  {
	playAnimation(1);
    //anim_pop_out();
    rtc.adjust(DateTime(2017, 0, 0, 0, 0, 0));
    setState(SET_TIME);
  }
}

void Velleman_K1200::clearArrays(){
	setAllLeds(0,true,true);
}

//------------------------------------------------------------------
/*
 * Function:  setHand 
 * --------------------
 *  Sets the arrays up to create hour or minute hands. Use this function (or multiple of these) before show() to display your desired hands.
 *  
 *  led = 0 -> 11
 *  brightness = 0 -> 255
 *  minutehand = true/false
 *  hourhand = true/false
 * 
 *  returns: nothing
 */
 //------------------------------------------------------------------
void Velleman_K1200::setHand(int led, int brightness, bool minutehand, bool hourhand){
  
  int handbrightness = map(brightness, 0, 255, 0, step);
  
  //led--; //convert from human form to machine form
	if(led > 11)
		led = 11;
  if (minutehand){
    _setMinuteLeds[led] = handbrightness;
  }

  if (hourhand){
    _setHourLeds[led] = handbrightness;
  }
}

void Velleman_K1200::setAllLeds(int brightness,bool minutes,bool hours){
	int allbrightness = map(brightness, 0, 255, 0, step); 

  if (minutes){
    for (int i=0; i < ledcount; i++){
        _setMinuteLeds[i] = allbrightness;
    }
  }

  if (hours){
    for (int i=0; i < ledcount; i++){
        _setHourLeds[i] = allbrightness;
    } 
  }
}

void Velleman_K1200::setState(watchState state){
	_state = state;
}

watchState Velleman_K1200::getState(){
	return _state;
}

void Velleman_K1200::executeState()
{
	 switch(_state){
    case SHOW_TIME:
      showClock(clocktime);
      break;
    case SET_TIME:
      configureTime();
      break;
    case DO_GAME:
      executeGame();
      break;
    case GO_TO_SLEEP:
      sleep();
      break;
    default:
      _state = SHOW_TIME;
      break;
  }
}

DateTime Velleman_K1200::getTime(){
	return rtc.now();
}

void Velleman_K1200::setTime(DateTime time){
	rtc.adjust(time);
}

//------------------------------------------------------------------
/*
 * Function:  showArray 
 * --------------------
 *  Displays the values in the "set..." arrays for an amount of time (runtime) 
 *  Multiplexes all the leds so dimming is possible. 
 *  
 *  runtime =  how long this function wil run/display
 *  
 *  returns: nothing
 */
 //------------------------------------------------------------------
void Velleman_K1200::showArray(int runtime) {

  for (int a=0; a <= (runtime/10); a++){

    if(isButtonPressed()&&(clockshow == true)){
      buttoncounter++;
      if(buttoncounter >= longpress){
		setState(SET_TIME);
        break;
      } 
    }
      
    digitalWrite(_minuteSinkPin, HIGH);
    for (int i=0; i < ledcount; i++){
      if (_setMinuteLeds[i] != 0)
      {
        digitalWrite(_ledpins[i], HIGH);
      }
      delayMicroseconds(_setMinuteLeds[i]);
      digitalWrite(_ledpins[i], LOW);
      delayMicroseconds(step-_setMinuteLeds[i]);
    }
    digitalWrite(_minuteSinkPin, LOW);
    
    digitalWrite(_hourSinkPin, HIGH);
    for (int i=0; i < ledcount; i++){
      if (_setHourLeds[i] != 0)
      {
        digitalWrite(_ledpins[i], HIGH);
      }
      delayMicroseconds(_setHourLeds[i]);
      digitalWrite(_ledpins[i], LOW);
      delayMicroseconds(step-_setHourLeds[i]);
    }
    digitalWrite(_hourSinkPin, LOW);
  }
}

//------------------------------------------------------------------
/*
 * Function:  doclock 
 * --------------------
 *  Displays the current time with roll in/out animations. Communicates with rtc to get actual time. Wehn bogus data is received "state" will be set to "SET_TIME"
 *  Exit out of routine by buttonpress, go to set time routine by longpress
 *  
 *  showtime = how long actual time wil be displayed between animation
 *  
 *  returns: nothing
 */
 //------------------------------------------------------------------
void Velleman_K1200::showClock(int showtime) {

  int animationspeed = 25;
  int wait = 25;
  int hours;
  int minutes;
  
  clockshow = true;
  setState(GO_TO_SLEEP);
  
  //anim_pop_out();
  playAnimation(1);
  
  DateTime now = getTime();
  hours = now.hour();
  minutes = now.minute();
 
  //bringing back from 24h notation
  if (hours > 12){
    hours = hours-12;
  }

  //scaling down to array size 
  hours = hours - 1;
  minutes = (minutes/5) - 1;
  
  //fixing zero
    
  if (minutes < 0){
	  minutes = 1;
  }
  if (hours < 0){
    hours = 11;
  }

  if ((hours > 11) || (minutes > 11)){
	setState(SET_TIME);
    return;
  }

  clearArrays();
  
  //animating in
  setHand(11,255, false, true);
  showArray(animationspeed);
  clearArrays();
  for (int h=0; h < hours; h++){
    setHand(h,255, false, true);
    showArray(animationspeed); 
    clearArrays();
  }
  setHand(hours,255, false, true); 
  
  //wait between hands
  showArray(wait);
  
  setHand(11,255, true, true);
  setHand(hours,255, false, true); 
  showArray(animationspeed);
  clearArrays();
  for (int m=0; m < minutes; m++){
    setHand(m,255, true, true); 
    setHand(hours,255, false, true); 
    showArray(animationspeed); 
    clearArrays();
  }

  //setting hours
  setHand(hours,255, false, true);

  if(specialhour == true){
    int hourplusone = hours+1;
    if (hourplusone == 12){
      hourplusone = 0;
    }
    if ((minutes > 5) && (minutes < 11)){
      setHand(hourplusone,10, false, true);
    }
  }

  //setting minutes
  setHand(minutes,255, true, true);

  //displaying time
  for (long i=0; i <= showtime; i++){
    showArray(0);
    if(isButtonPressed())
    {
      break;
    }
  }

  //animating out
  for (int h=hours; h < ledcount; h++){
    setHand(h,255, false, true);
    setHand(minutes,255, true, true);
    showArray(animationspeed); 
    clearArrays();
  }
  
  for (int m = minutes; m < ledcount; m++){
    setHand(m,255, true, true); 
    showArray(animationspeed); 
    clearArrays();
  }
  setHand(11,255, true, true);
  showArray(wait);
  clearArrays();
  delay(wait);
  //anim_pop_in();
  playAnimation(2);

  clockshow = false;
  buttoncounter = 0;
  while(isButtonPressed()){
    //wait...
  }
}

void Velleman_K1200::notShowingTime()
{
	clockshow = false;
}

//------------------------------------------------------------------
/*
 * Function:  setclock 
 * --------------------
 *  Sets the time
 *  1. set the hours (longpress to confirm)
 *  2. set the minutes (longpress to confirm) 
 *   - Watch will start counting time from this point.
 *  3. set the "special hour " indicator (see user-manual to learn what this indicator does)
 *  
 *  returns: nothing
 */
 //------------------------------------------------------------------
void Velleman_K1200::configureTime()
{
  int newhours = 11;
  int newminutes = 11;

  DateTime now = getTime();
  printDate();
  newhours = now.hour();
  newminutes = now.minute();

  //bringing back from 24h notation
  if (newhours > 12){
    newhours = newhours-12;
  }

  //scaling down to array size 
  newhours = newhours-1;
  newminutes = (newminutes/5)-1;

  //fixing zero
  if (newminutes < 0){
    newminutes = 11;
  }
  if (newhours < 0){
    newhours = 11;
  }

  if ((newhours > 11) || (newminutes > 11)){
    setState(SET_TIME);
    return;
  }
  
  clearArrays();
  //setting hours
  while(1){ 
    while(isButtonPressed()){
      buttoncounter++;
      setHand(newhours,255, false, true);
      setHand(newminutes,255, true, true);
      if(buttoncounter < longpress)
      {
          showArray(0);
      }
    } 
    clearArrays();
    if((buttoncounter < longpress)  && (buttoncounter > 0))
    {
      newhours++;
      buttoncounter = 0;
    }
    if(buttoncounter >= longpress)
    {
      buttoncounter = 0;
      break;
    }
    
    if (newhours >= 12){
      newhours = 0;
    }
    setHand(newhours,255, false, true);
    setHand(newminutes,255, true, true);
    showArray(75);
    delay(75);
  }
  //anim_pop_in();
  playAnimation(2);
  
  //setting minutes
  while(1){ 
    while(isButtonPressed()){
      buttoncounter++;
      setHand(newhours,255, false, true);
      setHand(newminutes,255, true, true);
      if(buttoncounter < longpress)
      {
		showArray(0);
      }
    } 
    clearArrays();
    if((buttoncounter < longpress)  && (buttoncounter > 0))
    {
      newminutes++;
      buttoncounter = 0;
    }
    if(buttoncounter >= longpress)
    {
      buttoncounter = 0;
      break;
    }
    
    if (newminutes >= 12){
      newminutes = 0;
    }
    setHand(newhours,255, false, true);
    setHand(newminutes,255, true, true);
    showArray(75);
    delay(75);
  }
  //anim_pop_in();
  playAnimation(2);

  newminutes++;
  newhours++;

  if (newminutes >= 12){
      newminutes = 0;
    }
  if (newhours >= 12){
      newhours = 12;
    }
    
  newminutes = newminutes*5;

	setTime(DateTime(2017, 0, 0, newhours, newminutes, 0));
  
  //setting special hours
  while(1){ 
    while(isButtonPressed()){
      buttoncounter++;
      setHand(9,255, true, true);
      setHand(5,255, false, true);
      if(specialhour == true){
        setHand(6,10, false, true);
      }
      if(buttoncounter < longpress)
      {
          showArray(0);
      }
    } 
    clearArrays();
    if((buttoncounter < longpress)  && (buttoncounter > 0))
    {
      specialhour = !specialhour;
      buttoncounter = 0;
    }
    if(buttoncounter >= longpress)
    {
      buttoncounter = 0;
      break;
    }
    setHand(9,255, true, true);
    setHand(5,255, false, true);
    if(specialhour == true){
      setHand(6,10, false, true);
    }
    showArray(75);
    delay(75);
  }
  //anim_pop_in();
  playAnimation(2);
  setState(GO_TO_SLEEP);
}

//------------------------------------------------------------------
/*
 * Function:  sleepnow 
 * --------------------
 *  When this function is called an intterupt will be attached to watchbutton, processor will be put in low power config, and will enter sleepmode.
 *  The watch will use very little power now.
 *  Processor can only be woken up by either a reset or pressing the watchbutton.
 *  
 *  When waking up processor will continue from "sleep_disable();" line
 *  
 *  1 press -> state = SHOW_TIME
 *  
 *  2 presses -> state = DO_GAME
 *   
 *  returns: nothing
 */
 //------------------------------------------------------------------
void Velleman_K1200::sleep(){

  // debugging
  #ifdef DEBUG
    Serial.println("Watch going to sleep");
    delay(5);
  #endif
  
  // Attach interrupt to pin so we can wakup the device
  //attachPinChangeInterrupt(_watchButtonPin, wakeup, RISING);
  enableInterrupt(_watchButtonPin,wakeup,RISING);
  
  // Choose our preferred sleep mode:
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Set sleep enable (SE) bit:
  sleep_enable();

  // Put the device to sleep:
  sleep_mode();
  
  // Upon waking up, watch continues from this point.
  sleep_disable();

  // Detaching interrupt so we can use the button
  //detachPinChangeInterrupt(_watchButtonPin);
  disableInterrupt(_watchButtonPin);

  // debugging
  #ifdef DEBUG
    Serial.println("Watch woke up");
  #endif
  
  // check for second press after wakeup
  delay(200);
  while(1){
    buttoncounter++;
    delay(1);
    if(buttoncounter >= 200)
    {
      buttoncounter = 0;
      setState(SHOW_TIME);
      break;
    }
    if(isButtonPressed())
    {
      buttoncounter = 0;
      setState(DO_GAME);
      break;
    }
  }
  
}


bool Velleman_K1200::isButtonPressed(){
	return digitalRead(_watchButtonPin);
}

void Velleman_K1200::setBeginAnimation(anim_ptr_t fptr)
{
	anim_ptr_array[0] = fptr;
}

void Velleman_K1200::setEndAnimation(anim_ptr_t fptr)
{
	anim_ptr_array[1] = fptr;
}

void Velleman_K1200::playAnimation(int posAnimation)
{
	if(posAnimation > 0)
	{
		anim_ptr_array[posAnimation-1]();
	}
}

void Velleman_K1200::addGame(game_ptr_t fptr){
	game = fptr;
}

void Velleman_K1200::executeGame(){
	if(game != NULL)
		game();
}

void Velleman_K1200::printDate(){
	#ifdef DEBUG
	char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	DateTime now = rtc.now();
	Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
	#endif
}