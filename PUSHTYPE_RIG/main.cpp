/*
 * *****************************************************************************
 * PUSHTYPE_RIG
 * *****************************************************************************
 * Program to control the Push Type 100.000 Cycles Test Rig "Solinger Karussell"
 * *****************************************************************************
 * Michael Wettstein
 * Dezember 2018, Zürich
 * *****************************************************************************
 * TODO:
 * check runtime
 * implement new logic for main cycle:
 * run if: (autoMode && autoModeRunning) || (!autoMode && stepModeRunning)
 * implement stateController
 * should: start screen write test rig in two words
 */


//#include <SD.h>
//#include <Arduino.h>

#include <Controllino.h>    // https://github.com/CONTROLLINO-PLC/CONTROLLINO_Library
#include <Nextion.h>        // https://github.com/chischte/nextion-platformio-dualstate.git
#include <Cylinder.h>       // https://github.com/chischte/cylinder-library
#include <Debounce.h>       // https://github.com/chischte/debounce-library
#include <EEPROM_Counter.h> // https://github.com/chischte/eeprom-counter-library
#include <Insomnia.h>       // https://github.com/chischte/insomnia-delay-library


//*****************************************************************************
// DECLARATION OF VARIABLES / DATA TYPES
//*****************************************************************************
// byte  (0-255)
// int   (-32,768 to 32,767)
// long  (-2,147,483,648 to 2,147,483,647)
// float (6-7 Digits)
//*****************************************************************************

// INPUT PINS:
const byte STOP_BUTTON = CONTROLLINO_A0;
const byte START_BUTTON = CONTROLLINO_A1;
const byte TEMP_SENSOR_PIN = CONTROLLINO_A2;
const byte SENSOR_PLOMBE = CONTROLLINO_A3;
const byte TOOL_END_SWITCH_PIN = CONTROLLINO_A4;
const byte BANDSENSOR_OBEN = CONTROLLINO_A5;
const byte BANDSENSOR_UNTEN = CONTROLLINO_A6;

// OUTPUT PINS:
const byte RED_LIGHT_PIN = CONTROLLINO_D11;
const byte GREEN_LIGHT_PIN = CONTROLLINO_D12;

//OTHER VARIABLES:
bool machineRunning = false;
bool stepMode = true;
bool clearancePlayPauseToggle = true;
bool clearanceNextStep = false;
bool errorBlink = false;
bool greenBlink = false;
bool sealAvailable = false;
bool upperStrapAvailable = false;
bool lowerStrapAvailable = false;
bool toolMotorState = 0;
bool temperatureMonitor = true;

byte cycleStep = 0;
byte upperStrapBlockCounter = 0;
byte lowerStrapBlockCounter = 0;

unsigned long runtime;
unsigned long runtimeStopwatch;
//String cycleName;

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
NexButton nex_but_slider1_left = NexButton(2, 5, "b1");
NexButton nex_but_slider1_right = NexButton(2, 6, "b2");
NexButton nex_but_slider2_left = NexButton(2, 8, "b0");
NexButton nex_but_slider2_right = NexButton(2, 9, "b3");
NexButton nex_but_slider3_left = NexButton(2, 19, "b6");
NexButton nex_but_slider3_right = NexButton(2, 18, "b5");
NexButton nex_but_slider4_left = NexButton(2, 22, "b7");
NexButton nex_but_slider4_right = NexButton(2, 23, "b8");

// PAGE 2 - RIGHT SIDE:
NexButton nex_but_reset_shorttimeCounter = NexButton(2, 16, "b4");
//*****************************************************************************
// END OF OBJECT DECLARATION
//*****************************************************************************

//******************************************************************************
// DECLARATION OF NEXTION VARIABLES:
//******************************************************************************
int CurrentPage;
byte nexPrevCycleStep;
unsigned int stopped_button_pushtime;
bool stopwatch_running;
bool resetStopwatchActive;

// UPDATE SWITCHSTATES PAGE 1:
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
// UPDATE VALUES PAGE 1:

//******************************************************************************
// UPDATE SWITCHSTATES PAGE 2:
long nex_prev_upperFeedtime;
long nex_prev_lowerFeedtime;
long nex_prev_shorttimeCounter;
long nex_prev_longtimeCounter;

