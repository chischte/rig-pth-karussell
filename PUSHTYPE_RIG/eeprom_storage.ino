/*
 * *****************************************************************************
 * EEPROM_storage.ino
 * Function to store long values on EEPROM memory
 * Michael Wettstein
 * November 2018, Zürich
 * *****************************************************************************
 */

//EEPROM size:4096 bytes ATmega2560.
//The EEPROM memory has a specified lifetime of 100,000 write/erase cycles
//The storage location will be changed after 100,000 cycles
//Storage data Type unsigned long, 4Bytes, value range 0 to 4,294,967,295 (2^32 - 1).
//*****************************************************************************
//STORAGE VARIABLES AND ADRESSES:
//*****************************************************************************
const int storeslots = 10; //use this value to define how many long variables can be stored
int STORELOCATION;
int valuecounter;
long eeprom_write_counter;
//*****************************************************************************

void eeprom_update()
{
  valuecounter = 0; //resets the storage location for the first value
  upper_feedtime = eeprom_updateroutine(upper_feedtime);
  lower_feedtime = eeprom_updateroutine(lower_feedtime);
  shorttime_counter = eeprom_updateroutine(shorttime_counter);
  longtime_counter = eeprom_updateroutine(longtime_counter);
}

bool initial_setup = false; //to control initial storelocation setup

//ARRAY TO CONTROL INITIAL VALUE SETUP:
bool eeprom_setupflag[storeslots - 1]; //0=value not initialized 1=initial value read from EEPROM

//PREVIOUS VALUES:
long eeprom_value; //to check if value changed

long eeprom_updateroutine(long current_value)
{
  if (initial_setup == false)
  {
    //FIND OUT WHERE THE VALUES ARE STORED:
    eeprom_read_block((void*) &STORELOCATION, (void*) 0, sizeof(2)); // destination / source / size
    //READ EEPROM WRITECOUNTER:
    eeprom_read_block((void*) &eeprom_write_counter, (void*) STORELOCATION,
        sizeof(4)); // destination / source / size
    initial_setup = true;
  }

  //SHIFT STORELOCATION 4 BYTES FOR EVERY LONG VALUE
  //THE FIRST 4 BYTES ARE RESERVED FOR THE SAVECOUNTER:
  int current_storelocation = STORELOCATION + 4 + (valuecounter * 4);

  //READ STORED VALUE FROM EEPROM:
  eeprom_read_block((void*) &eeprom_value, (void*) current_storelocation, 4); // destination / source / size

  //IN THE FIRST RUN SET VALUE SAVED ON EEPROM AS CURRENT VALUE:
  if (eeprom_setupflag[valuecounter] == 0)
  {
    current_value = eeprom_value;
    eeprom_setupflag[valuecounter] = 1;
  }
  else if (current_value != eeprom_value) //write value to EEPROM if it changed
  {
    eeprom_write_block((void*) &current_value, (void*) (current_storelocation),
        4); // source / destination / size
    eeprom_write_counter++;
    eeprom_write_block((void*) &eeprom_write_counter, (void*) (STORELOCATION),
        4); // source / destination / size

  }

  //IF STORELOCATION IS AT THE END OF LIFECYCLE, ASSIGN A NEW STORELOCATION
  //UPDATE NEW STORELOCATION:

  if (eeprom_write_counter >= 100000)
  {
    //ASSIGN A NEW STORELOCATION:
    STORELOCATION = STORELOCATION + 4 + (4 * storeslots); //4 bytes for the write_counter + 4 bytes for each store slot
    if (STORELOCATION >= (4095 - 40))
    {
      STORELOCATION = 2;
    }
    eeprom_write_block((void*) &STORELOCATION, (void*) 0, 2); // source / destination / size
    eeprom_write_counter = 0;
  }
  valuecounter++;
  return current_value;
}
