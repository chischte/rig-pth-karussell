//void read_n_toggle();
//void nextion_loop();
//void nextion_setup();
//void lights();
//void run_main_test_cycle();

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//PROGRAMM TO CONTROL THE PUSHTYPE 100.000 CYCLES TEST RIG "SOLINGER-KARUSELL"
//*****************************************************************************
//*****************************************************************************
//****************************************************************************
#include "Cylinder.h"
#include <Nextion.h>
#include <Controllino.h> //CONTROLLINO_xx aliases
///*
#define CONTROLLINO_D0   2 
#define CONTROLLINO_D1   3
#define CONTROLLINO_D2   4
#define CONTROLLINO_D3   5
#define CONTROLLINO_D4   6
#define CONTROLLINO_D5   7
#define CONTROLLINO_D6   8
#define CONTROLLINO_D7   9
#define CONTROLLINO_D8  10
#define CONTROLLINO_D9  11
#define CONTROLLINO_D10 12
#define CONTROLLINO_D11 13
#define CONTROLLINO_D12 42
#define CONTROLLINO_D13 43
#define CONTROLLINO_D14 44
#define CONTROLLINO_D15 45
#define CONTROLLINO_D16 46
#define CONTROLLINO_D17 47
#define CONTROLLINO_D18 48
#define CONTROLLINO_D19 49
#define CONTROLLINO_A0  54
#define CONTROLLINO_A1  55
#define CONTROLLINO_A2  56
#define CONTROLLINO_A3  57
#define CONTROLLINO_A4  58
#define CONTROLLINO_A5  59
#define CONTROLLINO_A6  60
#define CONTROLLINO_A7  61
#define CONTROLLINO_A8  62
#define CONTROLLINO_A9  63

//*****************************************************************************
//PRE-SETUP SECTION / PIN LAYOUT
//*****************************************************************************

//*****************************************************************************
//KNOBS AND POTENTIOMETERS
//*****************************************************************************
#define start_button CONTROLLINO_A1
#define stop_button CONTROLLINO_A0
#define  step_mode_button CONTROLLINO_A2
#define auto_mode_button CONTROLLINO_A4
#define green_light CONTROLLINO_D12
#define red_light CONTROLLINO_D11
//*****************************************************************************
//SENSORS
//*****************************************************************************
#define sensor_plombe CONTROLLINO_A3
//*****************************************************************************
//VALVES / MOTORS
//*****************************************************************************
Cylinder zyl_gummihalter (CONTROLLINO_D6);
Cylinder zyl_falltuerschieber (CONTROLLINO_D7);
Cylinder zyl_magnetarm (CONTROLLINO_D8);
Cylinder mot_feed_oben (CONTROLLINO_D0);
Cylinder mot_feed_unten (CONTROLLINO_D1);
Cylinder zyl_messer(CONTROLLINO_D3);
Cylinder zyl_revolverschieber (CONTROLLINO_D2);
Cylinder zyl_loescherblink(CONTROLLINO_D11);
//*****************************************************************************
//DECLARATION OF VARIABLES / DATA TYPES
//*****************************************************************************
//boolean (true/false)
//byte (0-255)
//int   (-32,768 to 32,767) / unsigned int: 0 to 65,535
//long  (-2,147,483,648 to 2,147,483,647)
//float (6-7 Digits)
//*****************************************************************************
boolean machine_running = false;
boolean step_mode = true;

boolean clearance_play_pause_toggle = true;
boolean clearance_next_step = false;
boolean error_blink = false;
boolean seal_available = false;

byte cycle_step = 1;
byte nex_prev_cycle_step;

int timer_next_step;

long upper_feedtime;//LONG because EEPROM function
long lower_feedtime;//LONG because EEPROM function
long shorttime_counter;//LONG because EEPROM function
long longtime_counter;//LONG because EEPROM function

unsigned long timer_error_blink;
unsigned long runtime;
unsigned long runtime_stopwatch;
unsigned long prev_time;

//*****************************************************************************
//******************######**#######*#######*#******#*######********************
//*****************#********#**********#****#******#*#*****#*******************
//******************####****#####******#****#******#*######********************
//***********************#**#**********#****#******#*#*************************
//*****************######***######*****#*****######**#*************************
//*****************************************************************************
void setup() {  
  //***************************************************************************
  Serial.begin(500000);//start serial connection

  nextion_setup();
 
  pinMode(stop_button, INPUT);
  pinMode(start_button, INPUT);
  pinMode(step_mode_button, INPUT);
  pinMode(auto_mode_button, INPUT);
  pinMode(green_light, OUTPUT);
  pinMode(red_light, OUTPUT);

  Serial.println("EXIT SETUP");
}
//*****************************************************************************
//*****************************************************************************
//********************#*********#####***#####***######*************************
//********************#********#*****#*#*****#**#*****#************************
//********************#********#*****#*#*****#**######*************************
//********************#********#*****#*#*****#**#******************************
//********************#######***#####***#####***#******************************
//*****************************************************************************
//*****************************************************************************
void loop() {

  read_n_toggle();
  lights();

  if (machine_running == true)
  {
    run_main_test_cycle();
  }
  nextion_loop();
  eeprom_update();

  //runtime = millis() - runtime_stopwatch;
  //Serial.println(runtime);
  //runtime_stopwatch = millis();

}//END MAIN LOOP
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
