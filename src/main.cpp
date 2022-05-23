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
#ifdef enableDHT
DHT dht(DHTPIN, DHTTYPE);
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


//NEWS temperature
  if (topicStr == NEWS_GET_TOPIC)
  {
    String ident = String(millis())+": NEWS temp ";
    #ifdef debug
    Serial.print(ident);
    #endif
    #ifdef enableWebSerial
    WebSerial.print(ident);
    #endif
    if (PayloadtoValidFloatCheck(payloadStr))
    {
      temp_NEWS = PayloadtoValidFloat(payloadStr,true);     //true to get output to serial and webserial
      lastNEWSSet = millis();
      temp_NEWS_count = 0;
//      receivedmqttdata = true;    //makes every second run mqtt send and influx
    }
  } else
//CO Pump Status
  if (topicStr == COPUMP_GET_TOPIC)
  {
    String ident = String(millis())+": Status CO Pomp ";
    #ifdef debug
    Serial.print(ident);
    #endif
    #ifdef enableWebSerial
    WebSerial.print(ident);
    #endif
    receivedmqttdata = true;
    if (PayloadStatus(payloadStr, true)) CO_PumpWorking = true;
    else if (PayloadStatus(payloadStr, false)) CO_PumpWorking = false;
    else
    {
      receivedmqttdata = false;
      #ifdef debug
      Serial.println("Unknown: "+String(payloadStr));
      #endif
      #ifdef enableWebSerial
      WebSerial.println("Unknown: "+String(payloadStr));
      #endif
    }
    if (receivedmqttdata) {
      #ifdef debug
      Serial.println(CO_PumpWorking ? "Active" : "Disabled" );
      #endif
      #ifdef enableWebSerial
      WebSerial.println(CO_PumpWorking ? "Active" : "Disabled" );
      #endif
    }
  } else
//BOILER OPENTHERM CO Status with flame
  if (topicStr == BOILER_FLAME_STATUS_TOPIC)
  {
    String ident = String(millis())+": Status Boiler flame and co pump ";
    #ifdef enableWebSerial
    WebSerial.print(ident);
    #endif
    receivedmqttdata = true;
    if (PayloadStatus(getJsonVal(payloadStr,BOILER_FLAME_STATUS_ATTRIBUTE),true) && PayloadStatus(getJsonVal(payloadStr,BOILER_COPUMP_STATUS_ATTRIBUTE),true)) CO_BoilerPumpWorking = true;
    else if (PayloadStatus(getJsonVal(payloadStr,BOILER_FLAME_STATUS_ATTRIBUTE),false) && PayloadStatus(getJsonVal(payloadStr,BOILER_COPUMP_STATUS_ATTRIBUTE),false)) CO_BoilerPumpWorking = false;
    else
    {
      receivedmqttdata = false;
      #ifdef debug
      Serial.println("Unknown: "+String(payloadStr));
      #endif
      #ifdef enableWebSerial
      WebSerial.println("Unknown: "+String(payloadStr));
      #endif
    }
    if (receivedmqttdata) {
      #ifdef debug
      Serial.println(CO_PumpWorking ? "Active Heating" : "Disabled Heating" );
      #endif
      #ifdef enableWebSerial
      WebSerial.println(CO_BoilerPumpWorking ? "Active Heating" : "Disabled Heating" );
      #endif
      }
  } else
  //TEMP_CUTOFF_SET_TOPIC Status with flame
  if (topicStr == TEMP_CUTOFF_SET_TOPIC)
  {
  }
//sensors for setpoint temperature
  for (int x=0;x<maxsensors;x++)
  {
  if ((topicStr == ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getIdentyfikator(x) + SET_LAST) and (room_temp[x].idpinout>0))
    {
      if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
      {
        #ifdef debug
        Serial.println(String(millis())+": ROOM"+String(getIdentyfikator(x))+" "));
        #endif
        #ifdef enableWebSerial
        WebSerial.println(String(millis())+": ROOM"+String(getIdentyfikator(x))+" ");
        #endif
        room_temp[x].tempset = PayloadtoValidFloat(payloadStr, true, roomtemplo, roomtemphi);
        receivedmqttdata = true;
      }
    }
  }

}



