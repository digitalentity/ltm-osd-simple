char *ItoaPadded(int val, char *str, uint8_t bytes, uint8_t decimalpos)  {
  // Val to convert
  // Return String
  // Length
  // Decimal position
  uint8_t neg = 0;
  if (val < 0) {
    neg = 1;
    val = -val;
  }

  str[bytes] = 0;
  for (;;) {
    if (bytes == decimalpos) {
      str[--bytes] = DECIMAL;
      decimalpos = 0;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if (bytes == 0 || (decimalpos == 0 && val == 0))
      break;
  }

  if (neg && bytes > 0)
    str[--bytes] = '-';

  while (bytes != 0)
    str[--bytes] = ' ';
  return str;
}

char *FormatGPSCoord(int32_t val, char *str, uint8_t p, char pos, char neg) {
  if (val < 0) {
    pos = neg;
    val = -val;
  }

  uint8_t bytes = p + 6;
  val = val / 100;

  str[bytes] = 0;
  str[--bytes] = pos;
  for (;;) {
    if (bytes == p) {
      str[--bytes] = DECIMAL;
      continue;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if (bytes == 0 || (bytes < 3 && val == 0))
      break;
  }

  while (bytes != 0)
    str[--bytes] = ' ';

  return str;
}

// Take time in Seconds and format it as 'MM:SS'
// Alternately Take time in Minutes and format it as 'HH:MM'
// If hhmmss is 1, display as HH:MM:SS
char *formatTime(uint32_t val, char *str, uint8_t hhmmss) {
  int8_t bytes = 5;
  if (hhmmss)
    bytes = 8;
  str[bytes] = 0;
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    str[--bytes] = '0' + (val % 6);
    val = val / 6;
    str[--bytes] = ':';
  } while (hhmmss-- != 0);
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
  } while (val != 0 && bytes != 0);

  while (bytes != 0)
    str[--bytes] = ' ';

  return str;
}

void displayIntro(void)
{
  MAX7456_WriteString_P(msgIntroString, OSD_INTRO_POSITION);
  MAX7456_WriteString_P(msgIntroVersion, OSD_INTRO_POSITION + LINE + LINE);
  MAX7456_WriteString(ItoaPadded(FW_VERSION, screenBuffer, 4, 2), OSD_INTRO_POSITION + 11 + LINE + LINE + 1);
}

void displayNumberOfSat(uint16_t pos)
{
  if (((uavData.flagLowSats) || (!uavData.gpsFix)) && (timer.Blink2hz)) {
    return;
  }

  screenBuffer[0] = SYM_SAT_L;
  screenBuffer[1] = SYM_SAT_R;
  itoa(uavData.gpsNumSat, screenBuffer + 2, 10);
  MAX7456_WriteString(screenBuffer, pos);
}

void displayGPSPosition(uint16_t pos)
{
  if (!uavData.gpsFix)
    return;

  screenBuffer[0] = SYM_LAT;
  FormatGPSCoord(uavData.gpsLatitude, screenBuffer + 1, 4, 'N', 'S');
  MAX7456_WriteString(screenBuffer, pos);

  screenBuffer[0] = SYM_LON;
  FormatGPSCoord(uavData.gpsLongitude, screenBuffer + 1, 4, 'E', 'W');
  MAX7456_WriteString(screenBuffer, pos + 13);
}

void displaySpeed(uint16_t pos)
{
  if (!uavData.gpsFix) return;

  int xx;
#ifdef METRIC
  xx = uavData.gpsSpeed * 0.036;           // From MWii cm/sec to Km/h
  screenBuffer[0] = SYM_KMH;
#else
  xx = uavData.gpsSpeed * 0.02236932;      // (0.036*0.62137)  From MWii cm/sec to mph
  screenBuffer[0] = SYM_MPH;
#endif

  itoa(xx, screenBuffer + 1, 10);
  MAX7456_WriteString(screenBuffer, pos);
}

void displayAltitude(uint16_t pos)
{
  int16_t altitude;

#ifdef METRIC
  altitude = uavData.altitude / 100;         // cm to mt
  screenBuffer[0] = SYM_ALTM;
#else
  altitude = uavData.altitude * 0.032808;    // cm to feet
  screenBuffer[0] = SYM_ALTFT;
#endif

  itoa(altitude, screenBuffer + 1, 10);
  MAX7456_WriteString(screenBuffer, pos);
}