long button_push_stopwatch;
long counterResetStopwatch;
long nex_prev_cycleDurationTime;
// UPDATE VALUES PAGE 2:
int nexPrevCurrentTemperature;
long nexPrevMaxTemperature;


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
            // PAGE 2 LEFT:
            &nex_page2, &nex_but_slider1_left, &nex_but_slider1_right, &nex_but_slider2_left,
            &nex_but_slider2_right, &nex_but_slider3_left, &nex_but_slider3_right,
            &nex_but_slider4_left, &nex_but_slider4_right,
            // PAGE 2 RIGHT:
            &nex_but_reset_shorttimeCounter,
            // END OF LISTEN LIST:
            NULL };
//*****************************************************************************
// END OF TOUCH EVENT LIST
//*****************************************************************************

// SET UP EEPROM COUNTER:
enum eepromCounter {
  upperFeedtime,
  lowerFeedtime,
  shorttimeCounter,
  longtimeCounter,
  cycleDurationTime,
  maxTemperature,
  endOfEepromEnum
};
int numberOfEepromValues = endOfEepromEnum;
int eepromMinAddress = 0;
int eepromMaxAddress = 4095;
EEPROM_Counter eepromCounter;

//*****************************************************************************
// GENERATE INSTANCES OF CLASSES:
//*****************************************************************************
Cylinder ZylGummihalter(CONTROLLINO_D6);
Cylinder ZylFalltuerschieber(CONTROLLINO_D7);
Cylinder ZylMagnetarm(CONTROLLINO_D8);
Cylinder MotFeedOben(CONTROLLINO_D0);
Cylinder MotFeedUnten(CONTROLLINO_D1);
Cylinder ZylMesser(CONTROLLINO_D3);
Cylinder ZylRevolverschieber(CONTROLLINO_D2);
Cylinder MotorTool(CONTROLLINO_R5);
Cylinder ZylSchild(CONTROLLINO_D9);
Cylinder ZylAirBlower(CONTROLLINO_D4);

Insomnia nextStepTimer;
Insomnia errorBlinkTimer;
Insomnia greenBlinkTimer;
Insomnia cycleDurationTimer;

Debounce motorStartButton(START_BUTTON);
Debounce endSwitch(TOOL_END_SWITCH_PIN);
Debounce bandsensorOben(BANDSENSOR_OBEN);
Debounce bandsensorUnten(BANDSENSOR_UNTEN);
//*****************************************************************************
// DEFINE NAMES AND SEQUENCE OF STEPS FOR THE MAIN CYCLE:
//*****************************************************************************
enum mainCycleSteps {
  /*VIBRIEREN,*/
  KLEMMEN,
  FALLENLASSEN,
  MAGNETARM_AUSFAHREN,
  BAND_UNTEN,
  ZURUECKFAHREN,
  BAND_OBEN,
  PRESSEN,
  SCHNEIDEN,
  BLASEN,
  REVOLVER,
  PAUSE,
  endOfMainCycleEnum
};

int numberOfMainCycleSteps = endOfMainCycleEnum;

void toolReset() {
  // SIMULIERE WIPPENHEBEL ZIEHEN:
  digitalWrite(CONTROLLINO_RELAY_08, LOW);  //WIPPENSCHALTER WHITE CABLE (NO)
  digitalWrite(CONTROLLINO_RELAY_09, HIGH); //WIPPENSCHALTER RED CABLE (NC)
  delay(200);
  // SIMULIERE WIPPENHEBEL LOSLASEN:
  digitalWrite(CONTROLLINO_RELAY_09, LOW);  //WIPPENSCHALTER RED   CABLE (NC)
  digitalWrite(CONTROLLINO_RELAY_08, HIGH); //WIPPENSCHALTER WHITE CABLE (NO)delay(200);
}

// DEFINE NAMES TO DISPLAY ON THE TOUCH SCREEN:
String cycleName[] = {       //
        /*"VIBRIEREN",*/     //
            "KLEMMEN",//
            "FALLENLASSEN",  //
            "AUSFAHREN",     //
            "BAND UNTEN",    //
            "ZURUECKFAHREN", //
            "BAND OBEN",     //
            "PRESSEN",       //
            "SCHNEIDEN",     //
            "BLASEN",        //
            "REVOLVER",      //
            "PAUSE"          //
        };

