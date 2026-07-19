//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2024 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// This file has been modified by Dave Bernazzani (wavemotion-dave)
// for optimized execution on the DS/DSi platform. Please seek the
// official Stella source distribution which is far cleaner, newer,
// and better maintained.
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include <sys/stat.h>
#include <sys/types.h>
#include <fat.h>
#include <dirent.h>
#include <rg_system.h>
#include "MT24LC256.hxx"
#include "System.hxx"
#include "SaveKey.hxx"

MT24LC256 *gSaveKeyEEprom = NULL;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SaveKey::SaveKey(Jack jack, const Event& event)
  : Controller(jack, event)
{
  DIR* dir = opendir(RG_BASE_PATH_CONFIG);
  if (dir)
  {
    closedir(dir);
  }
  else
  {
    mkdir(RG_BASE_PATH_CONFIG, 0777);
  }
  myEEPROM = new MT24LC256(RG_BASE_PATH_CONFIG "/StellaDS.EE");

        
  gSaveKeyEEprom = myEEPROM;

  bUseSaveKey = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SaveKey::~SaveKey()
{
  gSaveKeyEEprom = NULL;
  delete myEEPROM;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool SaveKey::read(DigitalPin pin)
{
  // We need to override the Controller::read() method, since the timing
  // of the actual read is important for the EEPROM (we can't just read
  // 60 times per second in the ::update() method)
  switch(pin)
  {
    // Pin 3: EEPROM SDA
    //        input data from the 24LC256 EEPROM using the I2C protocol
    case Three:
      return (bUseSaveKey ? (myEEPROM->readSDA()) : 1);

    default:
      return 1;
  }
}

Int32 SaveKey::read(AnalogPin pin)
{
    return maximumResistance;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SaveKey::write(DigitalPin pin, bool value)
{
  // Change the pin state based on value
  switch(pin)
  {
    // Pin 3: EEPROM SDA
    //        output data to the 24LC256 EEPROM using the I2C protocol
    case Three:
      myEEPROM->writeSDA(value);
      break;

    // Pin 4: EEPROM SCL
    //        output clock data to the 24LC256 EEPROM using the I2C protocol
    case Four:
      bUseSaveKey = true;
      myEEPROM->writeSCL(value);
      break;

    default:
      break;
  } 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SaveKey::systemCyclesReset()
{
  // The EEPROM keeps track of cycle counts, and needs to know when the
  // cycles are reset
  myEEPROM->systemCyclesReset();
}
