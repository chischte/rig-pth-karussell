/*
 * *****************************************************************************
 * PUSHTYPE_RIG
 * *****************************************************************************
 * Program to control the Push Type 100.000 Cycles Test Rig "Solinger Karussell"
 * *****************************************************************************
 * Michael Wettstein
 * Dezember 2018, Zürich
 * *****************************************************************************
 * TODO: (can, should, must)
 * can:    implement new logic for main cycle:
 * can:    run if: (autoMode && autoModeRunning) || (!autoMode && stepModeRunning)
 * can:    implement stateController
 * can:    make info text on touchscreen better visible
 * should: start screen write test rig in two words
 */

#include <Controllino.h>    // https://github.com/CONTROLLINO-PLC/CONTROLLINO_Library
#include <Nextion.h>        // https://github.com/itead/ITEADLIB_Arduino_Nextion
#include <Cylinder.h>       // https://github.com/chischte/cylinder-library
#include <Debounce.h>       // https://github.com/chischte/debounce-library
#include <EEPROM_Counter.h> // https://github.com/chischte/eeprom-counter-library
#include <Insomnia.h>       // https://github.com/chischte/insomnia-delay-library

//*****************************************************************************
// DECLARATION OF VARIABLES / DATA TYPES
//*****************************************************************************
// boolean (true/false)
// byte (0-255)
// int   (-32,768 to 32,767) / unsigned int: 0 to 65,535
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
// MORE OUTPUT PINS ARE DEFINED IN "GENERATE INSTANCES OF CLASSES"

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

byte cycleStep = 0;
byte upperStrapBlockCounter = 0;
byte lowerStrapBlockCounter = 0;

unsigned long runtime;
unsigned long runtimeStopwatch;
//String cycleName;

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
  BAND_OBEN,
  ZURUECKFAHREN,
  PRESSEN,
  SCHNEIDEN,
  BLASEN,
  REVOLVER,
  PAUSE,
  endOfMainCycleEnum
};

int numberOfMainCycleSteps = endOfMainCycleEnum;

// DEFINE NAMES TO DISPLAY ON THE TOUCH SCREEN:
String cycleName[] = {       //
        /*"VIBRIEREN",*/     //
            "KLEMMEN",//
            "FALLENLASSEN",  //
            "AUSFAHREN",     //
            "BAND UNTEN",    //
            "BAND OBEN",     //
            "ZURUECKFAHREN", //
            "PRESSEN",       //
            "SCHNEIDEN",     //
            "BLASEN",        //
            "REVOLVER",      //
            "PAUSE"          //
        };

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
void toolReset() {
  // SIMULIERE WIPPENHEBEL ZIEHEN:
  digitalWrite(CONTROLLINO_RELAY_08, LOW);  //WIPPENSCHALTER WHITE CABLE (NO)
  digitalWrite(CONTROLLINO_RELAY_09, HIGH); //WIPPENSCHALTER RED CABLE (NC)
  delay(200);
  // SIMULIERE WIPPENHEBEL LOSLASEN:
  digitalWrite(CONTROLLINO_RELAY_09, LOW);  //WIPPENSCHALTER RED   CABLE (NC)
  digitalWrite(CONTROLLINO_RELAY_08, HIGH); //WIPPENSCHALTER WHITE CABLE (NO)delay(200);
}
void runToolMotor() {

  // DEACTIVATE THE MOTOR IF THE END SWITCH HAS BEEN DETECTED
  if (endSwitch.switchedLow()) {
    Serial.println("END SWITCH DETECTED");
    MotorTool.set(0);
  }
}
int getTemperature() {
  float temperature = analogRead(TEMP_SENSOR_PIN);
  Serial.println(temperature);
  int temperatureInt=temperature;
  return temperatureInt;
}
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
