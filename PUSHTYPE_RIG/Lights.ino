void lights() {

  // GREEN LIGHT
  //*****************************************************************************
  //in automatic mode the green light is on permanently:
  //in cycle step pause it is reserved for the "green blink" function
  if (!stepMode && !greenBlink) {
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
    if (errorBlinkTimer.delayTimeUp(1000)) {
      digitalWrite((RED_LIGHT_PIN), !(digitalRead(RED_LIGHT_PIN)));
    }
  } else {
    digitalWrite((RED_LIGHT_PIN), LOW);
  }
  // GREEN LIGHT / PAUSE BLINKER
  //*****************************************************************************
  if (greenBlink) {
    if (greenBlinkTimer.delayTimeUp(1000)) {
      digitalWrite((GREEN_LIGHT_PIN), !(digitalRead(GREEN_LIGHT_PIN)));
    }
  }
}

