/*
 * *****************************************************************************
 * PUSHTYPE_RIG
 * *****************************************************************************
 * Program to control the Push Type 100.000 Cycles Test Rig "Solinger Karussell"
 * *****************************************************************************
 * Michael Wettstein
 * Dezember 2018, Zürich
 * *****************************************************************************
 */

#include <Controllino.h>
#include <Nextion.h>        // https://github.com/itead/ITEADLIB_Arduino_Nextion
#include <Cylinder.h>       // https://github.com/chischte/cylinder-library
#include <Debounce.h>       // https://github.com/chischte/debounce-library.git
#include <EEPROM_Counter.h> // https://github.com/chischte/eeprom-counter-library.git
#include <Insomnia.h>       // https://github.com/chischte/insomnia-delay-library.git

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

byte cycleStep = 1;
byte nexPrevCycleStep;

unsigned long runtime;
unsigned long runtimeStopwatch;

// SET UP EEPROM COUNTER:
enum counter {
  upperFeedtime, lowerFeedtime, shorttimeCounter, longtimeCounter, endOfEnum
};

int numberOfValues = endOfEnum;
int eepromSize = 4096;
EEPROM_Counter eepromCounter(eepromSize, numberOfValues);

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

Insomnia nextStepTimer;
Insomnia errorBlinkTimer;

Debounce motorStartButton(START_BUTTON);
Debounce endSwitch(TOOL_END_SWITCH_PIN);
//*****************************************************************************
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
  stepMode = true;
  cycleStep = 1;
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
  // Not the state of the motor-start-push-button is essential, but the state-change!
  // Like this it is easily possible to deactivate the motor, even when
  // the motor button is still being pushed.

  // ACTIVATE THE MOTOR IF THE START BUTTON HAS BEEN PUSHED:
  if (motorStartButton.switchedHigh()) { // button pushed
    digitalWrite(TOOL_MOTOR_RELAY, HIGH);
  }
  // DEACTIVATE THE MOTOR IF THE BUTTON HAS  BEEN RELEASED:
  if (motorStartButton.switchedLow()) { // button released
    digitalWrite(TOOL_MOTOR_RELAY, LOW);
  }
  // DEACTIVATE THE MOTOR IF THE END SWITCH HAS BEEN DETECTED
  if (endSwitch.switchedLow()) {
    digitalWrite(TOOL_MOTOR_RELAY, LOW);
    Serial.println("END SWITCH DETECTED");
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
  pinMode(TOOL_MOTOR_RELAY, INPUT);
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(RED_LIGHT_PIN, OUTPUT);
  TestRigReset();
  ToolReset();
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
