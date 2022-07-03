
//#include <Arduino.h>

//#include "main.h"

const unsigned long mqttUpdateInterval_ms = 1 * 60 * 1000,      //send data to mqtt and influxdb
                    LOOP_WAITTIME = 2*1000;    //for loop


unsigned long uptime = 0,
              lastloopRunTime = 0,        //for loop
              lastUpdatemqtt = 0;
bool starting = false,
     sendlogtomqtt = false,       //Send or not Logging to MQTT Topic
     receivedmqttdata = false;
char log_chars[256];      //for logging buffer to log_message function
int mqttReconnects = -1;





//***********************************************************************************************************************************************************************************************
void log_message(char* string, u_int specialforce)  //         log_message((char *)"WiFi lost, but softAP station connecting, so stop trying to connect to configured ssid...");
{
  #include "configmqtttopics.h"
  String send_string = String(millis()) + F(": ") + String(string);
  #ifdef debuglm
    Serial.println(send_string);
  #endif
  #ifdef enableWebSerial
    if (send_string.length() > 100) send_string[100] = '\0';
    if (starting == false) {WebSerial.println(send_string);}
  #endif
//   if (webSocket.connectedClients() > 0) {
//     webSocket.broadcastTXT(string, strlen(string));
//   }
  if (mqttclient.connected() and (sendlogtomqtt or (specialforce == 1)))
  {
    if (send_string.length() > 100) send_string[100] = '\0';
    if (!mqttclient.publish(LOG_TOPIC.c_str(), send_string.c_str())) {
      Serial.print(millis());
      Serial.print(F(": "));
      Serial.println(F("MQTT publish log message failed!"));
      mqttclient.disconnect();
    }
  }
}

//***********************************************************************************************************************************************************************************************
String uptimedana(unsigned long started_local = 0) {
  String wynik = " ";
  unsigned long  partia = millis() - started_local;
  if (partia<1000) return "< 1 "+String(t_sek)+" ";
  #ifdef debug2
    Serial.print(F("Uptimedana: "));
  #endif
  if (partia >= 24 * 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (24 * 60 * 60 * 1000);
    partia -= podsuma * 24 * 60 * 60 * 1000;
    wynik += (String)podsuma + " "+String(t_day)+" ";

  }
  if (partia >= 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 60 * 1000);
    partia -= podsuma * 60 * 60 * 1000;
    wynik += (String)podsuma + " "+String(t_hour)+" ";
  }
  if (partia >= 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 1000);
    partia -= podsuma * 60 * 1000;
    wynik += (String)podsuma + " "+String(t_min)+" ";
    //Serial.println(podsuma);
  }
  if (partia >= 1 * 1000 ) {
    unsigned long  podsuma = partia / 1000;
    partia -= podsuma * 1000;
    wynik += (String)podsuma + " "+String(t_sek)+" ";
    //Serial.println(podsuma);
  }
  #ifdef debug2
    Serial.println(wynik);
  #endif
  //wynik += (String)partia + "/1000";  //pomijam to wartosci <1sek
  return wynik;
}

//***********************************************************************************************************************************************************************************************
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
    if (json.indexOf("{",1) !=-1)
    {
      json.replace("{","\"jsonskip\",");
      json.replace("}","");
    }
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
    sprintf(log_chars, "Json %s, No mqttident contain searched value of %s", json.c_str(), tofind.c_str());
    log_message(log_chars);  //3150232: Json "mqtt reconnects":53, No mqttident contain searched value of room

  } else
  {
    sprintf(log_chars, "Inproper Json format or null: %s, to find: %s", json.c_str(), tofind.c_str());
    log_message(log_chars);
  }
  return "\0";
}

//***********************************************************************************************************************************************************************************************
bool isValidNumber(String str)
{
  bool valid = true;
  for (byte i = 0; i < str.length(); i++)
  {
    char ch = str.charAt(i);
    valid &= isDigit(ch) ||
             ch == '+' || ch == '-' || ch == ',' || ch == '.' ||
             ch == '\r' || ch == '\r';
  }
  return valid;
}

//***********************************************************************************************************************************************************************************************
String convertPayloadToStr(byte *payload, u_int length)
{
  char s[length + 1];
  s[length] = 0;
  for (u_int i = 0; i < length; ++i)
    s[i] = payload[i];
  String tempRequestStr(s);
  return tempRequestStr;
}

//***********************************************************************************************************************************************************************************************
int dBmToQuality(int dBm) {
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

//***********************************************************************************************************************************************************************************************
int getWifiQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  return dBmToQuality(WiFi.RSSI());
}

//***********************************************************************************************************************************************************************************************
int getFreeMemory() {
  //store total memory at boot time
  static uint32_t total_memory = 0;
  if ( 0 == total_memory ) total_memory = ESP.getFreeHeap();

  uint32_t free_memory   = ESP.getFreeHeap();
  return (100 * free_memory / total_memory ) ; // as a %
}

