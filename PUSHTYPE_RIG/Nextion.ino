/*
 * *****************************************************************************
 * nextion.ino
 * configuration of the Nextion touch display
 * Michael Wettstein
 * November 2018, Zürich
 * *****************************************************************************
 * an XLS-sheet to generate Nextion events can be found here:
 * https://github.com/chischte/user-interface/NEXTION/
 * *****************************************************************************
 * CONFIGURING THE LIBRARY:
 * Include the nextion library (the official one) https://github.com/itead/ITEADLIB_Arduino_Nextion
 * Make sure you edit the NexConfig.h file on the library folder to set the correct serial port for the display.
 * By default it's set to Serial1, which most arduino boards don't have.
 * Change "#define nexSerial Serial1" to "#define nexSerial Serial" if you are using arduino uno, nano, etc.
 * *****************************************************************************
 * NEXTION SWITCH STATES LIST
 * Every nextion switch button needs a switchstate variable (bool)
 * to update the screen after a page change (all buttons)
 * to control switchtoggle (dualstate buttons)
 * to prevent screen flickering (momentary buttons)
 * *****************************************************************************
 * VARIOUS COMMANDS:
 * Serial2.print("click bt1,1");//CLICK BUTTON
 * send_to_nextion();
 * A switch (Dual State Button)will be toggled with this command, a Button will be set permanently pressed)
 * Serial2.print("click b3,0"); releases a push button again, has no effect on a Dual State Button
 * send_to_nextion();
 * Serial2.print("vis t0,0");//HIDE OBJECT
 * send_to_nextion();
 * *****************************************************************************
 */

//******************************************************************************
// DECLARATION OF VARIABLES:
//******************************************************************************
int CurrentPage;
byte nexPrevCycleStep;

// SWITCHSTATES:
bool nex_state_ZylGummihalter;
bool nex_state_ZylFalltuerschieber;
bool nex_state_ZylMagnetarm;
bool nex_state_MotFeedOben;
bool nex_state_MotFeedUnten;
bool nex_state_ZylMesser;
bool nex_state_ZylRevolverschieber;
bool nexStateMachineRunning;
bool nex_state_sealAvailable;
bool nex_state_PressMotor;
bool nex_prev_stepMode = true;
//******************************************************************************
bool stopwatch_running;
bool resetStopwatchActive;
unsigned int stopped_button_pushtime;
long nex_prev_upperFeedtime;
long nex_prev_lowerFeedtime;
long nex_prev_shorttimeCounter;
long nex_prev_longtimeCounter;
long button_push_stopwatch;
long counterResetStopwatch;
//******************************************************************************
// DECLARATION OF OBJECTS TO BE READ FROM NEXTION
//******************************************************************************
// PAGE 0:
NexPage nex_page0 = NexPage(0, 0, "page0");
//PAGE 1 - LEFT SIDE:
NexPage nex_page1 = NexPage(1, 0, "page1");
NexButton nex_but_stepback = NexButton(1, 2, "b1");
NexButton nex_but_stepnxt = NexButton(1, 7, "b2");
NexButton nex_but_reset_cycle = NexButton(1, 1, "b0");
NexDSButton nex_switch_play_pause = NexDSButton(1, 4, "bt0");
NexDSButton nex_switch_mode = NexDSButton(1, 6, "bt1");
// PAGE 1 - RIGHT SIDE
NexDSButton nex_ZylGummihalter = NexDSButton(1, 13, "bt5");
NexDSButton nex_zyl_falltuer = NexDSButton(1, 12, "bt4");
NexDSButton nex_ZylMagnetarm = NexDSButton(1, 11, "bt3");
NexButton nex_mot_band_oben = NexButton(1, 10, "b5");
NexButton nex_mot_band_unten = NexButton(1, 9, "b4");
NexButton nex_ZylMesser = NexButton(1, 15, "b6");
NexButton nex_ZylRevolverschieber = NexButton(1, 8, "b3");
NexButton nex_PressMotor = NexButton(1, 16, "b7");

