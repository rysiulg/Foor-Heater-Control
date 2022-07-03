//other.cpp

#include <Arduino.h>

#include <EEPROM.h>



// load whats in EEPROM in to the local CONFIGURATION if it is a valid setting
bool LoadConfig() {
  #include "configmqtttopics.h"
  // is it correct?
  if (sizeof(CONFIGURATION)<1024) EEPROM.begin(1024); else EEPROM.begin(sizeof(CONFIGURATION)+128); //Size can be anywhere between 4 and 4096 bytes.
  EEPROM.get(1,runNumber);
  if (isnan(runNumber)) runNumber=0;  //or runNumber>80
  runNumber++;
  //EEPROM.get(1+sizeof(runNumber),flame_used_power_kwh);
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){

  // load (overwrite) the local configuration struct
    for (unsigned int i=0; i<sizeof(configuration_type); i++){
      *((char*)&CONFIGURATION + i) = EEPROM.read(CONFIG_START + i);
    }
    room_temp[0].tempset = CONFIGURATION.roomtempset1;
    room_temp[1].tempset = CONFIGURATION.roomtempset2;
    room_temp[2].tempset = CONFIGURATION.roomtempset3;
    room_temp[3].tempset = CONFIGURATION.roomtempset4;
    room_temp[4].tempset = CONFIGURATION.roomtempset5;
    room_temp[5].tempset = CONFIGURATION.roomtempset6;
    room_temp[6].tempset = CONFIGURATION.roomtempset7;
    room_temp[7].tempset = CONFIGURATION.roomtempset8;
    room_temp[8].tempset = CONFIGURATION.roomtempset9;
    room_temp[9].tempset = CONFIGURATION.roomtempset10;

    //roomtemp = CONFIGURATION.roomtemp;
    temp_NEWS = CONFIGURATION.temp_NEWS;
    strcpy(ssid, CONFIGURATION.ssid);
    strcpy(pass, CONFIGURATION.pass);
    strcpy(mqtt_server, CONFIGURATION.mqtt_server);
    strcpy(mqtt_user, CONFIGURATION.mqtt_user);
    strcpy(mqtt_password, CONFIGURATION.mqtt_password);
    mqtt_port = CONFIGURATION.mqtt_port;
    COPUMP_GET_TOPIC=(CONFIGURATION.COPUMP_GET_TOPIC);
    NEWS_GET_TOPIC=(CONFIGURATION.NEWS_GET_TOPIC);
    BOILER_FLAME_STATUS_TOPIC=(CONFIGURATION.BOILER_FLAME_STATUS_TOPIC);
    BOILER_FLAME_STATUS_ATTRIBUTE=(CONFIGURATION.BOILER_FLAME_STATUS_ATTRIBUTE);
    BOILER_COPUMP_STATUS_ATTRIBUTE=(CONFIGURATION.BOILER_COPUMP_STATUS_ATTRIBUTE);
    for (int i=0;i<maxsensors;i++)
    {
      if (String(room_temp[i].nameSensor) == String(tempcutoff).substring(0,namelength)) pumpOffVal = room_temp[i].tempset;      //assign pump to sensor
    }
    return true; // return 1 if config loaded
  }
  //try get only my important values

  return false; // return 0 if config NOT loaded
}

// save the CONFIGURATION in to EEPROM
void SaveConfig() {
  #include "configmqtttopics.h"
  log_message((char*)F("Saving config...........................prepare"));
  unsigned int temp =0;
  //firs read content of eeprom
  EEPROM.get(1,temp);
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){

  // load (overwrite) the local configuration struct
    for (unsigned int i=0; i<sizeof(configuration_type); i++){
      *((char*)&CONFTMP + i) = EEPROM.read(CONFIG_START + i);
    }
  }
