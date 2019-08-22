void lights()
{
  //GREEN LIGHT
  //*****************************************************************************
  //in automatic mode the green light is on permanently:
  if (step_mode == false)
  {
    if (machine_running == true)
    {
      digitalWrite((green_light), HIGH);
    }
    else
    {
      digitalWrite((green_light), LOW);
    }
  }

  //in step mode the green is off between steps:
  if (step_mode == true)
  {
    if (machine_running == true && clearance_next_step == true)
    {
      digitalWrite((green_light), HIGH);
    }
    else
    {
      digitalWrite((green_light), LOW);
    }
  }

  //RED LIGHT / ERROR BLINKER
  //*****************************************************************************
  if (error_blink == true)
  {

    if (millis() >= timer_error_blink)
    {
      digitalWrite((red_light), !(digitalRead(red_light)));
      timer_error_blink = millis() + 700;
    }
  }
  if (error_blink == false)
  {
    digitalWrite((red_light), LOW);
  }
}
