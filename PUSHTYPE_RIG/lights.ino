int lights()
{
  //GREEN LIGHT
  //*****************************************************************************
  if (step_mode == false)//IN AUTOMATIC MODE THE GREEN LIGHT IS ON PERMANENTLY
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

    if (step_mode == true)//IN STEP MODE THE GREEN IS OFF BETWEEN STEPS
    if (machine_running == true && clearance_next_step == true)
    {
      digitalWrite((green_light), HIGH);
    }
    else
    {
      digitalWrite((green_light), LOW);
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