//now compare and if changed than save
  if (temp != runNumber ||
      CONFTMP.roomtempset1 != room_temp[0].tempset ||
      CONFTMP.roomtempset2 != room_temp[1].tempset ||
      CONFTMP.roomtempset3 != room_temp[2].tempset ||
      CONFTMP.roomtempset4 != room_temp[3].tempset ||
      CONFTMP.roomtempset5 != room_temp[4].tempset ||
      CONFTMP.roomtempset6 != room_temp[5].tempset ||
      CONFTMP.roomtempset7 != room_temp[6].tempset ||
      CONFTMP.roomtempset8 != room_temp[7].tempset ||
      CONFTMP.roomtempset9 != room_temp[8].tempset ||
      CONFTMP.roomtempset10 != room_temp[9].tempset ||
      strcmp(CONFTMP.ssid, ssid) != 0 ||
      strcmp(CONFTMP.pass, pass) != 0 ||
      strcmp(CONFTMP.mqtt_server, mqtt_server) != 0 ||
      strcmp(CONFTMP.mqtt_user, mqtt_user) != 0 ||
      strcmp(CONFTMP.mqtt_password, mqtt_password) != 0 ||
      CONFTMP.mqtt_port != mqtt_port ||
      strcmp(CONFTMP.COPUMP_GET_TOPIC,String(COPUMP_GET_TOPIC).c_str()) != 0 ||
      strcmp(CONFTMP.NEWS_GET_TOPIC, String(NEWS_GET_TOPIC).c_str()) != 0 ||
      strcmp(CONFTMP.BOILER_FLAME_STATUS_TOPIC, String(BOILER_FLAME_STATUS_TOPIC).c_str()) != 0 ||
      strcmp(CONFTMP.BOILER_FLAME_STATUS_ATTRIBUTE, String(BOILER_FLAME_STATUS_ATTRIBUTE).c_str()) != 0 ||
      strcmp(CONFTMP.BOILER_COPUMP_STATUS_ATTRIBUTE, String(BOILER_COPUMP_STATUS_ATTRIBUTE).c_str()) != 0 ) {  //skip save if runnumber = saved runnumber to avoid too much memory save and wear eeprom
    EEPROM.put(1, runNumber);
    //EEPROM.put(1+sizeof(runNumber), flame_used_power_kwh);
    log_message((char*)F("Saving config........................... to EEPROM some data changed"));


    strcpy(CONFIGURATION.version,CONFIG_VERSION);
    CONFIGURATION.roomtempset1 = room_temp[0].tempset;
    CONFIGURATION.roomtempset2 = room_temp[1].tempset;
    CONFIGURATION.roomtempset3 = room_temp[2].tempset;
    CONFIGURATION.roomtempset4 = room_temp[3].tempset;
    CONFIGURATION.roomtempset5 = room_temp[4].tempset;
    CONFIGURATION.roomtempset6 = room_temp[5].tempset;
    CONFIGURATION.roomtempset7 = room_temp[6].tempset;
    CONFIGURATION.roomtempset8 = room_temp[7].tempset;
    CONFIGURATION.roomtempset9 = room_temp[8].tempset;
    CONFIGURATION.roomtempset10 = room_temp[9].tempset;

    CONFIGURATION.temp_NEWS = temp_NEWS;
    strcpy(CONFIGURATION.ssid,ssid);
    strcpy(CONFIGURATION.pass,pass);
    strcpy(CONFIGURATION.mqtt_server,mqtt_server);
    strcpy(CONFIGURATION.mqtt_user,mqtt_user);
    strcpy(CONFIGURATION.mqtt_password,mqtt_password);
    CONFIGURATION.mqtt_port = mqtt_port;
    strcpy(CONFIGURATION.COPUMP_GET_TOPIC,COPUMP_GET_TOPIC.c_str());
    strcpy(CONFIGURATION.NEWS_GET_TOPIC,NEWS_GET_TOPIC.c_str());
    strcpy(CONFIGURATION.BOILER_FLAME_STATUS_TOPIC,BOILER_FLAME_STATUS_TOPIC.c_str());
    strcpy(CONFIGURATION.BOILER_FLAME_STATUS_ATTRIBUTE,BOILER_FLAME_STATUS_ATTRIBUTE.c_str());
    strcpy(CONFIGURATION.BOILER_COPUMP_STATUS_ATTRIBUTE,BOILER_COPUMP_STATUS_ATTRIBUTE.c_str());

    for (unsigned int i=0; i<sizeof(configuration_type); i++)
      EEPROM.write(CONFIG_START + i, *((char*)&CONFIGURATION + i));
    EEPROM.commit();
  }
}




