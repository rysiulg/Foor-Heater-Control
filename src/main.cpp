//Main.cpp



#include "main.h"
#include <ESPmDNS.h>

#include "other.h"

#include "websrv_ota.h"


#include "common_functions.h"

//to read bus voltage in stats
//ADC_MODE(ADC_VCC);





void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  #include "configmqtttopics.h"

  const String topicStr(topic);

  String payloadStr = convertPayloadToStr(payload, length);

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
  //   }
  }

}



/*
 *  check_wifi will process wifi reconnecting managing
 */
void check_wifi()
{
  if ((WiFi.status() != WL_CONNECTED) || (!WiFi.localIP()))  {
    /*
     *  if we are not connected to an AP
     *  we must be in softAP so respond to DNS
     */
    dnsServer.processNextRequest();

    /* we need to stop reconnecting to a configured wifi network if there is a hotspot user connected
     *  also, do not disconnect if wifi network scan is active
     */
    if ((ssid[0] != '\0') && (WiFi.status() != WL_DISCONNECTED) && (WiFi.scanComplete() != -1) && (WiFi.softAPgetStationNum() > 0))  {
      log_message((char *)"WiFi lost, but softAP station connecting, so stop trying to connect to configured ssid...");
      WiFi.disconnect(true);
    }

    /*  only start this routine if timeout on
     *  reconnecting to AP and SSID is set
     */
    if ((ssid[0] != '\0') && ((unsigned long)(millis() - lastWifiRetryTimer) > WIFIRETRYTIMER ) )  {
      lastWifiRetryTimer = millis();
      if (WiFi.softAPSSID() == "") {
        log_message((char *)"WiFi lost, starting setup hotspot...");
        WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255, 255, 255, 0));
        WiFi.softAP(me_lokalizacja.c_str());
      }
      if ((WiFi.status() == WL_DISCONNECTED)  && (WiFi.softAPgetStationNum() == 0 )) {
        log_message((char *)"Retrying configured WiFi, ...");
        if (pass[0] == '\0') {
          WiFi.begin(ssid);
        } else {
          WiFi.begin(ssid, pass);
        }
      } else {
        log_message((char *)"Reconnecting to WiFi failed. Waiting a few seconds before trying again.");
        WiFi.disconnect(true);
      }
    }
  } else { //WiFi connected
    if (WiFi.softAPSSID() != "") {
//      log_message((char *)"WiFi (re)connected, shutting down hotspot...");
//      WiFi.softAPdisconnect(true);
//      MDNS.notifyAPChange();
      #ifndef ESP32
      experimental::ESP8266WiFiGratuitous::stationKeepAliveSetIntervalMs(5000); //necessary for some users with bad wifi routers
      #endif
    }

    if (firstConnectSinceBoot) { // this should start only when softap is down or else it will not work properly so run after the routine to disable softap
      firstConnectSinceBoot = false;
      lastmqtt_reconnect = 0; //initiate mqtt connection asap
//      setupOTA();
      MDNS.begin(me_lokalizacja.c_str());
      MDNS.addService("http", "tcp", 80);

      if (ssid[0] == '\0') {
        log_message((char *)"WiFi connected without SSID and password in settings. Must come from persistent memory. Storing in settings.");
        WiFi.SSID().toCharArray(ssid, 40);
        WiFi.psk().toCharArray(pass, 40);
        saveConfig(); //save to config file
      }
    }

    /*
       always update if wifi is working so next time on ssid failure
       it only starts the routine above after this timeout
    */
    lastWifiRetryTimer = millis();

    // Allow MDNS processing
//    MDNS.update();
  }
}

