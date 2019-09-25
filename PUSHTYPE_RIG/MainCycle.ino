//TODO:
//SYNCHRONIZE CYCLE NAMESCYCLE STEPS, CYCLE ORDER OF NEXTION AND MAIN CYCLE!
//FIND OUT HOW TO DETECT CORRECT USB SLOT!

void RunMainTestCycle() {

  enum mainCycleSteps {
    KLEMMEN,
    FALLENLASSEN,
    MAGNETARM_AUSFAHREN,
    ZENTRIEREN,
    BAND_UNTEN,
    BAND_OBEN,
    PRESSEN,
    ZURUECKFAHREN,
    SCHNEIDEN,
    REVOLVER,
    RESET
  };

  if (clearanceNextStep && nextStepTimer.timedOut()) {

    switch (cycleStep) {
    //***************************************************************************
    case KLEMMEN: // PLOMBEN WERDEN IM RUTSCH-SCHACHT FIXIERT
      //WENN DER SENSOR KEINE PLOMBE DETEKTIERT MUSS DER REVOLVER NEU GEFÜLLT WERDEN...
      //***************************************************************************
      //cycleName = "KLEMMEN";
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
    case FALLENLASSEN: // EINE PLOMBE WIRD FALLENGELASSEN
      //***************************************************************************
      //cycleName = "FALLENLASSEN";
      ZylFalltuerschieber.stroke(700, 0);    //(push time,release time)
      if (ZylFalltuerschieber.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case MAGNETARM_AUSFAHREN: // EINE PLOMBE WIRD ZUM ZANGENPAKET GEFAHREN
      //***************************************************************************
      //cycleName = "AUSFAHREN";
      ZylMagnetarm.set(1);
      ToolReset();    //reset tool "Wippenhebel ziehen"
      nextStepTimer.setTime(500);
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case ZENTRIEREN: // DIE PLOMBE WIRD ZENTRIERT
      //***************************************************************************
      //cycleName = "ZENTRIEREN";
      Pressmotor.stroke(100, 0);
      if (Pressmotor.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case BAND_UNTEN:        // DAS UNTERE BAND WIRD VORGESCHOBEN
      //PLOMBENFIXIERUNG IM RUTSCHSCHACHT WIRD AUFGEHOBEN
      //***************************************************************************
      //cycleName = "BAND UNTEN";
      ZylGummihalter.set(0);    //Plomben für nächsten Zyklus können nachrutschen
      MotFeedUnten.stroke(eepromCounter.getValue(lowerFeedtime), 0);
      if (MotFeedUnten.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case BAND_OBEN:    // DAS OBERE BAND WIRD VORGESCHOBEN
      //***************************************************************************
      //cycleName = "BAND OBEN";
      ZylMesser.set(0);    // sicherstellen dass das Messer zurückgezogen ist
      MotFeedOben.stroke(eepromCounter.getValue(upperFeedtime), 0);

      if (MotFeedOben.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case PRESSEN:    // DAS KLAUENSYSTEM WIRD AKTIVIERT
      //***************************************************************************
      //cycleName = "PRESSEN";
      digitalWrite(TOOL_MOTOR_RELAY, HIGH);
      nextStepTimer.setTime(1500);
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case ZURUECKFAHREN: // DER ZUFUHRZYLINDER FÄHRT ZURÜCK
      //***************************************************************************
      //cycleName = "ZURUECKFAHREN";
      if (ZylMagnetarm.stroke_completed()) {
        //Serial.println("Mangetarm zurückfahren...");
      }
      ZylMagnetarm.set(0);
      nextStepTimer.setTime(500);
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case SCHNEIDEN:    // DAS BAND WIRD ABGESCHNITTEN
      //***************************************************************************
      //cycleName = "SCHNEIDEN";
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
    case REVOLVER:    // WENN DER SENSOR KEINE PLOMBE DETEKTIERT DREHT DER REVOLVERKOPF
      //***************************************************************************
      //cycleName = "KARUSSELL";
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
    case RESET: // RESET FÜR NÄCHSTEN ZYKLUS
      //***************************************************************************
      //cycleName = "RESET";
      eepromCounter.countOneUp(shorttimeCounter);
      eepromCounter.countOneUp(longtimeCounter);
      cycleStep = 0;
      clearanceNextStep = false;
      break;
      //***************************************************************************
    } //END switch (cycleStep)
  }
} //END MAIN TEST CYCLE