void displayDistanceToHome(uint16_t pos)
{
  if (!(uavData.gpsFix && uavData.gpsFixHome))
    return;

  if (uavData.gpsHomeDistance <= 2 && timer.Blink2hz)
    return;

  int16_t dist;

#ifdef METRIC
  dist = uavData.gpsHomeDistance;                    // Mt
  screenBuffer[0] = SYM_DISTHOME_M;
#else
  dist = uavData.gpsHomeDistance * 3.2808;           // mt to feet
  screenBuffer[0] = SYM_DISTHOME_FT;
#endif

  itoa(dist, screenBuffer + 1, 10);
  MAX7456_WriteString(screenBuffer, pos);
}

void displayDirectionToHome(uint16_t pos)
{
  if (!(uavData.gpsFix && uavData.gpsFixHome))
    return;

  if (uavData.gpsHomeDistance <= 2 && timer.Blink2hz)
    return;

  int16_t d = uavData.heading + 180 + 360 - uavData.gpsHomeBearing;
  if (d < 0) d += 360; //normalization

  d *= 4;
  d += 45;
  d = (d / 90) % 16;
  screenBuffer[0] = SYM_ARROW_SOUTH + d;
  screenBuffer[1] = 0;
  MAX7456_WriteString(screenBuffer, pos);
}

void displayHeadingGraph(uint16_t pos)
{
  int xx;
  xx = (uavData.heading - 180) * 4;
  xx = xx + 720 + 45;
  xx = xx / 90;
  memcpy_P(screen + pos, headGraph + xx + 1, 9);
}

void displayBatteryVoltage(uint16_t pos)
{
  if (uavData.flagLowVolts && timer.Blink2hz)
    return;

  // detect batt cells
  int battev = uavData.batCellVoltage;

  battev = constrain(battev, 3400, 4200);
  battev = map(battev, 3400, 4200, 0, 6);
  screenBuffer[0] = SYM_BATT_EMPTY - battev;

  ItoaPadded(uavData.batVoltage / 100, screenBuffer + 1, 4, 3);
  screenBuffer[5] = SYM_VOLT;
  screenBuffer[6] = 0;
  MAX7456_WriteString(screenBuffer, pos - 1);
}

void displayTripTime(uint16_t pos)
{
  uint32_t displaytime;

  if (uavData.tripTime < 3600) {
    screenBuffer[0] = SYM_FLY_M;
    displaytime = uavData.tripTime;
  }
  else {
    screenBuffer[0] = SYM_FLY_M;
    displaytime = uavData.tripTime / 60;
  }

  formatTime(displaytime, screenBuffer + 1, 0);
  MAX7456_WriteString(screenBuffer, pos);
}

void displayHorizon(uint16_t pos)
{
  int rollAngle = uavData.angleRoll;
  int pitchAngle = uavData.anglePitch;

  uint16_t position = pos - (2 * LINE);

  if (pitchAngle > AHIPITCHMAX) pitchAngle = AHIPITCHMAX;
  if (pitchAngle < -AHIPITCHMAX) pitchAngle = - AHIPITCHMAX;
  if (rollAngle > AHIROLLMAX) rollAngle = AHIROLLMAX;
  if (rollAngle < -AHIROLLMAX) rollAngle = - AHIROLLMAX;

#ifndef AHICORRECT
#define AHICORRECT 10
#endif

  pitchAngle = pitchAngle + AHICORRECT;

#ifdef FULLAHI
  for (uint8_t X = 0; X <= 12; X++) {
    if (X == 5) X = 8;
    int Y = (rollAngle * (4 - X)) / 64;
    Y -= pitchAngle / 8;
    Y += 41;
    if (Y >= 0 && Y <= 81) {
      uint16_t pos = position - 2 + LINE * (Y / 9) + 3 - 2 * LINE + X;
      screen[pos] = SYM_AH_BAR9_0 + (Y % 9);
    }
  }
#else //FULLAHI
  for (uint8_t X = 0; X <= 8; X++) {
    if (X == 3) X = 6;
    int Y = (rollAngle * (4 - X)) / 64;
    Y -= pitchAngle / 8;
    Y += 41;
    if (Y >= 0 && Y <= 81) {
      uint16_t pos = position + LINE * (Y / 9) + 3 - 2 * LINE + X;
      screen[pos] = SYM_AH_BAR9_0 + (Y % 9);
    }
  }
#endif //FULLAHI


#ifdef AHI_ELEVATION
  for (int X = 2; X <= 6; X++) {
    if (X == 4) X = 5;
    int Y = (rollAngle * (4 - X)) / 64;
    Y -= pitchAngle / 8;
    Y += 41;
    if (Y >= 0 && Y <= 81) {
      uint16_t pos = position + LINE * (Y / 9) + 3 - 2 * LINE + X;
      pos = pos - 3 * LINE;
      if (pos >= 60 && pos <= 360)
        screen[pos] = SYM_AH_BAR9_0 + (Y % 9);
      pos = pos + 2 * 3 * LINE;
      if (pos >= 60 && pos <= 330)
        screen[pos] = SYM_AH_BAR9_0 + (Y % 9);
    }
  }
#endif

  screen[position + 2 * LINE + 7 - 1] = SYM_AH_CENTER_LINE;
  screen[position + 2 * LINE + 7 + 1] = SYM_AH_CENTER_LINE_RIGHT;
  screen[position + 2 * LINE + 7] =   SYM_AH_CENTER;

#ifdef AHI_DECORATION
  // Draw AH sides
  screen[position + 2 * LINE + 1] =   SYM_AH_LEFT;
  screen[position + 2 * LINE + 13] =  SYM_AH_RIGHT;
  for (int X = 0; X <= 4; X++) {
    screen[position + X * LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position + X * LINE + 14] =  SYM_AH_DECORATION_RIGHT;
  }
#endif
}

