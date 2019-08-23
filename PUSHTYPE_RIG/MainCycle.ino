void RunMainTestCycle()
{
  if (clearanceNextStep == true && millis() - prev_time > timeNextStep)
  {
    switch (cycleStep)
    {
    //***************************************************************************
    case 1: //PLOMBEN WERDEN IM RUTSCH-SCHACHT FIXIERT
      //WENN DER SENSOR KEINE PLOMBE DETEKTIERT MUSS DER REVOLVER NEU GEFÜLLT WERDEN...
      //***************************************************************************
      //Serial.println("Plomben fixieren...");
      if (sealAvailable == true)
      {
        errorBlink = false;  //Plomben verfügbar, Fehlerblinken beenden
        ZylGummihalter.set(1);        //obere Plomben werden fixiert
      }
      else //MAGAZIN LEER! ROTE SIGNALLEUCHTE BLINKT
      {
        machineRunning = false;
        //Serial.println("ABBRUCH, KEINE PLOMBE DETEKTIERT!");
        ZylGummihalter.set(0); //SCHACHT ÖFFNET SICH WIEDER ZUM BEFÜLLEN
        cycleStep = 0; //RESET FÜR NEUSTART NACH DEM BEFÜLLEN
        errorBlink = true;
      }
      ZylMagnetarm.set(0); //Um ein Verklemmen nach Reset zu verhindern
      prev_time = millis();
      timeNextStep = 500;
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case 2: //EINE PLOMBE WIRD FALLENGELASSEN
      //***************************************************************************
      //Serial.println("Plombe fallenlassen...");
      ZylFalltuerschieber.stroke(500, 0); //(push time,release time)
      if (ZylFalltuerschieber.stroke_completed() == true)
      {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 3: //EINE PLOMBE WIRD ZUM ZANGENPAKET GEFAHREN
      //***************************************************************************
      //Serial.println("Plombe ausfahren...");
      ZylMagnetarm.set(1);
      prev_time = millis();
      timeNextStep = 500;
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case 4:        //DAS UNTERE BAND WIRD VORGESCHOBEN
      //PLOMBENFIXIERUNG IM RUTSCHSCHACHT WIRD AUFGEHOBEN
      //***************************************************************************
      //Serial.println("Bandvorschub unten...");
      ZylGummihalter.set(0); //Plomben für nächsten Zyklus können nachrutschen
      MotFeedUnten.stroke(lowerFeedtime, 0); //vorschub läuft
      if (MotFeedUnten.stroke_completed() == true)
      {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 5: //DAS OBERE BAND WIRD VORGESCHOBEN
      //***************************************************************************
      //Serial.println("Bandvorschub oben...");
      ZylMesser.set(0);      //sicherstellen das Messer zurückgezogen ist
      MotFeedOben.stroke(upperFeedtime, 0);

      if (MotFeedOben.stroke_completed() == true)
      {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 6:        //DAS KLAUENSYSTEM WIRD AKTIVIERT
      //***************************************************************************
      //Serial.println("PRESSEN");
      //mot_pressmechanik.stroke(3000);
      timeNextStep = 500;
      //if(stroke completed)
      clearanceNextStep = false;
      cycleStep++;
      break;
      //***************************************************************************
    case 7:        //DAS BAND WIRD ABGESCHNITTEN
      //***************************************************************************
      if (ZylMesser.stroke_completed() == true)
      {
        //Serial.println("Band schneiden...");
      }
      ZylMesser.stroke(1500, 0); //(push time,release time)
      if (ZylMesser.stroke_completed() == true)
      {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 8: //DER ZUFUHRZYLINDER FÄHRT ZURÜCK
      //***************************************************************************
      if (ZylMagnetarm.stroke_completed() == true)
      {
        //Serial.println("Mangetarm zurückfahren...");
      }
      ZylMagnetarm.set(0);
      prev_time = millis();
      timeNextStep = 500;
      clearanceNextStep = false;
      cycleStep++;

      break;
      //***************************************************************************
    case 9: //WENN DER SENSOR KEINE PLOMBE DETEKTIERT DREHT DER REVOLVERKOPF
      //***************************************************************************
      //Serial.print("Check Plombensensor");
      if (sealAvailable == false) //keine Plombe detektiert
      {
        if (ZylRevolverschieber.stroke_completed() == true)
        {
          //Serial.println("   ... keine Plombe detektiert Revolver vorschieben ********");
        }
        ZylRevolverschieber.stroke(5000, 5000);
      }
      else
      {
        //Serial.println(" ... i.O. Plombe detektiert");
      }
      prev_time = millis();
      timeNextStep = 500;
      if (ZylRevolverschieber.stroke_completed() == true)
      {
        clearanceNextStep = true;
        cycleStep++;
      }
      break;
      //***************************************************************************
    case 10: //RESET FÜR NÄCHSTEN ZYKLUS
      //***************************************************************************
      shorttimeCounter++;
      longtimeCounter++;
      //Serial.println("*** Zyklus beendet ***");
      cycleStep = 1;
      clearanceNextStep = false;
      break;
      //***************************************************************************
    } //END switch (cycleStep)
  }
} //END MAIN TEST CYCLE
