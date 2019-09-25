void Lights() {

  // GREEN LIGHT
  //*****************************************************************************
  //in automatic mode the green light is on permanently:
  if (!stepMode) {
    if (machineRunning) {
      digitalWrite((GREEN_LIGHT_PIN), HIGH);
    } else {
      digitalWrite((GREEN_LIGHT_PIN), LOW);
    }
  }

  //in step mode the green is off between steps:
  if (stepMode) {
    if (machineRunning && clearanceNextStep) {
      digitalWrite((GREEN_LIGHT_PIN), HIGH);
    } else {
      digitalWrite((GREEN_LIGHT_PIN), LOW);
    }
  }

  // RED LIGHT / ERROR BLINKER
  //*****************************************************************************
  if (errorBlink) {
    if (errorBlinkTimer.delayTimeUp(700)) {
      digitalWrite((RED_LIGHT_PIN), !(digitalRead(RED_LIGHT_PIN)));
    }
  } else {
    digitalWrite((RED_LIGHT_PIN), LOW);
  }
}
