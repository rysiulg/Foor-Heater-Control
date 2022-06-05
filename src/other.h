//other.cpp

#include <Arduino.h>

const unsigned long extTempTimeout_ms = 180 * 1000,
                    statusUpdateInterval_ms = 60 * 1 * 1000,    //update pumps i dallas  -i check mqtt for active boiler ch pump and co pump and if active one or both change time to next value
                    statusUpdateShortenInterval_s = 60,          //update pumps i dallas  -i check mqtt for active boiler ch pump and co pump and if active one or both change time to this value
                    spOverrideTimeout_ms = 180 * 1000,
                    mqttUpdateInterval_ms = 1 * 60 * 1000,      //send data to mqtt and influxdb
                    temp_NEWS_interval_reduction_time_ms = 30 * 60 * 1000, // time to laps between temp_NEWS reduction by 5%
                    mqtt_offline_reconnect_after_ms = 15 * 60 * 1000,      // time when mqtt is offline to wait for next reconnect (15minutes)
                    save_config_every = 5 * 60 * 1000,                    // time saveing config values in eeprom (15minutes) -changed to shorter time -implemented save only if value are changed
                    WiFiinterval = 30 * 1000;

// upper and lower bounds on heater level
const float cutoffhi = 20,           // upper max cut-off temp above is heating CO disabled -range +-20
            cutofflo = -cutoffhi,    // lower min cut-off temp above is heating CO disabled
            roomtemphi = 30,         // upper max to set of room temperature
            roomtemplo = 15,         // lower min to set of room temperature
            noCommandSpOverride = 32; //heating water temperature for fail mode (no external temp provided) for co

unsigned int runNumber = 0, // count of restarts
             publishhomeassistantconfig = 4,                               // licznik wykonan petli -strat od 0
             publishhomeassistantconfigdivider = 5;                        // publishhomeassistantconfig % publishhomeassistantconfigdivider -publikacja gdy reszta z dzielenia =0 czyli co te ilosc wykonan petli opoznionej update jest wysylany config

float roomtemp_last = 0, // prior temperature
      ierr = 25,         // integral error
      dt = 0,            // time between measurements
      //       op = 0, //PID controller output
      retTemp = 0,                         // return temperature
      temp_NEWS = 0,                       // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South
      cutOffTemp = 2,                      // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
      op_override = noCommandSpOverride, // boiler tempset on heat mode
      room_temp_default = 20.5,     //domyslna temp pokoju
      pressure = 0,
      humiditycor = 0,              //DHT humidity
      tempcor = 0,                  //DHT temp and mayby dual with 18b20 onboard
      tempwe = 0;                   //temp. wejscia co
String LastboilerResponseError,
       kondygnacja = "\0";          //ident pietra

int temp_NEWS_count = 0,
    mqtt_offline_retrycount = 0,
    mqtt_offline_retries = 10; // retries to mqttconnect before timewait

unsigned long ts = 0, new_ts = 0, // timestamp
              lastUpdateTempPump = 0,
              lastUpdatemqtt = 0,
              lastTempSet = 0,
              lastcutOffTempSet = 0,
              lastNEWSSet = 0,
              lastmqtt_reconnect = 0,
              lastSaveConfig = 0,
              lastSpSet = 0,
              WiFipreviousMillis = 0,
              dhtreadtime = 0;

bool receivedmqttdata = false,
     CO_PumpWorking = false,
     CO_BoilerPumpWorking = false,
     tmanual = false;

const int lampPin = LED_BUILTIN;




