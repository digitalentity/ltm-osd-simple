/********************  HARDWARE PINS definitions  *********************/
#define LEDPIN        7

// All aircraft / FC types defaults...
#define HEADINGCORRECT              // required to correct for MWheading being 0>360 vs MWII -180>+180. Leave permanently enabled

/********************  OSD HARDWARE rule definitions  *********************/
#ifdef RUSHDUINO
#define MAX7456SELECT 10        // ss 
#define MAX7456RESET  9         // RESET
#else
#define MAX7456SELECT 6         // ss
#define MAX7456RESET  10        // RESET
#endif
