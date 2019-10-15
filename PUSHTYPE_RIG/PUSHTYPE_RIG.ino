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
 * ADD ERROR BLINK AND MESSAGE IF NO STRAP IS DETECTED AT THE BEGINNING OF THE
 * FEED CYCLE.
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

// KNOBS AND POTENTIOMETERS:
const byte START_BUTTON = CONTROLLINO_A1;
const byte STOP_BUTTON = CONTROLLINO_A0;
const byte GREEN_LIGHT_PIN = CONTROLLINO_D12;
const byte RED_LIGHT_PIN = CONTROLLINO_D11;
const byte TOOL_MOTOR_RELAY = CONTROLLINO_R5;
const byte TOOL_END_SWITCH_PIN = CONTROLLINO_A4;
const byte BANDSENSOR_OBEN = CONTROLLINO_A5;
const byte BANDSENOR_UNTEN = CONTROLLINO_A6;
//const byte STEP_MODE_BUTTON = CONTROLLINO_A2;
//const byte AUTO_MODE_BUTTON = CONTROLLINO_A4;

// SENSORS:
const byte SENSOR_PLOMBE = CONTROLLINO_A3;

//OTHER VARIABLES:
bool machineRunning = false;
bool stepMode = true;
bool clearancePlayPauseToggle = true;
bool clearanceNextStep = false;
bool errorBlink = false;
bool sealAvailable = false;
bool upperStrapAvailable = false;
bool lowerStrapAvailable = false;
bool toolMotorState = 0;

byte cycleStep = 0;

unsigned long runtime;
unsigned long runtimeStopwatch;
//String cycleName;

// SET UP EEPROM COUNTER:
enum eepromCounter {
  upperFeedtime, lowerFeedtime, shorttimeCounter, longtimeCounter, endOfEepromEnum
};
int numberOfEepromValues = endOfEepromEnum;
int eepromSize = 4096;
EEPROM_Counter eepromCounter(eepromSize, numberOfEepromValues);

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
Cylinder MotorTool(TOOL_MOTOR_RELAY);

Insomnia nextStepTimer;
Insomnia errorBlinkTimer;

Debounce motorStartButton(START_BUTTON);
Debounce endSwitch(TOOL_END_SWITCH_PIN);
//*****************************************************************************
// DEFINE NAMES AND SEQUENCE OF STEPS FOR THE MAIN CYCLE:
//*****************************************************************************
enum mainCycleSteps {
  VIBRIEREN,
  KLEMMEN,
  FALLENLASSEN,
  MAGNETARM_AUSFAHREN,
  BAND_UNTEN,
  ZENTRIEREN,
  BAND_OBEN,
  VORPRESSEN,
  ZURUECKFAHREN,
  PRESSEN,
  SCHNEIDEN,
  LEERLAUF,
  REVOLVER,
  RESET,
  endOfMainCycleEnum
};

int numberOfMainCycleSteps = endOfMainCycleEnum;
// DEFINE NAMES TO DISPLAY ON THE TOUCH SCREEN:
String cycleName[] = { "VIBRIEREN", "KLEMMEN", "FALLENLASSEN", "AUSFAHREN", "BAND UNTEN",
    "ZENTRIEREN", "BAND OBEN", "VORPRESSEN", "ZURUECKFAHREN", "PRESSEN", "SCHNEIDEN", "LEERLAUF",
    "REVOLVER", "RESET" };

void TestRigReset() {
  ToolReset();
  ZylGummihalter.set(0);
  ZylFalltuerschieber.set(0);
  ZylMagnetarm.set(0);
  MotFeedOben.set(0);
  MotFeedUnten.set(0);
  ZylMesser.set(0);
  ZylRevolverschieber.set(0);
  machineRunning = false;
  errorBlink = false;
  stepMode = true;
  cycleStep = 0;
}
void ToolReset() {
  // SIMULIERE WIPPENHEBEL ZIEHEN:
  digitalWrite(CONTROLLINO_RELAY_08, LOW);  //WIPPENSCHALTER WHITE CABLE (NO)
  digitalWrite(CONTROLLINO_RELAY_09, HIGH); //WIPPENSCHALTER RED   CABLE (NC)
  delay(200);
  // SIMULIERE WIPPENHEBEL LOSLASEN:
  digitalWrite(CONTROLLINO_RELAY_09, LOW);  //WIPPENSCHALTER RED   CABLE (NC)
  digitalWrite(CONTROLLINO_RELAY_08, HIGH); //WIPPENSCHALTER WHITE CABLE (NO)delay(200);
}
void RunToolMotor() {

  // DEACTIVATE THE MOTOR IF THE END SWITCH HAS BEEN DETECTED
  if (endSwitch.switchedLow()) {
    Serial.println("END SWITCH DETECTED");
    MotorTool.set(0);
  }
}
//*****************************************************************************
//******************######**#######*#######*#******#*######********************
//*****************#********#**********#****#******#*#*****#*******************
//******************####****#####******#****#******#*######********************
//***********************#**#**********#****#******#*#*************************
//*****************######***######*****#*****######**#*************************
//*****************************************************************************
void setup() {
  Serial.begin(115200);
  nextionSetup();
  pinMode(STOP_BUTTON, INPUT);
  pinMode(START_BUTTON, INPUT);
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(RED_LIGHT_PIN, OUTPUT);
  TestRigReset();
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

  upperStrapAvailable = digitalRead(BANDSENSOR_OBEN);
  if (!upperStrapAvailable) {
    MotFeedOben.set(0);
  }
  lowerStrapAvailable = digitalRead(BANDSENOR_UNTEN);
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
    RunMainTestCycle();
  }

  NextionLoop();
  RunToolMotor(); // if the right conditions apply
  Lights();
  sealAvailable = digitalRead(SENSOR_PLOMBE);

  //runtime = millis() - runtimeStopwatch;
  //Serial.println(runtime);
  //runtimeStopwatch = millis();
}
