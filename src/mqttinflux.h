//other.cpp

#include <Arduino.h>

#include <EEPROM.h>



void Assign_Name_Addr_Pinout(int i, String name, String address, int outpin) {
//  #include "configmqtttopics.h"
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
  room_temp[i].humidityread = 0;
  room_temp[i].switch_state=stoprun;
  if (name == String(tempcutoff).substring(0,namelength) and (room_temp[i].tempset==InitTemp))
  {
    room_temp[i].tempset=pumpOffVal;      //assign pump to sensor
    room_temp[i].switch_state=startrun;      //turn on pump for a while
  }
  room_temp[i].mqtt_tempset_floor_topic = ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getIdentyfikator(i) + SET_LAST;
  if (kondygnacja == "1" and i<8)
  {
    room_temp[i].mqtt_temp_floor_topic = FLOOR1_ROOM_TOPICS[i][0];
    room_temp[i].mqtt_humid_json = FLOOR1_ROOM_TOPICS[i][2];
    room_temp[i].mqtt_temp_json = FLOOR1_ROOM_TOPICS[i][1];
  }
  if (kondygnacja == "2"  and i<8)
  {
    room_temp[i].mqtt_temp_floor_topic = FLOOR2_ROOM_TOPICS[i][0];
    room_temp[i].mqtt_humid_json = FLOOR2_ROOM_TOPICS[i][2];
    room_temp[i].mqtt_temp_json = FLOOR2_ROOM_TOPICS[i][1];
  }
  //room_temp[i].
  #ifdef debug
  Serial.println(String(i)+": "+String(room_temp[i].nameSensor)+" "+String(address)+" "+String(outpin)+" "+String(name.length()));
  #endif

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
      case 0: Assign_Name_Addr_Pinout(i, String(dzawor1name), String((kondygnacja == "1")?(dzawor1addr1):(kondygnacja == "2")?(dzawor1addr2):"\0"), zawor1); break;
      case 1: Assign_Name_Addr_Pinout(i, String(dzawor2name), String((kondygnacja == "1")?(dzawor2addr1):(kondygnacja == "2")?(dzawor2addr2):"\0"), zawor2); break;
//        (String(kondygnacja) + String(zawor2name)).toCharArray(room_temp[i].nameSensor, namelength);
//        temps[i].idpinout = zawor2;
//        break;
      case 2: Assign_Name_Addr_Pinout(i, String(dzawor3name), String((kondygnacja == "1")?(dzawor3addr1):(kondygnacja == "2")?(dzawor3addr2):"\0"), zawor3); break;
      case 3: Assign_Name_Addr_Pinout(i, String(dzawor4name), String((kondygnacja == "1")?(dzawor4addr1):(kondygnacja == "2")?(dzawor4addr2):"\0"), zawor4); break;
      case 4: Assign_Name_Addr_Pinout(i, String(dzawor5name), String((kondygnacja == "1")?(dzawor5addr1):(kondygnacja == "2")?(dzawor5addr2):"\0"), zawor5); break;
      case 5: Assign_Name_Addr_Pinout(i, String(dzawor6name), String((kondygnacja == "1")?(dzawor6addr1):(kondygnacja == "2")?(dzawor6addr2):"\0"), zawor6); break;
      case 6: Assign_Name_Addr_Pinout(i, String(dzawor7name), String((kondygnacja == "1")?(dzawor7addr1):(kondygnacja == "2")?(dzawor7addr2):"\0"), zawor7); break;
      case 7: Assign_Name_Addr_Pinout(i, String(dzawor8name), String((kondygnacja == "1")?(dzawor8addr1):(kondygnacja == "2")?(dzawor8addr2):"\0"), zawor8); break;
      case 8: Assign_Name_Addr_Pinout(i, String(dzawor9name), String((kondygnacja == "1")?(dzawor9addr1):(kondygnacja == "2")?(dzawor9addr2):"\0"), zawor9); break;
      case 9: Assign_Name_Addr_Pinout(i, String(dzawor10name), String((kondygnacja == "1")?(dzawor10addr1):(kondygnacja == "2")?(dzawor10addr2):"\0"), zawor10); break;
      case 10: Assign_Name_Addr_Pinout(i, String(dzawor11name), String((kondygnacja == "1")?(dzawor11addr1):(kondygnacja == "2")?(dzawor11addr2):"\0"), zawor11); break;
      case 11: Assign_Name_Addr_Pinout(i, String(dzawor12name), String((kondygnacja == "1")?(dzawor12addr1):(kondygnacja == "2")?(dzawor12addr2):"\0"), zawor12); break;
      case 12: Assign_Name_Addr_Pinout(i, String(dzawor13name), String((kondygnacja == "1")?(dzawor13addr1):(kondygnacja == "2")?(dzawor13addr2):"\0"), zawor13); break;
    }
  }
}