// PAGE 2 - LEFT SIDE:
NexPage nex_page2 = NexPage(2, 0, "page2");
NexButton nex_but_slider1_left = NexButton(2, 6, "b1");
NexButton nex_but_slider1_right = NexButton(2, 7, "b2");
NexButton nex_but_slider2_left = NexButton(2, 9, "b0");
NexButton nex_but_slider2_right = NexButton(2, 11, "b3");
// PAGE 2 - RIGHT SIDE:
NexButton nex_but_reset_shorttimeCounter = NexButton(2, 18, "b4");
//*****************************************************************************
// END OF OBJECT DECLARATION
//*****************************************************************************
char buffer[100] = { 0 }; // This is needed only if you are going to receive a text from the display. You can remove it otherwise.
//*****************************************************************************
// TOUCH EVENT LIST //DECLARATION OF TOUCH EVENTS TO BE MONITORED
//*****************************************************************************
NexTouch *nex_listen_list[] = { //
            // PAGE 0:
            &nex_page0,
            // PAGE 1:
            &nex_page1, &nex_but_stepback, &nex_but_stepnxt, &nex_but_reset_cycle,
            &nex_switch_play_pause, &nex_switch_mode, &nex_ZylMesser, &nex_ZylMagnetarm,
            &nex_ZylGummihalter, &nex_zyl_falltuer, &nex_mot_band_oben, &nex_mot_band_unten,
            &nex_ZylRevolverschieber, &nex_PressMotor,
            // PAGE 2:
            &nex_page2, &nex_but_reset_shorttimeCounter, &nex_but_slider1_left,
            &nex_but_slider1_right, &nex_but_slider2_left, &nex_but_slider2_right, NULL };
