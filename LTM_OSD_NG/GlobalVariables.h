#define POS_MASK        0x01FF
#define PAL_MASK        0x0003
#define PAL_SHFT             9
#define DISPLAY_MASK    0xC000
#define DISPLAY_ALWAYS  0xC000
#define DISPLAY_NEVER   0x0000
#define DISPLAY_COND    0x4000
#define DISPLAY_MIN_OFF     0x8000

#define POS(pos, pal_off, disp)  (((pos)&POS_MASK)|((pal_off)<<PAL_SHFT)|(disp))
#if defined SHIFTDOWN
#define TOPSHIFT        LINE
#else
#define TOPSHIFT        0
#endif

#define LINE      30
#define LINE01    0
#define LINE02    30
#define LINE03    60
#define LINE04    90
#define LINE05    120
#define LINE06    150
#define LINE07    180
#define LINE08    210
#define LINE09    240
#define LINE10    270
#define LINE11    300
#define LINE12    330
#define LINE13    360
#define LINE14    390
#define LINE15    420
#define LINE16    450

/* OSD position */
#define OSD_INTRO_POSITION              (LINE + 4)

struct {
  uint8_t nextCharToRequest;
  uint8_t lastCharToRequest;
  uint8_t retransmitQueue;

  uint8_t videoMode;
} flags;

// Flight mode(0-19): 0: Manual, 1: Rate, 2: Attitude/Angle, 3: Horizon, 4: Acro, 5: Stabilized1, 6: Stabilized2, 7: Stabilized3,
// 8: Altitude Hold, 9: Loiter/GPS Hold, 10: Auto/Waypoints, 11: Heading Hold / headFree, 
// 12: Circle, 13: RTH, 14: FollowMe, 15: LAND, 16:FlybyWireA, 17: FlybywireB, 18: Cruise, 19: Unknown
#define FLIGHT_MODE_MANUAL          0
#define FLIGHT_MODE_RATE            1
#define FLIGHT_MODE_ANGLE           2
#define FLIGHT_MODE_HORIZON         3
#define FLIGHT_MODE_ACRO            4
#define FLIGHT_MODE_STABILIZED_1    5
#define FLIGHT_MODE_STABILIZED_2    6
#define FLIGHT_MODE_STABILIZED_3    7
#define FLIGHT_MODE_ALT_HOLD        8
#define FLIGHT_MODE_GPS_HOLD        9
#define FLIGHT_MODE_WAYPOINT        10
#define FLIGHT_MODE_HEADING_HOLD    11
#define FLIGHT_MODE_CIRCLE          12
#define FLIGHT_MODE_RTH             13
#define FLIGHT_MODE_FOLLOW_ME       14
#define FLIGHT_MODE_LAND            15

struct {
  uint8_t   flagTelemetryOk;      // Set by data receiver
  uint8_t   flagLowSats;          // set by uavSanityCheck
  uint8_t   flagLowVolts;

  uint8_t   isArmed;
  uint8_t   flightMode;
  uint8_t   isFailsafe;

  uint8_t   gpsNumSat;
  uint8_t   gpsFix;
  uint16_t  gpsSpeed;
  int32_t   gpsLatitude;
  int32_t   gpsLongitude;

  uint8_t   gpsFixHome;
  int32_t   gpsHomeLatitude;
  int32_t   gpsHomeLongitude;

  uint32_t  gpsHomeDistance;
  int       gpsHomeBearing;

  int16_t   altitude;

  int16_t   rssi;
  int16_t   airspeed;

  int       anglePitch;
  int       angleRoll;
  int       heading;

  uint16_t  batVoltage;    // mv
  uint16_t  batCurrent;    // mA
  uint16_t  batUsedCapacity;    // mA*h
  uint16_t  batCellVoltage;

  uint32_t  tripTime;
  uint32_t  tripDistance;
} uavData;

uint16_t detectBatteryCellVoltage(uint16_t millivolts)
{
  uint16_t battev = 0;

  if (uavData.batVoltage >= 3000 && uavData.batVoltage <= 4300)  // 1S
    battev = uavData.batVoltage;
  else if (uavData.batVoltage >= 3000 * 2 && uavData.batVoltage <= 4300 * 2) // 2S
    battev = uavData.batVoltage / 2;
  else if (uavData.batVoltage >= 3000 * 3 && uavData.batVoltage <= 4300 * 3) // 2S
    battev = uavData.batVoltage / 3;
  else if (uavData.batVoltage >= 3000 * 4 && uavData.batVoltage <= 4300 * 4) // 2S
    battev = uavData.batVoltage / 4;
  else if (uavData.batVoltage >= 3000 * 5 && uavData.batVoltage <= 4300 * 5) // 2S
    battev = uavData.batVoltage / 5;
  else if (uavData.batVoltage >= 3000 * 6 && uavData.batVoltage <= 4300 * 6) // 2S
    battev = uavData.batVoltage / 6;

  return battev;
}

//General use variables
struct {
  uint8_t tenthSec;
  uint8_t halfSec;
  uint8_t Blink2hz;                          // This is turing on and off at 2hz
  uint8_t Blink10hz;                         // This is turing on and off at 10hz
  int lastCallSign;                          // Callsign_timer
  uint32_t seconds;
  uint32_t elapsedSec;
}
timer;

// For Heading
const char headGraph[] PROGMEM = {
  0x1d, 0x1a, 0x1d, 0x1c, 0x1d, 0x18, 0x1d, 0x1c, 0x1d, 0x1a, 0x1d, 0x1c, 0x1d, 0x19, 0x1d, 0x1c, 0x1d, 0x1b, 0x1d, 0x1c, 0x1d, 0x18, 0x1d, 0x1c, 0x1d, 0x1b, 0x1d
};

const char msgIntroString[] PROGMEM = INTRO_VERSION;
const char msgIntroVersion[] PROGMEM = "FW VERSION: ";

const char blank_text[] PROGMEM    = "";
const char lowvolts_text[] PROGMEM  = "LOW VOLTS";
const char nodata_text[] PROGMEM    = " NO DATA";
const char satlow_text[] PROGMEM    = " LOW SATS";
const char armed_text[] PROGMEM     = "  ARMED";