void Assign_Name_Addr_Pinout(int i, String name, String address, int outpin) {
  #include "configmqtttopics.h"
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
  } else
  if (d == "OFF")
  {
      digitalWrite(BUILTIN_LED, LOW);
  } else
  if (d == "RESTART")
  {
    WebSerial.println(F("OK. Restarting... by command..."));
    restart();
  } else
  if (d == "RECONNECT")
  {
    mqtt_reconnect();
  } else
  if (d == "SAVE")
  {
    WebSerial.println(F("Saving config to EEPROM memory by command..."));
    WebSerial.println("Size CONFIG: " + String(sizeof(CONFIGURATION)));
    SaveConfig();
  } else
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
      WebSerial.println("Part: "+String(part));
      WebSerial.println("pumpno: "+String(pumpno));
      WebSerial.println("myvalue: "+String(myvalue));
      o = pumpno.toInt();
      state = myvalue.toInt()==1;
    }
    WebSerial.println(String(millis())+": "+("Toggle Pump State by command... to: "+String((statusUpdateInterval_ms - (millis() - lastUpdateTempPump))/1000)+"s max: "+String(statusUpdateInterval_ms/1000)+"s" ));
    WebSerial.println("numer: "+String(o)+" Stan: "+String(state));
    WebSerial.println("Stan: "+String(room_temp[o].switch_state)+" dig: "+String(digitalRead(room_temp[o].idpinout)));
    room_temp[o].switch_state = !state;
    digitalWrite(room_temp[o].idpinout, !state);    //aktywny stan niski
    WebSerial.println("Stan2: "+String(room_temp[o].switch_state)+" dig: "+String(digitalRead(room_temp[o].idpinout)));

  } else
  if (d == F("TEMPCUTOFF"))
  {
    String part = d.substring(d.indexOf(" "));
    part.trim();
    WebSerial.print(String(millis())+F(": TEMPCUTOFF: ")+String(pumpOffVal)+F("   "));
    if (d.indexOf(" ")!=-1) {
      if (PayloadtoValidFloatCheck(part)) {pumpOffVal = PayloadtoValidFloat(part,true,18,36);}
      WebSerial.println(" -> ZMIENIONO NA: " + String(pumpOffVal) + ("    Payload: ") + String(d));
      for (int i=0;i<maxsensors;i++)
      {
        if (String(room_temp[i].nameSensor) == String(tempcutoff).substring(0,namelength)) room_temp[i].tempset=pumpOffVal;      //assign pump to sensor
      }
    } else { WebSerial.println("");}
  } else
  if (d == F("RESET_CONFIG"))
  {
    sprintf(log_chars, "RESET config to DEFAULT VALUES and restart... Size CONFIG: %s", String(sizeof(CONFIGURATION)));
    log_message(log_chars);
    CONFIGURATION.version[0] = 'R';
    CONFIGURATION.version[1] = 'E';
    CONFIGURATION.version[2] = 'S';
    CONFIGURATION.version[3] = 'E';
    CONFIGURATION.version[4] = 'T';
    SaveConfig();
    restart();
  } else
   if (d == "HELP")
  {
    log_message((char*)F("KOMENDY:\n \
      TOGGLEPUMP n=0/1 -Zmiana statusu pompy n (numer od 0 do 10) na stan (0 wyłącza, 1 włącza),\n \
      TEMPCUTOFF val   -zmiana temperatury odłączenia pompy poniżej temp val.\n \
	    RESTART          -Uruchamia ponownie układ,\n \
	    RECONNECT        -Dokonuje ponownej próby połączenia z bazami,\n \
	    SAVE             -Wymusza zapis konfiguracji,\n \
	    RESET_CONFIG     -UWAGA!!!! Resetuje konfigurację do wartości domyślnych"));
    //WebSerial.println(F("  FORCECOBELOW xx  -Zmienia wartość xx 'wymusza pompe CO poniżej temperatury średniej zewnetrznej',"));
    //WebSerial.println(F("  COCUTOFFTEMP xx  -Zmienia wartość xx 'Temperatura graniczna na wymienniku oznacza ze piec sie grzeje',"));
  }
}



