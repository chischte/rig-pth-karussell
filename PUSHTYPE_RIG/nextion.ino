/*
* *****************************************************************************
* nextion.ino
* configuration of the Nextion touch display
* Michael Wettstein
* November 2018, Zürich
* *****************************************************************************
*/


//#include <Cylinder.h>
//#include <Nextion.h>

// Include the nextion library (the official one) https://github.com/itead/ITEADLIB_Arduino_Nextion
// Make sure you edit the NexConfig.h file on the library folder to set the correct serial port for the display.
// By default it's set to Serial1, which most arduino boards don't have.
// Change "#define nexSerial Serial1" to "#define nexSerial Serial" if you are using arduino uno, nano, etc.
//**************************************************************************************
//VARIOUS COMMANDS:
//**************************************************************************************
// Serial2.print("click bt1,1");//CLICK BUTTON
// send_to_nextion();
// A switch (Dual State Button)will be toggled with this command, a Button wil be set permanently pressed)

// Serial2.print("vis t0,0");//HIDE OBJECT
// send_to_nextion();

// Serial2.print("t0.txt=");//WRITE TEXT:
// Serial2.print("\"");
// Serial2.print("SCHNEIDEN");
// Serial2.print("\"");
// send_to_nextion();
//**************************************************************************************
//DECLARATION OF VARIABLES
//**************************************************************************************
int CurrentPage;
String cycle_name[] = {"1 KLEMMEN", "2 FALLENLASSEN", "3 AUSFAHREN", "4 BAND UNTEN", "5 BAND OBEN", "6 PRESSEN", "7 SCHNEIDEN", "8 ZURUECKFAHREN", "9 REVOLVER", "RESET"};
//***************************************************************************
//NEXTION SWITCH STATES LIST
//Every nextion switch button (dualstate) needs a switchstate variable to control switchtoggle
//Nextion buttons(momentary) need a variable too, to prevent screen flickering

bool nex_state_zyl_gummihalter;
bool nex_state_zyl_falltuerschieber;
bool nex_state_zyl_magnetarm;
bool nex_state_mot_feed_oben;
bool nex_state_mot_feed_unten;
bool nex_state_zyl_messer;
bool nex_state_zyl_revolverschieber;
bool nex_state_machine_running;
bool nex_state_seal_available = 1;
//***************************************************************************
bool nex_prev_step_mode = true;
bool stopwatch_running;
bool reset_stopwatch_active;
unsigned int nex_prev_upper_feedtime;
unsigned int nex_prev_lower_feedtime;
unsigned int stopped_button_pushtime;
unsigned long nex_prev_shorttime_counter;
unsigned long nex_prev_longtime_counter;
unsigned long button_push_stopwatch;
unsigned long counter_reset_stopwatch;

//**************************************************************************************
//DECLARATION OF OBJECTS TO BE READ FROM NEXTION
//**************************************************************************************

//PAGE 0:
NexPage nex_page0 = NexPage(0, 0, "page0");

//PAGE 1 - LEFT SIDE:
NexPage nex_page1 = NexPage(1, 0, "page1");
NexButton nex_but_stepback = NexButton(1, 6, "b1");
NexButton nex_but_stepnxt = NexButton(1, 7, "b2");
NexButton nex_but_reset_cycle = NexButton(1, 5, "b0");
NexDSButton nex_switch_play_pause = NexDSButton(1, 2, "bt0");
NexDSButton nex_switch_mode = NexDSButton(1, 4, "bt1");

//PAGE 1 - RIGHT SIDE
NexPage nex_page2 = NexPage(2, 0, "page2");
NexDSButton nex_zyl_gummihalter = NexDSButton(1, 15 , "bt5");
NexDSButton nex_zyl_falltuer = NexDSButton(1, 14 , "bt4");
NexDSButton nex_zyl_magnetarm = NexDSButton(1, 13, "bt3");
NexButton nex_mot_band_oben = NexButton(1, 12 , "b5");
NexButton nex_mot_band_unten = NexButton(1, 11 , "b4");
NexDSButton nex_zyl_messer = NexDSButton(1, 10, "bt2");
NexButton nex_zyl_revolverschieber = NexButton(1, 9 , "b3");

//PAGE 2 - LEFT SIDE:
NexButton nex_but_slider1_left = NexButton(2, 6, "b1");
NexButton nex_but_slider1_right = NexButton(2, 7, "b2");
NexButton nex_but_slider2_left = NexButton(2, 9 , "b0");
NexButton nex_but_slider2_right = NexButton(2, 11 , "b3");

