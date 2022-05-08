//Main.cpp



#include "main.h"
#include "dane.h"
#include <ESPmDNS.h>



AsyncWebServer webserver(wwwport);
// for ota webs



OneWire oneWire(ROOM_TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient client(espClient);

// ESP8266WebServer server(80);

#ifdef ENABLE_INFLUX
InfluxDBClient InfluxClient(INFLUXDB_URL, INFLUXDB_DB_NAME);
Point InfluxSensor(InfluxMeasurments);
#endif

#include "other.h"
#include "websrv_ota.h"




String convertPayloadToStr(byte *payload, unsigned int length)
{
  char s[length + 1];
  s[length] = 0;
  for (int i = 0; i < length; ++i)
    s[i] = payload[i];
  String tempRequestStr(s);
  return tempRequestStr;
}

bool isValidNumber(String str)
{
  bool valid = true;
  for (byte i = 0; i < str.length(); i++)
  {
    char ch = str.charAt(i);
    valid &= isDigit(ch) ||
             ch == '+' || ch == '-' || ch == ',' || ch == '.' ||
             ch == '\r' || ch == '\n';
  }
  return valid;
}


void callback(char *topic, byte *payload, unsigned int length)
{
  const String topicStr(topic);

  String payloadStr = convertPayloadToStr(payload, length);
  payloadStr.trim();

  if (topicStr == NEWS_GET_TOPIC)
  {
    payloadStr.replace(",", ".");
    float op_news = payloadStr.toFloat();
    if (isnan(op_news) || !isValidNumber(payloadStr))
    {
#ifdef debug
      Serial.println(F("NEWS is not a valid number, ignoring..."));
#endif
      WebSerial.println(F("NEWS is not a valid number, ignoring..."));
    }
    else
    {
      temp_NEWS = op_news;
      lastNEWSSet = millis();
      temp_NEWS_count = 0;
//      receivedmqttdata = true;    //makes every second run mqtt send and influx

      WebSerial.print(F("NEWS updated from MQTT, to: "));
      WebSerial.println(temp_NEWS);
    }
  }


  for (int x=0;x<maxsensors;x++)
  {
  if ((topicStr == ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getIdentyfikator(x) + SET_LAST) and (room_temp[x].idpinout>0))
    {
      payloadStr.replace(",", ".");
      float t1 = payloadStr.toFloat();
      if (isnan(t1) || !isValidNumber(payloadStr))
      {
  #ifdef debug
        Serial.println(F("Current temp set is not a valid number, ignoring..."));
  #endif
        WebSerial.println(F("Current temp set is not a valid number, ignoring..."));
      }
      else
      {
        if (t1 > roomtemphi)
          t1 = roomtemphi;
        if (t1 < roomtemplo)
          t1 = roomtemplo;
        room_temp[x].tempset = t1;
        // t = t1;
        receivedmqttdata = true;
        WebSerial.println("Room Temp set "+String(getIdentyfikator(x))+": " + payloadStr);
      }
    }
  }

}

String getIdentyfikator(int x)
{
  return "_"+String(x+1);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected() and mqtt_offline_retrycount < mqtt_offline_retries)
  {
#ifdef debug
    Serial.print(F("Attempting MQTT connection..."));
#endif
    WebSerial.print(String(millis())+": "+F("Attempting MQTT connection..."));
    const char *clientId = "MARM-test";

    if (client.connect(clientId, mqtt_user, mqtt_password))
    {
#ifdef debug
      Serial.println(F("ok"));
#endif
      WebSerial.println(F("ok"));
      mqtt_offline_retrycount = 0;

      for (int x=0;x<8;x++){
        String identyfikator=getIdentyfikator(x);
        client.subscribe((ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + identyfikator + SET_LAST).c_str());
      }

      client.subscribe(NEWS_GET_TOPIC.c_str());
      client.subscribe(COPUMP_GET_TOPIC.c_str());
      client.subscribe(TEMP_CUTOFF_SET_TOPIC.c_str());
    }
    else
    {
#ifdef debug
      Serial.print(F(" failed, rc="));
#endif
      WebSerial.print(F(" failed, rc="));
#ifdef debug
      Serial.print(client.state());
#endif
      WebSerial.print(client.state());
#ifdef debug
      Serial.println(F(" try again in 5 seconds"));
#endif
      WebSerial.println(F(" try again in 5 seconds"));
      mqtt_offline_retrycount++;
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void ReadTemperatures()
{
  bool assignedsensor=false;
  UnassignedTempSensor="";
   String addrstr = "";
  //read temperatures
  #ifdef debug
  Serial.println(F("...Reading 1wire sensors..."));
  #endif
  sensors.requestTemperatures();        //Send the command to get temperatures
  uint8_t  addr[8];
  float temp1w;
  int count = sensors.getDS18Count();
  #ifdef debug
  Serial.println("Sensors: "+String(count));
  #endif
  if (count==0) count = maxsensors;
  #ifdef debug
  Serial.println("Sensors new: "+String(count));
  #endif
  for (int j = 0; j < count; j++) {
      temp1w = sensors.getTempCByIndex(j);
      addrstr="";
      sensors.getAddress(addr, j);
      for (int i1 = 0; i1 < 8; i1++) if (String(addr[i1], HEX).length() == 1) addrstr += "0" + String(addr[i1], HEX); else addrstr += String(addr[i1], HEX); //konwersja HEX2StringHEX
      //zapisanie do zmiennej addr[8], addrstr, aktualiozacja index wskazany przez j i aktualozacja temp1w.
      for (int z = 0; z < maxsensors; z++) {
        #ifdef debug
        if (z < 10) Serial.print(String(" "));
        Serial.print(String(z)+": ");
        #endif
        if (array_cmp_8(room_temp[z].addressHEX, addr, sizeof(room_temp[z].addressHEX) / sizeof(room_temp[z].addressHEX[0]),sizeof(addr) / sizeof(addr[0])) == true) {
          assignedsensor=true;
          if (temp1w!=DS18B20nodata) room_temp[z].tempread = temp1w;
        }
      }
      if (!assignedsensor and UnassignedTempSensor.indexOf(addrstr)==-1) UnassignedTempSensor += ";"+String(addrstr)+" "+String(temp1w);
      assignedsensor=false;
  #ifdef debug
      Serial.println((j)+":  Collected ROM=: " + addrstr + "  Temp.: "+String(temp1w));
  #endif
      WebSerial.println(String(j)+":  Collected ROM=: " + addrstr + "  Temp.: "+String(temp1w));
    }

#ifdef debug
Serial.println("Unassigned Sensors: "+UnassignedTempSensor);
#endif
WebSerial.println(String(millis())+": "+"Unassigned Sensors: "+UnassignedTempSensor);

}

void updateInfluxDB()
{
  #ifdef ENABLE_INFLUX
  InfluxSensor.clearFields();
  // Report RSSI of currently connected network
  InfluxSensor.addField("rssi"+String(kondygnacja), (WiFi.RSSI()));
  InfluxSensor.addField("CRT"+String(kondygnacja),  (runNumber));
  float min = room_temp[0].tempread, max = room_temp[0].tempset;
  for (int x=0;x<maxsensors;x++)
  {
    String identyfikator=getIdentyfikator(x);
    if (room_temp[x].tempread!=InitTemp and room_temp[x].tempread>=roomtemplo and room_temp[x].tempread<=roomtemphi) {
      InfluxSensor.addField(String(ROOM_TEMPERATURE)+String(kondygnacja)+identyfikator, room_temp[x].tempread);
      if (min>room_temp[x].tempread) min = room_temp[x].tempread;
    }
    if (room_temp[x].tempset!=InitTemp and room_temp[x].tempset>=roomtemplo and room_temp[x].tempset<=roomtemphi) {
      InfluxSensor.addField(String(ROOM_TEMPERATURE_SETPOINT) +String(kondygnacja)+identyfikator, room_temp[x].tempset);
      if (max<room_temp[x].tempset) max = room_temp[x].tempset;
    }
  }
  if (min!=InitTemp and min>=roomtemplo and min<=roomtemphi) InfluxSensor.addField(String(ROOM_TEMPERATURE)+String(kondygnacja)+getIdentyfikator(-1), min);
  if (max!=InitTemp and max>=roomtemplo and max<=roomtemphi) InfluxSensor.addField(String(ROOM_TEMPERATURE_SETPOINT) +String(kondygnacja)+getIdentyfikator(-1), max);



  InfluxSensor.addField(String(TEMP_CUTOFF)+String(kondygnacja), cutOffTemp);
//  InfluxSensor.addField(String(DIAGS_OTHERS_FAULT), status_Fault ? "1" : "0");
//  InfluxSensor.addField(String(DIAGS_OTHERS_DIAG), status_Diagnostic ? "1" : "0");
  InfluxSensor.addField(String(INTEGRAL_ERROR_GET_TOPIC)+String(kondygnacja), ierr);
  InfluxSensor.addField(String(LOG_GET_TOPIC)+String(kondygnacja), LastboilerResponseError);

  // Print what are we exactly writing
  WebSerial.println(F("Writing to InfluxDB: "));
//  WebSerial.println(InfluxClient.pointToLineProtocol(InfluxSensor));
  // Write point
  if (!InfluxClient.writePoint(InfluxSensor))
  {
    WebSerial.print(F("InfluxDB write failed: "));
    WebSerial.println(InfluxClient.getLastErrorMessage());
  }
#endif
}








void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println(F("Starting... Delay3000..."));

  if (loadConfig())
  {
    Serial.println(F("Config loaded:"));
    Serial.println(CONFIGURATION.version);
    Serial.println(CONFIGURATION.ssid);
    Serial.println(CONFIGURATION.pass);
    Serial.println(CONFIGURATION.mqtt_server);
    Serial.println(CONFIGURATION.COPUMP_GET_TOPIC);
    Serial.println(CONFIGURATION.NEWS_GET_TOPIC);
  }
  else
  {
    Serial.println(F("Config not loaded!"));
    saveConfig(); // overwrite with the default settings
  }

  Serial.println(("Connecting to " + String(ssid)));

  #ifndef ESP32
  WiFi.hostname(String(me_lokalizacja).c_str());      //works for esp8266
  #else
  WiFi.disconnect(true);
  WiFi.config(((u32_t)0x0UL),((u32_t)0x0UL),((u32_t)0x0UL));//IPADDR_NONE, INADDR_NONE, INADDR_NONE); //none gives 255.255.255.255 error in libraries
  WiFi.setHostname((me_lokalizacja).c_str());  //for esp32
  #endif
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESPFL2");

  WiFi.begin(ssid, pass);

  int deadCounter = 20;
  while (WiFi.status() != WL_CONNECTED && deadCounter-- > 0)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(("Failed to connect to " + String(ssid)));
    while (true)
      ;
  }
  else
  {
    Serial.println(F("ok"));
  }
  WiFi.enableSTA(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

    // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp32.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    if (!MDNS.begin(me_lokalizacja.c_str())) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);

  Serial.println(WiFi.localIP());


  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

   // Init DS18B20 sensor
  sensors.begin();
  AssignSensors();
  sensors.requestTemperatures();
  sensors.setWaitForConversion(true); // switch to async mode
  ts = millis();
  lastTempSet = -extTempTimeout_ms;




  // Serial.print(String(sensors.getTempCByIndex(0)));
  // Serial.println("ºC");




  started = millis();

  WebSerial.begin(&webserver);
  WebSerial.msgCallback(recvMsg);
  WebServers();

  #ifdef ENABLE_INFLUX
  //InfluxDB
  InfluxClient.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
  // Alternatively, set insecure connection to skip server certificate validation
  InfluxClient.setInsecure();
  // Add tags
  InfluxSensor.addTag("device", me_lokalizacja);
    // Check server connection
  if (InfluxClient.validateConnection()) {
    WebSerial.print(String(millis())+": "+"Connected to InfluxDB: ");
    WebSerial.println(InfluxClient.getServerUrl());
  } else {
    WebSerial.print("InfluxDB connection failed: ");
    WebSerial.println(InfluxClient.getLastErrorMessage());
  }
  #endif


}

  #define abs(x) ((x)>0?(x):-(x))

void loop()
{
  //unsigned long now = millis() + 0; // TO AVOID compare -2>10000 which is true ??? why?
  // check mqtt is available and connected in other case check values in api.
  if (mqtt_offline_retrycount == mqtt_offline_retries)
  {
    if ((millis() - lastmqtt_reconnect) > mqtt_offline_reconnect_after_ms)
    {
      lastmqtt_reconnect = millis();
      mqtt_offline_retrycount = 0;
      WebSerial.println(String(millis())+": "+F("MQTT connection problem -now reset retry counter and try again..."));
    }
    else
    {
      #ifdef debug
      Serial.println(F("MQTT connection problem -now try get temp data alternative way (room temp and NEWS temp and Carbon CO Water pump status")); //Insert some delay to limit messages to webserial or errors
      #endif
      delay(1500);
      // best place to function get values from http when mqtt is unavailable
      //lastNEWSSet = now; // reset counter news temp with alternative parse value way
      //temp_NEWS_count = 0;
    }
  }
  else
  {
    if (!client.connected())
    {
      WebSerial.println(String(millis())+": "+F("MQTT connection problem -try to connect again..."));
      delay(500);
      reconnect();
    }
    else
    {
      client.loop();
    }
  }

// WebSerial.println("NOW: "+String(now)+ "last upd: "+String(lastUpdate)+" statusintr: "+String(statusUpdateInterval_ms));
// WebSerial.println("Warunek: "+String((((now - lastUpdate) > statusUpdateInterval_ms) or lastUpdate==0)));
// WebSerial.println("Warunek: "+String((((now - lastUpdate) > statusUpdateInterval_ms) )));
// delay(5000);

  if (((millis() - lastUpdateTempPump) > statusUpdateInterval_ms) or lastUpdateTempPump==0 or ((millis() < statusUpdateInterval_ms) and (int(millis()/1000)%30==0)) )   //after start for first defauylt 600s every 30s refresh
  {
    WebSerial.println("now: "+String(millis())+" temps+pumps "+"lastUpdate: "+String(lastUpdateTempPump)+" statusUpdateInterval_ms: "+String(statusUpdateInterval_ms));
    lastUpdateTempPump = millis();
    ReadTemperatures();
    Pump_Activate_Control();
//    opentherm_update_data(lastUpdatemqtt); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }

  if (((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms) or (receivedmqttdata == true) or lastUpdatemqtt==0)   //recived data ronbi co 800ms -wylacze ten sttus dla odebrania news
  {
    WebSerial.println("now: "+String(millis())+" mqtt+influ "+"lastUpdatemqtt: "+String(lastUpdatemqtt)+" receivedmqttdata: "+String(receivedmqttdata)+" mqttUpdateInterval_ms: "+String(mqttUpdateInterval_ms));
    WebSerial.println(String(lastUpdatemqtt)+": Update MQTT and InfluxDB data: ");
    receivedmqttdata = false;
    lastUpdatemqtt = millis();
    if (client.connected()) {
      updateMQTTData();
      updateInfluxDB(); //i have on same server mqtt and influx so when mqtt is down influx probably also ;(  This   if (InfluxClient.isConnected())  doesn't work forme 202205
    }

  }




  //#define abs(x) ((x)>0?(x):-(x))
  if ((millis() - lastNEWSSet) > temp_NEWS_interval_reduction_time_ms)
  { // at every 0,5hour lower temp NEWS when no communication why -2>1800000 is true ???
    WebSerial.println("now: "+String(millis())+" "+"lastNEWSSet: "+String(lastNEWSSet)+" "+"temp_NEWS_interval_reduction_time_ms: "+String(temp_NEWS_interval_reduction_time_ms));
    lastNEWSSet = millis();
    temp_NEWS_count++;
    if (temp_NEWS > cutOffTemp)
    {
      temp_NEWS = temp_NEWS - temp_NEWS * 0.05;
      WebSerial.println(String(millis())+": "+F("Lowering by 5% temp_NEWS (no communication) -after 10times execute every 30minutes lowered temp NEWS"));
    }
    else
    {
      temp_NEWS = cutOffTemp;
    }
    if (temp_NEWS_count > 10)
    {
      CO_PumpWorking = false; // assume that we loose mqtt connection to other system where is co pump controlled -so after 10 times lowered NEWS temp by 5% we also disable CO_Pump_Working to allow heat by this heater -default it counts 5hours no communication
      WebSerial.println(F("Force disable CO_PumpWorking flag -after 10times execute every 30minutes lowered temp NEWS"));
      temp_NEWS_count = 0;
    }
    // dobre miejsce do try get data by http api
    WebSerial.println(F("Korekta temperatury NEWS z braku połaczenia -pomniejszona o 5%"));
  }

  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (millis() - WiFipreviousMillis >= WiFiinterval))
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    WiFipreviousMillis = millis();
  }

  if ((millis() - lastSaveConfig) > save_config_every)
  {
    lastSaveConfig = millis();
    WebSerial.println(String(millis())+": "+F("Saving config to EEPROM memory..."));
    saveConfig(); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }

  static bool failFlag = false;
  bool fail = millis() - lastTempSet > extTempTimeout_ms && millis() - lastSpSet > spOverrideTimeout_ms + millis();
  if (fail)
  {
    if (!failFlag)
    {
      failFlag = true;
#ifdef debug
      Serial.printf("Neither temperature nor setpoint provided, setting heating water to %.1f\r\n", noCommandSpOverride);
#endif
      WebSerial.println(String(millis())+": "+"Neither temperature nor setpoint provided, setting heating water to " + String(noCommandSpOverride));
    }

    lastSpSet = millis();
    op_override = noCommandSpOverride;
  }
  else
  {
    failFlag = false;
  }
  // webserver.handleClient(); //handle http requests
}


void Pump_Activate_Control () {
  WebSerial.println(String(millis())+": Pump loop...");
  GetSpecificSensorData();
  for (int o = 0; o < maxsensors; o++) {
    if (room_temp[o].tempread == InitTemp) room_temp[o].switch_state = stop; //-wylacz gdy temp na wartosci inicjalnej
    //aktywacja przekaznika low, moja plyta HIGH
    if (o<8) if (room_temp[o].tempread>room_temp[o].tempset) room_temp[o].switch_state=stop; else room_temp[o].switch_state=start;  //stop if tempread is > tempset HIGH disable, LOW enable
    if (o>=8) if (room_temp[o].tempread<room_temp[o].tempset) room_temp[o].switch_state=stop; else room_temp[o].switch_state=start;  //stop if tempread is > tempset HIGH disable, LOW enable
    if (room_temp[o].idpinout!=0) {
      if (pump==stop) room_temp[o].switch_state = stop;  //turnoff floor switches when pump is off








//room_temp[o].switch_state = 1;








      digitalWrite(room_temp[o].idpinout, room_temp[o].switch_state); //chyba raczej do 8 bo tyle czujnikow w pokoju
      #ifdef debug
      Serial.println(String(o)+": Switching: "+String(room_temp[o].switch_state)+" pump: "+String(pump));
      #endif
      WebSerial.println(String(o)+": Switching: "+String(room_temp[o].switch_state)+" pump: "+String(pump));
      digitalWrite(lampPin, !digitalRead(lampPin));
    }
  }
//  if (pump == stop) digitalWrite(pompa_pin, stop_digital); else digitalWrite(pompa_pin, start_digital);

}

void GetSpecificSensorData () {
  float min = room_temp[0].tempread, max = room_temp[0].tempset;
  for (int x=0;x<8;x++) if (min>room_temp[x].tempread) min = room_temp[x].tempread;
  if (min<roomtemplo) min = roomtemplo;   //get minimal value of all rooms temp

  for (int o = 0; o < maxsensors; o++) {
    if (String(room_temp[o].nameSensor) == (String(kondygnacja)+sepkondname+String(tempcutoff)).substring(0,namelength)) {
      pumpOffVal = room_temp[o].tempset;
      if ((room_temp[o].tempread > min) and (room_temp[o].tempread > pumpOffVal)) pump = start; else pump = stop;  //raczej na wejsciu z 4d dol
      WebSerial.println("------------------------------"+String(o)+": SetPUMP: "+String(room_temp[o].switch_state)+" to pump: "+String(pump));
      room_temp[o].switch_state = pump;
    }
    if (String(room_temp[o].nameSensor) == (String(kondygnacja)+sepkondname+String(tempwename)).substring(0,namelength)) {
      tempwe = room_temp[o].tempread;
    }

      //tempwe = 34;
      //room_temp[o].tempset = pumpofftemp;
      //if ((tempwe > min) and (tempwe > pumpOffVal)) pump = start; else pump = stop; //"Wejscie podloga -gora"
      //room_temp[o].switch_state = pump;
#ifdef debug
      Serial.println(String(o)+": Pump: "+String(pump)+" tempwe: "+String(tempwe)+" MIN: "+String(min)+" PUMPoffTemp: "+String(pumpOffVal));
#endif
      WebSerial.println(String(o)+": Pump: "+String(pump)+" tempwe: "+String(tempwe)+" MIN: "+String(min)+" PUMPoffTemp: "+String(pumpOffVal));

  }
}