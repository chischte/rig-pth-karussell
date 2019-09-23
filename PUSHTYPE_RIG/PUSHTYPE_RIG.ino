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
 * Timer durch insomniatimer ersetzen
 * bool==true und ==false in der Regel entfernen
 * Globale Variablen minimieren
 * Compiler Warnungen anschauen
 * Tool Reset timer erhöhen
 * Fix green light
 * *****************************************************************************
 */

#include <Cylinder.h>       // https://github.com/chischte/cylinder-library
#include <EEPROM_Counter.h> // https://github.com/chischte/eeprom-counter-library.git
#include <Nextion.h>        // https://github.com/itead/ITEADLIB_Arduino_Nextion
#include <Insomnia.h>    // https://github.com/chischte/insomnia-delay-library.git
#include <Controllino.h> 

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
const byte STEP_MODE_BUTTON = CONTROLLINO_A2;
const byte AUTO_MODE_BUTTON = CONTROLLINO_A4;
const byte GREEN_LIGHT_PIN = 42; //CONTROLLINO_D12 alias did not work 42
const byte RED_LIGHT_PIN = CONTROLLINO_D11;
const byte TOOL_MOTOR_RELAY = CONTROLLINO_R5;
const byte TOOL_END_SWITCH_PIN = CONTROLLINO_A4;

// SENSORS:
const byte sensor_plombe = CONTROLLINO_A3;

//OTHER VARIABLES:
bool machineRunning = false;
bool stepMode = true;
bool clearancePlayPauseToggle = true;
bool clearanceNextStep = false;
bool errorBlink = false;
bool sealAvailable = false;

byte cycleStep = 1;
byte nexPrevCycleStep;

//long upperFeedtime; //LONG because EEPROM function
//long lowerFeedtime; //LONG because EEPROM function
//long shorttimeCounter; //LONG because EEPROM function
//long longtimeCounter; //LONG because EEPROM function

unsigned long timeNextStep;
unsigned long timerErrorBlink;
unsigned long runtime;
unsigned long runtimeStopwatch;
unsigned long prev_time;

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

Insomnia ToolResetTimer(60000); //reset the tool every 60 seconds
//*****************************************************************************
void ToolReset() {
  // SIMULIERE WIPPENHEBEL ZIEHEN:
  digitalWrite(CONTROLLINO_RELAY_08, LOW);  //WIPPENSCHALTER WHITE CABLE (NO)
  delay(50);
  digitalWrite(CONTROLLINO_RELAY_09, HIGH); //WIPPENSCHALTER RED   CABLE (NC)
  delay(200);
  // SIMULIERE WIPPENHEBEL LOSLASEN:
  digitalWrite(CONTROLLINO_RELAY_09, LOW);  //WIPPENSCHALTER RED   CABLE (NC)
  delay(50);
  digitalWrite(CONTROLLINO_RELAY_08, HIGH); //WIPPENSCHALTER WHITE CABLE (NO)
  delay(100);
}

bool toolMotorState = LOW;
bool previousEndSwitchState;

void CheckToolEndSwitchDetected() {
  bool endSwitchState = (digitalRead(TOOL_END_SWITCH_PIN));
  if (endSwitchState != previousEndSwitchState) {
    if (endSwitchState == HIGH) {
      toolMotorState = LOW;
      Serial.println("END SWITCH DETECTED");
    }
  }
  previousEndSwitchState = endSwitchState;
}

bool previousMotorButtonState;

void CheckMotorStartButton() {
  bool motorButtonState = (digitalRead(START_BUTTON));
  if (motorButtonState != previousMotorButtonState) {
    if (motorButtonState == HIGH) { // BUTTON PUSHED
      toolMotorState = HIGH;
    }
    if (motorButtonState == LOW) { // BUTTON RELEASED
      toolMotorState = LOW;
    }
  }
  previousMotorButtonState = motorButtonState;
}

void RunToolMotor() {
  digitalWrite(TOOL_MOTOR_RELAY, toolMotorState);
}
//*****************************************************************************
//******************######**#######*#######*#******#*######********************
//*****************#********#**********#****#******#*#*****#*******************
//******************####****#####******#****#******#*######********************
//***********************#**#**********#****#******#*#*************************
//*****************######***######*****#*****######**#*************************
//*****************************************************************************
void setup() {
  Serial.begin(115200); //start serial connection

  nextionSetup();

  pinMode(STOP_BUTTON, INPUT);
  pinMode(START_BUTTON, INPUT);
  pinMode(STEP_MODE_BUTTON, INPUT);
  pinMode(AUTO_MODE_BUTTON, INPUT);
  pinMode(TOOL_MOTOR_RELAY, INPUT);
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(RED_LIGHT_PIN, OUTPUT);
  TestRigReset();
  ToolReset();
  //digitalWrite(GREEN_LIGHT_PIN,HIGH);
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

  ReadNToggle();
  Lights();

  if (machineRunning) {
    RunMainTestCycle();
  } else if (ToolResetTimer.timedOut() && toolMotorState == LOW) {
    ToolReset(); //restart the timeout countdown
    ToolResetTimer.resetTime();
  }
  NextionLoop();

  CheckMotorStartButton();
  CheckToolEndSwitchDetected();
  RunToolMotor();

//runtime = millis() - runtimeStopwatch;
//Serial.println(runtime);
//runtimeStopwatch = millis();

}