String LocalVarsRemoteCommands(String command, size_t gethelp)
{ // for WebSerial
  if (gethelp == remoteHelperMenu) {
    return F(", TOGGLEPUMP, TEMPCUTOFF");
  } else
  if (gethelp == remoteHelperMenuCommand)
  {
    if (command.indexOf("TOGGLEPUMP") >=0) {
      log_message((char*)F("  TOGGLEPUMP nr_pompy=stan  -Toggle Pump State by command, nr_pompy = 0 to 9, stan = 0 lub 1"), logCommandResponse);
    } else
    if (command.indexOf("TEMPCUTOFF+") >=0) {
      log_message((char*)F(" TEMPCUTOFF wartosc  -Zmienia temperature cutoff -sygnal braku ciepla,"), logCommandResponse);
    } else
    return F("OK");
  } else
  if (gethelp == remoteHelperCommand) {
    if (command.indexOf("TOGGLEPUMP") !=- 1)
    {
      int o = maxsensors;    //assigned pump pin to toom_temp array
      bool state = false;
      if (command.indexOf(" ")!=-1 and command.indexOf("=")!=-1) {
        String part = command.substring(command.indexOf(" "));
        part.trim();
        String pumpno = part.substring(0,part.indexOf("="));
        String myvalue = part.substring(part.indexOf("=")+1);
        pumpno.trim();
        myvalue.trim();
        sprintf(log_chars,"Part: %s, pumpno: %s, myvalue: %s", String(part).c_str(), String(pumpno).c_str(), String(myvalue).c_str() );
        log_message(log_chars);
        o = pumpno.toInt();
        state = myvalue.toInt()==1;
      }
        sprintf(log_chars,"Toggle Pump State by command... to: %ss, max: %s, numer: %s, Stan: %s, Stan roomtemp: %s, dig: %s", String((statusUpdateInterval_ms - (millis() - lastUpdateTempPump))/1000).c_str(), String(statusUpdateInterval_ms/1000).c_str(),
        String(o).c_str(), String(state).c_str(), String(room_temp[o].switch_state).c_str(), String(digitalRead(room_temp[o].idpinout)).c_str() );
        log_message(log_chars);

      room_temp[o].switch_state = !state;
      digitalWrite(room_temp[o].idpinout, !state);    //aktywny stan niski
      sprintf(log_chars,"Stan2: %s, digital: %s", String(room_temp[o].switch_state).c_str(), String(digitalRead(room_temp[o].idpinout)).c_str() );
      log_message(log_chars);
    } else
    if (command == F("TEMPCUTOFF"))
    {
      String part = command.substring(command.indexOf(" "));
      part.trim();
      if (command.indexOf(" ")!=-1) {
        if (PayloadtoValidFloatCheck(part)) {pumpOffVal = PayloadtoValidFloat(part,true,18,36);}
        sprintf(log_chars, " TEMPCUTOFF: %s -> ZMIENIONO NA: %s    Payload: %s",String(pumpOffVal).c_str(), String(pumpOffVal).c_str(), String(command).c_str());
        log_message(log_chars);
        for (int i=0;i<maxsensors;i++)
        {
          if (String(room_temp[i].nameSensor) == String(tempcutoff).substring(0,namelength)) room_temp[i].tempset=pumpOffVal;      //assign pump to sensor
        }
      } else {
        sprintf(log_chars, " TEMPCUTOFF: %s",String(pumpOffVal).c_str());
        log_message(log_chars);
      }
    }
  } //remoteHelperCommand
  return "";
}