void updateInfluxDB()
{
  #ifdef ENABLE_INFLUX
  #include "configmqtttopics.h"

  InfluxSensor.clearFields();
  // Report RSSI of currently connected network
  InfluxSensor.addField("rssi"+String(kondygnacja), (WiFi.RSSI()));
  InfluxSensor.addField("HallSensorFH"+kondygnacja, (hallRead()));
  InfluxSensor.addField("CRT_F"+String(kondygnacja),  (runNumber));
  for (int x=0;x<maxsensors;x++)
  {
    String identyfikator=getIdentyfikator(x);
    if (room_temp[x].tempread!=InitTemp and room_temp[x].tempread>roomtemplo and room_temp[x].tempread<=roomtemphi) {
      InfluxSensor.addField(String(ROOM_TEMPERATURE)+String(kondygnacja)+identyfikator, room_temp[x].tempread);
    }
    if (room_temp[x].tempset!=InitTemp and room_temp[x].tempset>roomtemplo and room_temp[x].tempset<=roomtemphi) {
      InfluxSensor.addField(String(ROOM_TEMPERATURE_SETPOINT) +String(kondygnacja)+identyfikator, room_temp[x].tempset);
    }
  }
  if (min_temp!=InitTemp and min_temp>=roomtemplo and min_temp<=roomtemphi) InfluxSensor.addField(String(ROOM_TEMPERATURE)+String(kondygnacja)+getIdentyfikator(-1), min_temp);
  if (max_temp!=InitTemp and max_temp>=roomtemplo and max_temp<=roomtemphi) InfluxSensor.addField(String(ROOM_TEMPERATURE_SETPOINT) +String(kondygnacja)+getIdentyfikator(-1), max_temp);

  InfluxSensor.addField(String(ROOM_TEMPERATURE)+String(kondygnacja)+"_DHT_Temp", tempcor);
  InfluxSensor.addField(String(ROOM_HUMIDITY)+String(kondygnacja)+"_DHT_Humid", humiditycor);
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
#endif
}