//PAGE 2 - RIGHT SIDE:
NexButton nex_but_reset_shorttime_counter = NexButton(2, 18, "b4");

//**************************************************************************************
//END OF OBJECT DECLARATION
//**************************************************************************************

char buffer[100] = {0};  // This is needed only if you are going to receive a text from the display. You can remove it otherwise.

//**************************************************************************************
//TOUCH EVENT LIST //DECLARATION OF TOUCH EVENTS TO BE MONITORED
//**************************************************************************************
NexTouch *nex_listen_list[] =
{
  &nex_but_reset_shorttime_counter,
  &nex_but_stepback,
  &nex_but_stepnxt,
  &nex_but_reset_cycle,
  &nex_but_slider1_left,
  &nex_but_slider1_right,
  &nex_but_slider2_left,
  &nex_but_slider2_right,
  &nex_switch_play_pause,
  &nex_switch_mode,
  &nex_zyl_messer,
  &nex_zyl_magnetarm,
  &nex_page0,
  &nex_page1,
  &nex_page2,
  &nex_zyl_gummihalter,
  &nex_zyl_falltuer,
  &nex_mot_band_oben,
  &nex_mot_band_unten,
  &nex_zyl_revolverschieber,

  NULL //String terminated
};
//**************************************************************************************
//END OF TOUCH EVENT LIST
//**************************************************************************************

