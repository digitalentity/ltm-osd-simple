/* #################################################################################################################
* LightTelemetry protocol (LTM)
*
* Ghettostation one way telemetry protocol for really low bitrates (1200/2400 bauds).
*
* Protocol details: 3 different frames, little endian.
*   G Frame (GPS position) (2hz @ 1200 bauds , 5hz >= 2400 bauds): 18BYTES
*    0x24 0x54 0x47 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF  0xFF   0xC0
*     $     T    G  --------LAT-------- -------LON---------  SPD --------ALT-------- SAT/FIX  CRC
*   A Frame (Attitude) (5hz @ 1200bauds , 10hz >= 2400bauds): 10BYTES
*     0x24 0x54 0x41 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xC0
*      $     T   A   --PITCH-- --ROLL--- -HEADING-  CRC
*   S Frame (Sensors) (2hz @ 1200bauds, 5hz >= 2400bauds): 11BYTES
*     0x24 0x54 0x53 0xFF 0xFF  0xFF 0xFF    0xFF    0xFF      0xFF       0xC0
*      $     T   S   VBAT(mv)  Current(ma)   RSSI  AIRSPEED  ARM/FS/FMOD   CRC
* ################################################################################################################# */

#ifdef PROTOCOL_LIGHTTELEMETRY

#define LIGHTTELEMETRY_START1 0x24 //$
#define LIGHTTELEMETRY_START2 0x54 //T
#define LIGHTTELEMETRY_GFRAME 0x47 //G GPS + Baro altitude data ( Lat, Lon, Speed, Alt, Sats, Sat fix)
#define LIGHTTELEMETRY_AFRAME 0x41 //A Attitude data ( Roll,Pitch, Heading )
#define LIGHTTELEMETRY_SFRAME 0x53 //S Sensors/Status data ( VBat, Consumed current, Rssi, Airspeed, Arm status, Failsafe status, Flight mode )
#define LIGHTTELEMETRY_OFRAME 0x4F  //O OSD additionals data ( home pos, home alt, ddirection to home )
#define LIGHTTELEMETRY_GFRAMELENGTH 18
#define LIGHTTELEMETRY_AFRAMELENGTH 10
#define LIGHTTELEMETRY_SFRAMELENGTH 11
#define LIGHTTELEMETRY_OFRAMELENGTH 18

static uint8_t LTMserialBuffer[LIGHTTELEMETRY_GFRAMELENGTH - 4];
static uint8_t LTMreceiverIndex;
static uint8_t LTMcmd;
static uint8_t LTMrcvChecksum;
static uint8_t LTMreadIndex;
static uint8_t LTMframelength;
static uint8_t LTMpassed = 0;
static uint8_t crlf_count = 0;

uint8_t ltmread_u8()  {
    return LTMserialBuffer[LTMreadIndex++];
}

uint16_t ltmread_u16() {
    uint16_t t = ltmread_u8();
    t |= (uint16_t)ltmread_u8() << 8;
    return t;
}

uint32_t ltmread_u32() {
    uint32_t t = ltmread_u16();
    t |= (uint32_t)ltmread_u16() << 16;
    return t;
}

static uint8_t LTM_ok = 0;
static uint32_t lastLTMpacket;