void mqtt_reconnect()
{
  #include "configmqtttopics.h"

  unsigned long now = millis();
  if ((unsigned long)(now - lastmqtt_reconnect) > MQTTRECONNECTTIMER) { //only try reconnect each MQTTRECONNECTTIMER seconds or on boot when lastMqttReconnectAttempt is still 0
    lastmqtt_reconnect = now;
    log_message((char*)"Reconnecting to mqtt server ...");
    char topic[256];
    sprintf(topic, "%s", WILL_TOPIC);
    if (mqttclient.connect(me_lokalizacja.c_str(), mqtt_user, mqtt_password, topic, 1, true, "Offline"))
    {
      mqttReconnects++;
      sprintf(topic, "%s", WILL_TOPIC);
      mqttclient.publish(topic, "Online");
      sprintf(topic, "%s", IP_TOPIC);
      mqttclient.publish(topic, WiFi.localIP().toString().c_str(), true);
      for (int x=0;x<maxsensors;x++){
        //String identyfikator=getIdentyfikator(x);
        if (room_temp[x].idpinout!=0) mqttclient.subscribe((room_temp[x].mqtt_tempset_floor_topic).c_str());  //register tempset   ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getIdentyfikator(x) + SET_LAST
        if (room_temp[x].mqtt_temp_floor_topic != "\0") mqttclient.subscribe((room_temp[x].mqtt_temp_floor_topic).c_str()); //register remote temp sensors
      }
      mqttclient.subscribe(NEWS_GET_TOPIC.c_str());
      mqttclient.subscribe(COPUMP_GET_TOPIC.c_str());
      mqttclient.subscribe(BOILER_FLAME_STATUS_TOPIC.c_str());
    }

  }
}


void ReadTemperatures()
{
  bool assignedsensor=false;
  UnassignedTempSensor="";
   String addrstr = "";
  //read temperatures
  sensors.setWaitForConversion(true); //
  sensors.requestTemperatures();        //Send the command to get temperatures
  //sensors.setWaitForConversion(false); // switch to async mode
  uint8_t  addr[8];
  float temp1w;
  int count = sensors.getDS18Count();
  if (count == 0) count = sensors.getDeviceCount();
  sprintf(log_chars, "Reading 1wire 18B20 and DHT sensors... Count: %i", count);
  log_message(log_chars);
  if (count==0) count = maxsensors;

  for (int j = 0; j < count; j++) {
      temp1w = sensors.getTempCByIndex(j);
      addrstr="";
      sensors.getAddress(addr, j);
      for (int i1 = 0; i1 < 8; i1++) if (String(addr[i1], HEX).length() == 1) addrstr += "0" + String(addr[i1], HEX); else addrstr += String(addr[i1], HEX); //konwersja HEX2StringHEX
      //zapisanie do zmiennej addr[8], addrstr, aktualiozacja index wskazany przez j i aktualozacja temp1w.
      for (int z = 0; z < maxsensors; z++) {
        if (array_cmp_8(room_temp[z].addressHEX, addr, sizeof(room_temp[z].addressHEX) / sizeof(room_temp[z].addressHEX[0]),sizeof(addr) / sizeof(addr[0])) == true) {
          assignedsensor=true;
          if (temp1w!=DS18B20nodata and temp1w!=DS18B20nodata2 and temp1w!=InitTemp) room_temp[z].tempread = temp1w;
        }
      }
      if (!assignedsensor and UnassignedTempSensor.indexOf(addrstr)==-1) UnassignedTempSensor += ";"+String(addrstr)+" "+String(temp1w);
      assignedsensor=false;
      sprintf(log_chars, "%s: Collected 1wire ROMs=: %s, temp: %s", j, addrstr, temp1w);
      log_message(log_chars);
    }

  sprintf(log_chars, "Unassigned Sensors: %s", UnassignedTempSensor);
  log_message(log_chars);

  #ifdef enableDHT
  dht.read(true);
  delay(200);
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) or isnan(t)) {
    log_message((char*)F("Failed to read from DHT sensor!"));
    //if (isnan(t)) t=InitTemp;
    //if (isnan(h)) h=0;
    } else {
      dhtreadtime = millis();
    }

  sprintf(log_chars, "Get DHT values t= %s, humid: %s", String(t,2), String(h,1));
  log_message(log_chars);
  if (!isnan(h)) humiditycor = h;
  if (!isnan(t)) tempcor = t; //to check ds18b20 also ;)
  #endif
}

void setupMqtt() {
  mqttclient.setBufferSize(2048);
  mqttclient.setSocketTimeout(10); mqttclient.setKeepAlive(5); //fast timeout, any slower will block the main loop too long
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(mqtt_callback);
}