//*****************************************************************************
// END OF TOUCH EVENT LIST
//*****************************************************************************
void printOnTextField(String text, String textField) {
  Serial2.print(textField);
  Serial2.print(".txt=");
  Serial2.print("\"");
  Serial2.print(text);
  Serial2.print("\"");
  send_to_nextion();
}
void clearTextField(String textField) {
  Serial2.print(textField);
  Serial2.print(".txt=");
  Serial2.print("\"");
  Serial2.print("");    // erase text
  Serial2.print("\"");
  send_to_nextion();
}
void printOnValueField(int value, String valueField) {
  Serial2.print(valueField);
  Serial2.print(".val=");
  Serial2.print(value);
  send_to_nextion();
}
void send_to_nextion() {
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}
//*****************************************************************************
void nextionSetup()
//*****************************************************************************
{
  Serial2.begin(9600);

  // RESET NEXTION DISPLAY: (refresh display after PLC restart)
  send_to_nextion(); // needed for unknown reasons
  Serial2.print("rest");
  send_to_nextion();

  //*****************************************************************************
  // INCREASE BAUD RATE DOES NOT WORK YET
  // CHECK IF LIBRARY ALWAY SWITCHES BACK TO 9600
  //*****************************************************************************
  /*
   delay(100);
   send_to_nextion();
   Serial2.print("baud=38400");
   send_to_nextion();
   delay(100);
   Serial2.begin(38400);
   */
  //*****************************************************************************
  // REGISTER THE EVENT CALLBACK FUNCTIONS
  //*****************************************************************************
  //*****PUSH ONLY
  // PAGE 0:
  nex_page0.attachPush(nex_page0PushCallback);

  // PAGE 1:
  nex_page1.attachPush(nex_page1PushCallback);
  nex_but_stepback.attachPush(nex_but_stepbackPushCallback);
  nex_but_stepnxt.attachPush(nex_but_stepnxtPushCallback);
  nex_ZylMagnetarm.attachPush(nex_ZylMagnetarmPushCallback);
  nex_but_reset_cycle.attachPush(nex_but_reset_cyclePushCallback);
  nex_but_stepback.attachPush(nex_but_stepbackPushCallback);
  nex_but_stepnxt.attachPush(nex_but_stepnxtPushCallback);
  nex_switch_mode.attachPush(nex_switch_modePushCallback);
  nex_switch_play_pause.attachPush(nex_switch_play_pausePushCallback);
  nex_ZylMagnetarm.attachPush(nex_ZylMagnetarmPushCallback);
  nex_ZylGummihalter.attachPush(nex_ZylGummihalterPushCallback);
  nex_zyl_falltuer.attachPush(nex_zyl_falltuerPushCallback);

  // PAGE 2:
  nex_page2.attachPush(nex_page2PushCallback);
  nex_but_slider1_left.attachPush(nex_but_slider1_leftPushCallback);
  nex_but_slider1_right.attachPush(nex_but_slider1_rightPushCallback);
  nex_but_slider2_left.attachPush(nex_but_slider2_leftPushCallback);
  nex_but_slider2_right.attachPush(nex_but_slider2_rightPushCallback);

  //*****PUSH+POP:
  // PAGE 1:
  nex_mot_band_oben.attachPush(nex_mot_band_obenPushCallback);
  nex_mot_band_oben.attachPop(nex_mot_band_obenPopCallback);
  nex_mot_band_unten.attachPush(nex_mot_band_untenPushCallback);
  nex_mot_band_unten.attachPop(nex_mot_band_untenPopCallback);
  nex_ZylMesser.attachPush(nex_ZylMesserPushCallback);
  nex_ZylMesser.attachPop(nex_ZylMesserPopCallback);
  nex_PressMotor.attachPush(nex_PressMotorPushCallback);
  nex_PressMotor.attachPop(nex_PressMotorPopCallback);
  nex_ZylRevolverschieber.attachPush(nex_ZylRevolverschieberPushCallback);
  nex_ZylRevolverschieber.attachPop(nex_ZylRevolverschieberPopCallback);

  // PAGE 2:
  nex_but_reset_shorttimeCounter.attachPush(nex_but_reset_shorttimeCounterPushCallback);
  nex_but_reset_shorttimeCounter.attachPop(nex_but_reset_shorttimeCounterPopCallback);

  //*****************************************************************************
  // END OF REGISTER
  //*****************************************************************************
  delay(2000);
  sendCommand("page 1");  //SWITCH NEXTION TO PAGE X
  send_to_nextion();

}  // END OF NEXTION SETUP
//*****************************************************************************
void NextionLoop()
//*****************************************************************************
{
  nexLoop(nex_listen_list); //check for any touch event
  //*****************************************************************************
  if (CurrentPage == 1) {
    //*******************
    // PAGE 1 - LEFT SIDE:
    //*******************
    // UPDATE SWITCHSTATE "PLAY"/"PAUSE":
    if (nexStateMachineRunning != machineRunning) {
      Serial2.print("click bt0,1");
      send_to_nextion();
      nexStateMachineRunning = !nexStateMachineRunning;
    }

    // UPDATE SWITCHSTATE "STEP"/"AUTO"-MODE:
    if (stepMode != nex_prev_stepMode) {
      if (stepMode) {
        Serial2.print("click bt1,1");
      } else {
        Serial2.print("click bt1,1");
      }
      send_to_nextion();
      nex_prev_stepMode = stepMode;
    }

    // DISPLAY IF MAGAZINE IS EMPTY:
    if (nex_state_sealAvailable != sealAvailable) {
      if (!sealAvailable) {
        printOnTextField("MAGAZIN LEER!", "t4");
      } else {
        clearTextField("t4");
      }
      nex_state_sealAvailable = sealAvailable;
    }
    if (bandsensorOben.switchedLow()) {
      printOnTextField("BAND OBEN", "t4");
    }
    if (bandsensorUnten.switchedLow()) {
      printOnTextField("BAND UNTEN", "t4");
    }
    if(upperStrapBlockCounter==2||lowerStrapBlockCounter==2){
      printOnTextField("BAND BLOCKIERT", "t4");
    }

    //*******************
    // PAGE 1 - RIGHT SIDE:
    //*******************
    // UPDATE CYCLE NAME:
    if (nexPrevCycleStep != cycleStep) {
      nexPrevCycleStep = cycleStep;
      printOnTextField((cycleStep + 1) + (" " + cycleName[cycleStep]), "t0");
    }
    // UPDATE SWITCHBUTTON (dual state):
    if (ZylGummihalter.request_state() != nex_state_ZylGummihalter) {
      Serial2.print("click bt5,1");
      send_to_nextion();
      nex_state_ZylGummihalter = !nex_state_ZylGummihalter;
    }
    // UPDATE SWITCHBUTTON (dual state):
    if (ZylFalltuerschieber.request_state() != nex_state_ZylFalltuerschieber) {
      Serial2.print("click bt4,1");
      send_to_nextion();
      nex_state_ZylFalltuerschieber = !nex_state_ZylFalltuerschieber;
    }
    // UPDATE SWITCHBUTTON (dual state):
    if (ZylMagnetarm.request_state() != nex_state_ZylMagnetarm) {
      Serial2.print("click bt3,1");
      send_to_nextion();
      nex_state_ZylMagnetarm = !nex_state_ZylMagnetarm;
    }

    // UPDATE BUTTON (momentary):
    if (MotFeedOben.request_state() != nex_state_MotFeedOben) {
      if (MotFeedOben.request_state()) {
        Serial2.print("click b5,1");
      } else {
        Serial2.print("click b5,0");
      }
      send_to_nextion();
      nex_state_MotFeedOben = MotFeedOben.request_state();
    }

    // UPDATE BUTTON (momentary):
    if (MotFeedUnten.request_state() != nex_state_MotFeedUnten) {
      if (MotFeedUnten.request_state()) {
        Serial2.print("click b4,1");
      } else {
        Serial2.print("click b4,0");
      }
      send_to_nextion();
      nex_state_MotFeedUnten = MotFeedUnten.request_state();
    }

    // UPDATE BUTTON (momentary):
    if (ZylMesser.request_state() != nex_state_ZylMesser) {
      if (ZylMesser.request_state()) {
        Serial2.print("click b6,1");
      } else {
        Serial2.print("click b6,0");
      }
      send_to_nextion();
      nex_state_ZylMesser = ZylMesser.request_state();
    }

    // UPDATE BUTTON (momentary):
    if (ZylRevolverschieber.request_state() != nex_state_ZylRevolverschieber) {
      if (ZylRevolverschieber.request_state()) {
        Serial2.print("click b3,1");
      } else {
        Serial2.print("click b3,0");
      }
      send_to_nextion();
      nex_state_ZylRevolverschieber = ZylRevolverschieber.request_state();
    }

    // UPDATE BUTTON (momentary):
    if (MotorTool.request_state() != nex_state_PressMotor) {
      if (MotorTool.request_state()) {
        Serial2.print("click b7,1");
      } else {
        Serial2.print("click b7,0");
      }
      send_to_nextion();
      nex_state_PressMotor = MotorTool.request_state();
    }

  }    //END PAGE 1
//*****************************************************************************
  if (CurrentPage == 2) {
    //*******************
    // PAGE 2 - LEFT SIDE
    //*******************

    if (nex_prev_upperFeedtime != eepromCounter.getValue(upperFeedtime)) {
      printOnValueField((int) eepromCounter.getValue(upperFeedtime), "h0");
      printOnTextField(String(eepromCounter.getValue(upperFeedtime)) + " ms", "t4");
      nex_prev_upperFeedtime = eepromCounter.getValue(upperFeedtime);
    }

    if (nex_prev_lowerFeedtime != eepromCounter.getValue(lowerFeedtime)) {
      printOnValueField((int) eepromCounter.getValue(lowerFeedtime), "h1");
      printOnTextField(String(eepromCounter.getValue(lowerFeedtime)) + " ms", "t6");
      nex_prev_lowerFeedtime = eepromCounter.getValue(lowerFeedtime);
    }
    //*******************
    // PAGE 2 - RIGHT SIDE
    //*******************
    if (nex_prev_longtimeCounter != eepromCounter.getValue(longtimeCounter)) {

      printOnTextField(String(eepromCounter.getValue(longtimeCounter)), "t10");
      //printOnTextField((eepromCounter.getValue(longtimeCounter) + ("")), "t10");
      nex_prev_longtimeCounter = eepromCounter.getValue(longtimeCounter);
    }
    if (nex_prev_shorttimeCounter != eepromCounter.getValue(shorttimeCounter)) {
      printOnTextField(String(eepromCounter.getValue(shorttimeCounter)), "t12");
      nex_prev_shorttimeCounter = eepromCounter.getValue(shorttimeCounter);
    }
    if (resetStopwatchActive) {
      if (millis() - counterResetStopwatch > 5000) {
        eepromCounter.set(longtimeCounter, 0);
      }
    }
  }    // END PAGE 2
}    // END OF NEXTION LOOP
//*****************************************************************************
// TOUCH EVENT FUNCTIONS //PushCallback = Press event //PopCallback = Release event
//*****************************************************************************
//*************************************************
// TOUCH EVENT FUNCTIONS PAGE 1 - LEFT SIDE
//*************************************************
void nex_switch_play_pausePushCallback(void *ptr) {
  machineRunning = !machineRunning;
  if (machineRunning) {
    clearanceNextStep = true;
  }
  if (!machineRunning) {
    //abort running processes:
    MotFeedOben.set(0);
    MotFeedUnten.set(0);
    ZylMesser.set(0);
    ZylRevolverschieber.set(0);
  }
  nexStateMachineRunning = !nexStateMachineRunning;
}
void nex_switch_modePushCallback(void *ptr) {
  stepMode = !stepMode;
  Serial2.print("click bt1,1");
  send_to_nextion();
}
void nex_but_stepbackPushCallback(void *ptr) {
  if (cycleStep > 0) {
    cycleStep = cycleStep - 1;
  }
}
void nex_but_stepnxtPushCallback(void *ptr) {
  if (cycleStep < numberOfMainCycleSteps - 1) {
    cycleStep++;
  }
}
void nex_but_reset_cyclePushCallback(void *ptr) {
  ResetTestRig();
  clearTextField("t4");
}
//*************************************************
// TOUCH EVENT FUNCTIONS PAGE 1 - RIGHT SIDE
//*************************************************
void nex_ZylGummihalterPushCallback(void *ptr) {
  // WENN DER GUMMIHALTER GESCHLOSSEN IST...
  if (ZylGummihalter.request_state()) {
    // ...UND DER FALLTUERSCHIEBER OFFEN IST...
    if (ZylFalltuerschieber.request_state()) {
      //...ERST FALLTUERSCHIEBER SCHLIESSEN:
      ZylFalltuerschieber.toggle();
    }
  }
  ZylGummihalter.toggle();
  nex_state_ZylGummihalter = !nex_state_ZylGummihalter;
}
void nex_zyl_falltuerPushCallback(void *ptr) {
  // WENN DIE FALLTUER GESCHLOSSEN IST...
  if (!ZylFalltuerschieber.request_state()) {
    // ...UND DER GUMMIHALTER OFFEN IST...
    if (!ZylGummihalter.request_state()) {
      // ...ERST GUMMIHALTER SCHLIESSEN:
      ZylGummihalter.toggle();
    }
    // WENN DER MAGNETARM NICHT VORNE IST ...
    if (ZylMagnetarm.request_state()) {
      // FEHLERMELDUNG ANZEIGEN:
      printOnTextField("MAGNETARM!", "t4");
    } else
      ZylFalltuerschieber.toggle();
  } else {
    // WENN DIE FALLTUER OFFEN IST, DARF SIE IMMER GESCHLOSSEN WERDEN:
    ZylFalltuerschieber.toggle();
  }
  nex_state_ZylFalltuerschieber = !nex_state_ZylFalltuerschieber;
}
void nex_ZylMagnetarmPushCallback(void *ptr) {
  // WENN DER MAGNETARM ZURÜCKGEFAHREN IST ...
  if (!ZylMagnetarm.request_state()) {
    // ... UND DIE FALLTÜR OFFEN IST ...
    if (ZylFalltuerschieber.request_state()) {
      // ... ERST DIE FALLTÜR SCHLIESSEN
      ZylFalltuerschieber.toggle();
    }
  }
  ZylMagnetarm.toggle();
  nex_state_ZylMagnetarm = !nex_state_ZylMagnetarm;
}