//***********************************************************************************************************************************************************************************************
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

//***********************************************************************************************************************************************************************************************
bool PayloadtoValidFloatCheck(String payloadStr)
{
  if (PayloadtoValidFloat(payloadStr)==InitTemp) return false; else return true;
}

//***********************************************************************************************************************************************************************************************
float PayloadtoValidFloat(String payloadStr, bool withtemps_minmax, float mintemp, float maxtemp)  //bool withtemps_minmax=false, float mintemp=InitTemp,float
{
  payloadStr.trim();
  payloadStr.replace(",", ".");
  float valuefromStr = payloadStr.toFloat();
  if (isnan(valuefromStr) || !isValidNumber(payloadStr))
  {
    sprintf(log_chars, "Value is not a valid number, ignoring... payload: %s", payloadStr.c_str());
    log_message(log_chars);
    return InitTemp;
  } else
  {
    if (!withtemps_minmax)
    {
      // sprintf(log_chars, "Value is valid number without minmax: %s", String(valuefromStr,2).c_str());
      // log_message(log_chars);
      // return valuefromStr;
    } else {
      if (valuefromStr>maxtemp and maxtemp!=InitTemp) valuefromStr = maxtemp;
      if (valuefromStr<mintemp and mintemp!=InitTemp) valuefromStr = mintemp;
      sprintf(log_chars, "Value is valid number: %s", String(valuefromStr,2).c_str());
      log_message(log_chars);
    }
    return valuefromStr;
    //if here is sprintf and return -i have dpouble reading because first i check is ok and second get value
  }
}

//***********************************************************************************************************************************************************************************************
void restart()
{
  delay(1500);
//  WiFi.forceSleepBegin();
  webserver.end();
  WiFi.disconnect();
//      wifi.disconnect();
  delay(5000);
//  WiFi.forceSleepBegin(); wdt_reset();
ESP.restart(); while (1)ESP.restart();
//wdt_reset();
ESP.restart();
}

//***********************************************************************************************************************************************************************************************
String getIdentyfikator(int x)
{
  return "_"+String(x+1);
}

//***********************************************************************************************************************************************************************************************
void doubleResetDetect() {
  #ifdef doubleResDet
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset())
  {
    Serial.println(F("DRD"));
    CONFIGURATION.version[0] = 'R';
    CONFIGURATION.version[1] = 'E';
    CONFIGURATION.version[2] = 'S';
    CONFIGURATION.version[3] = 'E';
    CONFIGURATION.version[4] = 'T';
    SaveConfig();
    WiFi.persistent(true);
    WiFi.disconnect();
    WiFi.persistent(false);
    delay(4000);
    restart();

  }
  #endif
}

//***********************************************************************************************************************************************************************************************
#ifdef enableArduinoOTA
void setupOTA() {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(me_lokalizacja.c_str());

  // Set authentication
  //ArduinoOTA.setPassword("admin");

  ArduinoOTA.onStart([]() {
    log_message((char*)F("Starting update by OTA"),1);
  });
  ArduinoOTA.onEnd([]() {
    log_message((char*)F("Finished update by OTA. Restart in 2sec..."),1);
    delay(2000);
    restart();

  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if ((int)((progress/total)*100) % 5 == 0)
    {
      sprintf(log_chars,"Update in progress (Total %s/%s bytes).  %s%%",String(progress).c_str(), String(total).c_str(), String(float((float)progress/(float)total)*100,0).c_str());
      log_message(log_chars);
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    sprintf(log_chars,"There is error upgradeing by OTA: %s",String(error).c_str());
    log_message(log_chars);
  });
  ArduinoOTA.begin();
}
#endif
//***********************************************************************************************************************************************************************************************
//Ex.) char cVal[10];  float fVal=((22.0 /7.0)*256)-46.85;
// dtoa(fVal,cVal,4); Serial.println (String(cVal));

  //arguments...
  // float-double value, char array to fill, precision (4 is .xxxx)
  //and... it rounds last digit
char* dtoa(double dN, char *cMJA, int iP) {
  char *ret = cMJA; long lP=1; byte bW=iP;
  while (bW>0) { lP=lP*10;  bW--;  }
  long lL = long(dN); double dD=(dN-double(lL))* double(lP);
  if (dN>=0) { dD=(dD + 0.5);  } else { dD=(dD-0.5); }
  long lR=abs(long(dD));  lL=abs(lL);
  if (lR==lP) { lL=lL+1;  lR=0;  }
  if ((dN<0) & ((lR+lL)>0)) { *cMJA++ = '-';  }
  ltoa(lL, cMJA, 10);
  if (iP>0) { while (*cMJA != '\0') { cMJA++; } *cMJA++ = '.'; lP=10;
  while (iP>1) { if (lR< lP) { *cMJA='0'; cMJA++; } lP=lP*10;  iP--; }
  ltoa(lR, cMJA, 10); }  return ret; }

//***********************************************************************************************************************************************************************************************

#ifdef enableMESHNETWORK //painlessmesh  TaskScheduler  ESPAsyncTCP
#ifndef MESH_PREFIX
#define MESH_PREFIX     "Meshname"
#endif
#ifndef MESH_PASSWORD
#define MESH_PASSWORD   "meshpassword"
#endif
#ifndef MESH_PORT
#define MESH_PORT       5555 // default port
#endif

#include "painlessMesh.h"

void MeshWiFi_sendMessage();

Scheduler MeshWiFi_userScheduler;
painlessMesh  MeshWiFi;
Task MeshWiFi_taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &MeshWiFi_sendMessage );