#ifdef ENABLE_INFLUX
void updateInfluxDB()
{
//  #include "configmqtttopics.h"

  InfluxSensor.clearFields();
  // Report RSSI of currently connected network
  InfluxSensor.addField("rssi"+String(kondygnacja), (WiFi.RSSI()));
  InfluxSensor.addField("HallSensorFH"+kondygnacja, (hallRead()));
  InfluxSensor.addField("CRT_F"+String(kondygnacja),  (CRTrunNumber));
  for (int x=0;x<maxsensors;x++)
  {
    String identyfikator=getIdentyfikator(x);
    if (check_isValidTemp(room_temp[x].tempread) and room_temp[x].tempread>roomtemplo and room_temp[x].tempread<=roomtemphi) {
      InfluxSensor.addField(String(ROOM_TEMPERATURE)+String(kondygnacja)+identyfikator, room_temp[x].tempread);
    }
    if (check_isValidTemp(room_temp[x].tempset) and room_temp[x].tempset>roomtemplo and room_temp[x].tempset<=roomtemphi) {
      InfluxSensor.addField(String(ROOM_TEMPERATURE_SETPOINT) +String(kondygnacja)+identyfikator, room_temp[x].tempset);
    }
  }
  if (check_isValidTemp(min_temp) and min_temp>=roomtemplo and min_temp<=roomtemphi) InfluxSensor.addField(String(ROOM_TEMPERATURE)+String(kondygnacja)+getIdentyfikator(-1), min_temp);
  if (check_isValidTemp(max_temp) and max_temp>=roomtemplo and max_temp<=roomtemphi) InfluxSensor.addField(String(ROOM_TEMPERATURE_SETPOINT) +String(kondygnacja)+getIdentyfikator(-1), max_temp);

  #ifdef enableDHT
  if (check_isValidTemp(tempcor))
  {
    InfluxSensor.addField(String(ROOM_TEMPERATURE)+String(kondygnacja)+"_DHT_Temp", tempcor);
    InfluxSensor.addField(String(ROOM_HUMIDITY)+String(kondygnacja)+"_DHT_Humid", humiditycor);
  }
  #endif
  InfluxSensor.addField(String(ROOM_PUMPSTATE)+"_Floor_"+String(kondygnacja), !pump?1:0);

//   InfluxSensor.addField(String(TEMP_CUTOFF)+String(kondygnacja), cutOffTemp);
// //  InfluxSensor.addField(String(DIAGS_OTHERS_FAULT), status_Fault ? "1" : "0");
// //  InfluxSensor.addField(String(DIAGS_OTHERS_DIAG), status_Diagnostic ? "1" : "0");
//   InfluxSensor.addField(String(INTEGRAL_ERROR_GET_TOPIC)+String(kondygnacja), ierr);
//   InfluxSensor.addField(String(LOG_GET_TOPIC)+String(kondygnacja), LastboilerResponseError);

  // Print what are we exactly writing
  sprintf(log_chars, "Writing to InfluxDB: %s", "\0"); //InfluxClient.pointToLineProtocol(InfluxSensor));
  log_message(log_chars);
  // Write point
  if (!InfluxClient.writePoint(InfluxSensor))
  {
    sprintf(log_chars, "InfluxDB write failed: %s", String(InfluxClient.getLastErrorMessage()));
    log_message(log_chars);
  }
}
#endif



