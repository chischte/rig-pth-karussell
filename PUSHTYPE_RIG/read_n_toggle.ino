int read_n_toggle()
{
  //*****************************************************************************
  //READ MODE BUTTON (STEP or AUTO-MODE):

  if (digitalRead(step_mode_button) == HIGH)
  {
    step_mode = true;
  }
  if (digitalRead(auto_mode_button) == HIGH)
  {
    step_mode = false;
  }
  //*****************************************************************************
  //IN AUTO MODE, MACHINE RUNS FROM STEP TO STEP AUTOMATICALLY:

  if (step_mode == false)//=AUTO MODE
  {
    clearance_next_step = true;
  }
  //*****************************************************************************
  //IN STEP MODE, MACHINE STOPS AFTER EVERY COMPLETED CYCLYE:

  if (step_mode == true && clearance_next_step == false)
  {
    machine_running = false;
  }
  //*****************************************************************************
  //START TEST RIG:

  if (digitalRead(start_button) == HIGH)
  {
    machine_running = true;
    clearance_next_step = true;
  }
  //*****************************************************************************
  //STOP TEST_RIG:

  if (digitalRead(stop_button) == HIGH)
  {
    machine_running = false;
  }
  //*****************************************************************************
  //READ SEAL DETECTION SENSOR:

  if (digitalRead(sensor_plombe) == false)//=Plombe detektiert
  {
    seal_available = true;
  }
  else
  {
    seal_available = false;
  }
  //*****************************************************************************
}//END OF READ_N_TOGGLE
