/*
* *****************************************************************************
* Cylinder.cpp
* Library for switching pneumatic cylinder valves.
* Michael Wettstein
* November 2018, Zürich
* *****************************************************************************
*/

#include "Arduino.h"
#include "Cylinder.h"

Cylinder::Cylinder(int pin)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
}
//***************************************************************************
//LIBRARY FUNCTIONS
//***************************************************************************
void Cylinder::stroke(int push_time, int release_time)
{
  if (_stroke_completed == true) //RESET TIMER ONCE AT BEGINNING
  {
    digitalWrite(_pin, HIGH);
    _state = HIGH;
    _prev_time = millis();
    _stroke_completed = false;
  }
  if (millis() - _prev_time > push_time)
  {
    digitalWrite(_pin, LOW);
    _state = LOW;
    if (millis() - _prev_time > push_time + release_time)
    {
      _stroke_completed = true; //TRIGGERS NEXT CYCLE STATE
    }
  }
}
//***************************************************************************
bool Cylinder::stroke_completed()
{
  return _stroke_completed;
}
//***************************************************************************
void Cylinder::toggle()
{
  _state = !_state;
  digitalWrite((_pin), _state);
}
//***************************************************************************
void Cylinder::set(bool set_state)
{
  digitalWrite(_pin, set_state);
  _state = set_state;
}
//***************************************************************************
bool Cylinder::request_state()
{
  _state = (digitalRead(_pin));
  return _state;
}
//***************************************************************************
