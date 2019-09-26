//TODO:
//SYNCHRONIZE CYCLE NAMESCYCLE STEPS, CYCLE ORDER OF NEXTION AND MAIN CYCLE!
//FIND OUT HOW TO DETECT CORRECT USB SLOT!

void RunMainTestCycle() {
  if (clearanceNextStep && nextStepTimer.timedOut()) {

    switch (cycleStep) {
    case KLEMMEN: // PLOMBEN IM RUTSCH-SCHACHT FIXIEREN
      errorBlink = !sealAvailable;
      if (sealAvailable) {
        ZylGummihalter.set(1); // Plomben fixieren
      } else { // MAGAZIN LEER!
        machineRunning = false;
        ZylGummihalter.set(0); // zum Befüllen zurückziehen
        break;
      }
      ZylMagnetarm.set(0);
      clearanceNextStep = false;
      nextStepTimer.setTime(500);
      cycleStep++;
      break;
    case FALLENLASSEN: // PLOMBE FALLENLASSEN
      ZylFalltuerschieber.stroke(700, 200);    //(push time,release time)
      if (ZylFalltuerschieber.stroke_completed()) {
        ZylGummihalter.set(0); // Plombenfixieren lösen
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
    case MAGNETARM_AUSFAHREN: // PLOMBE ZUM ZANGENPAKET FAHREN
      ZylMagnetarm.set(1);
      ToolReset();    //reset tool "Wippenhebel ziehen"
      nextStepTimer.setTime(500);
      clearanceNextStep = false;
      cycleStep++;
      break;
    case BAND_UNTEN: // UNTERES BAND VORSCHIEBEN
      ZylGummihalter.set(0);    //Plomben für nächsten Zyklus können nachrutschen
      MotFeedUnten.stroke(eepromCounter.getValue(lowerFeedtime), 0);
      if (MotFeedUnten.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
    case ZENTRIEREN: // PLOMBE DURCH PRESSMECHANIK ZENTRIEREN
      Pressmotor.stroke(200, 0);
      if (Pressmotor.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
    case BAND_OBEN: // OBERES BAND VORSCHIEBEN
      ZylMesser.set(0); // Messer muss zurückgezogen sein
      MotFeedOben.stroke(eepromCounter.getValue(upperFeedtime), 0);
      if (MotFeedOben.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
    case VORPRESSEN: // PLOMBE DURCH PRESSMECHANIK ZENTRIEREN
      Pressmotor.stroke(300, 0);
      if (Pressmotor.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
    case ZURUECKFAHREN: // MAGNETARM ZURÜCKZIEHEN
      if (ZylMagnetarm.stroke_completed()) {
        //Serial.println("Mangetarm zurückfahren...");
      }
      ZylMagnetarm.set(0);
      nextStepTimer.setTime(500);
      clearanceNextStep = false;
      cycleStep++;
      break;
    case SCHNEIDEN: // BAND ABSCHNEIDEN
      if (ZylMesser.stroke_completed()) {
      }
      ZylMesser.stroke(1000, 0); // push,release [ms]
      if (ZylMesser.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
    case PRESSEN: // CRIMPVORGANG STARTEN
      digitalWrite(TOOL_MOTOR_RELAY, HIGH);
      nextStepTimer.setTime(1500);
      clearanceNextStep = false;
      cycleStep++;
      break;
    case REVOLVER: // KARUSSELL DREHEN FALLS KEINE PLOMBE DETEKTIERT
      if (!sealAvailable) { // keine Plombe detektiert
        ZylRevolverschieber.stroke(5000, 5000);
      }
      if (ZylRevolverschieber.stroke_completed()) {
        nextStepTimer.setTime(500);
        clearanceNextStep = true;
        cycleStep++;
      }
      break;
    case RESET: // RESET FÜR NÄCHSTEN ZYKLUS
      eepromCounter.countOneUp(shorttimeCounter);
      eepromCounter.countOneUp(longtimeCounter);
      cycleStep = 0;
      clearanceNextStep = false;
      break;
    }
  }
}
