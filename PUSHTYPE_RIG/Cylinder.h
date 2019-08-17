/*
* *****************************************************************************
* Cylinder.h
* Library for switching pneumatic cylinder valves.
* Michael Wettstein
* November 2018, Zürich
* *****************************************************************************
*/

#ifndef Cylinder_h
#define Cylinder_h

#include "Arduino.h"

class Cylinder
{
  public:
    Cylinder(int pin);
    void stroke(int push_time, int release_time);
    void toggle();
    void set(bool set_state);
    bool request_state();
    bool stroke_completed();

  private:
    int _pin;
    bool _state;
    unsigned long _prev_time;
    bool _stroke_completed = true;
};

#endif