void Assign_Name_Addr_Pinout(int i, String name, String address, int outpin) {
  //Konwersja adres String to array uint
  uint8_t  adres[address.length() + 1];
  address.toCharArray((char*)adres, address.length() + 1);
  for (int j = 0; j < address.length() / 2; j++) room_temp[i].addressHEX[j] = convertCharToHex((int)adres[j * 2]) * 16 + convertCharToHex((int)adres[(j * 2) + 1]);
  name=kondygnacja+sepkondname+name;
  name.toCharArray((char*)room_temp[i].nameSensor, namelength+1); //cutted last char
  room_temp[i].idpinout = outpin;
  if (outpin>-1) {
    pinMode (outpin, OUTPUT);       //set pin as output
    digitalWrite (outpin, start_digital);    //set active high
    if (room_temp[i].tempset==InitTemp or room_temp[i].tempset==0) room_temp[i].tempset = room_temp_default;
  }
  room_temp[i].switch_state=start;
  if (name == String(tempcutoff).substring(0,namelength) and (room_temp[i].tempset==InitTemp)) room_temp[i].tempset=pumpOffVal;      //assign pump to sensor
  #ifdef debug
  Serial.println(String(i)+": "+String(room_temp[i].nameSensor)+" "+String(address)+" "+String(outpin)+" "+String(name.length()));
  #endif

      pinMode(lampPin, OUTPUT);
    digitalWrite(lampPin, LOW); // initially off


}

boolean array_cmp_8( uint8_t *a, uint8_t *b, int len_a, int len_b) {
  int n;
//  int len_a,  len_b;
//  len_a = sizeof(a) / sizeof(a[0]);
//  len_b = sizeof(b) / sizeof(b[0]);
  // Serial.print("_");
  // if their lengths are different, return false
  if (len_a != len_b) return false;

  // test each element to be the same. if not, return false
  for (n = 0; n < len_a; n++) {
    #ifdef debug
     //Serial.print(String(a[n],HEX));Serial.print(":");Serial.print(String(b[n],HEX)); Serial.println("  lena: "+String(len_a)+"-"+String(len_a));
     #endif
    if (a[n] != b[n]) return false;
  }
  //ok, if we have not returned yet, they are equal :)
  return true;
}

int convertCharToHex(char ch)
{
  int returnType;
  ch = toupper(ch);
  switch (ch)
  {
    case '0':
      returnType = 0;
      break;
    case  '1' :
      returnType = 1;
      break;
    case  '2':
      returnType = 2;
      break;
    case  '3':
      returnType = 3;
      break;
    case  '4' :
      returnType = 4;
      break;
    case  '5':
      returnType = 5;
      break;
    case  '6':
      returnType = 6;
      break;
    case  '7':
      returnType = 7;
      break;
    case  '8':
      returnType = 8;
      break;
    case  '9':
      returnType = 9;
      break;
    case  'A':
      returnType = 10;
      break;
    case  'B':
      returnType = 11;
      break;
    case  'C':
      returnType = 12;
      break;
    case  'D':
      returnType = 13;
      break;
    case  'E':
      returnType = 14;
      break;
    case  'F' :
      returnType = 15;
      break;
    default:
      returnType = 0;
      break;
  }
  return returnType;
}

void AssignSensors()
{
  for (int i = 0; i < maxsensors; i++) { //jest 8 elektrozaworow termicznych
    room_temp[i].tempread = InitTemp;
    switch (i) {
      case 0: Assign_Name_Addr_Pinout(i, String(zawor1name), String(zawor1addr), zawor1); break;
      case 1: Assign_Name_Addr_Pinout(i, String(zawor2name), String(zawor2addr), zawor2); break;
//        (String(kondygnacja) + String(zawor2name)).toCharArray(room_temp[i].nameSensor, namelength);
//        temps[i].idpinout = zawor2;
//        break;
      case 2: Assign_Name_Addr_Pinout(i, String(zawor3name), String(zawor3addr), zawor3); break;
      case 3: Assign_Name_Addr_Pinout(i, String(zawor4name), String(zawor4addr), zawor4); break;
      case 4: Assign_Name_Addr_Pinout(i, String(zawor5name), String(zawor5addr), zawor5); break;
      case 5: Assign_Name_Addr_Pinout(i, String(zawor6name), String(zawor6addr), zawor6); break;
      case 6: Assign_Name_Addr_Pinout(i, String(zawor7name), String(zawor7addr), zawor7); break;
      case 7: Assign_Name_Addr_Pinout(i, String(zawor8name), String(zawor8addr), zawor8); break;
      case 8: Assign_Name_Addr_Pinout(i, String(zawor9name), String(zawor9addr), zawor9); break;
      case 9: Assign_Name_Addr_Pinout(i, String(zawor10name), String(zawor10addr), zawor10); break;
      case 10: Assign_Name_Addr_Pinout(i, String(zawor11name), String(zawor11addr), zawor11); break;
      case 11: Assign_Name_Addr_Pinout(i, String(zawor12name), String(zawor12addr), zawor12); break;
      case 12: Assign_Name_Addr_Pinout(i, String(zawor13name), String(zawor13addr), zawor13); break;
    }
  }
}

