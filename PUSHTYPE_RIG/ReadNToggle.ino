void ReadNToggle() {
  /*
   //*****************************************************************************
   //READ MODE BUTTON (STEP or AUTO-MODE):

   if (digitalRead(STEP_MODE_BUTTON) == HIGH)
   {
   stepMode = true;
   }
   if (digitalRead(AUTO_MODE_BUTTON) == HIGH)
   {
   stepMode = false;
   }
   */
  //*****************************************************************************
  //IN AUTO MODE, MACHINE RUNS FROM STEP TO STEP AUTOMATICALLY:
  if (stepMode == false)  //=AUTO MODE
          {
    clearanceNextStep = true;
  }
  //*****************************************************************************
  //IN STEP MODE, MACHINE STOPS AFTER EVERY COMPLETED CYCLYE:

  if (stepMode == true && clearanceNextStep == false) {
    machineRunning = false;
  }
  /*
  //*****************************************************************************
  //START TEST RIG:

  if (digitalRead(START_BUTTON) == HIGH) {
    machineRunning = true;
    clearanceNextStep = true;
  }
  */
    //*****************************************************************************
  //STOP TEST_RIG:

  if (digitalRead(STOP_BUTTON) == HIGH) {
    machineRunning = false;
  }
  //*****************************************************************************
  //READ SEAL DETECTION SENSOR:

  if (!digitalRead(sensor_plombe) == false)  //=Plombe detektiert
          {
    sealAvailable = true;
  } else {
    sealAvailable = false;
  }
  //*****************************************************************************

}