#if defined enableMQTT || defined enableMQTTAsync
void updateMQTTData() {
  const String payloadvalue_startend_val = ""; // value added before and after value send to mqtt queue
  int createhasensors = maxsensors;  //jest tylko 1 temp wejscia !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  #include "configmqtttopics.h"

  const String me_version = String(version);
  String mqttdeviceid = "\"dev\":{\"ids\":\""+String(me_lokalizacja)+"\",\"name\":\""+String(me_lokalizacja)+"\",\"sw\":\"" + String(me_version) + "\",\"mdl\": \""+String(me_lokalizacja)+"\",\"mf\":\"" + String(MFG) + "\"}";
  #ifdef enableMQTTAsync
  uint16_t packetIdSub;
  #endif

  String tmpbuilder="{";
  tmpbuilder += "\"rssi\":"+ String(WiFi.RSSI());
  tmpbuilder += ",\"HallSensor"+kondygnacja+"\":"+String(hallRead());
  tmpbuilder += ",\"CRT\":"+ String(CRTrunNumber);

  // #ifdef enableMQTT
  // mqttclient.publish(String(LOG_GET_TOPIC).c_str(), LastboilerResponseError.c_str());
  // #endif //
  // #ifdef enableMQTTAsync
  // uint16_t packetIdSub;
  // packetIdSub = mqttclient.publish(String(LOG_GET_TOPIC).c_str(), QOS, mqtt_Retain, LastboilerResponseError.c_str());
  // if (packetIdSub == 0) packetIdSub = 0;
  // #endif

  for (int x=0;x<createhasensors;x++)
  {
    String identyfikator = getIdentyfikator(x);
    if (check_isValidTemp(room_temp[x].tempread) and room_temp[x].tempread > roomtemplo) {
      tmpbuilder += ",\"" + String(OT) + String(ROOM_TEMPERATURE) + String(identyfikator) + "\": " + payloadvalue_startend_val + String(room_temp[x].tempread) + payloadvalue_startend_val;

    }
    if (check_isValidTemp(room_temp[x].tempset) and room_temp[x].idpinout>0 and room_temp[x].tempread > roomtemplo) {
      tmpbuilder += ",\"" + String(OT) + String(ROOM_TEMPERATURE_SETPOINT) + String(identyfikator) + "\": " + payloadvalue_startend_val + String(room_temp[x].tempset) + payloadvalue_startend_val;

    }
  }
  if (check_isValidTemp(min_temp)) tmpbuilder += ",\"" + String(OT) + String(ROOM_TEMPERATURE) + String(getIdentyfikator(-1)) + "\": " + payloadvalue_startend_val + String(min_temp) + payloadvalue_startend_val;
  if (check_isValidTemp(max_temp)) tmpbuilder += ",\"" + String(OT) + String(ROOM_TEMPERATURE_SETPOINT) + String(getIdentyfikator(-1)) + "\": " + payloadvalue_startend_val + String(max_temp) + payloadvalue_startend_val;

  #ifdef enableDHT
  if (check_isValidTemp(tempcor))
  {
    tmpbuilder += ",\"" + String(OT) + String(ROOM_TEMPERATURE) + String(getIdentyfikator(-1)) + "_DHT_Temp\": " + payloadvalue_startend_val + String(tempcor) + payloadvalue_startend_val;
    tmpbuilder += ",\"" + String(OT) + String(ROOM_HUMIDITY) + String(getIdentyfikator(-1)) + "_DHT_Humid\": " + payloadvalue_startend_val + String(humiditycor) + payloadvalue_startend_val;
  }
  #endif
  tmpbuilder += ",\"" + String(OT) + String(ROOM_PUMPSTATE) + String(getIdentyfikator(-1)) + "_pumpstate\": " + payloadvalue_startend_val + String(!pump?1:0) + payloadvalue_startend_val;
  tmpbuilder += "}";

  #ifdef enableMQTT
  mqttclient.publish(String(ROOMS_TOPIC_SENSOR).c_str(), String(tmpbuilder).c_str(), mqtt_Retain);
  #endif //
  #ifdef enableMQTTAsync
  packetIdSub = mqttclient.publish(String(ROOMS_TOPIC_SENSOR).c_str(), QOS, mqtt_Retain, tmpbuilder.c_str());
  if (packetIdSub == 0) packetIdSub = 0;
  #endif


  // client.publish(ROOMS_TOPIC.c_str(),
  //                ("{\"" + String(OT) + ROOM_TEMPERATURE + "_1\": " + payloadvalue_startend_val + String(room_temp[0].tempread) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + "_1\": " + payloadvalue_startend_val + String(room_temp[0].tempset) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE + "_2\": " + payloadvalue_startend_val + String(room_temp[1].tempread) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + "_2\": " + payloadvalue_startend_val + String(room_temp[1].tempset) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE + "_3\": " + payloadvalue_startend_val + String(room_temp[2].tempread) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + "_3\": " + payloadvalue_startend_val + String(room_temp[2].tempset) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE + "_4\": " + payloadvalue_startend_val + String(room_temp[3].tempread) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + "_4\": " + payloadvalue_startend_val + String(room_temp[3].tempset) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE + "_5\": " + payloadvalue_startend_val + String(room_temp[4].tempread) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + "_5\": " + payloadvalue_startend_val + String(room_temp[4].tempset) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE + "_6\": " + payloadvalue_startend_val + String(room_temp[5].tempread) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + "_6\": " + payloadvalue_startend_val + String(room_temp[5].tempset) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE + "_7\": " + payloadvalue_startend_val + String(room_temp[6].tempread) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + "_7\": " + payloadvalue_startend_val + String(room_temp[6].tempset) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE + "_8\": " + payloadvalue_startend_val + String(room_temp[7].tempread) + payloadvalue_startend_val +
  //                 ",\"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + "_8\": " + payloadvalue_startend_val + String(room_temp[7].tempset) + payloadvalue_startend_val +
  //                 "}").c_str(), mqtt_Retain); //"heat" : "off")

  publishhomeassistantconfig++; // zwiekszamy licznik wykonan wyslania mqtt by co publishhomeassistantconfigdivider wysłań wysłać autoconfig discovery dla homeassisatnt
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
  {
    // homeassistant/sensor/BB050B_OPENTHERM_OT10_lo/config = {"name":"Opentherm OPENTHERM OT10 lo","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_lo","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['lo']}}"} (retained) problem
    // 21:16:02.724 MQT: homeassistant/sensor/BB050B_OPENTHERM_OT10_hi/config = {"name":"Opentherm OPENTHERM OT10 hi","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_hi","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['hi']}}"} (retained)
 //   const String temperature_class = "\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\"";
 //   const String temperature_class_lines = "\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer-lines\"";
    for (int x=0;x<createhasensors;x++){
      String identyfikator = getIdentyfikator(x);
      if (room_temp[x].idpinout>0 and check_isValidTemp(room_temp[x].tempread)) HADiscovery(String(ROOMS_TOPIC_SENSOR), String(OT), String(ROOM_TEMPERATURE) + String(identyfikator), String(HA_SENSORS_TOPIC), "temperature");
//      if (room_temp[x].tempread!=InitTemp) mqttclient.publish((HA_SENSORS_TOPIC + "_" + ROOM_TEMPERATURE + identyfikator + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(room_temp[x].nameSensor) + identyfikator+"\",\"uniq_id\": \"" + String(OT) + ROOM_TEMPERATURE + identyfikator+"\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + String(OT) + ROOM_TEMPERATURE + identyfikator+"}}\","+temperature_class+",\"qos\":" + String(QOS) + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
      //mqttclient.publish((HA_SENSORS_TOPIC + "_" + ROOM_TEMPERATURE + identyfikator + "/config").c_str(), ("{\"name\":\"" + String(OT) + ROOM_TEMPERATURE + identyfikator+"\",\"uniq_id\": \"" + String(OT) + ROOM_TEMPERATURE + identyfikator+"\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + String(OT) + ROOM_TEMPERATURE + identyfikator+"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
      if (room_temp[x].idpinout>0 and check_isValidTemp(room_temp[x].tempread)) HADiscovery(String(ROOMS_TOPIC_SENSOR), String(OT), String(ROOM_TEMPERATURE_SETPOINT) + String(identyfikator), String(HA_SENSORS_TOPIC), "temperature");
      //if (room_temp[x].idpinout>0 and check_isValidTemp(room_temp[x].tempread)) mqttclient.publish(((String(HA_SENSORS_TOPIC) + "_" + String(ROOM_TEMPERATURE_SETPOINT) + String(identyfikator) + "/config").c_str(), ("{\"name\":\"" +  String(OT) + String(room_temp[x].nameSensor) + identyfikator +"SP\",\"uniq_id\": \"" + String(OT) + ROOM_TEMPERATURE_SETPOINT + identyfikator+"\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + String(OT) + ROOM_TEMPERATURE_SETPOINT + identyfikator+"}}\","+temperature_class_lines+",\"qos\":" + String(QOS) + "," + mqttdeviceid + "}").c_str(), String(mqtt_Retain));
      if (room_temp[x].idpinout>0 and check_isValidTemp(room_temp[x].tempread)) {
      String tmpbuilderpart = ("{\"name\":\"" + String(OT) + String(ROOM_TEMP) + String(room_temp[x].nameSensor) + identyfikator + "\",\"uniq_id\": \"" + String(OT) + String(ROOM_TEMP) + identyfikator + "\", \
\"modes\":[\"off\",\"cool\",\"heat\"], \
\"mode_state_topic\": \"" + ROOMS_TOPIC_SENSOR + "\", \
\"mode_state_template\": \"{{'heat' if now() > today_at('0:00') else 'heat'}}\", \
\"icon\": \"mdi:radiator\", \
\"current_temperature_topic\":\"" + String(ROOMS_TOPIC_SENSOR) + "\", \
\"current_temperature_template\":\"{{value_json." + String(OT) + String(ROOM_TEMPERATURE) + String(identyfikator) + "}}\", \
\"temperature_command_topic\":\"" + String(ROOM_TEMPERATURE_SETPOINT_SET_TOPIC) + String(identyfikator) + String(SET_LAST) + "\", \
\"temperature_state_topic\":\"" + String(ROOMS_TOPIC_SENSOR) + "\", \
\"temperature_state_template\":\"{{value_json." + String(OT) + String(ROOM_TEMPERATURE_SETPOINT) + String(identyfikator) + "}}\", \
\"temperature_unit\":\"C\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + String(roomtemplo) + ", \
\"max_temp\": " + String(roomtemphi) + ", \
\"qos\":" + String(QOS) + "," + String(mqttdeviceid) + "}");
  #ifdef enableMQTT
  mqttclient.publish((String(HA_CLIMATE_TOPIC) + identyfikator + "_climate/config").c_str(), String(tmpbuilderpart).c_str(), mqtt_Retain);
  #endif //
  #ifdef enableMQTTAsync
  packetIdSub = mqttclient.publish((String(HA_CLIMATE_TOPIC) + String(ROOM_TEMP) + identyfikator + "_climate/config").c_str(), QOS, mqtt_Retain, tmpbuilderpart.c_str());
  if (packetIdSub == 0) packetIdSub = 0;
  #endif
    }


    }
    //max tempset for rooms and min temp in rooms

    HADiscovery(String(ROOMS_TOPIC_SENSOR), String(OT), String(ROOM_TEMPERATURE) + String(getIdentyfikator(-1)), String(HA_SENSORS_TOPIC), "temperature");
    HADiscovery(String(ROOMS_TOPIC_SENSOR), String(OT), String(ROOM_TEMPERATURE) + String(getIdentyfikator(-1)), String(HA_SENSORS_TOPIC), "temperature");
//    mqttclient.publish(String(String(HA_SENSORS_TOPIC) + "_" + String(ROOM_TEMPERATURE) + getIdentyfikator(-1) + "/config").c_str(), String("{\"name\":\"" + String(OT) + String(ROOM_TEMPERATURE) + getIdentyfikator(-1)+"_min\",\"uniq_id\": \"" + String(OT) + String(ROOM_TEMPERATURE) + getIdentyfikator(-1)+"\",\"stat_t\":\"" + String(ROOMS_TOPIC_SENSOR) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(ROOM_PUMPSTATE) + getIdentyfikator(-1)+"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(mqttdeviceid) + "}").c_str(), String(mqtt_Retain));
//    mqttclient.publish(String(String(HA_SENSORS_TOPIC) + "_" + String(ROOM_TEMPERATURE_SETPOINT) + getIdentyfikator(-1)+"/config").c_str(), String("{\"name\":\"" + String(OT) + String(ROOM_TEMPERATURE_SETPOINT) + getIdentyfikator(-1)+"_max\",\"uniq_id\": \"" + String(OT) + String(ROOM_TEMPERATURE_SETPOINT) + getIdentyfikator(-1)+"\",\"stat_t\":\"" + String(ROOMS_TOPIC_SENSOR) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(ROOM_TEMPERATURE_SETPOINT) + getIdentyfikator(-1)+"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer-lines\",\"qos\":" + String(QOS) + "," + String(mqttdeviceid) + "}").c_str(), String(mqtt_Retain));
    #ifdef enableDHT
    HADiscovery(String(ROOMS_TOPIC_SENSOR), String(OT), String(ROOM_HUMIDITY) + String(getIdentyfikator(-1)) + "_DHT_Humid", String(HA_SENSORS_TOPIC), "humidity");
    HADiscovery(String(ROOMS_TOPIC_SENSOR), String(OT), String(ROOM_TEMPERATURE) + String(getIdentyfikator(-1)) + "_DHT_Temp", String(HA_SENSORS_TOPIC), "temperature");
    #endif
    HADiscovery(String(ROOMS_TOPIC_SENSOR), String(OT), String(ROOM_PUMPSTATE) + String(getIdentyfikator(-1)) + "_pumpstate", String(HA_BINARY_TOPIC));
//mqttclient.publish(String(String(HA_SENSORS_TOPIC) + "_" + String(ROOM_HUMIDITY) + getIdentyfikator(-1) + "_DHT_Humid/config").c_str(), String("{\"name\":\"" + String(OT) + String(ROOM_HUMIDITY) + getIdentyfikator(-1)+"_DHT_Humid\",\"uniq_id\": \"" + String(OT) + String(ROOM_HUMIDITY) + getIdentyfikator(-1)+"_DHT_Humid\",\"stat_t\":\"" + String(ROOMS_TOPIC_SENSOR) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(ROOM_HUMIDITY) + getIdentyfikator(-1) + "_DHT_Humid" +"}}\",\"dev_cla\":\"humidity\",\"unit_of_meas\": \"%\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(mqttdeviceid) + "}").c_str(), String(mqtt_Retain));
//mqttclient.publish(String(String(HA_SENSORS_TOPIC) + "_" + String(ROOM_TEMPERATURE) + getIdentyfikator(-1) + "_DHT_Temp/config").c_str(), String("{\"name\":\"" + String(OT) + String(ROOM_TEMPERATURE) + getIdentyfikator(-1)+"_DHT_Temp\",\"uniq_id\": \"" + String(OT) + String(ROOM_TEMPERATURE) + getIdentyfikator(-1)+"_DHT_Temp\",\"stat_t\":\"" + String(ROOMS_TOPIC_SENSOR) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(ROOM_PUMPSTATE) + getIdentyfikator(-1) + "_DHT_Temp" +"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(mqttdeviceid) + "}").c_str(), String(mqtt_Retain));
//mqttclient.publish(String(String(HA_SWITCH_TOPIC) + "_" + String(ROOM_PUMPSTATE) + getIdentyfikator(-1) + "_pumpstate/config").c_str(), String("{\"name\":\"" + String(OT) + String(ROOM_PUMPSTATE) + getIdentyfikator(-1)+"_pumpstate\",\"uniq_id\": \"" + String(OT) + String(ROOM_PUMPSTATE) + getIdentyfikator(-1)+"_pumpstate\",\"stat_t\":\"" + String(ROOMS_TOPIC_SENSOR) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(ROOM_PUMPSTATE) + getIdentyfikator(-1) + "_pumpstate" +"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(mqttdeviceid) + "}").c_str(), String(mqtt_Retain));



/*
https://www.home-assistant.io/integrations/climate.mqtt/
\"modes\":[\"off\",\"heat\"], \

\"mode_state_topic\": \"" + ROOMS_TOPIC + "\", \
\"mode_state_template\": \"{{value_json." + String(OT) + HOT_WATER_SOFTWARE_CH_STATE + "}}\", \
\"mode_command_topic\": \"" + STATE_DHW_SET_TOPIC + "\", \
\"mode_command_template\" : \"{% set values = { 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \

*/

  }
  log_message((char*)F("MQTT Data Sended..."));

}
#endif


