int run_main_test_cycle()
{
  if (clearance_next_step == true && millis() - prev_time > timer_next_step)
  {
    switch (cycle_step)
    {
      //***************************************************************************
      case 1://PLOMBEN WERDEN IM RUTSCH-SCHACHT FIXIERT
        //WENN DER SENSOR KEINE PLOMBE DETEKTIERT MUSS DER REVOLVER NEU GEFÜLLT WERDEN...
        //***************************************************************************
        //Serial.println("Plomben fixieren...");
        if (seal_available == true)
        {
          error_blink = false;//Plomben verfügbar, Fehlerblinken beenden
          zyl_gummihalter.set(1);//obere Plomben werden fixiert
        }
        else //MAGAZIN LEER! ROTE SIGNALLEUCHTE BLINKT
        {
          machine_running = false;
          //Serial.println("ABBRUCH, KEINE PLOMBE DETEKTIERT!");
          zyl_gummihalter.set(0); //SCHACHT ÖFFNET SICH WIEDER ZUM BEFÜLLEN
          cycle_step = 0; //RESET FÜR NEUSTART NACH DEM BEFÜLLEN
          error_blink = true;
        }
        zyl_magnetarm.set(0); //Um ein Verklemmen nach Reset zu verhindern
        prev_time = millis();
        timer_next_step = 500;
        clearance_next_step = false;
        cycle_step++;
        break;
      //***************************************************************************
      case 2://EINE PLOMBE WIRD FALLENGELASSEN
        //***************************************************************************
        //Serial.println("Plombe fallenlassen...");
        zyl_falltuerschieber.stroke(500, 0); //(push time,release time)
        if (zyl_falltuerschieber.stroke_completed() == true)
        {
          clearance_next_step = false;
          cycle_step++;
        }
        break;
      //***************************************************************************
      case 3://EINE PLOMBE WIRD ZUM ZANGENPAKET GEFAHREN
        //***************************************************************************
        //Serial.println("Plombe ausfahren...");
        zyl_magnetarm.set(1);
        prev_time = millis();
        timer_next_step = 500;
        clearance_next_step = false;
        cycle_step++;
        break;
      //***************************************************************************
      case 4://DAS UNTERE BAND WIRD VORGESCHOBEN
        //PLOMBENFIXIERUNG IM RUTSCHSCHACHT WIRD AUFGEHOBEN
        //***************************************************************************
        //Serial.println("Bandvorschub unten...");
        zyl_gummihalter.set(0);//Plomben für nächsten Zyklus können nachrutschen
        mot_feed_unten.stroke(lower_feedtime, 0); //vorschub läuft
        if (mot_feed_unten.stroke_completed() == true)
        {
          clearance_next_step = false;
          cycle_step++;
        }
        break;
      //***************************************************************************
      case 5://DAS OBERE BAND WIRD VORGESCHOBEN
        //***************************************************************************
        //Serial.println("Bandvorschub oben...");
        mot_feed_oben.stroke(upper_feedtime, 0);

        if (mot_feed_oben.stroke_completed() == true)
        {
          clearance_next_step = false;
          cycle_step++;
        }
        break;
      //***************************************************************************
      case 6://DAS KLAUENSYSTEM WIRD AKTIVIERT
        //***************************************************************************
        //Serial.println("PRESSEN");
        //mot_pressmechanik.stroke(3000);
        timer_next_step = 500;
        //if(stroke completed)
        clearance_next_step = false;
        cycle_step++;
        break;
      //***************************************************************************
      case 7://DAS BAND WIRD ABGESCHNITTEN
        //***************************************************************************
        if (zyl_messer.stroke_completed() == true)
        {
          //Serial.println("Band schneiden...");
        }
        zyl_messer.stroke(1500, 0); //(push time,release time)
        if (zyl_messer.stroke_completed() == true)
        {
          clearance_next_step = false;
          cycle_step++;
        }
        break;
      //***************************************************************************
      case 8://DER ZUFUHRZYLINDER FÄHRT ZURÜCK
        //***************************************************************************
        if (zyl_magnetarm.stroke_completed() == true)
        {
          //Serial.println("Mangetarm zurückfahren...");
        }
        zyl_magnetarm.set(0);
        prev_time = millis();
        timer_next_step = 500;
        clearance_next_step = false;
        cycle_step++;

        break;
      //***************************************************************************
      case 9://WENN DER SENSOR KEINE PLOMBE DETEKTIERT DREHT DER REVOLVERKOPF
        //***************************************************************************
        //Serial.print("Check Plombensensor");
        if (seal_available == false) //keine Plombe detektiert
        {
          if (zyl_revolverschieber.stroke_completed() == true)
          {
            //Serial.println("   ... keine Plombe detektiert Revolver vorschieben ********");
          }
          zyl_revolverschieber.stroke(5000, 5000);
        }
        else
        {
          //Serial.println(" ... i.O. Plombe detektiert");
        }
        prev_time = millis();
        timer_next_step = 500;
        if (zyl_revolverschieber.stroke_completed() == true)
        {
          clearance_next_step = true;
          cycle_step++;
        }
        break;
      //***************************************************************************
      case 10://RESET FÜR NÄCHSTEN ZYKLUS
        //***************************************************************************
        shorttime_counter++;
        longtime_counter++;
        //Serial.println("*** Zyklus beendet ***");
        cycle_step = 1;
        clearance_next_step = false;
        break;
        //***************************************************************************
    } //END switch (cycle_step)
  }
}//END MAIN TEST CYCLE