//**************************************************************************************
int send_to_nextion()
{
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
void nextion_setup()
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
{ //START NEXTION SETUP
  Serial2.begin(9600);  // Start serial comunication at baud=9600

  //**************************************************************************************
  //INCREASE BAUD RATE
  //**************************************************************************************
  /*
    delay(500);
    Serial2.print("baud=38400");
    send_to_nextion();
    Serial2.end();
    Serial2.begin(38400);
  */
  //**************************************************************************************
  //REGISTER THE EVENT CALLBACK FUNCTIONS
  //**************************************************************************************


  nex_but_stepback.attachPush(nex_but_stepbackPushCallback);
  nex_but_stepnxt.attachPush(nex_but_stepnxtPushCallback);
  nex_zyl_magnetarm.attachPush(nex_zyl_magnetarmPushCallback);
  nex_but_reset_cycle.attachPush(nex_but_reset_cyclePushCallback);
  nex_but_slider1_left.attachPush(nex_but_slider1_leftPushCallback);
  nex_but_slider1_right.attachPush(nex_but_slider1_rightPushCallback);
  nex_but_slider2_left.attachPush(nex_but_slider2_leftPushCallback);
  nex_but_slider2_right.attachPush(nex_but_slider2_rightPushCallback);
  nex_but_stepback.attachPush(nex_but_stepbackPushCallback);
  nex_but_stepnxt.attachPush(nex_but_stepnxtPushCallback);
  nex_page0.attachPush(nex_page0PushCallback);
  nex_page1.attachPush(nex_page1PushCallback);
  nex_page2.attachPush(nex_page2PushCallback);
  nex_switch_mode.attachPush(nex_switch_modePushCallback);
  nex_switch_play_pause.attachPush(nex_switch_play_pausePushCallback);
  nex_zyl_magnetarm.attachPush(nex_zyl_magnetarmPushCallback);
  nex_zyl_messer.attachPush(nex_zyl_messerPushCallback);
  nex_zyl_gummihalter.attachPush(nex_zyl_gummihalterPushCallback);
  nex_zyl_falltuer.attachPush(nex_zyl_falltuerPushCallback);

  //*****PUSH+POP:
  nex_mot_band_oben.attachPush(nex_mot_band_obenPushCallback);
  nex_mot_band_oben.attachPop(nex_mot_band_obenPopCallback);
  nex_mot_band_unten.attachPush(nex_mot_band_untenPushCallback);
  nex_mot_band_unten.attachPop(nex_mot_band_untenPopCallback);
  nex_zyl_revolverschieber.attachPush(nex_zyl_revolverschieberPushCallback);
  nex_zyl_revolverschieber.attachPop(nex_zyl_revolverschieberPopCallback);
  nex_but_reset_shorttime_counter.attachPush(nex_but_reset_shorttime_counterPushCallback);
  nex_but_reset_shorttime_counter.attachPop(nex_but_reset_shorttime_counterPopCallback);

  //**************************************************************************************
  //END OF REGISTER
  //**************************************************************************************
  delay(2000);
  sendCommand("page 1");//SWITCH NEXTION TO PAGE X
  send_to_nextion();

}//END OF NEXTION SETUP

//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
void nextion_loop()
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
{ //START NEXTION LOOP

  nexLoop(nex_listen_list); //check for any touch event
  //**************************************************************************************
  if (CurrentPage == 1)//START PAGE 1
  {
    //*******************
    //PAGE 1 - LEFT SIDE:
    //*******************
    //UPDATE SWITCHSTATE "PLAY"/"PAUSE"
    if (nex_state_machine_running != machine_running)
    {
      Serial2.print("click bt0,1");//CLICK BUTTON
      send_to_nextion();
      nex_state_machine_running = !nex_state_machine_running;
    }

    //UPDATE SWITCHSTATE "STEP"/"AUTO"-MODE

    if (step_mode != nex_prev_step_mode)
    {
      if (step_mode == true)
      {
        Serial2.print("click bt1,1");//CLICK BUTTON
        send_to_nextion();
        Serial2.print("bt1.txt=");
        Serial2.print("\"");
        Serial2.print("STEP MODE");
        Serial2.print("\"");
        send_to_nextion();
      }
      else
      {
        Serial2.print("click bt1,1");//CLICK BUTTON
        send_to_nextion();
        Serial2.print("bt1.txt=");
        Serial2.print("\"");
        Serial2.print("AUTO MODE");
        Serial2.print("\"");
        send_to_nextion();
      }
      nex_prev_step_mode = step_mode;
    }

    //DISPLAY IF MAGAZINE IS EMPTY
    if (nex_state_seal_available != seal_available)
    {
      if (seal_available == false)
      {
        Serial2.print("t4.txt=");
        Serial2.print("\"");
        Serial2.print("MAGAZIN LEER!");
        Serial2.print("\"");
        send_to_nextion();
      }
      else
      {
        Serial2.print("t4.txt=");
        Serial2.print("\"");
        Serial2.print("");//ERASE TEXT
        Serial2.print("\"");
        send_to_nextion();
      }
      nex_state_seal_available = seal_available;
    }

    //*******************
    //PAGE 1 - RIGHT SIDE:
    //*******************
    //UPDATE CYCLE NAME
    if (nex_prev_cycle_step != cycle_step)
    {
      Serial2.print("t0.txt=");
      Serial2.print("\"");
      Serial2.print(cycle_name[cycle_step - 1]);
      Serial2.print("\"");
      send_to_nextion();
      nex_prev_cycle_step = cycle_step;
    }
    //UPDATE SWITCHBUTTON (dual state):
    if (zyl_gummihalter.request_state() != nex_state_zyl_gummihalter)
    {
      Serial2.print("click bt5,1");//CLICK BUTTON
      send_to_nextion();
      nex_state_zyl_gummihalter = !nex_state_zyl_gummihalter;
    }
    //UPDATE SWITCHBUTTON (dual state):
    if (zyl_falltuerschieber.request_state() != nex_state_zyl_falltuerschieber)
    {
      Serial2.print("click bt4,1");//CLICK BUTTON
      send_to_nextion();
      nex_state_zyl_falltuerschieber = !nex_state_zyl_falltuerschieber;
    }
    //UPDATE SWITCHBUTTON (dual state):
    if (zyl_magnetarm.request_state() != nex_state_zyl_magnetarm)
    {
      Serial2.print("click bt3,1");//CLICK BUTTON
      send_to_nextion();
      nex_state_zyl_magnetarm = !nex_state_zyl_magnetarm;
    }

    //UPDATE BUTTON (momentary)
    if (mot_feed_oben.request_state() != nex_state_mot_feed_oben)
    {

      if (mot_feed_oben.request_state() == HIGH)
      {
        Serial2.print("click b5,1");//CLICK BUTTON
        send_to_nextion();
      }
      else
      {
        Serial2.print("click b5,0");//CLICK BUTTON
        send_to_nextion();
      }
      nex_state_mot_feed_oben = mot_feed_oben.request_state();
    }

    //UPDATE BUTTON (momentary)
    if (mot_feed_unten.request_state() != nex_state_mot_feed_unten)
    {
      if (mot_feed_unten.request_state() == HIGH)
      {
        Serial2.print("click b4,1");//CLICK BUTTON
        send_to_nextion();
      }
      else
      {
        Serial2.print("click b4,0");//CLICK BUTTON
        send_to_nextion();
      }
      nex_state_mot_feed_unten = mot_feed_unten.request_state();
    }

    //UPDATE SWITCHBUTTON (dual state):
    if (zyl_messer.request_state() != nex_state_zyl_messer)
    {
      Serial2.print("click bt2,1");//CLICK BUTTON
      send_to_nextion();
      nex_state_zyl_messer = !nex_state_zyl_messer;
    }
    //UPDATE BUTTON (momentary)
    if (zyl_revolverschieber.request_state() != nex_state_zyl_revolverschieber)
    {
      if (zyl_revolverschieber.request_state() == HIGH)
      {
        Serial2.print("click b3,1");//CLICK BUTTON
        send_to_nextion();
      }
      else
      {
        Serial2.print("click b3,0");//CLICK BUTTON
        send_to_nextion();
      }
      nex_state_zyl_revolverschieber = zyl_revolverschieber.request_state();
    }

  }//END PAGE 1
  //**************************************************************************************
  if (CurrentPage == 2)//START PAGE 2
  {
    //*******************
    //PAGE 2 - LEFT SIDE
    //*******************

    if (nex_prev_upper_feedtime != upper_feedtime)
    {
      Serial2.print("h0.val=");
      Serial2.print(upper_feedtime);
      send_to_nextion();
      Serial2.print("t4.txt=");
      Serial2.print("\"");
      Serial2.print(upper_feedtime);
      Serial2.print(" ms");
      Serial2.print("\"");
      send_to_nextion();
      nex_prev_upper_feedtime = upper_feedtime;
    }

    if (nex_prev_lower_feedtime != lower_feedtime)
    {
      Serial2.print("h1.val=");
      Serial2.print(lower_feedtime);
      send_to_nextion();

      Serial2.print("t6.txt=");
      Serial2.print("\"");
      Serial2.print(lower_feedtime);
      Serial2.print(" ms");
      Serial2.print("\"");
      send_to_nextion();
      nex_prev_lower_feedtime = lower_feedtime;
    }
    //*******************
    //PAGE 2 - RIGHT SIDE
    //*******************
    if (nex_prev_longtime_counter != longtime_counter)
    {
      Serial2.print("t10.txt=");
      Serial2.print("\"");
      Serial2.print(longtime_counter);
      Serial2.print("\"");
      send_to_nextion();
      nex_prev_longtime_counter = longtime_counter;
    }
    if (nex_prev_shorttime_counter != shorttime_counter)
    {
      Serial2.print("t12.txt=");
      Serial2.print("\"");
      Serial2.print(shorttime_counter);
      Serial2.print("\"");
      send_to_nextion();
      nex_prev_shorttime_counter = shorttime_counter;
    }
    if (reset_stopwatch_active == true)
    {
      if (millis() - counter_reset_stopwatch > 5000)
      {
        shorttime_counter = 0;
        longtime_counter = 0;
      }
    }
  }//END PAGE 2

}//END OF NEXTION LOOP
//**************************************************************************************
//**************************************************************************************
//***************************************************************************************

//**************************************************************************************
//TOUCH EVENT FUNCTIONS //PushCallback = Press event //PopCallback = Release event
//**************************************************************************************

//*************************************************
//TOUCH EVENT FUNCTIONS PAGE 1 - LEFT SIDE
//*************************************************
void nex_switch_play_pausePushCallback(void *ptr)
{
  machine_running = !machine_running;
  if (machine_running == true)
  {
    clearance_next_step = true;
  }
  if (machine_running == false)
  {
    //abort running processes:
    mot_feed_oben.set(0);
    mot_feed_unten.set(0);
    zyl_messer.set(0);
    zyl_revolverschieber.set(0);

  }
  nex_state_machine_running = !nex_state_machine_running;
}
void nex_switch_modePushCallback(void *ptr)
{
  step_mode = !step_mode;
  Serial2.print("click bt1,1");//CLICK BUTTON
  send_to_nextion();
}
void nex_but_stepbackPushCallback(void *ptr)
{
  if (cycle_step > 1)
  {
    cycle_step = cycle_step - 1;
  }
}
void nex_but_stepnxtPushCallback(void *ptr)
{
  if (cycle_step < 10)
  {
    cycle_step++;
  }
}
void nex_but_reset_cyclePushCallback(void *ptr)
{
  reset();
}
//*************************************************
//TOUCH EVENT FUNCTIONS PAGE 1 - RIGHT SIDE
//*************************************************
void nex_zyl_gummihalterPushCallback(void *ptr)
{
  zyl_gummihalter.toggle();
  nex_state_zyl_gummihalter = !nex_state_zyl_gummihalter;
}

void nex_zyl_falltuerPushCallback(void *ptr)
{
  zyl_falltuerschieber.toggle();
  nex_state_zyl_falltuerschieber = !nex_state_zyl_falltuerschieber;
}

void nex_zyl_magnetarmPushCallback(void *ptr)
{
  zyl_magnetarm.toggle();
  nex_state_zyl_magnetarm = !nex_state_zyl_magnetarm;
}

void nex_mot_band_obenPushCallback(void *ptr)
{
  mot_feed_oben.set(1);
  button_push_stopwatch = millis();
  stopwatch_running = true;
}

void nex_mot_band_obenPopCallback(void *ptr)
{
  mot_feed_oben.set(0);
  stopped_button_pushtime = millis() - button_push_stopwatch;
  stopwatch_running = false;
  Serial2.print("t4.txt=");
  Serial2.print("\"");
  Serial2.print(stopped_button_pushtime);
  Serial2.print(" ms");
  Serial2.print("\"");
  send_to_nextion();
}

void nex_mot_band_untenPushCallback(void *ptr)
{
  mot_feed_unten.set(1);
  button_push_stopwatch = millis();
  stopwatch_running = true;
}
void nex_mot_band_untenPopCallback(void *ptr)
{
  mot_feed_unten.set(0);
  stopped_button_pushtime = millis() - button_push_stopwatch;
  stopwatch_running = false;
  Serial2.print("t4.txt=");
  Serial2.print("\"");
  Serial2.print(stopped_button_pushtime);
  Serial2.print(" ms");
  Serial2.print("\"");
  send_to_nextion();
}

void nex_zyl_messerPushCallback(void *ptr)
{
  zyl_messer.toggle();
  nex_state_zyl_messer = !nex_state_zyl_messer;
}

void nex_zyl_revolverschieberPushCallback(void *ptr)
{
  zyl_revolverschieber.set(1);
}
void nex_zyl_revolverschieberPopCallback(void *ptr)
{
  zyl_revolverschieber.set(0);
}

//*************************************************
//TOUCH EVENT FUNCTIONS PAGE 2 - LEFT SIDE
//*************************************************

void nex_but_slider1_leftPushCallback(void *ptr)
{
  upper_feedtime = upper_feedtime - 100;
  if (upper_feedtime < 0)
  {
    upper_feedtime = 0;
  }
}

void nex_but_slider1_rightPushCallback(void *ptr)
{
  upper_feedtime = upper_feedtime + 100;
  if (upper_feedtime > 5000)
  {
    upper_feedtime = 5000;
  }
}
void nex_but_slider2_leftPushCallback(void *ptr)
{
  lower_feedtime = lower_feedtime - 100;
  if (lower_feedtime < 0)
  {
    lower_feedtime = 0;
  }
}
void nex_but_slider2_rightPushCallback(void *ptr)
{
  lower_feedtime = lower_feedtime + 100;
  if (lower_feedtime > 5000)
  {
    lower_feedtime = 5000;
  }
}
//*************************************************
//TOUCH EVENT FUNCTIONS PAGE 2 - RIGHT SIDE
//*************************************************
void nex_but_reset_shorttime_counterPushCallback(void *ptr)
{
  shorttime_counter = 0;
  //RESET LONGTIME COUNTER IF RESET BUTTON IS PRESSED LONG ENOUGH:
  counter_reset_stopwatch = millis();
  reset_stopwatch_active = true;

}
void nex_but_reset_shorttime_counterPopCallback(void *ptr)
{
  reset_stopwatch_active = false;
}
//*************************************************
//TOUCH EVENT FUNCTIONS PAGE CHANGES
//*************************************************
void nex_page0PushCallback(void *ptr)
{
  CurrentPage = 0;

}
void nex_page1PushCallback(void *ptr)
{
  CurrentPage = 1;

  //REFRESH BUTTON STATES:
  nex_prev_cycle_step = 0;
  nex_prev_step_mode = true;

  nex_state_zyl_gummihalter = 0;
  nex_state_zyl_falltuerschieber = 0;
  nex_state_zyl_magnetarm = 0;
  nex_state_mot_feed_oben = 0;
  nex_state_mot_feed_unten = 0;
  nex_state_zyl_messer = 0;
  nex_state_zyl_revolverschieber = 0;
  nex_state_machine_running = 0;
  nex_state_seal_available = !seal_available;
}

void nex_page2PushCallback(void *ptr)
{
  CurrentPage = 2;
  //REFRESH BUTTON STATES:
  nex_prev_upper_feedtime = 0;
  nex_prev_lower_feedtime = 0;
  nex_prev_shorttime_counter = 0;
  nex_prev_longtime_counter = 0;
}

//**************************************************************************************
//END OF TOUCH EVENT FUNCTIONS
//**************************************************************************************
