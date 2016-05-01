#ifdef PROTOCOL_MAVLINK

#define ToDeg(x) (x*57.2957795131)  // *180/pi

static uint32_t lastTelemetryPacket;

mavlink_message_t msg;
mavlink_status_t status;

uint8_t mavlinkToLTMFlightMode(uint8_t mavMode)
{
  switch(mavMode) {
    case 0: // COPTER_MODE_STABILIZE
      return FLIGHT_MODE_ANGLE;
    case 1: // COPTER_MODE_ACRO
      return FLIGHT_MODE_ACRO;
    case 2: // COPTER_MODE_ALTHOLD
      return FLIGHT_MODE_ALT_HOLD;
    case 3: // COPTER_MODE_AUTO
    case 4: // COPTER_MODE_LOITER
    case 16: // COPTER_MODE_LOITER
      return FLIGHT_MODE_WAYPOINT;
    case 5: // COPTER_MODE_LOITER
      return FLIGHT_MODE_GPS_HOLD;
    case 6: // COPTER_MODE_RTL
      return FLIGHT_MODE_RTH;
    case 7: // COPTER_MODE_CIRCLE
      return FLIGHT_MODE_CIRCLE;
    case 9: // COPTER_MODE_LAND
      return FLIGHT_MODE_LAND;
    default:
      return FLIGHT_MODE_MANUAL;
  } 
}

void parseMavlinkMessage(mavlink_message_t *msg)
{
  uint8_t base_mode;

  switch(msg->msgid) {
  case MAVLINK_MSG_ID_HEARTBEAT:
    base_mode = mavlink_msg_heartbeat_get_base_mode(msg);

    uavData.isArmed = (base_mode & MAV_MODE_FLAG_SAFETY_ARMED) ? 1 : 0;
    uavData.isFailsafe = 0;
    uavData.flightMode = mavlinkToLTMFlightMode(mavlink_msg_heartbeat_get_custom_mode(msg));

    //mavdata.custom_mode = (unsigned char) mavlink_msg_heartbeat_get_custom_mode(msg);
    //mavdata.base_mode = mavlink_msg_heartbeat_get_base_mode(msg);
    break;
  case MAVLINK_MSG_ID_SYS_STATUS:
    uavData.batVoltage = mavlink_msg_sys_status_get_voltage_battery(msg);
    uavData.batUsedCapacity = 0;
    uavData.batCurrent = mavlink_msg_sys_status_get_current_battery(msg) * 10;
    uavData.batCellVoltage = detectBatteryCellVoltage(uavData.batVoltage);  // LTM does not have this info, calculate ourselves
    break;
  case MAVLINK_MSG_ID_GPS_RAW_INT:
    uavData.gpsLatitude = (int32_t)mavlink_msg_gps_raw_int_get_lat(msg);
    uavData.gpsLongitude = (int32_t)mavlink_msg_gps_raw_int_get_lon(msg);
    //uavData.gpsSpeed = mavlink_msg_gps_raw_int_get_v(msg) * 100;  // m/s -> cm/s
    //uavData.altitude = mavlink_msg_gps_raw_int_get_alt(msg) / 10;
    uavData.gpsNumSat = mavlink_msg_gps_raw_int_get_satellites_visible(msg);
    uavData.gpsFix    = mavlink_msg_gps_raw_int_get_fix_type(msg) < 2 ? 0 : 1;
    break;
  case MAVLINK_MSG_ID_VFR_HUD:
    uavData.altitude = mavlink_msg_vfr_hud_get_alt(msg) * 100;
    uavData.gpsSpeed = mavlink_msg_vfr_hud_get_groundspeed(msg) * 100;  // m/s -> cm/s
    //mavdata.vfr_hud.heading = mavlink_msg_vfr_hud_get_heading(msg);
    break;
  case MAVLINK_MSG_ID_ATTITUDE:
    uavData.anglePitch = -ToDeg(mavlink_msg_attitude_get_pitch(msg)) * 10;
    uavData.angleRoll = ToDeg(mavlink_msg_attitude_get_roll(msg)) * 10;
    uavData.heading = ToDeg(mavlink_msg_attitude_get_yaw(msg));
    if (uavData.heading < 0 ) uavData.heading = uavData.heading + 360; //convert from -180/180 to 0/360°
    break;
  case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
    uavData.gpsFixHome = true;
    uavData.gpsHomeBearing = mavlink_msg_nav_controller_output_get_target_bearing(msg);
    uavData.gpsHomeDistance = mavlink_msg_nav_controller_output_get_wp_dist(msg);
    break;
  default:
      break;
  }

  lastTelemetryPacket = millis();
}

void readTelemetry() {
  uavData.flagTelemetryOk = ((millis() - lastTelemetryPacket) < 1500) ? 1 : 0;

  while (Serial.available()) {
    uint8_t c = Serial.read();

    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
      parseMavlinkMessage(&msg);
    }
  }
}

#endif