void MeshWiFi_sendMessage()
{
  log_message((char*)F("Start Sending Mesh msg...."));
  // Serializing in JSON Format
  DynamicJsonDocument doc(1024);
//  float h = dht.readHumidity();
//  float t = dht.readTemperature();
//  doc["TEMP"] = t;
//  doc["HUM"] = h;
  doc["IP"] = WiFi.localIP();
  String msg ;
  serializeJson(doc, msg);

  MeshWiFi.sendBroadcast( msg );
  sprintf(log_chars,"Mesh Message Send: %s", msg.c_str());
  log_message(log_chars);
  MeshWiFi_taskSendMessage.setInterval((TASK_SECOND * 1, TASK_SECOND * 10));
}
// Needed for painless library
void MeshWiFi_receivedCallback( uint32_t from, String &msg ) {
  //Whenever there will be a message in the network, receivedCallback() comes in the action. The receivedCallback() function prints the message sender (from) and the content of the message using msg.c_str(). As data is serialized in sending function, it must be de-serialized while receiving.
  sprintf(log_chars,"Mesh Message Received: %s", msg.c_str());
  log_message(log_chars);
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    sprintf(log_chars,"deserialize Json failed:: %s", error.c_str());
    log_message(log_chars);
  }
//  LState = doc["Button"];
//  digitalWrite(LED, LState);
}
void MeshWiFi_newConnectionCallback(uint32_t nodeId) {
  //the newConnectionCallback() runs whenever a new node joins the network. This function prints the chip ID of the new node.
  sprintf(log_chars,"Mesh --> startHere: New Connection, nodeId = %u\n", nodeId);
  log_message(log_chars);
}
void MeshWiFi_changedConnectionCallback() {
  //The changedConnectionCallback() runs whenever a connection changes on the network that is when a node joins or leaves the network.
  sprintf(log_chars,"Mesh Changed connections %s", "\0");
  log_message(log_chars);
}
void MeshWiFi_nodeTimeAdjustedCallback(int32_t offset) {
  //The nodeTimeAdjustedCallback() runs when the network adjusts the time, so that all nodes are synchronized.
  sprintf(log_chars,"Mesh Adjusted time: %u. Offset = %d\n", MeshWiFi.getNodeTime(), offset);
  log_message(log_chars);
}

//***********************************************************************************************************************************************************************************************
void MeshWiFi_Setup()
{
  MeshWiFi.setDebugMsgTypes( ERROR | STARTUP );
  //Here, debug function is used to get any error occurred while making a connection with the nodes.
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on except GENERAL
  MeshWiFi.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA);
  MeshWiFi.onReceive(&MeshWiFi_receivedCallback);
  //When a new node make connection this callback is called.
  MeshWiFi.onNewConnection(&MeshWiFi_newConnectionCallback);
  //When there is a change in mesh topology this callback is called.
  MeshWiFi.onChangedConnections(&MeshWiFi_changedConnectionCallback);
  //Enable the taskSendMessage to start sending the messages to the mesh.
  MeshWiFi.onNodeTimeAdjusted(&MeshWiFi_nodeTimeAdjustedCallback);
  //In this, scheduler executes the task and send the appropriate message to nodes.
  MeshWiFi.stationManual(SSID_Name, SSID_PAssword);
  MeshWiFi.setHostname(me_lokalizacja.c_str());
  MeshWiFi_userScheduler.addTask( MeshWiFi_taskSendMessage );
  MeshWiFi_taskSendMessage.enable();
}
#endif

//***********************************************************************************************************************************************************************************************
void MainCommonSetup()
{
  Serial.println(("Connecting to " + String(ssid)));
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
  WiFi.enableSTA(true);
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.persistent(true);

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
    Serial.println(F("ok"));
  }
  Serial.println(WiFi.getHostname());
  Serial.println(WiFi.localIP());

#ifdef enableWebSerial
  WebSerial.begin(&webserver);
  WebSerial.msgCallback(recvMsg);
