void RunMainTestCycle() {

  if (clearanceNextStep && nextStepTimer.timedOut()) {

    switch (cycleStep) {
    //***************************************************************************
    case 1: // PLOMBEN WERDEN IM RUTSCH-SCHACHT FIXIERT
      //WENN DER SENSOR KEINE PLOMBE DETEKTIERT MUSS DER REVOLVER NEU GEFÜLLT WERDEN...
      //***************************************************************************
      // PLOMBEN FIXIEREN:
      errorBlink = !sealAvailable;
      if (sealAvailable) {
        ZylGummihalter.set(1); // Plomben fixieren
      } else { // MAGAZIN LEER!
        machineRunning = false;
        ZylGummihalter.set(0); // Gummihalter zum Befüllen zurückziehen
        cycleStep = 0; //reset für Neustart nach dem Befüllen
      }
      ZylMagnetarm.set(0); // Um ein Verklemmen nach Reset zu verhindern
      clearanceNextStep = false;
      nextStepTimer.setTime(500);
      cycleStep++;
      break;
      //***************************************************************************
    case 2: //EINE PLOMBE WIRD FALLENGELASSEN
      //***************************************************************************
      ZylFalltuerschieber.stroke(500, 0); //(push time,release time)
      if (ZylFalltuerschieber.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 3: //EINE PLOMBE WIRD ZUM ZANGENPAKET GEFAHREN
      //***************************************************************************
      ZylMagnetarm.set(1);
      ToolReset(); //reset tool "Wippenhebel ziehen"
      nextStepTimer.setTime(500);
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case 4: // DAS UNTERE BAND WIRD VORGESCHOBEN
      //PLOMBENFIXIERUNG IM RUTSCHSCHACHT WIRD AUFGEHOBEN
      //***************************************************************************
      ZylGummihalter.set(0); //Plomben für nächsten Zyklus können nachrutschen
      MotFeedUnten.stroke(eepromCounter.getValue(lowerFeedtime), 0);
      if (MotFeedUnten.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 5: // DAS OBERE BAND WIRD VORGESCHOBEN
      //***************************************************************************
      ZylMesser.set(0); // sicherstellen dass das Messer zurückgezogen ist
      MotFeedOben.stroke(eepromCounter.getValue(upperFeedtime), 0);

      if (MotFeedOben.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 6: // DAS KLAUENSYSTEM WIRD AKTIVIERT
      //***************************************************************************
      //Serial.println("PRESSEN");
      digitalWrite(TOOL_MOTOR_RELAY, HIGH);
      nextStepTimer.setTime(1500);
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case 7: // DAS BAND WIRD ABGESCHNITTEN
      //***************************************************************************
      if (ZylMesser.stroke_completed()) {
        //Serial.println("SCHNEIDEN");
      }
      ZylMesser.stroke(1500, 0); //(push time,release time)
      if (ZylMesser.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 8: // DER ZUFUHRZYLINDER FÄHRT ZURÜCK
      //***************************************************************************
      if (ZylMagnetarm.stroke_completed()) {
        //Serial.println("Mangetarm zurückfahren...");
      }
      ZylMagnetarm.set(0);
      nextStepTimer.setTime(500);
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case 9: // WENN DER SENSOR KEINE PLOMBE DETEKTIERT DREHT DER REVOLVERKOPF
      //***************************************************************************
      if (!sealAvailable) { // keine Plombe detektiert
        ZylRevolverschieber.stroke(5000, 5000);
      }
      if (ZylRevolverschieber.stroke_completed()) {
        nextStepTimer.setTime(500);
        clearanceNextStep = true;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 10: // RESET FÜR NÄCHSTEN ZYKLUS
      //***************************************************************************
      eepromCounter.countOneUp(shorttimeCounter);
      eepromCounter.countOneUp(longtimeCounter);
      cycleStep = 1;
      clearanceNextStep = false;
      break;
      //***************************************************************************
    } //END switch (cycleStep)
  }
} //END MAIN TEST CYCLE