bool getRemoteTempHumid(String payloadStr, u_int roomnotmp, u_int action, String json_temp = F("\0"), String json_humid = F("\0"))
{
//  #include "configmqtttopics.h"
  String messagestr = "\0";
  if (json_temp == "\0") {messagestr = payloadStr;} else {messagestr = getJsonVal(payloadStr, json_temp);}
  log_message((char*)messagestr.c_str());
  String logtempval = String(F("Floor_")) + kondygnacja + String(F("_Room_")) + String(roomnotmp + 1) + String(F("_")) + room_temp[roomnotmp].nameSensor;
  if (PayloadtoValidFloatCheck(messagestr))           //invalid val is displayed in funct
  {
    float tmp = 0;

    tmp = PayloadtoValidFloat(messagestr, true, roomtemplo, roomtemphi);     //true to get output to serial and webserial
    logtempval += (action == action_temp)?" -Temperature":(action == action_tempset)?" -Setpoint Temperature":" -Unknown Action";
    logtempval += F(" updated from MQTT to ");
    logtempval += String(tmp);//+" Payl: "+payloadStr);
    if (action == action_temp) room_temp[roomnotmp].tempread = tmp;
    if (action == action_tempset)
    {
      room_temp[roomnotmp].tempset = tmp;
      if (String(room_temp[roomnotmp].nameSensor) == String(cutOffTempVAL).substring(0,namelength)) {cutOffTemp = room_temp[roomnotmp].tempset;}

    }

    if (json_humid != "\0") if (PayloadtoValidFloatCheck(getJsonVal(payloadStr, json_humid)))           //invalid val is displayed in funct
    {
      room_temp[roomnotmp].humidityread = PayloadtoValidFloat(getJsonVal(payloadStr, json_humid),true);     //true to get output to serial and webserial
      logtempval += F(" and humidity to ");
      logtempval += String(room_temp[roomnotmp].humidityread);//+" Payl: "+payloadStr);
      log_message((char*)logtempval.c_str());
      return true;
    }
  } else
  {
    logtempval += F(" - wrong data.");
    log_message((char*)logtempval.c_str());
  }
  return false;
}  //48385: Inproper Json format or null: 26 to find: emptyjhumid