void recvMsg(uint8_t *data, size_t len)
{ // for WebSerial
  WebSerial.println("Received Data...");
  String d = "";
  for (int i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  d.toUpperCase();
  WebSerial.println("Received: " + String(d));
  if (d == "ON")
  {
      digitalWrite(BUILTIN_LED, HIGH);
  }
  if (d == "OFF")
  {
      digitalWrite(BUILTIN_LED, LOW);
  }
  if (d == "RESTART")
  {
    WebSerial.println(F("OK. Restarting... by command..."));
    restart();
  }
  if (d == "RECONNECT")
  {
    reconnect();
  }
  if (d == "SAVE")
  {
    WebSerial.println(F("Saving config to EEPROM memory by command..."));
    WebSerial.println("Size CONFIG: " + String(sizeof(CONFIGURATION)));
    saveConfig();
  }
  if (d.indexOf("TOGGLEPUMP") !=- 1)
  {
    int o = maxsensors;    //assigned pump pin to toom_temp array
    bool state = false;
    if (d.indexOf(" ")!=-1 and d.indexOf("=")!=-1) {
      String part = d.substring(d.indexOf(" "));
      part.trim();
      String pumpno = part.substring(0,part.indexOf("="));
      String myvalue = part.substring(part.indexOf("=")+1);
      pumpno.trim();
      myvalue.trim();
      //#ifdef enableWebSerial
      WebSerial.println("Part: "+String(part));
      WebSerial.println("pumpno: "+String(pumpno));
      WebSerial.println("myvalue: "+String(myvalue));
      //#endif
      o = pumpno.toInt();
      state = myvalue.toInt()==1;
    }
    WebSerial.println(String(millis())+": "+("Toggle Pump State by command... to: "+String((statusUpdateInterval_ms - (millis() - lastUpdateTempPump))/1000)+"s max: "+String(statusUpdateInterval_ms/1000)+"s" ));
    WebSerial.println("numer: "+String(o)+" Stan: "+String(state));
    WebSerial.println("Stan: "+String(room_temp[o].switch_state)+" dig: "+String(digitalRead(room_temp[o].idpinout)));
    room_temp[o].switch_state = state;
    digitalWrite(room_temp[o].idpinout, state);
    WebSerial.println("Stan2: "+String(room_temp[o].switch_state)+" dig: "+String(digitalRead(room_temp[o].idpinout)));

  }

  if (d == "RESET_CONFIG")
  {
    WebSerial.println(F("RESET config to DEFAULT VALUES and restart..."));
    WebSerial.println("Size CONFIG: " + String(sizeof(CONFIGURATION)));
    CONFIGURATION.version[0] = 'R';
    CONFIGURATION.version[1] = 'E';
    CONFIGURATION.version[2] = 'S';
    CONFIGURATION.version[3] = 'E';
    CONFIGURATION.version[4] = 'T';
    saveConfig();
    restart();
  }
   if (d == "HELP")
  {
    WebSerial.println(F("KOMENDY: RESTART, RECONNECT, SAVE, RESET_CONFIG, TOGGLEPUMP numer=0/1"));
  }
}



bool PayloadStatus(String payloadStr, bool state)
{
  payloadStr.toUpperCase();
  payloadStr.trim();
  payloadStr.replace(",", ".");      //for localization correction
  if (state and (payloadStr == "ON" or payloadStr == "TRUE" or payloadStr == "START" or payloadStr == "1"  or payloadStr == "ENABLE" or payloadStr == "HEAT")) return true;
  else
  if (!state and (payloadStr == "OFF" or payloadStr == "FALSE" or payloadStr == "STOP" or payloadStr == "0" or payloadStr == "DISABLE")) return true;
  else return false;
}


bool PayloadtoValidFloatCheck(String payloadStr)
{
  if (PayloadtoValidFloat(payloadStr)==InitTemp) return false; else return true;
}
float PayloadtoValidFloat(String payloadStr, bool withtemps_minmax, float mintemp, float maxtemp)  //bool withtemps_minmax=false, float mintemp=InitTemp,float
{
  payloadStr.trim();
  payloadStr.replace(",", ".");
  float valuefromStr = payloadStr.toFloat();
  if (isnan(valuefromStr) || !isValidNumber(payloadStr))
  {
    #ifdef debug
    Serial.println(F("Value is not a valid number, ignoring..."));
    #endif
    WebSerial.println(F("Value is not a valid number, ignoring..."));
    return InitTemp;
  } else
  {
    if (!withtemps_minmax)
    {
      return valuefromStr;
    } else {
      #ifdef debug
      Serial.println("Value is valid number: "+String(valuefromStr,2));
      #endif
      WebSerial.println("Value is valid number: "+String(valuefromStr,2));
      if (valuefromStr>maxtemp and maxtemp!=InitTemp) valuefromStr = maxtemp;
      if (valuefromStr<mintemp and mintemp!=InitTemp) valuefromStr = mintemp;
      return valuefromStr;
    }
  }
}

String getIdentyfikator(int x)
{
  return "_"+String(x+1);
}

void updateMQTTData() {
  const String payloadvalue_startend_val = ""; // value added before and after value send to mqtt queue
  int createhasensors = maxsensors;  //jest tylko 1 temp wejscia !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  #include "configmqtttopics.h"
  mqttclient.setBufferSize(2048);

  String tmpbuilder="{";
  tmpbuilder += "\"rssi\":"+ String(WiFi.RSSI());
  tmpbuilder += ",\"HallSensor"+kondygnacja+"\":"+String(hallRead());
  tmpbuilder += ",\"CRT\":"+ String(runNumber);
  float min = room_temp[0].tempread, max = room_temp[0].tempset;
  for (int x=0;x<createhasensors;x++)
  {
    String identyfikator = getIdentyfikator(x);
    if (room_temp[x].tempread!=InitTemp) {
      tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE + identyfikator + "\": " + payloadvalue_startend_val + String(room_temp[x].tempread) + payloadvalue_startend_val;
      if (min>room_temp[x].tempread and x<8) min = room_temp[x].tempread;
    }
    if (room_temp[x].tempread!=InitTemp and room_temp[x].idpinout>0) {
      tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + identyfikator + "\": " + payloadvalue_startend_val + String(room_temp[x].tempset) + payloadvalue_startend_val;
      if (max<room_temp[x].tempset and x<8) max = room_temp[x].tempset;
    }
  }
  if (min!=InitTemp) tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE + getIdentyfikator(-1) + "\": " + payloadvalue_startend_val + String(min) + payloadvalue_startend_val;
  if (max!=InitTemp) tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + getIdentyfikator(-1) + "\": " + payloadvalue_startend_val + String(max) + payloadvalue_startend_val;
  mqttclient.publish(ROOMS_TOPIC_SENSOR.c_str(),(tmpbuilder+"}").c_str(), mqtt_Retain);




  // client.publish(ROOMS_TOPIC.c_str(),
  //                ("{\"" + OT + ROOM_TEMPERATURE + "_1\": " + payloadvalue_startend_val + String(room_temp[0].tempread) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + "_1\": " + payloadvalue_startend_val + String(room_temp[0].tempset) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE + "_2\": " + payloadvalue_startend_val + String(room_temp[1].tempread) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + "_2\": " + payloadvalue_startend_val + String(room_temp[1].tempset) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE + "_3\": " + payloadvalue_startend_val + String(room_temp[2].tempread) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + "_3\": " + payloadvalue_startend_val + String(room_temp[2].tempset) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE + "_4\": " + payloadvalue_startend_val + String(room_temp[3].tempread) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + "_4\": " + payloadvalue_startend_val + String(room_temp[3].tempset) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE + "_5\": " + payloadvalue_startend_val + String(room_temp[4].tempread) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + "_5\": " + payloadvalue_startend_val + String(room_temp[4].tempset) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE + "_6\": " + payloadvalue_startend_val + String(room_temp[5].tempread) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + "_6\": " + payloadvalue_startend_val + String(room_temp[5].tempset) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE + "_7\": " + payloadvalue_startend_val + String(room_temp[6].tempread) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + "_7\": " + payloadvalue_startend_val + String(room_temp[6].tempset) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE + "_8\": " + payloadvalue_startend_val + String(room_temp[7].tempread) + payloadvalue_startend_val +
  //                 ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + "_8\": " + payloadvalue_startend_val + String(room_temp[7].tempset) + payloadvalue_startend_val +
  //                 "}").c_str(), mqtt_Retain); //"heat" : "off")

  publishhomeassistantconfig++; // zwiekszamy licznik wykonan wyslania mqtt by co publishhomeassistantconfigdivider wysłań wysłać autoconfig discovery dla homeassisatnt
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
  {
    // homeassistant/sensor/BB050B_OPENTHERM_OT10_lo/config = {"name":"Opentherm OPENTHERM OT10 lo","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_lo","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['lo']}}"} (retained) problem
    // 21:16:02.724 MQT: homeassistant/sensor/BB050B_OPENTHERM_OT10_hi/config = {"name":"Opentherm OPENTHERM OT10 hi","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_hi","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['hi']}}"} (retained)
    const String temperature_class = "\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\"";
    const String temperature_class_lines = "\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer-lines\"";
    for (int x=0;x<createhasensors;x++){
      String identyfikator = getIdentyfikator(x);
      if (room_temp[x].tempread!=InitTemp) mqttclient.publish((ROOMS_HA_TOPIC + "_" + ROOM_TEMPERATURE + identyfikator + "/config").c_str(), ("{\"name\":\"" + OT + String(room_temp[x].nameSensor) + identyfikator+"\",\"uniq_id\": \"" + OT + ROOM_TEMPERATURE + identyfikator+"\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_TEMPERATURE + identyfikator+"}}\","+temperature_class+",\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
      //mqttclient.publish((ROOMS_HA_TOPIC + "_" + ROOM_TEMPERATURE + identyfikator + "/config").c_str(), ("{\"name\":\"" + OT + ROOM_TEMPERATURE + identyfikator+"\",\"uniq_id\": \"" + OT + ROOM_TEMPERATURE + identyfikator+"\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_TEMPERATURE + identyfikator+"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
      if (room_temp[x].idpinout>0 and room_temp[x].tempread!=InitTemp) mqttclient.publish((ROOMS_HA_TOPIC + "_" + ROOM_TEMPERATURE_SETPOINT + identyfikator+"/config").c_str(), ("{\"name\":\"" +  OT + String(room_temp[x].nameSensor) + identyfikator +"SP\",\"uniq_id\": \"" + OT + ROOM_TEMPERATURE_SETPOINT + identyfikator+"\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_TEMPERATURE_SETPOINT + identyfikator+"}}\","+temperature_class_lines+",\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
      if (room_temp[x].idpinout>0 and room_temp[x].tempread!=InitTemp) mqttclient.publish((ROOMS_HACLI_TOPIC + identyfikator + "_climate/config").c_str(), ("{\"name\":\"" + OT + ROOM_TEMP + String(room_temp[x].nameSensor) + identyfikator + "\",\"uniq_id\": \"" + OT + ROOM_TEMP + identyfikator + "\", \
\"modes\":[\"heat\"], \
\"mode_state_topic\": \"" + ROOMS_TOPIC_SENSOR + "\", \
\"mode_state_template\": \"{{'heat' if now() > today_at('0:00') else 'heat'}}\", \
\"icon\": \"mdi:radiator\", \
\"current_temperature_topic\":\"" + ROOMS_TOPIC_SENSOR + "\", \
\"current_temperature_template\":\"{{value_json." + OT + ROOM_TEMPERATURE + identyfikator + "}}\", \
\"temperature_command_topic\":\"" + ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + identyfikator + SET_LAST + "\", \
\"temperature_state_topic\":\"" + ROOMS_TOPIC_SENSOR + "\", \
\"temperature_state_template\":\"{{value_json." + OT + ROOM_TEMPERATURE_SETPOINT + identyfikator + "}}\", \
\"temperature_unit\":\"C\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + roomtemplo + ", \
\"max_temp\": " + roomtemphi + ", \
\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
    }
    //max tempset for rooms and min temp in rooms
    mqttclient.publish((ROOMS_HA_TOPIC + "_" + ROOM_TEMPERATURE + getIdentyfikator(-1) + "/config").c_str(), ("{\"name\":\"" + OT + ROOM_TEMPERATURE + getIdentyfikator(-1)+"_min\",\"uniq_id\": \"" + OT + ROOM_TEMPERATURE + getIdentyfikator(-1)+"\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_TEMPERATURE + getIdentyfikator(-1)+"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((ROOMS_HA_TOPIC + "_" + ROOM_TEMPERATURE_SETPOINT + getIdentyfikator(-1)+"/config").c_str(), ("{\"name\":\"" + OT + ROOM_TEMPERATURE_SETPOINT + getIdentyfikator(-1)+"_max\",\"uniq_id\": \"" + OT + ROOM_TEMPERATURE_SETPOINT + getIdentyfikator(-1)+"\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_TEMPERATURE_SETPOINT + getIdentyfikator(-1)+"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer-lines\",\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);

/*
https://www.home-assistant.io/integrations/climate.mqtt/
\"modes\":[\"off\",\"heat\"], \

\"mode_state_topic\": \"" + ROOMS_TOPIC + "\", \
\"mode_state_template\": \"{{value_json." + OT + HOT_WATER_SOFTWARE_CH_STATE + "}}\", \
\"mode_command_topic\": \"" + STATE_DHW_SET_TOPIC + "\", \
\"mode_command_template\" : \"{% set values = { 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \

*/

  }
  #ifdef debug
  Serial.println(String(millis())+": MQTT Data Sended...");
  #endif
  #ifdef enableWebSerial
  WebSerial.println(String(millis())+": MQTT Data Sended...");
  #endif

}
String getJsonVal(String json, String tofind)
{ //function to get value from json payload
  json.trim();
  tofind.trim();
  #ifdef debugweb
  WebSerial.println("json0: "+json);
  #endif
  if (!json.isEmpty() and !tofind.isEmpty() and json.startsWith("{") and json.endsWith("}"))  //check is starts and ends as json data and nmqttident null
  {
    json=json.substring(1,json.length()-1);                             //cut start and end brackets json
    #ifdef debugweb
    WebSerial.println("json1: "+json);
    #endif
    int tee=0; //for safety ;)
    #define maxtee 500
    while (tee!=maxtee)
    {         //parse all nodes
      int pos = json.indexOf(",",1);                //position to end of node:value
      if (pos==-1) {tee=maxtee;}
      String part;
      if (pos>-1) {part = json.substring(0,pos);} else {part=json; }       //extract parameter node:value
      part.replace("\"","");                      //clean from text indent
      part.replace("'","");
      json=json.substring(pos+1);                      //cut input for extracted node:value
      if (part.indexOf(":",1)==-1) {
        #ifdef debugweb
        WebSerial.println("Return no : data");
        #endif
        break;
      }
      String node=part.substring(0,part.indexOf(":",1));    //get node name
      node.trim();
      String nvalue=part.substring(part.indexOf(":",1)+1); //get node value
      nvalue.trim();
      #ifdef debugweb
      WebSerial.println("jsonx: "+json);
      WebSerial.println("tee: "+String(tee)+" tofind: "+tofind+" part: "+part+" node: "+node +" nvalue: "+nvalue + " indexof , :"+String(json.indexOf(",",1)));
      #endif
      if (tofind==node)
      {
         #ifdef debugweb
         WebSerial.println("Found node return val");
         #endif
        return nvalue;
        break;
      }
      tee++;
      #ifdef debugweb
      delay(1000);
      #endif
      if (tee>maxtee) {
        #ifdef debugweb
         WebSerial.println("tee exit: "+String(tee));
        #endif
        break;  //safety bufor
      }
    }
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": Json "+json+"  No mqttident contain searched value of "+tofind);
    #endif
  } else
  {
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": Inproper Json format or null: "+json+" to find: "+tofind);
    #endif
  }
  return "\0";
}