void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected() and mqtt_offline_retrycount < mqtt_offline_retries)
  {
    #ifdef debug
    Serial.print(F("Attempting MQTT connection..."));
    #endif
    #ifdef enableWebSerial
    WebSerial.print(String(millis())+": "+F("Attempting MQTT connection..."));
    #endif
    const char *clientId = "MARM-test";

    if (client.connect(clientId, mqtt_user, mqtt_password))
    {
      #ifdef debug
      Serial.println(F("ok"));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(F("ok"));
      #endif
      mqtt_offline_retrycount = 0;

      for (int x=0;x<maxsensors;x++){
        //String identyfikator=getIdentyfikator(x);
        if (room_temp[x].idpinout!=0) client.subscribe((ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getIdentyfikator(x) + SET_LAST).c_str());
      }
      client.subscribe(NEWS_GET_TOPIC.c_str());
      client.subscribe(COPUMP_GET_TOPIC.c_str());
      client.subscribe(BOILER_FLAME_STATUS_TOPIC.c_str());

      client.subscribe(TEMP_CUTOFF_SET_TOPIC.c_str());  //tego nie ma w obsludze

    } else {
      #ifdef debug
      Serial.print(F(" failed, rc="));
      #endif
      #ifdef enableWebSerial
      WebSerial.print(F(" failed, rc="));
      #endif
      #ifdef debug
      Serial.print(client.state());
      #endif
      #ifdef enableWebSerial
      WebSerial.print(client.state());
      #endif
      #ifdef debug
      Serial.println(F(" try again in 5 seconds"));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(F(" try again in 5 seconds"));
      #endif
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
  sensors.setWaitForConversion(true); //
  sensors.requestTemperatures();        //Send the command to get temperatures
  sensors.setWaitForConversion(false); // switch to async mode
  uint8_t  addr[8];
  float temp1w;
  int count = sensors.getDS18Count();
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
      Serial.println((String(millis())+": "+j)+":  Collected ROM=: " + addrstr + "  Temp.: "+String(temp1w));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(String(millis())+": "+String(j)+":  Collected ROM=: " + addrstr + "  Temp.: "+String(temp1w));
      #endif
    }

  #ifdef debug
  Serial.println("Unassigned Sensors: "+UnassignedTempSensor);
  #endif
  #ifdef enableWebSerial
  WebSerial.println(String(millis())+": "+"Unassigned Sensors: "+UnassignedTempSensor);
  #endif

  #ifdef enableDHT
  float t = dht.readTemperature();
  if (isnan(t)) t=InitTemp;
  float h = dht.readHumidity();
  if (isnan(h)) h=0;
  if (isnan(h) or isnan(t)) {
    #ifdef debug
    Serial.println("Failed to read from DHT sensor!");
    #endif
    #ifdef enableWebSerial
      WebSerial.println("Failed to read from DHT sensor!");
    #endif
    }
  #endif
  humiditycor = h;
  tempcor = t; //to check ds18b20 also ;)
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
  #ifdef enableWebSerial
  WebSerial.println(String(millis())+": "+("Writing to InfluxDB: "));
//  WebSerial.println(InfluxClient.pointToLineProtocol(InfluxSensor));
  #endif
  // Write point
  if (!InfluxClient.writePoint(InfluxSensor))
  {
    #ifdef debug
    Serial.print(String(millis())+": "+("InfluxDB write failed: "));
    Serial.println(InfluxClient.getLastErrorMessage());
    #endif
    #ifdef enableWebSerial
    WebSerial.print(String(millis())+": "+("InfluxDB write failed: "));
    WebSerial.println(InfluxClient.getLastErrorMessage());
    #endif
  }
#endif
}