#endif
  starting = false;
  setupMqtt();
  starthttpserver();

#ifdef ENABLE_INFLUX
  // InfluxDB
  InfluxClient.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
  // Alternatively, set insecure connection to skip server certificate validation
  InfluxClient.setInsecure();
  // Add tags
  InfluxSensor.addTag("device", me_lokalizacja);
  // Check server connection
  if (InfluxClient.validateConnection())
  {
    sprintf(log_chars, "Connected to InfluxDB: %s", String(InfluxClient.getServerUrl()).c_str());
    log_message(log_chars,0);
  }
  else
  {
    sprintf(log_chars, "InfluxDB connection failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
    log_message(log_chars,0);
  }
#endif

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.localIP());
  #ifdef enableArduinoOTA
  setupOTA();
  #endif
  MDNS.begin(me_lokalizacja.c_str());
  MDNS.addService("http", "tcp", 80);
  uptime = millis();
  #ifdef enableMESHNETWORK
  MeshWiFi_Setup();
  #endif
}
//***********************************************************************************************************************************************************************************************
void MainCommonLoop()
{
  #include "configmqtttopics.h"
  #ifdef doubleResDet
  drd->loop();
  #endif
  check_wifi();
  #ifdef enableArduinoOTA
  // Handle OTA first.
  ArduinoOTA.handle();
  #endif
  #ifdef enableMESHNETWORK
  MeshWiFi.update();
  #endif

  mqttclient.loop();

  if ((unsigned long)(millis() - lastloopRunTime) > (LOOP_WAITTIME))
  {
    lastloopRunTime = millis();
    // check mqtt
    if ((WiFi.isConnected()) && (!mqttclient.connected()))
    {
      log_message((char *)"Lost MQTT connection! Trying Reconnect");
      mqtt_reconnect();
    }
    // log stats
    //    #include "configmqtttopics.h"
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
//    message += F("%");
    log_message((char *)message.c_str());

    String stats = F("{\"uptime\":");
    stats += String(millis());
    stats += F(",\"version\":");
    stats += me_version;
    stats += F(",\"voltage\":");
    stats += 0;
    stats += F(",\"free memory\":");
    stats += getFreeMemory();
    stats += F(",\"ESP_cyclecount\":");
    stats += ESP.getCycleCount();
    stats += F(",\"wifi\":");
    stats += getWifiQuality();
    stats += F(",\"mqtt_reconnects\":");
    stats += mqttReconnects;
    stats += F("}");
    mqttclient.publish(STATS_TOPIC.c_str(), stats.c_str(), mqtt_Retain);
    // get new data
    //  if (!heishamonSettings.listenonly) send_panasonic_query();

    // Make sure the LWT is set to Online, even if the broker have marked it dead.
    mqttclient.publish(WILL_TOPIC.c_str(), "Online");
    if (WiFi.isConnected())
    {
      //      MDNS.announce();
    }
  }

  if (((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms) or lastUpdatemqtt == 0 or receivedmqttdata == true) // recived data ronbi co 800ms -wylacze ten sttus dla odebrania news
  {
    sprintf(log_chars, "mqtt+influxDB lastUpdatemqtt: %s receivedmqttdata: %s mqttUpdateInterval_ms: %s", String(lastUpdatemqtt).c_str(), String(receivedmqttdata).c_str(), String(mqttUpdateInterval_ms).c_str());
    log_message(log_chars,0);
    receivedmqttdata = false;
    lastUpdatemqtt = millis();
#ifdef ENABLE_INFLUX
    updateInfluxDB(); // i have on same server mqtt and influx so when mqtt is down influx probably also ;(  This   if (InfluxClient.isConnected())  doesn't work forme 202205
#endif
    updateMQTTData();
  }

  if ((millis() % 600) < 100) digitalWrite(lampPin, !digitalRead(lampPin));
}

//***********************************************************************************************************************************************************************************************
void setupMqtt()
{
  mqttclient.setBufferSize(2048);
  mqttclient.setSocketTimeout(10);
  mqttclient.setKeepAlive(5); // fast timeout, any slower will block the main loop too long
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(mqtt_callback);
  mqtt_reconnect();
}

//***********************************************************************************************************************************************************************************************


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
    if ((ssid[0] != '\0') && (WiFi.status() != WL_DISCONNECTED) && (WiFi.scanComplete() != -1) ){ // && (WiFi.softAPgetStationNum() > 0))  {
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
      if ((WiFi.status() == WL_DISCONNECTED) ) {//} && (WiFi.softAPgetStationNum() == 0 )) {
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
//      experimental::ESP8266WiFiGratuitous::stationKeepAliveSetIntervalMs(5000); //necessary for some users with bad wifi routers
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
        SaveConfig(); //save to config file
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

//***********************************************************************************************************************************************************************************************
