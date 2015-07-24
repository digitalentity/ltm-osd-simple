

/*
LTM-NG OSD ...

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version. see http://www.gnu.org/licenses/

This work is based on the following open source work :-
 Scarab OSD                https://github.com/ShikOfTheRa/scarab-osd
 Rushduino                 http://code.google.com/p/rushduino-osd/
 Rush OSD Development      https://code.google.com/p/rush-osd-development/
 Minim OSD                 https://code.google.com/p/arducam-osd/wiki/minimosd

 All credit and full acknowledgement to the incredible work and hours from the many developers, contributors and testers that have helped along the way.
 Jean Gabriel Maurice. He started the revolution. He was the first....
*/

//------------------------------------------------------------------------
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "Config.h"
#include "Def.h"
#include "symbols.h"
#include "GlobalVariables.h"
#include "math.h"

char screen[480];      // Main screen ram for MAX7456
char screenBuffer[20];

unsigned long previous_millis_low = 0;
unsigned long previous_millis_high = 0;

#define SLOW_CYCLE_TIME 100
#define FAST_CYCLE_TIME 50


//------------------------------------------------------------------------
void setup()
{
  Serial.begin(BAUDRATE);
  Serial.flush();

  pinMode(LEDPIN, OUTPUT);

#ifndef STARTUPDELAY
#define STARTUPDELAY 1000
#endif

  delay(STARTUPDELAY);

  MAX7456Setup();
}

void uavSanityCheck()
{
  if (!uavData.isArmed) {
    uavData.gpsSpeed = 0;
  }

#ifdef WARN_LOW_VOLTAGE
  uavData.flagLowVolts = (uavData.batCellVoltage < WARN_LOW_VOLTAGE) ? 1 : 0;
#else
  uavData.flagLowVolts = 0;
#endif

  uavData.flagLowSats = (uavData.gpsNumSat < 6);
}

//------------------------------------------------------------------------
void loop()
{
  // Blink Basic Sanity Test Led at 0.5hz
  if (timer.Blink2hz)
    digitalWrite(LEDPIN, HIGH);
  else
    digitalWrite(LEDPIN, LOW);

  ltm_read();

  uavSanityCheck();

  //---------------  Start Timed Service Routines  ---------------------------------------
  unsigned long currentMillis = millis();

  if ((currentMillis - previous_millis_low) >= SLOW_CYCLE_TIME) // 10 Hz (Executed every 100ms)
  {
    previous_millis_low = previous_millis_low + SLOW_CYCLE_TIME;
    timer.halfSec++;
    timer.Blink10hz = !timer.Blink10hz;
    calculateTrip();
  }  // End of slow Timed Service Routine (100ms loop)

  if ((currentMillis - previous_millis_high) >= FAST_CYCLE_TIME) // 20 Hz (Executed every 50ms)
  {
    previous_millis_high = previous_millis_high + FAST_CYCLE_TIME;
    MAX7456_DrawScreen();

#ifndef INTRO_DELAY
#define INTRO_DELAY 8
#endif
    if ( timer.elapsedSec < INTRO_DELAY ) {
      displayIntro();
    }
    else
    {
#ifdef OSD_GPS_SATS
      displayNumberOfSat(OSD_GPS_SATS);
#endif

#ifdef OSD_GPS_SATS
      displayGPSPosition(OSD_GPS_POSITION);
#endif

#ifdef OSD_BAT_VOLTAGE
      displayBatteryVoltage(OSD_BAT_VOLTAGE);
#endif

#ifdef OSD_TRIP_TIME
      displayTripTime(OSD_TRIP_TIME);
#endif

#ifdef OSD_AHI
      displayHorizon(OSD_AHI);
#endif

#ifdef OSD_GPS_SPEED
      displaySpeed(OSD_GPS_SPEED);
#endif

#ifdef OSD_ALTITUDE
      displayAltitude(OSD_ALTITUDE);
#endif

#ifdef OSD_HOME_DIRECTION
      displayDirectionToHome(OSD_HOME_DIRECTION);
#endif

#ifdef OSD_HOME_DISTANCE
      displayDistanceToHome(OSD_HOME_DISTANCE);
#endif

#ifdef OSD_HEADING_GRAPH
      displayHeadingGraph(OSD_HEADING_GRAPH);
#endif

#ifdef OSD_WARNINGS
      displayWarnings(OSD_WARNINGS);
#endif

#ifdef OSD_FLIGHT_MODE
      displayFlightMode(OSD_FLIGHT_MODE);
#endif
    }
  }  // End of fast Timed Service Routine (50ms loop)

  if (timer.halfSec >= 5) {
    timer.halfSec = 0;
    timer.Blink2hz = ! timer.Blink2hz;
  }

  if (currentMillis > timer.seconds + 1000)     // this execute 1 time a second
  {
    timer.seconds += 1000;

#ifdef MAXSTALLDETECT
    MAX7456Stalldetect();
#endif

    if (uavData.isArmed) {
      uavData.tripTime++;
    }

    timer.elapsedSec++;
  }
}  // End of main loop

//------------------------------------------------------------------------
// MISC

void resetFunc(void)
{
  asm volatile ("  jmp 0");
}

void calculateTrip(void)
{
  static float tripSum = 0;
  if (uavData.gpsFix && uavData.isArmed && (uavData.gpsSpeed > 0)) {
#ifdef METRIC
    tripSum += uavData.gpsSpeed * 0.0010;       //  100/(100*1000)=0.0005               cm/sec ---> mt/50msec (trip var is float)
#else
    tripSum += uavData.gpsSpeed * 0.0032808;    //  100/(100*1000)*3.2808=0.0016404     cm/sec ---> ft/50msec
#endif
  }
  uavData.tripDistance = (uint32_t) tripSum;
}
