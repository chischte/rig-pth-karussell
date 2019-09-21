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

#include <Cylinder.h>       // https://github.com/chischte/cylinder-library
#include <EEPROM_Counter.h> // https://github.com/chischte/eeprom-counter-library.git
#include <Nextion.h>        // https://github.com/itead/ITEADLIB_Arduino_Nextion
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
const byte GREEN_LIGHT_PIN = CONTROLLINO_D12;
const byte RED_LIGHT_PIN = CONTROLLINO_D11;

// SENSORS:
const byte sensor_plombe = CONTROLLINO_A3;

//OTHER VARIABLES:
boolean machineRunning = false;
boolean stepMode = true;
boolean clearancePlayPauseToggle = true;
boolean clearanceNextStep = false;
boolean errorBlink = false;
boolean sealAvailable = false;

byte cycleStep = 1;
byte nexPrevCycleStep;

long upperFeedtime; //LONG because EEPROM function
long lowerFeedtime; //LONG because EEPROM function
long shorttimeCounter; //LONG because EEPROM function
long longtimeCounter; //LONG because EEPROM function

unsigned long timeNextStep;
unsigned long timerErrorBlink;
unsigned long runtime;
unsigned long runtimeStopwatch;
unsigned long prev_time;

//*****************************************************************************
// GENERATE INSTANCES OF CLASS "Cylinder" FOR VALVES / MOTORS:
//*****************************************************************************
Cylinder ZylGummihalter(CONTROLLINO_D6);
Cylinder ZylFalltuerschieber(CONTROLLINO_D7);
Cylinder ZylMagnetarm(CONTROLLINO_D8);
Cylinder MotFeedOben(CONTROLLINO_D0);
Cylinder MotFeedUnten(CONTROLLINO_D1);
Cylinder ZylMesser(CONTROLLINO_D3);
Cylinder ZylRevolverschieber(CONTROLLINO_D2);

//*****************************************************************************
//******************######**#######*#######*#******#*######********************
//*****************#********#**********#****#******#*#*****#*******************
//******************####****#####******#****#******#*######********************
//***********************#**#**********#****#******#*#*************************
//*****************######***######*****#*****######**#*************************
//*****************************************************************************
void setup()
{
  Serial.begin(57600); //start serial connection

  nextionSetup();

  pinMode(STOP_BUTTON, INPUT);
  pinMode(START_BUTTON, INPUT);
  pinMode(STEP_MODE_BUTTON, INPUT);
  pinMode(AUTO_MODE_BUTTON, INPUT);
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(RED_LIGHT_PIN, OUTPUT);
  Reset();
  Serial.println("EXIT SETUP");
}
//*****************************************************************************
//********************#*********#####***#####***######*************************
//********************#********#*****#*#*****#**#*****#************************
//********************#********#*****#*#*****#**######*************************
//********************#********#*****#*#*****#**#******************************
//********************#######***#####***#####***#******************************
//*****************************************************************************
void loop()
{

  ReadNToggle();
  Lights();

  if (machineRunning == true)
  {
    RunMainTestCycle();
  }
  NextionLoop();
  EEPROM_Update();

  //runtime = millis() - runtimeStopwatch;
  //Serial.println(runtime);
  //runtimeStopwatch = millis();

}