// --------------------------------------------------------------------------------------
// Decoded received commands
void ltm_check() {
    uint32_t dummy;

    LTMreadIndex = 0;
    LTM_ok = 1;
    lastLTMpacket = millis();

    if (LTMcmd == LIGHTTELEMETRY_GFRAME)
    {
        uavData.gpsLatitude = (int32_t)ltmread_u32();
        uavData.gpsLongitude = (int32_t)ltmread_u32();
        uavData.gpsSpeed = ltmread_u8() * 100;            // LTM gives m/s, we expect cm/s
        uavData.altitude = ((int32_t)ltmread_u32());      // altitude from cm to m.
        uint8_t ltm_satsfix = ltmread_u8();

        uavData.gpsNumSat = (ltm_satsfix >> 2) & 0xFF;
        uavData.gpsFix    = ((ltm_satsfix & 0b00000011) <= 1) ? 0 : 1;

        // hpdate home distance and bearing
        if (uavData.gpsFixHome) {
            float rads = fabs((float)uavData.gpsHomeLatitude / 10000000.0f) * 0.0174532925f;
            float scaleLongDown = cos(rads);
            float dstlon, dstlat;

            //DST to Home
            dstlat = fabs(uavData.gpsHomeLatitude - uavData.gpsLatitude) * 1.113195f;
            dstlon = fabs(uavData.gpsHomeLongitude - uavData.gpsLongitude) * 1.113195f * scaleLongDown;
            uavData.gpsHomeDistance = sqrt(sq(dstlat) + sq(dstlon)) / 100.0;

            //DIR to Home
            dstlon = (uavData.gpsHomeLongitude - uavData.gpsLongitude); //OffSet_X
            dstlat = (uavData.gpsHomeLatitude - uavData.gpsLatitude) * (1.0f / scaleLongDown); //OffSet Y

            float bearing = 90 + (atan2(dstlat, -dstlon) * 57.295775); //absolut home direction
            if (bearing < 0) bearing += 360;//normalization
            bearing = bearing - 180;//absolut return direction
            if (bearing < 0) bearing += 360;//normalization
            uavData.gpsHomeBearing = bearing;
        }
        else {
            uavData.gpsHomeBearing = 0;
        }

        LTMpassed = 1;
    }

    if (LTMcmd == LIGHTTELEMETRY_AFRAME)
    {
        uavData.anglePitch = (int16_t)ltmread_u16() * 10;
        uavData.angleRoll = (int16_t)ltmread_u16() * 10 ;
        uavData.heading = (int16_t)ltmread_u16();
        if (uavData.heading < 0 ) uavData.heading = uavData.heading + 360; //convert from -180/180 to 0/360°
        LTMpassed = 1;
    }
    if (LTMcmd == LIGHTTELEMETRY_SFRAME)
    {
        uavData.batVoltage = ltmread_u16();
        uavData.batUsedCapacity = ltmread_u16();
        uavData.rssi = ltmread_u8();
        uavData.airspeed = ltmread_u8();

        uint8_t ltm_armfsmode = ltmread_u8();
        uavData.isArmed = (ltm_armfsmode & 0b00000001) ? 1 : 0;
        uavData.isFailsafe = (ltm_armfsmode >> 1) & 0b00000001;
        uavData.flightMode = (ltm_armfsmode >> 2) & 0b00111111;

        uavData.batCellVoltage = detectBatteryCellVoltage(uavData.batVoltage);  // LTM does not have this info, calculate ourselves
        uavData.batCurrent = calculateCurrentFromConsumedCapacity(uavData.batUsedCapacity);
    }

    if (LTMcmd == LIGHTTELEMETRY_OFRAME)
    {
        uavData.gpsHomeLatitude = (int32_t)ltmread_u32();
        uavData.gpsHomeLongitude = (int32_t)ltmread_u32();
        dummy = (int32_t)(ltmread_u32()) / 100.0f; // altitude from cm to m.
        dummy  = ltmread_u8();
        uavData.gpsFixHome = ltmread_u8();
        LTMpassed = 1;
    }
}

void readTelemetry() {
    uint8_t c;

    static enum _serial_state {
        IDLE,
        HEADER_START1,
        HEADER_START2,
        HEADER_MSGTYPE,
        HEADER_DATA
    }
    c_state = IDLE;

    uavData.flagTelemetryOk = ((millis() - lastLTMpacket) < 500) ? 1 : 0;

    while (Serial.available()) {
        c = char(Serial.read());
        if (c_state == IDLE) {
            c_state = (c == '$') ? HEADER_START1 : IDLE;
            //Serial.println("header $" );
        }
        else if (c_state == HEADER_START1) {
            c_state = (c == 'T') ? HEADER_START2 : IDLE;
            //Serial.println("header T" );
        }
        else if (c_state == HEADER_START2) {
            switch (c) {
            case 'G':
                LTMframelength = LIGHTTELEMETRY_GFRAMELENGTH;
                c_state = HEADER_MSGTYPE;
                break;
            case 'A':
                LTMframelength = LIGHTTELEMETRY_AFRAMELENGTH;
                c_state = HEADER_MSGTYPE;
                break;
            case 'S':
                LTMframelength = LIGHTTELEMETRY_SFRAMELENGTH;
                c_state = HEADER_MSGTYPE;
                break;
            case 'O':
                LTMframelength = LIGHTTELEMETRY_OFRAMELENGTH;
                c_state = HEADER_MSGTYPE;
                break;
            default:
                c_state = IDLE;
            }
            LTMcmd = c;
            LTMreceiverIndex = 0;
        }
        else if (c_state == HEADER_MSGTYPE) {
            if (LTMreceiverIndex == 0) {
                LTMrcvChecksum = c;
            }
            else {
                LTMrcvChecksum ^= c;
            }
            if (LTMreceiverIndex == LTMframelength - 4) { // received checksum byte
                if (LTMrcvChecksum == 0) {
                    ltm_check();
                    c_state = IDLE;
                }
                else {                                                   // wrong checksum, drop packet
                    c_state = IDLE;

                }
            }
            else LTMserialBuffer[LTMreceiverIndex++] = c;
        }
    }
}



#endif