void sendToNextion() {
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

void printOnTextField(String text, String textField) {
  Serial2.print(textField);
  Serial2.print(".txt=");
  Serial2.print("\"");
  Serial2.print(text);
  Serial2.print("\"");
  sendToNextion();
}
void clearTextField(String textField) {
  Serial2.print(textField);
  Serial2.print(".txt=");
  Serial2.print("\"");
  Serial2.print("");    // erase text
  Serial2.print("\"");
  sendToNextion();
}
void printOnValueField(int value, String valueField) {
  Serial2.print(valueField);
  Serial2.print(".val=");
  Serial2.print(value);
  sendToNextion();
}

void showInfoField() {
  if (CurrentPage == 1) {
    Serial2.print("vis t4,1");
    sendToNextion();
  }
}
void hideInfoField() {
  if (CurrentPage == 1) {
    Serial2.print("vis t4,0");
    sendToNextion();
  }
}

void resetTestRig() {

  upperStrapBlockCounter = 0;
  lowerStrapBlockCounter = 0;
  toolReset();
  ZylGummihalter.set(0);
  ZylAirBlower.set(0);
  ZylFalltuerschieber.set(0);
  ZylMagnetarm.set(0);
  MotFeedOben.set(0);
  MotFeedUnten.set(0);
  ZylMesser.set(0);
  ZylRevolverschieber.set(0);
  ZylSchild.set(0);
  machineRunning = false;
  errorBlink = false;
  greenBlink = false;
  stepMode = true;
  cycleStep = 0;
  cycleDurationTimer.setTime(eepromCounter.getValue(cycleDurationTime) * 1000);
  hideInfoField();
}

void runToolMotor() {

  // DEACTIVATE THE MOTOR IF THE END SWITCH HAS BEEN DETECTED
  if (endSwitch.switchedLow()) {
    Serial.println("END SWITCH DETECTED");
    MotorTool.set(0);
  }
}

int getTemperature() {
  const float voltsPerUnit = 0.03; // DATASHEET
  const float maxVoltage = 10;
  const float maxSensorTemp = 200;
  float sensorValue = analogRead(TEMP_SENSOR_PIN);
  float sensorVoltage = sensorValue * voltsPerUnit;
  float temperature = sensorVoltage / maxVoltage * maxSensorTemp;
  int temperatureInt = temperature;
  if (temperatureMonitor) {
    if (temperatureInt > eepromCounter.getValue(maxTemperature)) {
      errorBlink = true;
    }
  }

  return temperatureInt;
}

void lights() {

  // GREEN LIGHT
  //*****************************************************************************
  //in automatic mode the green light is on permanently:
  //in cycle step pause it is reserved for the "green blink" function
  if (!stepMode && !greenBlink) {
    if (machineRunning) {
      digitalWrite((GREEN_LIGHT_PIN), HIGH);
    } else {
      digitalWrite((GREEN_LIGHT_PIN), LOW);
    }
  }

  //in step mode the green is off between steps:
  if (stepMode) {
    if (machineRunning && clearanceNextStep) {
      digitalWrite((GREEN_LIGHT_PIN), HIGH);
    } else {
      digitalWrite((GREEN_LIGHT_PIN), LOW);
    }
  }

  // RED LIGHT / ERROR BLINKER
  //*****************************************************************************
  if (errorBlink) {
    if (errorBlinkTimer.delayTimeUp(1000)) {
      digitalWrite((RED_LIGHT_PIN), !(digitalRead(RED_LIGHT_PIN)));
    }
  } else {
    digitalWrite((RED_LIGHT_PIN), LOW);
  }
  // GREEN LIGHT / PAUSE BLINKER
  //*****************************************************************************
  if (greenBlink) {
    if (greenBlinkTimer.delayTimeUp(1000)) {
      digitalWrite((GREEN_LIGHT_PIN), !(digitalRead(GREEN_LIGHT_PIN)));
    }
  }
}

void runMainTestCycle() {

  if (clearanceNextStep && nextStepTimer.timedOut()) {
    static byte subStep = 1;

    switch (cycleStep) {

    //    case VIBRIEREN: // PLOMBE FALLENLASSEN
//      eepromCounter.getValue(cycleDurationTime);
//      cycleDurationTimer.setTime(eepromCounter.getValue(cycleDurationTime) * 1000);
//      ZylRevolverschieber.stroke(250, 300);    //(push time,release time)
//      if (ZylRevolverschieber.stroke_completed()) {
//        clearanceNextStep = false;
//        cycleStep++;
//      }
//      break;

    case KLEMMEN: // PLOMBEN IM RUTSCH-SCHACHT FIXIEREN
      errorBlink = !sealAvailable;
      if (sealAvailable) {
        ZylGummihalter.set(1); // Plomben fixieren
      } else { // MAGAZIN LEER!
        machineRunning = false;
        ZylGummihalter.set(0); // zum Befüllen zurückziehen
        break;
      }
      ZylMagnetarm.set(0);
      clearanceNextStep = false;
      nextStepTimer.setTime(300);
      cycleStep++;
      break;

    case FALLENLASSEN: // PLOMBE FALLENLASSEN
      if (subStep == (1)) {
        ZylFalltuerschieber.stroke(150, 40);    //(push time,release time)
        if (ZylFalltuerschieber.stroke_completed()) {
          subStep++;
        }
      }
      if (subStep == 2 || subStep == 3) {
        ZylFalltuerschieber.stroke(40, 40);    //(push time,release time)
        if (ZylFalltuerschieber.stroke_completed()) {
          subStep++;
        }
      }
      if (subStep == 4) {
        ZylFalltuerschieber.stroke(300, 40);    //(push time,release time)
        if (ZylFalltuerschieber.stroke_completed()) {
          subStep = 1;
          clearanceNextStep = false;
          cycleStep++;
        }
      }
      break;

    case MAGNETARM_AUSFAHREN:
      // PLOMBE ZUM ZANGENPAKET FAHREN

      // ZUERST SICHERSTELLEN DASS FALLTÜRE GESCHLOSSEN IST:
      if (subStep == 1) {
        ZylSchild.set(0);
        ZylFalltuerschieber.set(0);
        subStep++;
        break;
      }
      if (subStep == 2) {
        ZylMagnetarm.set(1);
        toolReset();    //reset tool "Wippenhebel ziehen"
        ZylGummihalter.set(0);    // Plombenfixieren lösen
        nextStepTimer.setTime(3000);
        clearanceNextStep = false;
        subStep = 1;
        cycleStep++;
      }
      break;

    case BAND_UNTEN:
      // UNTERES BAND VORSCHIEBEN
      ZylSchild.set(1);
      ZylGummihalter.set(0);    //Plomben für nächsten Zyklus können nachrutschen
      if (lowerStrapAvailable && upperStrapAvailable) {
        MotFeedUnten.stroke(eepromCounter.getValue(lowerFeedtime), 400);
        if (MotFeedUnten.stroke_completed()) {
          lowerStrapBlockCounter = 0;
          clearanceNextStep = false;
          cycleStep++;
        }
      } else {
        MotFeedUnten.abort_stroke();
        lowerStrapBlockCounter++;
        if (lowerStrapBlockCounter == 2) {
          machineRunning = false;
          errorBlink = true;
        }
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case ZURUECKFAHREN:
      // MAGNETARM ZURÜCKZIEHEN
      if (ZylMagnetarm.stroke_completed()) {
        //Serial.println("Mangetarm zurückfahren...");
      }
      ZylMagnetarm.set(0);
      nextStepTimer.setTime(600);
      clearanceNextStep = false;
      cycleStep++;
      break;

    case BAND_OBEN:
      // OBERES BAND VORSCHIEBEN
      ZylMesser.set(0);    // Messer muss zurückgezogen sein
      if (upperStrapAvailable && lowerStrapAvailable) {
        MotFeedOben.stroke(eepromCounter.getValue(upperFeedtime), 400);
        if (MotFeedOben.stroke_completed()) {
          upperStrapBlockCounter = 0;
          clearanceNextStep = false;
          cycleStep++;
        }
      } else {
        MotFeedOben.abort_stroke();
        upperStrapBlockCounter++;
        if (upperStrapBlockCounter == 2) {
          machineRunning = false;
          errorBlink = true;
        }
        nextStepTimer.setTime(500);
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case PRESSEN:
      // CRIMPVORGANG STARTEN
      if (subStep == 1) {
        MotorTool.set(1);
        subStep++;
      }
      if (subStep == 2) {
        if (!MotorTool.request_state()) {
          subStep = 1;
          clearanceNextStep = false;
          nextStepTimer.setTime(1000);
          cycleStep++;
        }
      }
      break;

    case SCHNEIDEN:
      // BAND ABSCHNEIDEN
      ZylSchild.set(1);
      ZylMesser.stroke(1500, 200);    // push,release [ms]
      if (ZylMesser.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case BLASEN:
      // BAND ABSCHNEIDEN
      ZylAirBlower.stroke(50, 50);    // push,release [ms]
      if (ZylAirBlower.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case REVOLVER:
      // KARUSSELL DREHEN FALLS KEINE PLOMBE DETEKTIERT:
      if (subStep == 1) {
        if (!sealAvailable) {
          subStep = 2;
        } else {
          subStep = 3;
        }
      }
      if (subStep == 2) {
        ZylRevolverschieber.stroke(3000, 2500);
        if (ZylRevolverschieber.stroke_completed()) {
          subStep++;
        }
      }
      if (subStep == 3) {
        ZylSchild.set(0);
        clearanceNextStep = false;
        subStep = 1;
        cycleStep++;
      }
      break;

    case PAUSE:
      // WARTEN AUF NÄCHSTEN ZYKLUS
      greenBlink = true;
      //Serial.println(cycleDurationTimer.remainingTimeoutTime()/1000);
      if (cycleDurationTimer.timedOut()) {
        cycleStep = 0;
        clearanceNextStep = false;
        //stepMode = true; // activate this line to deactivate auto mode after every cycle
        hideInfoField();
        greenBlink = false;
        eepromCounter.countOneUp(shorttimeCounter);
        eepromCounter.countOneUp(longtimeCounter);
      }
      break;
    }
  }
}

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
  sendToNextion();
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
  resetTestRig();
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

void nex_but_slider3_leftPushCallback(void *ptr) {
  eepromCounter.set(cycleDurationTime, eepromCounter.getValue(cycleDurationTime) - 10);
  if (eepromCounter.getValue(cycleDurationTime) < 0) {
    eepromCounter.set(cycleDurationTime, 0);
  }
}
void nex_but_slider3_rightPushCallback(void *ptr) {
  eepromCounter.set(cycleDurationTime, eepromCounter.getValue(cycleDurationTime) + 10);
  if (eepromCounter.getValue(cycleDurationTime) > 240) {
    eepromCounter.set(cycleDurationTime, 240);
  }
}
void nex_but_slider4_leftPushCallback(void *ptr) {
  eepromCounter.set(maxTemperature, eepromCounter.getValue(maxTemperature) - 5);
  if (eepromCounter.getValue(maxTemperature) < 20) {
    eepromCounter.set(maxTemperature, 20);
  }
}
void nex_but_slider4_rightPushCallback(void *ptr) {
  eepromCounter.set(maxTemperature, eepromCounter.getValue(maxTemperature) + 5);
  if (eepromCounter.getValue(maxTemperature) > 105) {
    eepromCounter.set(maxTemperature, 105);
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
  hideInfoField();

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
  nex_prev_cycleDurationTime = 0;
  nexPrevMaxTemperature = 0;
}
//*************************************************
// END OF TOUCH EVENT FUNCTIONS
//*************************************************

//*****************************************************************************
void nextionSetup()
//*****************************************************************************
{
  Serial2.begin(9600);

  // RESET NEXTION DISPLAY: (refresh display after PLC restart)
  sendToNextion(); // needed for unknown reasons
  Serial2.print("rest");
  sendToNextion();

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
  // PAGE 0 PUSH ONLY:
  nex_page0.attachPush(nex_page0PushCallback);

  // PAGE 1 PUSH ONLY:
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
  // PAGE 1 PUSH+POP:
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

  // PAGE 2 PUSH ONLY:
  nex_page2.attachPush(nex_page2PushCallback);
  nex_but_slider1_left.attachPush(nex_but_slider1_leftPushCallback);
  nex_but_slider1_right.attachPush(nex_but_slider1_rightPushCallback);
  nex_but_slider2_left.attachPush(nex_but_slider2_leftPushCallback);
  nex_but_slider2_right.attachPush(nex_but_slider2_rightPushCallback);
  nex_but_slider3_left.attachPush(nex_but_slider3_leftPushCallback);
  nex_but_slider3_right.attachPush(nex_but_slider3_rightPushCallback);
  nex_but_slider4_left.attachPush(nex_but_slider4_leftPushCallback);
  nex_but_slider4_right.attachPush(nex_but_slider4_rightPushCallback);

  // PAGE 2 PUSH+POP:
  nex_but_reset_shorttimeCounter.attachPush(nex_but_reset_shorttimeCounterPushCallback);
  nex_but_reset_shorttimeCounter.attachPop(nex_but_reset_shorttimeCounterPopCallback);

  //*****************************************************************************
  // END OF REGISTER
  //*****************************************************************************
  delay(2000);
  sendCommand("page 1");  //SWITCH NEXTION TO PAGE X
  sendToNextion();

}  // END OF NEXTION SETUP
//*****************************************************************************
void nextionLoop()
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
      sendToNextion();
      nexStateMachineRunning = !nexStateMachineRunning;
    }

    // UPDATE SWITCHSTATE "STEP"/"AUTO"-MODE:
    if (stepMode != nex_prev_stepMode) {
      if (stepMode) {
        Serial2.print("click bt1,1");
      } else {
        Serial2.print("click bt1,1");
      }
      sendToNextion();
      nex_prev_stepMode = stepMode;
    }

    // DISPLAY IF MAGAZINE IS EMPTY:
    if (nex_state_sealAvailable != sealAvailable) {
      if (!sealAvailable) {
        showInfoField();
        printOnTextField("MAGAZIN LEER!", "t4");
      } else {
        clearTextField("t4");
        hideInfoField();
      }
      nex_state_sealAvailable = sealAvailable;
    }
    if (bandsensorOben.switchedLow()) {
      showInfoField();
      printOnTextField("BAND OBEN", "t4");
    }
    if (bandsensorUnten.switchedLow()) {
      showInfoField();
      printOnTextField("BAND UNTEN", "t4");
    }
    if (upperStrapBlockCounter == 2 || lowerStrapBlockCounter == 2) {
      showInfoField();
      printOnTextField("BAND BLOCKIERT", "t4");
    }
    if (cycleStep == PAUSE) {
      String remainingWaitingTime = String(cycleDurationTimer.remainingTimeoutTime() / 1000);
      Serial.println(remainingWaitingTime);
      showInfoField();
      printOnTextField((remainingWaitingTime + " s"), "t4");
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
      sendToNextion();
      nex_state_ZylGummihalter = !nex_state_ZylGummihalter;
    }
    // UPDATE SWITCHBUTTON (dual state):
    if (ZylFalltuerschieber.request_state() != nex_state_ZylFalltuerschieber) {
      Serial2.print("click bt4,1");
      sendToNextion();
      nex_state_ZylFalltuerschieber = !nex_state_ZylFalltuerschieber;
    }
    // UPDATE SWITCHBUTTON (dual state):
    if (ZylMagnetarm.request_state() != nex_state_ZylMagnetarm) {
      Serial2.print("click bt3,1");
      sendToNextion();
      nex_state_ZylMagnetarm = !nex_state_ZylMagnetarm;
    }

    // UPDATE BUTTON (momentary):
    if (MotFeedOben.request_state() != nex_state_MotFeedOben) {
      if (MotFeedOben.request_state()) {
        Serial2.print("click b5,1");
      } else {
        Serial2.print("click b5,0");
      }
      sendToNextion();
      nex_state_MotFeedOben = MotFeedOben.request_state();
    }

    // UPDATE BUTTON (momentary):
    if (MotFeedUnten.request_state() != nex_state_MotFeedUnten) {
      if (MotFeedUnten.request_state()) {
        Serial2.print("click b4,1");
      } else {
        Serial2.print("click b4,0");
      }
      sendToNextion();
      nex_state_MotFeedUnten = MotFeedUnten.request_state();
    }

    // UPDATE BUTTON (momentary):
    if (ZylMesser.request_state() != nex_state_ZylMesser) {
      if (ZylMesser.request_state()) {
        Serial2.print("click b6,1");
      } else {
        Serial2.print("click b6,0");
      }
      sendToNextion();
      nex_state_ZylMesser = ZylMesser.request_state();
    }

    // UPDATE BUTTON (momentary):
    if (ZylRevolverschieber.request_state() != nex_state_ZylRevolverschieber) {
      if (ZylRevolverschieber.request_state()) {
        Serial2.print("click b3,1");
      } else {
        Serial2.print("click b3,0");
      }
      sendToNextion();
      nex_state_ZylRevolverschieber = ZylRevolverschieber.request_state();
    }

    // UPDATE BUTTON (momentary):
    if (MotorTool.request_state() != nex_state_PressMotor) {
      if (MotorTool.request_state()) {
        Serial2.print("click b7,1");
      } else {
        Serial2.print("click b7,0");
      }
      sendToNextion();
      nex_state_PressMotor = MotorTool.request_state();
    }

  }    //END PAGE 1
//*****************************************************************************
  if (CurrentPage == 2) {
    //*******************
    // PAGE 2 - LEFT SIDE
    //*******************

    if (nex_prev_upperFeedtime != eepromCounter.getValue(upperFeedtime)) {
      printOnTextField(String(eepromCounter.getValue(upperFeedtime)) + " ms", "t4");
      nex_prev_upperFeedtime = eepromCounter.getValue(upperFeedtime);
    }

    if (nex_prev_lowerFeedtime != eepromCounter.getValue(lowerFeedtime)) {
      printOnTextField(String(eepromCounter.getValue(lowerFeedtime)) + " ms", "t6");
      nex_prev_lowerFeedtime = eepromCounter.getValue(lowerFeedtime);
    }
    if (nex_prev_cycleDurationTime != eepromCounter.getValue(cycleDurationTime)) {
      printOnTextField(String(eepromCounter.getValue(cycleDurationTime)) + " s", "t1");
      nex_prev_cycleDurationTime = eepromCounter.getValue(cycleDurationTime);
    }
    if (nexPrevMaxTemperature != eepromCounter.getValue(maxTemperature)) {
      if (eepromCounter.getValue(maxTemperature) < 105) {
        temperatureMonitor = true;
        printOnTextField(String(eepromCounter.getValue(maxTemperature)), "t14");
      } else {
        temperatureMonitor = false;
        printOnTextField("OFF", "t14");
      }
      nexPrevMaxTemperature = eepromCounter.getValue(maxTemperature);
    }
    // TODO: IF TEMPERATURE HAS CHANGED MORE THAN ONE DEGREE, UPDATE:
    //if (abs(nexPrevCurrentTemperature - getTemperature()) > 1) {
    if (nexPrevCurrentTemperature != getTemperature()) {
      printOnTextField("t=" + String(getTemperature()), "t15");
      nexPrevCurrentTemperature = getTemperature();
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
//******************######**#######*#######*#******#*######********************
//*****************#********#**********#****#******#*#*****#*******************
//******************####****#####******#****#******#*######********************
//***********************#**#**********#****#******#*#*************************
//*****************######***######*****#*****######**#*************************
//*****************************************************************************
void setup() {
  eepromCounter.setup(eepromMinAddress, eepromMaxAddress, numberOfEepromValues);
  Serial.begin(115200);
  nextionSetup();
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(RED_LIGHT_PIN, OUTPUT);
  resetTestRig();
  motorStartButton.setDebounceTime(10);
  endSwitch.setDebounceTime(10);
  Serial.println("EXIT SETUP");
}
//*****************************************************************************
//********************#*********#####***#####***######*************************
//********************#********#*****#*#*****#**#*****#************************
//********************#********#*****#*#*****#**######*************************
//********************#********#*****#*#*****#**#******************************
//********************#######***#####***#####***#******************************
//*****************************************************************************
void loop() {

  getTemperature();

  upperStrapAvailable = bandsensorOben.requestButtonState();
  if (!upperStrapAvailable) {
    MotFeedOben.set(0);
  }
  lowerStrapAvailable = bandsensorUnten.requestButtonState();
  if (!lowerStrapAvailable) {
    MotFeedUnten.set(0);
  }

  // IN AUTO MODE, MACHINE RUNS FROM STEP TO STEP AUTOMATICALLY:
  if (!stepMode) {  // = AUTO MODE
    clearanceNextStep = true;
  }

  // IN STEP MODE, MACHINE STOPS AFTER EVERY COMPLETED CYCLYE:
  if (stepMode && !clearanceNextStep) {
    machineRunning = false;
  }

  if (machineRunning) {
    runMainTestCycle();
  }

  nextionLoop();
  runToolMotor(); // if the right conditions apply
  lights();
  sealAvailable = digitalRead(SENSOR_PLOMBE);
  getTemperature();

  //runtime = millis() - runtimeStopwatch;
  //Serial.println(runtime);
  //runtimeStopwatch = millis();
}