void setup()
{
  Serial.begin(74880);
    //first get total memory before we do anything
  getFreeMemory();
  #ifdef doubleResDet
    //double reset detect from start
  doubleResetDetect();
  #endif
  pinMode(lampPin, OUTPUT );
  pinMode(choosepin,INPUT_PULLUP);    // sets the digital pin 13 as output
  Serial.println(F("Starting... Delay3000..."));

  //get Configuration floor 1 or 2 -if set is 2
  if (digitalRead(choosepin)==1) {
    kondygnacja = "2";
  } else {
    kondygnacja = "1";
  }
  AssignSensors();  //po tym ginie mi me_lokalizacja
  me_lokalizacja += kondygnacja;//+"_mqqt_MARM";
  mqttident += kondygnacja + "_";
  mqttdeviceid.replace("me_lokalizacja",me_lokalizacja);
  Serial.println(String(millis())+": "+"choosePin: "+String(choosepin)+" status: "+String(digitalRead(choosepin))+" czyli: "+(digitalRead(choosepin)? "Kondygnacja 2":"Kondygnacja 1")+" - "+String(kondygnacja)+" : "+String(me_lokalizacja));
 // pinMode(choosepin, DISABLED );


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
  }
  saveConfig(); // overwrite with the default settings or save CRT
//  #include "configmqtttopics.h"
// Serial.println ("compare:     ");
// Serial.println(strcmp(CONFIGURATION.BOILER_COPUMP_STATUS_ATTRIBUTE, String(BOILER_FLAME_STATUS_ATTRIBUTE).c_str()) );
  Serial.println(("Connecting to " + String(ssid) +" from: "+String(me_lokalizacja)));

  #ifndef ESP32
  WiFi.hostname(String(me_lokalizacja).c_str());      //works for esp8266
  #else
  btStop();   //disable bluetooth
//  setCpuFrequencyMhz(80);   //STANDARD 240mHz


  WiFi.disconnect(true);
  WiFi.config(((u32_t)0x0UL),((u32_t)0x0UL),((u32_t)0x0UL));//IPADDR_NONE, INADDR_NONE, INADDR_NONE); //none gives 255.255.255.255 error in libraries
  WiFi.setHostname((me_lokalizacja).c_str());  //for esp32
  #endif
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(String(me_lokalizacja).c_str());

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
    while (true);
  }
  else
  {
    Serial.print(F("ok "));
    Serial.println(String(me_lokalizacja));
  }
  WiFi.enableSTA(true);

  #ifndef ESP32
  experimental::ESP8266WiFiGratuitous::stationKeepAliveSetIntervalMs(5000); //necessary for some users with bad wifi routers
  #endif

  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

    // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp32.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network

    // if (!MDNS.begin(me_lokalizacja.c_str())) {
    //     Serial.println("Error setting up MDNS responder!");
    //     while(1) {
    //         delay(1000);
    //     }
    // }
    // Serial.println("mDNS responder started");
    // // Add service to MDNS-SD
    // MDNS.addService("http", "tcp", 80);

  Serial.println(WiFi.localIP());

  ts = millis();
  lastTempSet = -extTempTimeout_ms;
Serial.println("webserwer webserial");
  #ifdef enableWebSerial
  WebSerial.begin(&webserver);
  WebSerial.msgCallback(recvMsg);
  #endif
  starting = false;
  setupMqtt();
  WebServers();

Serial.println("Influx");
  #ifdef ENABLE_INFLUX
  //InfluxDB
  InfluxClient.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
  // Alternatively, set insecure connection to skip server certificate validation
  InfluxClient.setInsecure();
  // Add tags
  sprintf(log_chars, "me-lokalizacja: %s", me_lokalizacja);
  log_message(log_chars);
  InfluxSensor.addTag("device", me_lokalizacja);
    // Check server connection
    #ifdef enableWebSerial
    if (InfluxClient.validateConnection()) {
      sprintf(log_chars, "Connected to InfluxDB: %s", ""); //InfluxClient.getServerUrl());
      log_message(log_chars);
    } else {
      sprintf(log_chars, "InfluxDB connection failed: %s", InfluxClient.getLastErrorMessage());
      log_message(log_chars);
    }
    #endif
  #endif
Serial.println("18B20 begin");
   // Init DS18B20 sensor
  sensors.begin();
  #ifdef enableDHT
  dht.begin();
  #endif
  sensors.setWaitForConversion(true); //
    //sensors.setWaitForConversion(true); //
  sensors.requestTemperatures();

  started = millis();


  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.localIP());

}

  #define abs(x) ((x)>0?(x):-(x))