void mqttCallbackAsString(String topicStr, String payloadStr)
{
//tttopics.h"

//NEWS temperature
  if (topicStr == NEWS_GET_TOPIC)               //NEWS averange temp -outside temp
  {
    String ident = F(" NEWS temp ");
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr,NEWStemp_json)))           //invalid val is displayed in funct
    {
      temp_NEWS = PayloadtoValidFloat(getJsonVal(payloadStr,NEWStemp_json),true);     //true to get output to serial and webserial
      if (temp_NEWS < temptoheat) modestate = MODEHEAT;
      if (temp_NEWS > temptocool) modestate = MODECOOL;
      lastNEWSSet = millis();
      temp_NEWS_count = 0;
//      receivedmqttdata = true;    //makes every second run mqtt send and influx
      ident += " updated from MQTT to " + String(temp_NEWS);//+" Payl: "+payloadStr);
    } else
    {
      ident += " Not updated...";
    }
    log_message((char*)ident.c_str());
   } else
  if (topicStr == COPUMP_GET_TOPIC)                                                                   //external CO Pump Status
  {
    String ident = F(" Wood/coax CO Pump Status ");
    receivedmqttdata = true;
    bool tmp = false;
    if (PayloadStatus(getJsonVal(payloadStr,COPumpStatus_json),true) && PayloadStatus(getJsonVal(payloadStr,COPumpStatus_json),true)) tmp = true;
    if (CO_BoilerPumpWorking!=tmp) receivedmqttdata = true;
    if (PayloadStatus(getJsonVal(payloadStr,COPumpStatus_json), true)) {CO_PumpWorking = true;}
    else if (PayloadStatus(getJsonVal(payloadStr,COPumpStatus_json), false)) {CO_PumpWorking = false;}
    else
    {
      receivedmqttdata = false;
      ident += ("Unknown: "+String(payloadStr));
    }
    if (receivedmqttdata) {
      ident += (CO_PumpWorking ? "Active" : "Disabled" );
    }
    log_message((char*)ident.c_str());
  } else
