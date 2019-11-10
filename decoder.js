function get_num(x, min, max, precision, round) {

  var range = max - min;
  var new_range = (Math.pow(2, precision) - 1) / range;
  var back_x = x / new_range;

  if (back_x === 0) {
    back_x = min;
  }
  else if (back_x === (max - min)) {
    back_x = max;
  }
  else {
    back_x += min;
  }
  return Math.round(back_x * Math.pow(10, round)) / Math.pow(10, round);
}

function Decoder(bytes) {

  var decoded = {};

  var resetCause_dict = {
    0: "POWERON",
    1: "EXTERNAL",
    2: "SOFTWARE",
    3: "WATCHDOG",
    4: "FIREWALL",
    5: "OTHER",
    6: "STANDBY"
  };
  
  var cnt=0;

  if (port === 2) {
    var system = {};
    system.resetCause = resetCause_dict[bytes[cnt++]];
    system.vbus = get_num(bytes[cnt++], 0, 3.6, 8, 2);
    system.battery_avg = get_num(bytes[cnt++], 0, 4000, 8, 1);
    system.battery_min = get_num(bytes[cnt++], 0, 4000, 8, 1);
    system.battery_max = get_num(bytes[cnt++], 0, 4000, 8, 1);
    system.input_analog_avg = get_num(bytes[cnt++], 0, 4000, 8, 1);
    system.input_analog_min = get_num(bytes[cnt++], 0, 4000, 8, 1);
    system.input_analog_max = get_num(bytes[cnt++], 0, 4000, 8, 1);
    system.temperature_avg = get_num(bytes[cnt++], -20, 80, 8, 1);
    system.temperature_min = get_num(bytes[cnt++], -20, 80, 8, 1);
    system.temperature_max = get_num(bytes[cnt++], -20, 80, 8, 1);
    decoded.system=system;
    //error - tbd
  }
  else if (port === 3) {
    decoded.lat = ((bytes[0] << 16) >>> 0) + ((bytes[1] << 8) >>> 0) + bytes[2];
    decoded.lat = (decoded.lat / 16777215.0 * 180) - 90;
    decoded.lon = ((bytes[3] << 16) >>> 0) + ((bytes[4] << 8) >>> 0) + bytes[5];
    decoded.lon = (decoded.lon / 16777215.0 * 360) - 180;
    decoded.alt = (bytes[7] << 8) | bytes[6];
    decoded.satellites = (bytes[8] >> 4);
    decoded.hdop = (bytes[8] & 0x0f);
    decoded.time_to_fix = bytes[9];
    decoded.epe = bytes[10];
    decoded.snr = bytes[11];
    decoded.lux = bytes[12];
    decoded.motion = bytes[13];
  }
  else if (port === 4) {
    var pira = {};
    pira.empty_space = bytes[cnt++] | (bytes[cnt++] << 8);
    pira.photo_count = bytes[cnt++] | (bytes[cnt++] << 8);
    pira.status_time = bytes[cnt++] | (bytes[cnt++] << 8)| (bytes[cnt++] << 16)| (bytes[cnt++] << 24);
    var d= new Date(pira.status_time*1000);
    pira.status_time_decoded =d.toLocaleString();
    pira.error_values = bytes[cnt++] | (bytes[cnt++] << 8);
    decoded.pira=pira;

    //error - tbd
  }

  return decoded;
}