void updateMQTTData() {
  const String payloadvalue_startend_val = ""; // value added before and after value send to mqtt queue
  int createhasensors = maxsensors;  //jest tylko 1 temp wejscia !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  #include "configmqtttopics.h"

  String tmpbuilder="{";
  tmpbuilder += "\"rssi\":"+ String(WiFi.RSSI());
  tmpbuilder += ",\"HallSensor"+kondygnacja+"\":"+String(hallRead());
  tmpbuilder += ",\"CRT\":"+ String(runNumber);

  for (int x=0;x<createhasensors;x++)
  {
    String identyfikator = getIdentyfikator(x);
    if (room_temp[x].tempread!=InitTemp and room_temp[x].tempread > roomtemplo) {
      tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE + identyfikator + "\": " + payloadvalue_startend_val + String(room_temp[x].tempread) + payloadvalue_startend_val;

    }
    if (room_temp[x].tempread!=InitTemp and room_temp[x].idpinout>0 and room_temp[x].tempread > roomtemplo) {
      tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + identyfikator + "\": " + payloadvalue_startend_val + String(room_temp[x].tempset) + payloadvalue_startend_val;

    }
  }
  if (min_temp!=InitTemp) tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE + getIdentyfikator(-1) + "\": " + payloadvalue_startend_val + String(min_temp) + payloadvalue_startend_val;
  if (max_temp!=InitTemp) tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE_SETPOINT + getIdentyfikator(-1) + "\": " + payloadvalue_startend_val + String(max_temp) + payloadvalue_startend_val;

  tmpbuilder += ",\"" + OT + ROOM_TEMPERATURE + getIdentyfikator(-1) + "_DHT_Temp\": " + payloadvalue_startend_val + String(tempcor) + payloadvalue_startend_val;
  tmpbuilder += ",\"" + OT + ROOM_HUMIDITY + getIdentyfikator(-1) + "_DHT_Humid\": " + payloadvalue_startend_val + String(humiditycor) + payloadvalue_startend_val;

  tmpbuilder += ",\"" + OT + ROOM_PUMPSTATE + getIdentyfikator(-1) + "_pumpstate\": " + payloadvalue_startend_val + String(!pump?1:0) + payloadvalue_startend_val;

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
\"modes\":[\"off\",\"cool\",\"heat\"], \
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

mqttclient.publish((ROOMS_HA_TOPIC + "_" + ROOM_HUMIDITY + getIdentyfikator(-1) + "_DHT_Humid/config").c_str(), ("{\"name\":\"" + OT + ROOM_HUMIDITY + getIdentyfikator(-1)+"_DHT_Humid\",\"uniq_id\": \"" + OT + ROOM_HUMIDITY + getIdentyfikator(-1)+"_DHT_Humid\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_HUMIDITY + getIdentyfikator(-1) + "_DHT_Humid" +"}}\",\"dev_cla\":\"humidity\",\"unit_of_meas\": \"%\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
mqttclient.publish((ROOMS_HA_TOPIC + "_" + ROOM_TEMPERATURE + getIdentyfikator(-1) + "_DHT_Temp/config").c_str(), ("{\"name\":\"" + OT + ROOM_TEMPERATURE + getIdentyfikator(-1)+"_DHT_Temp\",\"uniq_id\": \"" + OT + ROOM_TEMPERATURE + getIdentyfikator(-1)+"_DHT_Temp\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_TEMPERATURE + getIdentyfikator(-1) + "_DHT_Temp" +"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);
mqttclient.publish((ROOMS_HA_TOPIC_SWITCH + "_" + ROOM_PUMPSTATE + getIdentyfikator(-1) + "_pumpstate/config").c_str(), ("{\"name\":\"" + OT + ROOM_PUMPSTATE + getIdentyfikator(-1)+"_pumpstate\",\"uniq_id\": \"" + OT + ROOM_PUMPSTATE + getIdentyfikator(-1)+"_pumpstate\",\"stat_t\":\"" + ROOMS_TOPIC_SENSOR + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_PUMPSTATE + getIdentyfikator(-1) + "_pumpstate" +"}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + mqttdeviceid + "}").c_str(), mqtt_Retain);

/*
https://www.home-assistant.io/integrations/climate.mqtt/
\"modes\":[\"off\",\"heat\"], \

\"mode_state_topic\": \"" + ROOMS_TOPIC + "\", \
\"mode_state_template\": \"{{value_json." + OT + HOT_WATER_SOFTWARE_CH_STATE + "}}\", \
\"mode_command_topic\": \"" + STATE_DHW_SET_TOPIC + "\", \
\"mode_command_template\" : \"{% set values = { 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \

*/

  }
  log_message((char*)F("MQTT Data Sended..."));

}


bool getRemoteTempHumid(String payloadStr, u_int roomnotmp, u_int action, String json_temp = F("\0"), String json_humid = F("\0"))
{
  #include "configmqtttopics.h"
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