void loop()
{
  #ifdef doubleResDet
  drd->loop();
  #endif
  // check wifi
  check_wifi();
  // Handle OTA first.
  //ArduinoOTA.handle();
  // then handle HTTP
  //httpServer.handleClient();
  // handle Websockets
  //webSocket.loop();
  mqttclient.loop();
//  read_panasonic_data();


  // run the data query only each WAITTIME
  if ((unsigned long)(millis() - lastloopRunTime) > (LOOP_WAITTIME)) {
    lastloopRunTime = millis();
    //check mqtt
    if ( (WiFi.isConnected()) && (!mqttclient.connected()) )
    {
      log_message((char *)"Lost MQTT connection!");
      mqtt_reconnect();
    }

    //log stats
    #include "configmqtttopics.h"
    String message = F("stats: Uptime: ");
    message += uptimedana();
    message += F(" ## Free memory: ");
    message += getFreeMemory();
    message += F("% ");
    message += ESP.getFreeHeap();
    message += F(" bytes ## Wifi: ");
    message += getWifiQuality();
    message += F("% ## Mqtt reconnects: ");
    message += mqttReconnects;
    message += F("%");
    log_message((char*)message.c_str());

    String stats = F("{\"uptime\":");
    stats += String(millis());
    stats += F(",\"voltage\":");
    stats += 0;
    stats += F(",\"free memory\":");
    stats += getFreeMemory();
    stats += F(",\"ESP cyclecount\":");
    stats += ESP.getCycleCount();
    stats += F(",\"wifi\":");
    stats += getWifiQuality();
    stats += F(",\"mqtt reconnects\":");
    stats += mqttReconnects;
    stats += F("}");
    sprintf(log_chars, "%s", STATS_TOPIC);
    mqttclient.publish(log_chars, stats.c_str(), mqtt_Retain);

    //get new data
  //  if (!heishamonSettings.listenonly) send_panasonic_query();

    //Make sure the LWT is set to Online, even if the broker have marked it dead.
    mqttclient.publish(WILL_TOPIC.c_str(), "Online");


    if (WiFi.isConnected()) {
//      MDNS.announce();
    }


    if ((millis() % 1500) < 100) digitalWrite(lampPin, !digitalRead(lampPin));

    if (((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms) or (receivedmqttdata == true) or (lastUpdatemqtt == 0) )   //recived data ronbi co 800ms -wylacze ten sttus dla odebrania news
    {
      sprintf(log_chars, "Update mqtt+influ last was ago lastUpdatemqtt: %ds, receivedmqttdata: %d, mqttUpdateInterval_ms: %d", (millis()-lastUpdatemqtt) /1000, receivedmqttdata, mqttUpdateInterval_ms);
      log_message(log_chars);
      //log_message((char*)F("Lowering by 5% temp_NEWS (no communication) -after 10times execute every 30minutes lowered temp NEWS"));
      receivedmqttdata = false;
      lastUpdatemqtt = millis();

      if (mqttclient.connected()) {
        updateMQTTData();
        updateInfluxDB(); //i have on same server mqtt and influx so when mqtt is down influx probably also ;(  This   if (Influxclient.isConnected())  doesn't work forme 202205      digitalWrite(lampPin, !digitalRead(lampPin));
      } else {
        log_message((char*)F("Disconnected -Unable send data to MQTT and InfluxDB"));
      }
    }

    if ((millis() - lastSaveConfig) > save_config_every)
    {
      lastSaveConfig = millis();
      log_message((char*)F("Saving config to EEPROM memory..."));
      saveConfig(); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
    }
  }


  if (((millis() - lastUpdateTempPump) > statusUpdateInterval_ms) or lastUpdateTempPump==0 ) //or (((millis() < statusUpdateInterval_ms) or CO_PumpWorking or CO_BoilerPumpWorking) and (int(millis()/1000)%statusUpdateShortenInterval_s==0)) )   //after start for first defauylt 600s every 60s refresh
  {
    sprintf(log_chars, "temps+pumps lastUpdate: %d, statusUpdateInterval_ms: %d", lastUpdateTempPump, statusUpdateInterval_ms);
    log_message(log_chars);
    lastUpdateTempPump = millis();
    ReadTemperatures();
    Pump_Activate_Control();
//    opentherm_update_data(lastUpdatemqtt); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }








  // //#define abs(x) ((x)>0?(x):-(x))
  // if ((millis() - lastNEWSSet) > temp_NEWS_interval_reduction_time_ms)
  // { // at every 0,5hour lower temp NEWS when no communication why -2>1800000 is true ???
  //   sprintf(log_chars, "lastNEWSSet: %d, temp_NEWS_interval_reduction_time_ms: %d", lastNEWSSet, temp_NEWS_interval_reduction_time_ms);
  //   log_message(log_chars);
  //   temp_NEWS_count++;
  //   if (temp_NEWS > cutOffTemp)
  //   {
  //     temp_NEWS = temp_NEWS - temp_NEWS * 0.05;
  //     log_message((char*)F("Lowering by 5% temp_NEWS (no communication) -after 10times execute every 30minutes lowered temp NEWS"));
  //   }
  //   else
  //   {
  //     temp_NEWS = cutOffTemp;
  //   }
  //   if (temp_NEWS_count > 10)
  //   {
  //     CO_PumpWorking = false; // assume that we loose mqtt connection to other system where is co pump controlled -so after 10 times lowered NEWS temp by 5% we also disable CO_Pump_Working to allow heat by this heater -default it counts 5hours no communication
  //     log_message((char*)F("Force disable CO_PumpWorking flag -after 10times execute every 30minutes lowered temp NEWS"));
  //     //temp_NEWS_count = 0;
  //   }
  //   // dobre miejsce do try get data by http api
  //   log_message((char*)F("Korekta temperatury NEWS z braku połaczenia -pomniejszona o 5%"));
  // } else { temp_NEWS_count =0; }

  // // if WiFi is down, try reconnecting
  // if ((WiFi.status() != WL_CONNECTED) && (millis() - WiFipreviousMillis >= WiFiinterval))
  // {
  //   Serial.print(millis());
  //   Serial.println(": Reconnecting to WiFi...");
  //   WiFi.disconnect();
  //   WiFi.begin(ssid, pass);
  //   WiFipreviousMillis = millis();
  // }



}


void Pump_Activate_Control () {
  log_message((char *)F("Pump loop..."));
  if (temp_NEWS < temptoheat or min_temp < (max_temp - histereza)) modestate = MODEHEAT;   //min i max co drugie wykonanie ;()
  if (temp_NEWS > temptocool or min_temp > (max_temp + histereza)) modestate = MODECOOL;
  if ((tempwe > 21 or tempwe != InitTemp) and modestate == MODECOOL) modestate = MODEOFF;
  min_max_state = stoprun;
  GetSpecificSensorData();
  String tmpval="\r";

  for (int o = 0; o < maxsensors; o++) {
    if (room_temp[o].tempread == InitTemp) room_temp[o].switch_state = stoprun; //-wylacz gdy temp na wartosci inicjalnej
    //aktywacja przekaznika low, moja plyta HIGH
    if (o<8 and room_temp[o].tempread == InitTemp and temp_NEWS < temptoheat) {room_temp[o].switch_state=startrun;}
    if (o<8 and room_temp[o].tempread != InitTemp) {
      if (room_temp[o].tempread > room_temp[o].tempset and modestate == MODEHEAT) {room_temp[o].switch_state=stoprun;}
      if (room_temp[o].tempread > room_temp[o].tempset and modestate == MODECOOL) {room_temp[o].switch_state=startrun;}
      if (room_temp[o].tempread < (room_temp[o].tempset + histereza) and modestate == MODEHEAT) {room_temp[o].switch_state=startrun;}
      if (room_temp[o].tempread < (room_temp[o].tempset + histereza) and modestate == MODECOOL) {room_temp[o].switch_state=stoprun;}
      if (room_temp[o].switch_state == startrun) min_max_state = startrun;
    }  //stop if tempread is > tempset HIGH disable, LOW enable
    if (o == 8 and min_max_state == stoprun) pump = stoprun;


  //  if (o>=8) if (room_temp[o].tempread<room_temp[o].tempset) room_temp[o].switch_state=stop; else room_temp[o].switch_state=start;  //stop if tempread is > tempset HIGH disable, LOW enable
    if (room_temp[o].idpinout>0) {
      if (pump==stoprun) room_temp[o].switch_state = stoprun;  //turnoff floor switches when pump is off

//room_temp[o].switch_state = 1;

      tmpval += String("    ")+String(o)+": Switching: "+String(digitalRead(room_temp[o].idpinout)?"OFF":"ON")+" -> "+String((room_temp[o].switch_state)?"OFF":"ON")+" pump: "+String(pump?"OFF":"ON") + "\r";
      digitalWrite(room_temp[o].idpinout, room_temp[o].switch_state); //chyba raczej do 8 bo tyle czujnikow w pokoju
    }
  }
  log_message((char*)tmpval.c_str());
  GetSpecificSensorData();

//  if (pump == stop) digitalWrite(pompa_pin, stop_digital); else digitalWrite(pompa_pin, start_digital);

}

void GetSpecificSensorData () {
  min_temp = room_temp[0].tempread, max_temp = room_temp[0].tempset;
  String tmpval="\0";
  for (int o = 0; o < maxsensors; o++) {
    if (room_temp[o].tempread == InitTemp or room_temp[o].tempread == DS18B20nodata or room_temp[o].tempread == DS18B20nodata2 or room_temp[o].tempread == roomtemplo) room_temp[o].tempread = InitTemp;
    if (o<8) {
      if (min_temp>room_temp[o].tempread and room_temp[o].tempread!=InitTemp and room_temp[o].tempread != roomtemplo) min_temp = room_temp[o].tempread;
      if (max_temp<room_temp[o].tempset and room_temp[o].tempread!=InitTemp and room_temp[o].tempread != roomtemphi) max_temp = room_temp[o].tempset;
      if (room_temp[o].switch_state == startrun) min_max_state = startrun;
    }
    if (String(room_temp[o].nameSensor) == (String(kondygnacja)+sepkondname+String(tempcutoff)).substring(0,namelength) and room_temp[o].tempread != InitTemp) {
      if (modestate == MODEHEAT or modestate == MODEOFF) pumpOffVal = room_temp[o].tempset;
      if (modestate == MODECOOL and min_max_state == startrun) {pumpOffVal = max_temp - histereza; pump = startrun;}


      if (room_temp[o].tempread > pumpOffVal and modestate == MODEHEAT and min_max_state == startrun) pump = startrun;  //raczej na wejsciu z 4d dol
      if (room_temp[o].tempread < (pumpOffVal + histereza) and modestate == MODEHEAT) pump = stoprun;   //raczej na wejsciu z 4d dol
      //if (room_temp[o].tempread > pumpOffVal and modestate == MODECOOL) pump = stop;  //raczej na wejsciu z 4d dol
      //if (room_temp[o].tempread < (pumpOffVal + histereza) and modestate == MODECOOL) pump = start;  //raczej na wejsciu z 4d dol

      if (min_max_state = stoprun or modestate == MODEOFF) {pump=stoprun; start_pump=0;}  //turn off pump if all switches are disabled

      if (start_pump == 0 and pump == startrun) {start_pump=millis(); pump = stoprun;}
      if ((millis()-start_pump) > (start_pump_delay*1.2) and pump == stoprun) start_pump = 0;
      if ((millis()-start_pump) > start_pump_delay and min_max_state == startrun) pump = startrun;

      sprintf(log_chars, "------------------------------ %i: SetPUMP: %s, to pump: %s ----> modestate: %s", o, room_temp[o].switch_state?"OFF":"ON", pump?"OFF":"ON", modestate==MODEHEAT?"heating":modestate==MODECOOL?"cooling":String(modestate));
      log_message(log_chars);

      room_temp[o].switch_state = pump;
      digitalWrite(room_temp[o].idpinout, pump);
    }
    if (String(room_temp[o].nameSensor) == (String(kondygnacja)+sepkondname+String(tempwename)).substring(0,namelength) and room_temp[o].tempread != InitTemp) {
      tempwe = room_temp[o].tempread;
      room_temp[o].switch_state = stoprun;
    }

      //tempwe = 34;
      //room_temp[o].tempset = pumpofftemp;
      //if ((tempwe > min) and (tempwe > pumpOffVal)) pump = start; else pump = stop; //"Wejscie podloga -gora"
      //room_temp[o].switch_state = pump;
  }
    sprintf(log_chars, " Pump: %s, tempwe: %f, MIN: %f, PUMPoffTemp: %f", pump?"OFF":"ON", tempwe, min_temp, pumpOffVal);
    log_message(log_chars);
}