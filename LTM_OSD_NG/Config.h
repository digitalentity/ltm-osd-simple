/*--------------------------       configurable parameters      ----------------------------------------------------*/
#define FW_VERSION          1

//#define LOADFONT

/********************       OSD HARDWARE settings      *********************/
//Choose ONLY ONE option:
#define MINIMOSD                    // Uncomment this if using standard MINIMOSD hardware (default for 95% of boards) 

//#define USE_ADC_VOLTAGE

/********************       PROTOCOL settings      *********************/
//#define PROTOCOL_LIGHTTELEMETRY
#define PROTOCOL_MAVLINK

/******************** Serial speed settings *********************/
// Choose ONLY ONE option:
#define BAUDRATE 115200
//#define BAUDRATE 57600
//#define BAUDRATE 38400
//#define BAUDRATE 19200
//#define BAUDRATE 9600

#define METRIC
//#define IMPERIAL

/********************       Information settings      *********************/
#define  OSD_GPS_SATS            (LINE04 + 2)
#define  OSD_GPS_POSITION        (LINE02 + 2)
#define  OSD_BAT_VOLTAGE         (LINE15 + 3)
#define  OSD_TRIP_TIME           (LINE15 + 22)
#define  OSD_AHI                 (LINE08 + 7)
#define  OSD_GPS_SPEED           (LINE08 + 3)
#define  OSD_ALTITUDE            (LINE08 + 23)
#define  OSD_HOME_DIRECTION      (LINE04 + 14)
#define  OSD_HOME_DISTANCE       (LINE04 + 24)
#define  OSD_HEADING_GRAPH       (LINE15 + 10)
#define  OSD_WARNINGS            (LINE12 + 10)
#define  OSD_FLIGHT_MODE         (LINE06 + 2)

#define  WARN_LOW_VOLTAGE        3700          // show warning if less than 50% capacity

/********************       CALLSIGN settings      *********************/
#define   CALLSIGNINTERVAL 60      // How frequently to display Callsign (in seconds)
#define   CALLSIGNDURATION 4       // How long to display Callsign (in seconds)


/********************       STARTUP settings      *********************/
#define INTRO_VERSION             "LTM OSD - 1.0" // Call the OSD something else if you prefer. KVOSD is not permitted - LOL. 
#define INTRO_DELAY 5             // Seconds intro screen should show for. Default is 10 
#define STARTUPDELAY 1000         // Enable alternative startup delay (in ms) to allow MAX chip voltage to rise fully and initialise before configuring 


/********************       Display Settings         ************************/
#define MAXSTALLDETECT             // Enable to attempt to detect MAX chip stall from bad power. Attempts to restart.
#define PAL                        // Use PAL mode by default (undef to use NTSC)
#define AUTOCAM                   // Disable if no screen display. Enables autodetect Camera type PAL/NTSC. Overrides GUI/OSD settings.
//#define AUTOCAMWAIT               // **UNTESTED** - Use with AUTOCAM - waits until camera is ready - i.e. if power up cameras after FC.
#define DECIMAL '.'                 // Decimal point character, change to what suits you best (.) (,)
//#define SHIFTDOWN                 // Select if your monitor cannot display top line fully. It shifts top 3 lines down. Not suitable for all layouts
#define FASTPIXEL                 // Optional - may improve resolution - especially hi res cams
#define WHITEBRIGHTNESS 0x01      // Optional change from default 0x00=120%,0x01=100%,0x10=90%,0x11=80%  default is 0x01=100%
#define BLACKBRIGHTNESS 0x00      // Optional change from default 0x00=0%,0x01=10%,0x10=20%0x11=30%  default is 0x00=0%
#define AHIPITCHMAX 200             // Specify maximum AHI pitch value displayed. Default 200 = 20.0 degrees
#define AHIROLLMAX  400             // Specify maximum AHI roll value displayed. Default 400 = 40.0 degrees 
#define FULLAHI                   // Enable to display a slightly longer AHI line
//#define AHI_ELEVATION
#define AHI_DECORATION