//BOILER OPENTHERM CO Status with flame
  if (topicStr == BOILER_FLAME_STATUS_TOPIC)
  {
    String ident = F(" Status Boiler flame and co pump ");
    bool tmp = false;
    if (PayloadStatus(getJsonVal(payloadStr,BOILER_FLAME_STATUS_ATTRIBUTE),true) && PayloadStatus(getJsonVal(payloadStr,BOILER_COPUMP_STATUS_ATTRIBUTE),true)) tmp = true;
    else if (PayloadStatus(getJsonVal(payloadStr,BOILER_FLAME_STATUS_ATTRIBUTE),false) && PayloadStatus(getJsonVal(payloadStr,BOILER_COPUMP_STATUS_ATTRIBUTE),false)) tmp = false;
    else
    {
      receivedmqttdata = false;
      ident += "Unknown: "+String(payloadStr);
    }
    if (CO_BoilerPumpWorking!=tmp) receivedmqttdata = true;
    CO_BoilerPumpWorking = tmp;
    if (receivedmqttdata) {
      ident += CO_BoilerPumpWorking ? "Active Heating" : "Disabled Heating";
      }
    log_message((char*)ident.c_str());
  } else

//sensors for setpoint temperature
  for (int x=0;x<maxsensors;x++)
  {
    if (topicStr == room_temp[x].mqtt_temp_floor_topic) receivedmqttdata = getRemoteTempHumid(payloadStr, x, action_temp, room_temp[x].mqtt_temp_json, room_temp[x].mqtt_humid_json);
    if (topicStr == room_temp[x].mqtt_tempset_floor_topic) receivedmqttdata = getRemoteTempHumid(payloadStr, x, action_tempset, room_temp[x].mqtt_temp_json);

  // if ((topicStr == ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getIdentyfikator(x) + SET_LAST) and (room_temp[x].idpinout>0))
  //   {
  //     if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
  //     {
  //       float tmp = PayloadtoValidFloat(payloadStr, true, roomtemplo, roomtemphi);
  //       if (tmp!=room_temp[x].tempset) {receivedmqttdata = true;}
  //       room_temp[x].tempset = tmp;
  //       sprintf(log_chars, " Setpoint temperature for ROOM %s: %f", String(getIdentyfikator(x)), room_temp[x].tempset);
  //       log_message(log_chars);
  //     } else
  //     {
  //       log_message((char*)F("Wrong payload on setpoint temp"));
  //     }
  //   }mqttclient
  }

}

