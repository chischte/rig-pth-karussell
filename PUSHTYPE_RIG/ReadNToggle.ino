void ReadNToggle() {

  // IN AUTO MODE, MACHINE RUNS FROM STEP TO STEP AUTOMATICALLY:
  if (!stepMode) {  // =AUTO MODE
    clearanceNextStep = true;
  }

  // IN STEP MODE, MACHINE STOPS AFTER EVERY COMPLETED CYCLYE:
  if (stepMode && !clearanceNextStep) {
    machineRunning = false;
  }

  // STOP TEST_RIG:
  //machineRunning = digitalRead(STOP_BUTTON);

  // READ SEAL DETECTION SENSOR:
  sealAvailable = digitalRead(SENSOR_PLOMBE);
}
