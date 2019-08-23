/*
 * *****************************************************************************
 * EEPROM_storage.ino
 * Function to store long values on EEPROM memory
 * Michael Wettstein
 * November 2018, Zürich
 * *****************************************************************************
 * EEPROM size:4096 bytes ATmega2560.
 * The EEPROM memory has a specified lifetime of 100,000 write/erase cycles
 * The storage location will be changed after 100,000 cycles
 * Storage data Type unsigned long (4Bytes)
 * *****************************************************************************
 */

//*****************************************************************************
//STORAGE VARIABLES AND ADRESSES:
//*****************************************************************************
const int STORESLOTS = 10; //use this value to define how many long variables can be stored
int storeLocation;
int valuecounter;
long EEPROM_WriteCounter;
//*****************************************************************************

void EEPROM_Update()
{
  valuecounter = 0; //resets the storage location for the first value
  upperFeedtime = eeprom_updateroutine(upperFeedtime);
  lowerFeedtime = eeprom_updateroutine(lowerFeedtime);
  shorttimeCounter = eeprom_updateroutine(shorttimeCounter);
  longtimeCounter = eeprom_updateroutine(longtimeCounter);
}

bool initial_setup = false; //to control initial storelocation setup

//ARRAY TO CONTROL INITIAL VALUE SETUP:
bool eeprom_setupflag[STORESLOTS - 1]; //0=value not initialized 1=initial value read from EEPROM

//PREVIOUS VALUES:
long eeprom_value; //to check if value changed

long eeprom_updateroutine(long current_value)
{
  if (initial_setup == false)
  {
    //FIND OUT WHERE THE VALUES ARE STORED:
    eeprom_read_block((void*) &storeLocation, (void*) 0, sizeof(2)); // destination / source / size
    //READ EEPROM WRITECOUNTER:
    eeprom_read_block((void*) &EEPROM_WriteCounter, (void*) storeLocation, sizeof(4)); // destination / source / size
    initial_setup = true;
  }

  //SHIFT STORELOCATION 4 BYTES FOR EVERY LONG VALUE
  //THE FIRST 4 BYTES ARE RESERVED FOR THE SAVECOUNTER:
  int current_storelocation = storeLocation + 4 + (valuecounter * 4);

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
    eeprom_write_block((void*) &current_value, (void*) (current_storelocation), 4); // source / destination / size
    EEPROM_WriteCounter++;
    eeprom_write_block((void*) &EEPROM_WriteCounter, (void*) (storeLocation), 4); // source / destination / size

  }

  //IF STORELOCATION IS AT THE END OF LIFECYCLE, ASSIGN A NEW STORELOCATION
  if (EEPROM_WriteCounter >= 100000)
  {
    //ASSIGN A NEW STORELOCATION:
    storeLocation = storeLocation + 4 + (4 * STORESLOTS); // 4 bytes for the write_counter + 4 bytes for each store slot
    if (storeLocation >= (4095 - 40))
    {
      storeLocation = 2;
    }
    eeprom_write_block((void*) &storeLocation, (void*) 0, 2); // source / destination / size
    EEPROM_WriteCounter = 0;
  }
  valuecounter++;
  return current_value;
}
