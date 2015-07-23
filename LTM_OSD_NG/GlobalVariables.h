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

struct {
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
  
  uint32_t  tripTime;
  uint32_t  tripDistance;
} uavData;

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
  0x1d,0x1a,0x1d,0x1c,0x1d,0x19,0x1d,0x1c,0x1d,0x1b,0x1d,0x1c,0x1d,0x18,0x1d,0x1c,0x1d,0x1a,0x1d,0x1c,0x1d,0x19,0x1d,0x1c,0x1d,0x1b,0x1d};

const char msgIntroString[] PROGMEM = INTRO_VERSION;
const char msgIntroVersion[] PROGMEM = "FW VERSION: ";