#ifdef enableMQTT
void mqtt_reconnect()
{
//  #include "configmqtttopics.h"

  if (me_lokalizacja.lastIndexOf(kondygnacja) != me_lokalizacja.length()-1) me_lokalizacja += kondygnacja;//+"_mqqt_MARM";

  unsigned long now = millis();
  if ((unsigned long)(now - lastmqtt_reconnect) > MQTTRECONNECTTIMER) { //only try reconnect each MQTTRECONNECTTIMER seconds or on boot when lastMqttReconnectAttempt is still 0
    lastmqtt_reconnect = now;
    log_message((char*)"Reconnecting to mqtt server ...");
    sprintf(log_chars,"me_lokalizacja: %s, kondygnacja: %s, len: %s, lastindexof: %s",me_lokalizacja, kondygnacja, String(me_lokalizacja.length()), String(me_lokalizacja.lastIndexOf(kondygnacja)));
    log_message(log_chars);
    char topic[256];
    sprintf(topic, "%s", WILL_TOPIC);
    mqttclient.disconnect();
    if (mqttclient.connect(me_lokalizacja.c_str(), mqtt_user, mqtt_password, topic, 1, true, "Offline"))
    {
      mqttReconnects++;
      sprintf(topic, "%s", WILL_TOPIC);
      mqttclient.publish(topic, "Online");
      sprintf(topic, "%s", IP_TOPIC);
      mqttclient.publish(topic, WiFi.localIP().toString().c_str(), true);

      mqttReconnect_subscribe_list();

    }

  }
}
#endif   //enableMQTT

void mqttReconnect_subscribe_list() {
  log_message((char*)F("MQTT Reconnect Subscribe List..."));
  #ifdef enableMQTT
  for (int x=0;x<maxsensors;x++){
    //String identyfikator=getIdentyfikator(x);
    if (room_temp[x].idpinout!=0) mqttclient.subscribe((room_temp[x].mqtt_tempset_floor_topic).c_str());  //register tempset   ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getIdentyfikator(x) + SET_LAST
    if (room_temp[x].mqtt_temp_floor_topic != "\0") mqttclient.subscribe((room_temp[x].mqtt_temp_floor_topic).c_str()); //register remote temp sensors
  }
  mqttclient.subscribe(NEWS_GET_TOPIC.c_str());
  mqttclient.subscribe(COPUMP_GET_TOPIC.c_str());
  mqttclient.subscribe(BOILER_FLAME_STATUS_TOPIC.c_str());
  #endif
  #ifdef enableMQTTAsync
  uint16_t packetIdSub;
  for (int x=0;x<maxsensors;x++){
    //String identyfikator=getIdentyfikator(x);
    if (room_temp[x].idpinout!=0) packetIdSub = mqttclient.subscribe((room_temp[x].mqtt_tempset_floor_topic).c_str(), QOS);  //register tempset   ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getIdentyfikator(x) + SET_LAST
    if (room_temp[x].mqtt_temp_floor_topic != "\0") packetIdSub = mqttclient.subscribe((room_temp[x].mqtt_temp_floor_topic).c_str(), QOS); //register remote temp sensors
  }
  packetIdSub = mqttclient.subscribe(NEWS_GET_TOPIC.c_str(), QOS);
  packetIdSub = mqttclient.subscribe(COPUMP_GET_TOPIC.c_str(), QOS);
  packetIdSub = mqttclient.subscribe(BOILER_FLAME_STATUS_TOPIC.c_str(), QOS);
  if (packetIdSub == 0) packetIdSub = 0; //return topicnr if qos 1 or 2
  #endif
}

//additional for load/save
String addusage_local_values_save(int EpromPosition){
 return "\0";
}
void addusage_local_values_load(String dane, int EpromPosition){

}