void nex_mot_band_obenPushCallback(void *ptr) {
  if (upperStrapAvailable) {
    MotFeedOben.set(1);
    button_push_stopwatch = millis();
    stopwatch_running = true;
  }
}
void nex_mot_band_obenPopCallback(void *ptr) {
  MotFeedOben.set(0);
}
void nex_mot_band_untenPushCallback(void *ptr) {
  if (lowerStrapAvailable) {
    MotFeedUnten.set(1);
  }
}
void nex_mot_band_untenPopCallback(void *ptr) {
  MotFeedUnten.set(0);
}
void nex_ZylMesserPushCallback(void *ptr) {
  ZylMesser.set(1);
}
void nex_ZylMesserPopCallback(void *ptr) {
  ZylMesser.set(0);
}
void nex_PressMotorPushCallback(void *ptr) {
  MotorTool.set(1);
}
void nex_PressMotorPopCallback(void *ptr) {
  MotorTool.set(0);
}
void nex_ZylRevolverschieberPushCallback(void *ptr) {
  ZylRevolverschieber.set(1);
}
void nex_ZylRevolverschieberPopCallback(void *ptr) {
  ZylRevolverschieber.set(0);
}
//*************************************************
// TOUCH EVENT FUNCTIONS PAGE 2 - LEFT SIDE
//*************************************************
void nex_but_slider1_leftPushCallback(void *ptr) {
  eepromCounter.set(upperFeedtime, eepromCounter.getValue(upperFeedtime) - 50);
  if (eepromCounter.getValue(upperFeedtime) < 0) {
    eepromCounter.set(upperFeedtime, 0);
  }
}
void nex_but_slider1_rightPushCallback(void *ptr) {
  eepromCounter.set(upperFeedtime, eepromCounter.getValue(upperFeedtime) + 50);
  if (eepromCounter.getValue(upperFeedtime) > 5000) {
    eepromCounter.set(upperFeedtime, 5000);
  }
}
void nex_but_slider2_leftPushCallback(void *ptr) {
  eepromCounter.set(lowerFeedtime, eepromCounter.getValue(lowerFeedtime) - 50);
  if (eepromCounter.getValue(lowerFeedtime) < 0) {
    eepromCounter.set(lowerFeedtime, 0);
  }
}
void nex_but_slider2_rightPushCallback(void *ptr) {
  eepromCounter.set(lowerFeedtime, eepromCounter.getValue(lowerFeedtime) + 50);
  if (eepromCounter.getValue(lowerFeedtime) > 5000) {
    eepromCounter.set(lowerFeedtime, 5000);
  }
}
//*************************************************
// TOUCH EVENT FUNCTIONS PAGE 2 - RIGHT SIDE
//*************************************************
void nex_but_reset_shorttimeCounterPushCallback(void *ptr) {
  eepromCounter.set(shorttimeCounter, 0);
// RESET LONGTIME COUNTER IF RESET BUTTON IS PRESSED LONG ENOUGH:
  counterResetStopwatch = millis();
  resetStopwatchActive = true;
}
void nex_but_reset_shorttimeCounterPopCallback(void *ptr) {
  resetStopwatchActive = false;
}
//*************************************************
// TOUCH EVENT FUNCTIONS PAGE CHANGES
//*************************************************
void nex_page0PushCallback(void *ptr) {
  CurrentPage = 0;
}
void nex_page1PushCallback(void *ptr) {
  CurrentPage = 1;

// REFRESH BUTTON STATES:
  nexPrevCycleStep = !cycleStep;
  nex_prev_stepMode = true;
  nex_state_ZylGummihalter = 0;
  nex_state_ZylFalltuerschieber = 0;
  nex_state_ZylMagnetarm = 0;
  nex_state_MotFeedOben = 0;
  nex_state_MotFeedUnten = 0;
  nex_state_ZylMesser = 0;
  nex_state_ZylRevolverschieber = 0;
  nex_state_PressMotor = 0;
  nexStateMachineRunning = 0;
  nex_state_sealAvailable = !sealAvailable;
}
void nex_page2PushCallback(void *ptr) {
  CurrentPage = 2;
// REFRESH BUTTON STATES:
  nex_prev_upperFeedtime = 0;
  nex_prev_lowerFeedtime = 0;
  nex_prev_shorttimeCounter = 0;
  nex_prev_longtimeCounter = 0;
}
//*************************************************
// END OF TOUCH EVENT FUNCTIONS
//*************************************************