void displayFlightMode(uint16_t pos)
{
  switch (uavData.flightMode) {
    case FLIGHT_MODE_MANUAL:
      screenBuffer[0] = SYM_PASS;
      screenBuffer[1] = SYM_PASS1;
      break;
    case FLIGHT_MODE_RATE:
      screenBuffer[0] = SYM_ACRO;
      screenBuffer[1] = SYM_ACRO1;
      break;
    case FLIGHT_MODE_ANGLE:
      screenBuffer[0] = SYM_STABLE;
      screenBuffer[1] = SYM_STABLE1;
      break;
    case FLIGHT_MODE_HORIZON:
      screenBuffer[0] = SYM_HORIZON;
      screenBuffer[1] = SYM_HORIZON1;
      break;
    case FLIGHT_MODE_ACRO:
      screenBuffer[0] = SYM_ACRO;
      screenBuffer[1] = SYM_ACRO1;
      break;
    case FLIGHT_MODE_STABILIZED_1:
    case FLIGHT_MODE_STABILIZED_2:
    case FLIGHT_MODE_STABILIZED_3:
      screenBuffer[0] = SYM_STABLE;
      screenBuffer[1] = SYM_STABLE1;
      break;
    case FLIGHT_MODE_ALT_HOLD:
      screenBuffer[0] = SYM_ALT;
      screenBuffer[1] = SYM_ALT1;
      break;
    case FLIGHT_MODE_GPS_HOLD:
      screenBuffer[0] = SYM_GHOLD;
      screenBuffer[1] = SYM_GHOLD1;
      break;
    case FLIGHT_MODE_WAYPOINT:
      screenBuffer[0] = SYM_GMISSION;
      screenBuffer[1] = SYM_GMISSION1;
      break;
    case FLIGHT_MODE_HEADING_HOLD:
      break;
    case FLIGHT_MODE_CIRCLE:
      screenBuffer[0] = SYM_GHOLD;
      screenBuffer[1] = SYM_GHOLD1;
      break;
    case FLIGHT_MODE_RTH:
      screenBuffer[0] = SYM_GHOME;
      screenBuffer[1] = SYM_GHOME1;
      break;
    case FLIGHT_MODE_FOLLOW_ME:
      screenBuffer[0] = SYM_GMISSION;
      screenBuffer[1] = SYM_GMISSION1;
      break;
    case FLIGHT_MODE_LAND:
      screenBuffer[0] = SYM_GLAND;
      screenBuffer[1] = SYM_GLAND1;
      break;
  }

  screenBuffer[2] = 0;
  MAX7456_WriteString(screenBuffer, pos);
}

void displayWarnings(uint16_t pos)
{
  if (timer.Blink2hz)
    return;

  if (!uavData.flagTelemetryOk) {
    strcpy_P(screenBuffer, (char*)nodata_text);
    MAX7456_WriteString(screenBuffer, pos);
    return;
  }

  if (uavData.flagLowVolts) {
    strcpy_P(screenBuffer, (char*)lowvolts_text);
    MAX7456_WriteString(screenBuffer, pos);
    return;
  }

  if (uavData.flagLowSats) {
    strcpy_P(screenBuffer, (char*)satlow_text);
    MAX7456_WriteString(screenBuffer, pos);
    return;
  }

  if (uavData.isArmed) {
    strcpy_P(screenBuffer, (char*)armed_text);
    MAX7456_WriteString(screenBuffer, pos);
    return;
  }
}