void setup()
{
  Serial.begin(115200);
  pinMode(choosepin, INPUT_PULLUP);    // sets the digital pin 13 as output
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
  //get Configuration floor 1 or 2 -if set is 2

  if (digitalRead(choosepin)==0) {
    zawor1addr  = String(F(dzawor1addr2));
    zawor2addr  = String(F(dzawor2addr2));
    zawor3addr  = String(F( dzawor3addr2));
    zawor4addr  = String(F(dzawor4addr2));
    zawor5addr  = String(F(dzawor5addr2));
    zawor6addr  = String(F(dzawor6addr2));
    zawor7addr  = String(F(dzawor7addr2));
    zawor8addr  = String(F(dzawor8addr2));
    zawor9addr  = String(F(dzawor9addr2));
    zawor10addr = String(F(dzawor10addr2));
    zawor11addr = String(F(dzawor11addr2));
    zawor12addr = String(F(dzawor12addr2));
    zawor13addr = String(F(dzawor13addr2));
    kondygnacja = "2";
  } else {
    zawor1addr  = String(F(dzawor1addr1));
    zawor2addr  = String(F(dzawor2addr1));
    zawor3addr  = String(F(dzawor3addr1));
    zawor4addr  = String(F(dzawor4addr1));
    zawor5addr  = String(F(dzawor5addr1));
    zawor6addr  = String(F(dzawor6addr1));
    zawor7addr  = String(F(dzawor7addr1));
    zawor8addr  = String(F(dzawor8addr1));
    zawor9addr  = String(F(dzawor9addr1));
    zawor10addr = String(F(dzawor10addr1));
    zawor11addr = String(F(dzawor11addr1));
    zawor12addr = String(F(dzawor12addr1));
    zawor13addr = String(F(dzawor13addr1));
    kondygnacja = "1";
  }
  me_lokalizacja += kondygnacja;//+"_mqqt_MARM";
  mqttident += kondygnacja + "_";
  mqttdeviceid.replace("me_lokalizacja",me_lokalizacja);
  OT += kondygnacja+"_";

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
  sensors.setWaitForConversion(true); // switch to async mode
  sensors.requestTemperatures();
  sensors.setWaitForConversion(false); // switch to async mode
  ts = millis();
  lastTempSet = -extTempTimeout_ms;




  // Serial.print(String(sensors.getTempCByIndex(0)));
  // Serial.println("ºC");




  started = millis();
  #ifdef enableWebSerial
  WebSerial.begin(&webserver);
  WebSerial.msgCallback(recvMsg);
  #endif
  WebServers();

  #ifdef ENABLE_INFLUX
  //InfluxDB
  InfluxClient.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
  // Alternatively, set insecure connection to skip server certificate validation
  InfluxClient.setInsecure();
  // Add tags
  InfluxSensor.addTag("device", me_lokalizacja);
    // Check server connection
    #ifdef enableWebSerial
    if (InfluxClient.validateConnection()) {
      WebSerial.print(String(millis())+": "+"Connected to InfluxDB: ");
      WebSerial.println(InfluxClient.getServerUrl());
    } else {
      WebSerial.print("InfluxDB connection failed: ");
      WebSerial.println(InfluxClient.getLastErrorMessage());
    }
    #endif
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
      #ifdef debug
      Serial.println(String(millis())+": "+F("MQTT connection problem -now reset retry counter and try again..."));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(String(millis())+": "+F("MQTT connection problem -now reset retry counter and try again..."));
      #endif
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
      #ifdef debug
      Serial.println(String(millis())+": "+F("MQTT connection problem -try to connect again..."));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(String(millis())+": "+F("MQTT connection problem -try to connect again..."));
      #endif
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

  if (((millis() - lastUpdateTempPump) > statusUpdateInterval_ms) or lastUpdateTempPump==0 or (((millis() < statusUpdateInterval_ms) or CO_PumpWorking or CO_BoilerPumpWorking) and (int(millis()/1000)%statusUpdateShortenInterval_s==0)) )   //after start for first defauylt 600s every 60s refresh
  {
    #ifdef debug
    Serial.println("now: "+String(millis())+" temps+pumps "+"lastUpdate: "+String(lastUpdateTempPump)+" statusUpdateInterval_ms: "+String(statusUpdateInterval_ms));
    #endif
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": temps+pumps "+"lastUpdate: "+String(lastUpdateTempPump)+" statusUpdateInterval_ms: "+String(statusUpdateInterval_ms));
    #endif
    lastUpdateTempPump = millis();
    ReadTemperatures();
    Pump_Activate_Control();
//    opentherm_update_data(lastUpdatemqtt); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }

  if (((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms) or (receivedmqttdata == true) or lastUpdatemqtt==0)   //recived data ronbi co 800ms -wylacze ten sttus dla odebrania news
  {
    #ifdef debug
    Serial.println(String(millis())+" mqtt+influ "+"lastUpdatemqtt: "+String(lastUpdatemqtt)+" receivedmqttdata: "+String(receivedmqttdata)+" mqttUpdateInterval_ms: "+String(mqttUpdateInterval_ms));
    Serial.println(String(lastUpdatemqtt)+": Update MQTT and InfluxDB data: ");
    #endif
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+" mqtt+influ "+"lastUpdatemqtt: "+String(lastUpdatemqtt)+" receivedmqttdata: "+String(receivedmqttdata)+" mqttUpdateInterval_ms: "+String(mqttUpdateInterval_ms));
    WebSerial.println(String(lastUpdatemqtt)+": Update MQTT and InfluxDB data: ");
    #endif
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
    #ifdef debug
    Serial.println("now: "+String(millis())+" "+"lastNEWSSet: "+String(lastNEWSSet)+" "+"temp_NEWS_interval_reduction_time_ms: "+String(temp_NEWS_interval_reduction_time_ms));
    #endif
    #ifdef enableWebSerial
    WebSerial.println("now: "+String(millis())+" "+"lastNEWSSet: "+String(lastNEWSSet)+" "+"temp_NEWS_interval_reduction_time_ms: "+String(temp_NEWS_interval_reduction_time_ms));
    #endif
    lastNEWSSet = millis();
    temp_NEWS_count++;
    if (temp_NEWS > cutOffTemp)
    {
      temp_NEWS = temp_NEWS - temp_NEWS * 0.05;
      #ifdef debug
      Serial.println(String(millis())+": "+F("Lowering by 5% temp_NEWS (no communication) -after 10times execute every 30minutes lowered temp NEWS"));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(String(millis())+": "+F("Lowering by 5% temp_NEWS (no communication) -after 10times execute every 30minutes lowered temp NEWS"));
      #endif
    }
    else
    {
      temp_NEWS = cutOffTemp;
    }
    if (temp_NEWS_count > 10)
    {
      CO_PumpWorking = false; // assume that we loose mqtt connection to other system where is co pump controlled -so after 10 times lowered NEWS temp by 5% we also disable CO_Pump_Working to allow heat by this heater -default it counts 5hours no communication
      #ifdef debug
      Serial.println(F("Force disable CO_PumpWorking flag -after 10times execute every 30minutes lowered temp NEWS"));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(F("Force disable CO_PumpWorking flag -after 10times execute every 30minutes lowered temp NEWS"));
      #endif
      temp_NEWS_count = 0;
    }
    // dobre miejsce do try get data by http api
    #ifdef debug
    Serial.println(F("Korekta temperatury NEWS z braku połaczenia -pomniejszona o 5%"));
    #endif
    #ifdef enableWebSerial
    WebSerial.println(F("Korekta temperatury NEWS z braku połaczenia -pomniejszona o 5%"));
    #endif
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
    #ifdef debug
    Serial.println(String(millis())+": "+F("Saving config to EEPROM memory..."));
    #endif
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": "+F("Saving config to EEPROM memory..."));
    #endif
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
  #ifdef debug
  Serial.println(String(millis())+": Pump loop...");
  #endif
  #ifdef enableWebSerial
  WebSerial.println(String(millis())+": Pump loop...");
  #endif
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
      #ifdef enableWebSerial
      WebSerial.println(String(o)+": Switching: "+String(room_temp[o].switch_state)+" pump: "+String(pump));
      #endif
      digitalWrite(lampPin, !digitalRead(lampPin));
    }
  }
  digitalWrite(lampPin, false);
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
      #ifdef debug
      Serial.println("------------------------------"+String(o)+": SetPUMP: "+String(room_temp[o].switch_state)+" to pump: "+String(pump));
      #endif
      #ifdef enableWebSerial
      WebSerial.println("------------------------------"+String(o)+": SetPUMP: "+String(room_temp[o].switch_state)+" to pump: "+String(pump));
      #endif
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
#ifdef enableWebSerial
      WebSerial.println(String(o)+": Pump: "+String(pump)+" tempwe: "+String(tempwe)+" MIN: "+String(min)+" PUMPoffTemp: "+String(pumpOffVal));
#endif
  }
}