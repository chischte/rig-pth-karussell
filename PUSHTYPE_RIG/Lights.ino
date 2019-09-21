void Lights() {
  //GREEN LIGHT
  //*****************************************************************************
  //in automatic mode the green light is on permanently:
  if (stepMode == false) {
    if (machineRunning == true) {
      digitalWrite((GREEN_LIGHT_PIN), HIGH);
    } else {
      digitalWrite((GREEN_LIGHT_PIN), LOW);
    }
  }

  //in step mode the green is off between steps:
  if (stepMode == true) {
    if (machineRunning == true && clearanceNextStep == true) {
      digitalWrite((GREEN_LIGHT_PIN), HIGH);
    } else {
      digitalWrite((GREEN_LIGHT_PIN), LOW);
    }
  }

  //RED LIGHT / ERROR BLINKER
  //*****************************************************************************
  if (errorBlink == true) {

    if (millis() >= timerErrorBlink) {
      digitalWrite((RED_LIGHT_PIN), !(digitalRead(RED_LIGHT_PIN)));
      timerErrorBlink = millis() + 700;
    }
  }
  if (errorBlink == false) {
    digitalWrite((RED_LIGHT_PIN), LOW);
  }
}
