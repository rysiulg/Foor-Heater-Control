
//     https://maximeborges.github.io/esp-stacktrace-decoder/
//Nice Info about ESP: https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html

#define ATOMIC_FS_UPDATE   //For uploading compressed filesystems, the application must be built with ATOMIC_FS_UPDATE defined because, otherwise, eboot will not be involved in writing the filesystem.
#ifndef ESP32xx
#define CONFIG_FREERTOS_UNICORE 1
#define CONFIG_ASYNC_TCP_RUNNING_CORE 1
#endif
#ifndef SSE_MAX_QUEUED_MESSAGES
#define SSE_MAX_QUEUED_MESSAGES 32
#endif
#include <Arduino.h>
#include "config.h"
#include "localconfig.h"
//#if defined enableMQTT || defined enableMQTTAsync
#include "configmqtttopics.h"
//#endif
#include "declarations_for_websync.h"


#define wwwport 80

#ifdef enableWebSocket
#define websocketendpoint "/ws"
#endif

#define SerialSpeed 115200
#ifdef enableArduinoOTA
#define OTA_Port 8266
#endif



#ifdef ESP32
  #include <WiFi.h>
  #ifdef enableWebUpdate
    #include <Update.h>
  #endif //enableWebUpdate
  #include <AsyncTCP.h>
//  #include <AsyncUDP.h>
  #include <ESPmDNS.h>
//  #include <AsyncDNSServer.h>
  #include "esp_task_wdt.h"
#else //not ESP32
  #ifdef enableWebUpdate
    #include <Updater.h>
  #endif //enableWebUpdate
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266mDNS.h>
//  #include <ESPAsyncUDP.h>
 // #include <ESPAsyncDNSServer.h>
#endif

#include <ESPAsyncWebServer.h>  //also must be <ESPAsyncTCP.h> <ESP8266WiFi.h>  ottowinter/ESPAsyncWebServer-esphome @ ^2.1.0)
#include <Ticker.h>             //for events for wifi reconnect and asyncMQTT

#ifdef enableMQTT
 #include <PubSubClient.h>
#endif //enableMQTT
#ifdef enableMQTTAsync
 #include <AsyncMqttClient.h>   // ottowinter/AsyncMqttClient-esphome @ ^0.8.6
#endif //enableMQTTAsync

#ifdef enableArduinoOTA
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
#endif //enableArduinoOTA

#ifdef enableWebSerial
  #include <WebSerial.h>
#endif //enableWebSerial

#ifdef ENABLE_INFLUX
  #ifdef ESP32
   // #include <HTTPClient.h>
    #include <InfluxDb.h>
  #else
    //#include <ESP8266HTTPClient.h>
    #include <InfluxDb.h>
  #endif
#endif

#ifdef ESP32
#include <FS.h>
#define CONFIG_VFS_SUPPORT_DIR 1
#define CONFIG_LITTLEFS_SPIFFS_COMPAT 1
#define USE_LittleFS
#include <LITTLEFS.h>
#define SPIFFS LITTLEFS
//#define SPIFFS LittleFS
#else
//#include <FS.h>                 //for spiffs files
#include "LittleFS.h" // LittleFS is declared
#define SPIFFS LittleFS       //4kB less after conversion but filesystem is higher allocation min 4kB fo LF and 256B for SPIFFS
#endif


#ifdef doubleResDet           // khoih-prog/ESP_DoubleResetDetector @ ^1.3.1)
  #include <ESP_DoubleResetDetector.h>
#endif

#include <DNSServer.h>



AsyncWebServer webserver(wwwport);
#ifndef ESP32
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
#endif
Ticker wifiReconnectTimer;

DNSServer dnsServer;

#ifdef enableWebSocket
  AsyncWebSocket WebSocket(websocketendpoint);
#endif //enableWebSocket
#ifdef doubleResDet
  #define DRD_TIMEOUT 0.1
  // address to the block in the RTC user memory
  // change it if it collides with another usageb
  // of the address block
  #define DRD_ADDRESS 0x00
  DoubleResetDetector* drd;
#endif //doubleResDet

#ifdef enableMQTT
  WiFiClient espClient;
  PubSubClient mqttclient(espClient);
#endif //enableMQTT

#ifndef decimalPlaces
  #define decimalPlaces 1   //how much decimal places to show on www
#endif
#define DS18B20nodata 85
#define DS18B20nodata1 127

//*********************************************************************************************************************************************
//         SPECIFIC functions outside from this file to define it to compile without errors
//*********************************************************************************************************************************************

#define ValuesToWSWPinJSON 1
#define ValuesToWSWPforWebProcessor 2
#define wwwstatus_on "on"
#define wwwstatus_off "off"
#define logStandard 0               //standard log send
#define logCommandResponse 1        //send response to websocket after received command
#define remoteHelperMenu 0          //werbserial -remote commands helper: get menu help
#define remoteHelperMenuCommand 1   //werbserial -remote commands helper: get menucommand line
#define remoteHelperCommand 2       //werbserial -remote commands helper: run command
#ifndef ASS_Num
#define ASS_Num 50
#endif
typedef struct
{
  String Value;
  String ValueHumid;
} all_sensors_struct;
all_sensors_struct ASS[ASS_Num];        //for get values from local variables to send to websocket
String get_PlaceholderName(u_int i);    //replace specific placeholder -return String for specific ASS value  zestaw nazw z js i css i html dopasowania do liczb do łatwiejszego dopasowania
void updateDatatoWWW_received(u_int i); //update local var from received val from websocket -Received data from web and here are converted values to variables of local
void updateDatatoWWW();                 //update ASS.Value for websocket -update data in ASS.Value by local variables value before resend to web
String local_specific_web_processor_vars(String var);  //eg. Update config page
String addusage_local_values_save(int EpromPosition);
void addusage_local_values_load(String dane, int EpromPosition);
String LocalVarsRemoteCommands(String command, size_t gethelp); //get local data for remotecommands like help menu, items in menu and commands torun
//mqttinfluxsend
#ifdef ENABLE_INFLUX
void updateInfluxDB();      //send data to Influxdb
#endif
#if defined enableMQTT || defined enableMQTTAsync
void updateMQTTData();       //send data to MQTT                                //Send to mqtt data
void mqttCallbackAsString(String &topicStrFromMQTT, String &payloadStrFromMQTT);  //additional parsing local node values from MQTT return
void mqttReconnect_subscribe_list();                                            //Subscribe list for MQTT topics
#endif
//*********************************************************************************************************************************************



const unsigned long mqttUpdateInterval_ms = 1 * 60 * 1000,      //send data to mqtt and influxdb
                    LOOP_WAITTIME = (2.1*1000),    //for loop
                    WIFIRETRYTIMER = 25 * 1000;

unsigned long
              lastloopRunTime = 0,        //for loop
              lastUpdatemqtt = 0,
              lastWifiRetryTimer = 0;

bool starting = true,
     receivedmqttdata = false,
     receivedwebsocketdata = false;
#ifdef enableDebug2Serial
bool debugSerial = true;
#endif
#if defined enableMQTT || defined enableMQTTAsync
bool sendlogtomqtt = false;       //Send or not Logging to MQTT Topic
#endif //#if defined enableMQTT || defined enableMQTTAsync
#ifdef ENABLE_INFLUX
bool InfluxStatus = false;  //MQTTT Send Status
#endif
#ifndef ecoModeMaxTemp
  #define ecoModeMaxTemp 39       //economical low temperature heating known as condensation heat
#endif
#ifndef ecoModeDisabledMaxTemp
  #define ecoModeDisabledMaxTemp 60
#endif

float opcohi = ecoModeDisabledMaxTemp,             // upper max heat boiler to CO
      ecohi = ecoModeMaxTemp;

#ifndef InitTempst
#define InitTempst 255
#endif
const float ophi = 65,               // upper max heat water
            opcohistatic = opcohi,
            oplo = 29,               // lower min heat water
            opcolo = oplo,           // lower min heat boiler to CO
            cutoffhi = 20,           // upper max cut-off temp above is heating CO disabled -range +-20
            cutofflo = -cutoffhi,    // lower min cut-off temp above is heating CO disabled
            roomtemphi = 29,         // upper max to set of room temperature
            roomtemplo = 15,         // lower min to set of room temperature
		InitTemp = InitTempst;

#ifndef maxLogSize
#define maxLogSize 1023
#endif
#ifndef maxLenMQTTTopic
#define maxLenMQTTTopic 165
#endif
#ifndef maxLenMQTTPayload
#define maxLenMQTTPayload maxLogSize
#endif

#if defined enableWebSocketlog && defined enableWebSocket || enableWebSerial
bool WebSocketlog = true;
#endif


char log_chars[maxLogSize];      //for logging buffer to log_message function
int temp_NEWS_count = 0;
unsigned int CRTrunNumber = 0; // count of restarts

#if defined enableMQTT || defined enableMQTTAsync
int mqttReconnects = -1,
    mqtt_offline_retrycount = 0,
    mqtt_offline_retries = 2, // retries to mqttconnect before timewait
    publishhomeassistantconfig = 4,                               // licznik wykonan petli -strat od 0
    publishhomeassistantconfigdivider = 5;                        // publishhomeassistantconfig % publishhomeassistantconfigdivider -publikacja gdy reszta z dzielenia =0 czyli co te ilosc wykonan petli opoznionej update jest wysylany config
#endif  //defined enableMQTT || defined enableMQTTAsync
size_t content_len;

#if defined enableMQTTAsync
  String receivedtmpString ="\0";  //for combine input payloads
  size_t receivedtmpIdx = 0;
#endif //enableMQTTAsync
String ESPlastResetReason = "\0"; //last reset reason
//other_Modules
const String configfile = "/config.cfg";

char ssid[sensitive_size] = SSID_Name;
char pass[sensitive_size] = SSID_PAssword;

#if defined enableMQTT || defined enableMQTTAsync
// Your MQTT broker address and credentials
char mqtt_server[sensitive_size * 2] = MQTT_servername;
char mqtt_user[sensitive_size] = MQTT_username;
char mqtt_password[sensitive_size] = MQTT_Password_data;
int mqtt_port = MQTT_port_No;
const int mqtt_Retain = 1;
#endif //defined enableMQTT || defined enableMQTTAsync
#ifdef ENABLE_INFLUX
char influx_server[sensitive_size * 2] = INFLUXDB_URL;
char influx_user[sensitive_size] = INFLUXDB_USER;
char influx_password[sensitive_size] = INFLUXDB_PASSWORD;
char influx_database[sensitive_size] = INFLUXDB_DB_NAME;
char influx_measurments[sensitive_size] = InfluxMeasurments;
  InfluxDBClient InfluxClient(influx_server, influx_database);
  Point InfluxSensor(influx_measurments);
#endif //ENABLE_INFLUX

#ifdef enableMQTTAsync
  AsyncMqttClient mqttclient;
  Ticker mqttReconnectTimer;
#endif //enableMQTTAsync


#ifndef ASS_uptimedana
#define ASS_uptimedana 0
#endif
#ifndef ASS_Statusy
#define ASS_Statusy 1
#endif
#ifndef ASS_MemStats
#define ASS_MemStats 2
#endif
#ifndef ASS_uptimedanaStr
#define ASS_uptimedanaStr "uptimedana"
#endif
#ifndef ASS_StatusyStr
#define ASS_StatusyStr "Statusy"
#endif
#ifndef ASS_MemStatsStr
#define ASS_MemStatsStr "MemStats"
#endif
const String noTempStr = "--.-";


TaskHandle_t Task1;



//common_functions.h
#if defined enableMQTT || defined enableMQTTAsync
void mqtt_callback(char *topic, byte *payload, unsigned int length);
#endif
String get_lastResetReason();
bool check_isValidTemp(float temptmp);
void log_message(char* string, u_int specialforce);
String uptimedana(unsigned long started_local, bool startFromZero, bool forlogall);
String getJsonVal(String &jsoninput, String tofind);
String getJsonVal_old(String json, String tofind);
bool isValidNumber(String &str);
String convertPayloadToStr(byte *payload, u_int length);
int dBmToQuality(int dBm);
int getWifiQuality();
int getFreeMemory();
bool PayloadStatus(String &payloadStr, bool state);
bool PayloadtoValidFloatCheck(String &payloadStr);
float PayloadtoValidFloat(String &payloadStr, bool withtemps_minmax, float mintemp, float maxtemp);
void restart(String messagewhy);
String getIdentyfikator(int x);
#ifdef doubleResDet
void doubleResetDetect();
#endif
#ifdef enableArduinoOTA
void Setup_OTA();
#endif
char* dtoa(double dN, char *cMJA, int iP);
#ifdef enableMESHNETWORK
void Setup_MeshWiFi();
void MeshWiFi_sendMessage();
void MeshWiFi_receivedCallback( uint32_t from, String &msg );
void MeshWiFi_newConnectionCallback(uint32_t nodeId)
void Setup_MeshWiFi();
#endif
void connectToWifi();
void Setup_WiFi();
#ifdef ENABLE_INFLUX
void Setup_Influx();
#endif
void Setup_DNS();
String getFSDir (bool html, String path);    //default path="/"
void Setup_FileSystem();
#ifdef enableWebSocket
void Setup_WebSocket();
void Setup_WebSocket();
void notifyClients(String &Message);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void Event_WebSocket(AsyncWebSocket *webserver, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
#endif
String web_processor(const String var);
bool webhandleFileRead(AsyncWebServerRequest *request, String path);
String getValuesToWebSocket_andWebProcessor(u_int function, String processorAsk);
void handleWebSocketMessage_sensors(String message);
//vhar edit.html
void Setup_WebServer();
void webhandleNotFound(AsyncWebServerRequest *request);
void webhandleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
char* toCharPointer (String ptrS);
void updateDatatoWWW_common();
void SaveAssValue(uint8_t ASSV, String source_str, bool humid);
void MainCommonSetup();
void MainCommonLoop();
#if defined enableMQTT || defined enableMQTTAsync
void Setup_Mqtt();
#endif
#ifdef enableMQTT
void mqttReconnect();
#endif
#ifndef ESP32
void onWifiConnect(const WiFiEventStationModeGotIP& event);                 //event async to connect wifi after disconnected
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event);       //event async to connect wifi after disconnected
#else
void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info);
void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);
#endif
#ifdef enableMQTTAsync
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttMessage(char* topicAMCP, char* payloadAMCP, AsyncMqttClientMessageProperties properties, size_t lenAMCP, size_t index, size_t total);
void onMqttPublish(uint16_t packetId);
#endif  //enableMQTTAsync
String formatBytes(size_t bytes);
String webgetContentType(String filename);
void check_wifi();
#ifdef enableWebUpdate
void SetupWebUpdate();
void handleUpdate(AsyncWebServerRequest *request);
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void printProgress(size_t prg, size_t sz);
#endif
String PrintHex8(const uint8_t *data, char separator, uint8_t length);

#ifdef enableMQTTAsync
void connectToMqtt();
#endif
#if defined enableMQTT || defined enableMQTTAsync
void HADiscovery(String sensorswitchValTopic, String appendname, String nameval, String discoverytopic, String DeviceClass, String unitClass, String stateClass, String HAicon, const String payloadvalue_startend_val, const String payloadON, const String payloadOFF);
uint16_t publishMQTT(String &mqttTopicxx, String &mqttPayloadxx, int mqtt_Retainv, u_int qos);
#endif //enableMQTTAsync
String build_JSON_Payload(String MQTTname, String MQTTvalue, bool notAddSeparator, String Payload_StartEnd);
bool loadConfig();
bool SaveConfig();
void RemoteCommandReceived(uint8_t *data, size_t len);  //commands from remote -from serial input and websocket input and from webserial







//***********************************************************************************************************************************************************************************************
const char version[10+1] =
{
   // YY year
   //__DATE__[7], __DATE__[8],
   __DATE__[9], __DATE__[10],

   // First month letter, Oct Nov Dec = '1' otherwise '0'
   (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0',

   // Second month letter
   (__DATE__[0] == 'J') ? ( (__DATE__[1] == 'a') ? '1' :       // Jan, Jun or Jul
                            ((__DATE__[2] == 'n') ? '6' : '7') ) :
   (__DATE__[0] == 'F') ? '2' :                                // Feb
   (__DATE__[0] == 'M') ? (__DATE__[2] == 'r') ? '3' : '5' :   // Mar or May
   (__DATE__[0] == 'A') ? (__DATE__[1] == 'p') ? '4' : '8' :   // Apr or Aug
   (__DATE__[0] == 'S') ? '9' :                                // Sep
   (__DATE__[0] == 'O') ? '0' :                                // Oct
   (__DATE__[0] == 'N') ? '1' :                                // Nov
   (__DATE__[0] == 'D') ? '2' :                                // Dec
   0,

   // First day letter, replace space with digit
   __DATE__[4]==' ' ? '0' : __DATE__[4],

   // Second day letter
   __DATE__[5],
   __TIME__[0],__TIME__[1],
   __TIME__[3],__TIME__[4],
  '\0'
};

uint8_t mac[6] = {(uint8_t)strtol(WiFi.macAddress().substring(0,2).c_str(),0,16), (uint8_t)strtol(WiFi.macAddress().substring(3,5).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(6,8).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(9,11).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(12,14).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(15,17).c_str(),0,16)};
#ifdef ESP32
String verbose_print_reset_reason(int reason)
{
  switch ( reason)
  {
    case 0  : return ("0: normal startup by power on?");break;
    case 1  : return ("1:POWERON_RESET Vbat power on reset");break;
    case 2  : return ("2: ");break;
    case 3  : return ("3:SW_RESET Software reset digital core");break;
    case 4  : return ("4:OWDT_RESET Legacy watch dog reset digital core");break;
    case 5  : return ("5:DEEPSLEEP_RESET Deep Sleep reset digital core");break;
    case 6  : return ("6:SDIO_RESET Reset by SLC module, reset digital core");break;
    case 7  : return ("7:TG0WDT_SYS_RESET Timer Group0 Watch dog reset digital core");break;
    case 8  : return ("8:TG1WDT_SYS_RESET Timer Group1 Watch dog reset digital core");break;
    case 9  : return ("9:RTCWDT_SYS_RESET RTC Watch dog Reset digital core");break;
    case 10 : return ("10:INTRUSION_RESET Instrusion tested to reset CPU");break;
    case 11 : return ("11:TGWDT_CPU_RESET Time Group reset CPU");break;
    case 12 : return ("12:SW_CPU_RESET Software reset CPU");break;
    case 13 : return ("13:RTCWDT_CPU_RESET RTC Watch dog Reset CPU");break;
    case 14 : return ("14:EXT_CPU_RESET for APP CPU, reseted by PRO CPU");break;
    case 15 : return ("15:RTCWDT_BROWN_OUT_RESET Reset when the vdd voltage is not stable");break;
    case 16 : return ("16:RTCWDT_RTC_RESET RTC Watch dog reset digital core and rtc module");break;
    default : return ("NO_MEAN");
  }
}
#endif
//***********************************************************************************************************************************************************************************************
#if defined enableMQTT || defined enableMQTTAsync
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  //#include "configmqtttopics.h"
  String topicStr(topic);
  String payloadStr = convertPayloadToStr(payload, length);
  mqttCallbackAsString(topicStr, payloadStr);
}
#endif
//***********************************************************************************************************************************************************************************************
String get_lastResetReason()
{
  #ifndef ESP32
  rst_info *resetInfo;
  String lrr = "\0";
  resetInfo = ESP.getResetInfoPtr();
  int resetNO = resetInfo->reason;
  switch (resetNO){
    case REASON_DEFAULT_RST: lrr = F("0: normal startup by power on"); break;
    case REASON_WDT_RST: lrr = F("1: hardware watch dog reset"); break;
    case REASON_EXCEPTION_RST: lrr = F("2: exception reset, GPIO status won’t change"); break;
    case REASON_SOFT_WDT_RST: lrr = F("3: software watch dog reset, GPIO status won’t change"); break;
    case REASON_SOFT_RESTART: lrr = F("4: software restart, system_restart , GPIO status won’t change"); break;
    case REASON_DEEP_SLEEP_AWAKE: lrr = F("5 wake up from deep-sleep"); break;
    case REASON_EXT_SYS_RST: lrr = F("6 external system reset"); break;
    default: lrr = String(resetNO) + ": unknown"; break;
  }
  return lrr;
  #else
    // #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
    // #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
    // #include "esp32/rom/rtc.h"
    // #elif CONFIG_IDF_TARGET_ESP32S2
    // #include "esp32s2/rom/rtc.h"
    // #elif CONFIG_IDF_TARGET_ESP32C3
    // #include "esp32c3/rom/rtc.h"
    // #elif CONFIG_IDF_TARGET_ESP32S3
    // #include "esp32s3/rom/rtc.h"
    // #else
    // #error Target CONFIG_IDF_TARGET is not supported
    // #endif
    // #else // ESP32 Before IDF 4.0
    // #include "rom/rtc.h"
    // #endif
    String lrr = F("Core0: ");
    lrr += verbose_print_reset_reason(esp_rom_get_reset_reason(0));
    lrr += F(",\nCore1: ");
    lrr += verbose_print_reset_reason(esp_rom_get_reset_reason(1));
    return lrr;
    #endif
}
//***********************************************************************************************************************************************************************************************
bool check_isValidTemp(float temptmp)
{
  if (temptmp!=InitTemp && temptmp!=-InitTemp && temptmp!=DS18B20nodata && temptmp!=-DS18B20nodata && temptmp!=DS18B20nodata1 && temptmp!=-DS18B20nodata1 && !isnan(temptmp)) return true; else return false;
}
//***********************************************************************************************************************************************************************************************
void log_message(char* string, u_int specialforce = logStandard)  //         log_message((char *)"WiFi lost, but softAP station connecting, so stop trying to connect to configured ssid...");
{
  // macros from DateTime.h
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)
//  #include "configmqtttopics.h"
sprintf(log_chars,"%s: %s", uptimedana(millis(), true, true).c_str(), string);
  String send_string = log_chars;
  send_string.trim();

  //Send to serial port COM
  #ifdef enableDebug2Serial
  //debugSerial = true;
  if (debugSerial == true) {
    Serial.println(log_chars);
  }
  #endif

//  if (send_string.length()> maxLogSize ) send_string[maxLogSize] = '\0';
//abandoned webserial

  #ifdef enableWebSerial
    if (starting == false and WebSocketlog) {WebSerial.println(log_chars);}
  #endif

  //char send_string_ch[maxLogSize];
  sprintf(log_chars, "{\"log\":\"%s\"}", send_string.c_str());
  if (sizeof(log_chars)>512) log_chars[511] = '\0';
  send_string = log_chars;

#if defined enableWebSocketlog && defined enableWebSocket
  if (!starting && (WebSocketlog || specialforce == logCommandResponse)) notifyClients(send_string);
  #endif

  #if defined enableMQTT || defined enableMQTTAsync
  if (sendlogtomqtt == true) { //|| specialforce == 2) {
    // send_string.replace("\"","'");
    // send_string.replace("\\","");
    if (mqttclient.connected() && !starting)
    {
      #if defined enableMQTTAsync
      uint16_t packetIdSub;
      packetIdSub = mqttclient.publish(String(LOG_TOPIC).c_str(), QOS, mqtt_Retain, send_string.c_str());
      if (packetIdSub == 0) packetIdSub = 0;
      #endif
      #if defined enableMQTT
      if (!mqttclient.publish(String(LOG_TOPIC).c_str(), send_string.c_str())) {
        mqttclient.disconnect();
        Serial.print(millis());
        Serial.print(F(": "));
        Serial.println(F("MQTT publish log message failed!"));
        mqttReconnect();    //not needed for Async ver
      }
      #endif
    }
  }
  #endif

}

//***********************************************************************************************************************************************************************************************
String uptimedana(unsigned long started_local = 0, bool startFromZero = false, bool forlogall = false) {
  String wynik = "\0";
  unsigned long  partia;
  if (startFromZero) partia = started_local; else partia = millis() - started_local;
  if (partia<1000) {
    if (!forlogall) {
      return "< 1 "+String(t_sek)+" ";
    } else {
      if (partia<10) {
        return "00d00h00m00s.00" + String(partia);
      }
      if (partia<100) {
        return "00d00h00m00s.0" + String(partia);
      } else {
        return "00d00h00m00s." + String(partia);
      }
    }
  }

  if (partia >= 24 * 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (24 * 60 * 60 * 1000);
    partia -= podsuma * 24 * 60 * 60 * 1000;
    if (!forlogall) {
      wynik += (String)podsuma; wynik += ""; wynik += String(t_day); wynik +=" ";
    } else {
      if (podsuma<10) { wynik += "0"; }
      wynik += (String)podsuma; wynik += "d";
    }
  }
  if (partia >= 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 60 * 1000);
    partia -= podsuma * 60 * 60 * 1000;
    if (!forlogall) {
      wynik += (String)podsuma; wynik += ""; wynik +=String(t_hour); wynik +=" ";
    } else {
      if (podsuma<10) { wynik += "0"; }
      wynik += (String)podsuma; wynik += ":";
    }
  } else {
    if (forlogall) wynik += "00:";
  }
  if (partia >= 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 1000);
    partia -= podsuma * 60 * 1000;
    if (!forlogall) {
      wynik += (String)podsuma; wynik += ""; wynik += String(t_min); wynik +=" ";
    } else {
      if (podsuma<10) { wynik += "0"; }
      wynik += (String)podsuma; wynik += ":";
    }
  } else {
    if (forlogall) wynik += "00:";
  }
  if (partia >= 1 * 1000 ) {
    unsigned long  podsuma = partia / 1000;
    partia -= podsuma * 1000;
    if (!forlogall) {
      wynik += (String)podsuma; wynik += ""; wynik += String(t_sek); wynik += " ";
    } else {
      if (podsuma<10) { wynik += "0"; }
      wynik += (String)podsuma; wynik += "";
    }
  } else {
    if (forlogall) wynik += "00";
  }
  if (forlogall) {
    if (partia<10) {
      wynik += ".00"; wynik += String(started_local % 1000);
    } else
    if (partia<100) {
      wynik += ".0"; wynik += String(started_local % 1000);
    } else {
      wynik += "."; wynik += String(started_local % 1000);
    }
  }
  //wynik += (String)partia + "/1000";  //pomijam to wartosci <1sek
  return wynik;
}

//***********************************************************************************************************************************************************************************************
String getJsonVal(String &jsoninput, String tofind)
{ //function to get value from json payload
  String json = jsoninput;
  json.trim();
  tofind.trim();
  #ifdef debugweb
  sprintf(log_chars,"json0: %s",json.c_str());
  //log_message(log_chars);
  #endif
  if (!json.isEmpty() and !tofind.isEmpty() and json.startsWith("{") and json.endsWith("}"))  //check is starts and ends as json data and nmqttident null
  {
    json=json.substring(1,json.length()-1);                             //cut start and end brackets json
    if (json.indexOf("{",1) !=-1)
    {
      json.replace("{","\"jsonskip\":\"0\",");      //was "{","\"jsonskip\",");
      json.replace("}","");
    }
    if (json.indexOf(tofind)>=0 )
    {
      //Found string and get value
      u_int valu_start_position = json.indexOf(":", json.indexOf(tofind)) + 1;
      u_int valu_end_position = json.indexOf(",", valu_start_position);

      String nvalue=json.substring(valu_start_position, valu_end_position); //get node value
      nvalue.replace("\"","");
      nvalue.trim();
      #ifdef debugweb
      sprintf(log_chars,"Found node: %s, return val: %s", String(tofind).c_str(), String(nvalue).c_str());
      log_message(log_chars);
      #endif
      return nvalue;
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
String getJsonVal_old(String json, String tofind)
{ //function to get value from json payload
  json.trim();
  tofind.trim();
  #ifdef debugweb
  sprintf(log_chars,"json0: %s",json.c_str());
  log_message(log_chars);
  #endif
  if (!json.isEmpty() and !tofind.isEmpty() and json.startsWith("{") and json.endsWith("}"))  //check is starts and ends as json data and nmqttident null
  {
    json=json.substring(1,json.length()-1);                             //cut start and end brackets json
    if (json.indexOf("{",1) !=-1)
    {
      json.replace("{","\"jsonskip\":\"0\",");      //was "{","\"jsonskip\",");
      json.replace("}","");
    }
    #ifdef debugweb
    sprintf(log_chars,"json1: %s",json.c_str());
    log_message(log_chars);
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
          sprintf(log_chars,"Return no data");
          log_message(log_chars);
        #endif
        break;
      }
      String node=part.substring(0,part.indexOf(":",1));    //get node name
      node.trim();
      String nvalue=part.substring(part.indexOf(":",1)+1); //get node value
      nvalue.trim();
      #ifdef debugweb
      sprintf(log_chars,"jsonx: %s, tee: %s, tofind: %s, part: %s, node: %s, nvalue: %s, indexof ,: %s",String(json).c_str(), String(tee).c_str(), String(tofind).c_str(), String(part).c_str(), String(node).c_str(), String(nvalue).c_str(), String(json.indexOf(",",1)).c_str());
      log_message(log_chars);
      #endif
      if (tofind.indexOf(node) >=0 )
      {
         nvalue.replace("\"","");
         nvalue.replace(",",".");   //to get valid decimal number
         #ifdef debugweb
         sprintf(log_chars,"Found node: %s, return val: %s", String(tofind).c_str(), String(nvalue).c_str());
         log_message(log_chars);
         #endif
        return nvalue;
        break;
      }
      tee++;
      if (tee>maxtee) {
        #ifdef debugweb
        sprintf(log_chars,"tee exit: %s", String(tee).c_str());
        log_message(log_chars);
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
bool isValidNumber(String &str)
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
{ //converts byte[] to string
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
bool PayloadStatus(String &payloadStr, bool state)
{
  payloadStr.toUpperCase();
  payloadStr.replace("\"","");
  payloadStr.trim();
  #ifdef debug
  sprintf(log_chars,"PayloadStatus. PayloadStr: %s", payloadStr.c_str());
  log_message(log_chars);
  #endif

  if (state and (payloadStr == "ON" or payloadStr == "TRUE" or payloadStr == "START" or payloadStr == "1"  or payloadStr == "ENABLE" or payloadStr == "HEAT")) return true;
  else
  if (!state and (payloadStr == "OFF" or payloadStr == "OF" or payloadStr == "FALSE" or payloadStr == "STOP" or payloadStr == "0" or payloadStr == "DISABLE")) return true;
  else return false;
}

//***********************************************************************************************************************************************************************************************
bool PayloadtoValidFloatCheck(String &payloadStr)
{
  if (isValidNumber(payloadStr)) return true; else return false;
}

//***********************************************************************************************************************************************************************************************
float PayloadtoValidFloat(String &payloadStr, bool withtemps_minmax, float mintemp = 0, float maxtemp = 0)  //bool withtemps_minmax=false, float mintemp=InitTemp,float
{
  payloadStr.trim();
  if (isValidNumber(payloadStr))  payloadStr.replace(",", ".");
  float valuefromStr = payloadStr.toFloat();
  if (isnan(valuefromStr) || !isValidNumber(payloadStr))
  {
    #ifdef debug
    sprintf(log_chars, "Value is not a valid number, ignoring... payload: %s", payloadStr.c_str());
    log_message(log_chars);
    #endif
    return InitTemp;
  } else
  {
    if (!withtemps_minmax)
    {
      // sprintf(log_chars, "Value is valid number without minmax: %s", String(valuefromStr,2).c_str());
      // log_message(log_chars);
      // return valuefromStr;
    } else {
      if (valuefromStr>maxtemp and !check_isValidTemp(maxtemp)) valuefromStr = maxtemp;
      if (valuefromStr<mintemp and !check_isValidTemp(mintemp)) valuefromStr = mintemp;
      #ifdef debug
      sprintf(log_chars, "Value is valid number: %s", String(valuefromStr,2).c_str());
      log_message(log_chars);
      #endif
    }
    if (!check_isValidTemp(valuefromStr)) valuefromStr = InitTemp;
    return valuefromStr;
    //if here is sprintf and return -i have dpouble reading because first i check is ok and second get value
  }
}

//***********************************************************************************************************************************************************************************************
void restart(String messagewhy)
{
  //inform to log
  messagewhy = "RESTART: " + messagewhy;
  log_message((char*)messagewhy.c_str());
  long nowre = millis();
  //wait 4000msec with yield to restart
  while (millis() - nowre < 4000) {
    yield();
  }
  #if defined enableMQTT || defined enableMQTTAsync
  #ifdef enableMQTTAsync
  mqttReconnectTimer.detach();
  #endif //enableMQTTAsync
  mqttclient.disconnect();
  #endif
//  WiFi.forceSleepBegin();
  webserver.end();
  WiFi.disconnect();
  nowre = millis();
  while (millis() - nowre < 500) {
    yield();
  }
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
#ifdef doubleResDet
void doubleResetDetect() {
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset()) {
    // DRDActiveStatus = true;
    log_message((char*)F("DRD ActiveStatus"));
    drd->stop();
    SPIFFS.begin();
    //SPIFFS.format();
    SaveConfig();
    WiFi.persistent(true);
    WiFi.disconnect();
    WiFi.persistent(false);
    #ifdef enableArduinoOTA
    Setup_WiFi();
    Setup_OTA();
    Setup_DNS();
    #ifdef enableWebUpdate
    SetupWebUpdate();
    webserver.begin();
    #endif
    unsigned long long timerek = 0;
    while (true) {
      #ifdef enableArduinoOTA
      ArduinoOTA.handle();
      if (millis()-timerek > 5000) { //reduce display time to 5sec
        timerek = millis();
        log_message((char*)F("Waiting for OTA update connection..."));
      }
      #endif
      #ifdef enableWebUpdate
      if (millis()-timerek > 5000) { //reduce display time to 5sec
        timerek = millis();
        log_message((char*)F("Waiting for OTA update connection..."));
      }
      #endif
      check_wifi();
      if (millis() > ( 5 * 60 * 1000)) {delay(500); restart("DRD Restart from loop");}
    }
    #endif
    delay(500);
    restart("doubleResetDetector");
  }
}
#endif

//***********************************************************************************************************************************************************************************************
#ifdef enableArduinoOTA
#ifndef OTA_Port
#define OTA_Port 8266
#endif
void Setup_OTA() {
  log_message((char*)"Setup Arduino OTA...");
  // Port defaults to 8266
  ArduinoOTA.setPort(OTA_Port);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(String(me_lokalizacja).c_str());

  // Set authentication
  //ArduinoOTA.setPassword("admin");

  ArduinoOTA.onStart([]() {
    #ifdef doubleResDet
    drd->stop();
    #endif
  #ifdef enableWebSocket
   // Disable client connections
   WebSocket.enable(false);
  #if defined enableMQTT || defined enableMQTTAsync
      #ifdef enableMQTTAsync
      mqttReconnectTimer.detach();
      #endif //enableMQTTAsync
    mqttclient.disconnect();
  #endif
   // Advertise connected clients what's going on
   String tmpStrt = F("OTA Update Started");
   notifyClients(tmpStrt);
   // Close them
   WebSocket.closeAll();
   webserver.end();
  #endif
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    sprintf(log_chars,"Starting update by OTA type: %s", type.c_str());
    log_message(log_chars);
  });
  ArduinoOTA.onEnd([]() {
    log_message((char*)F("Finished update by OTA. Restart in 2sec..."),1);
    restart(F("Arduino OTA from port "));

  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if ((int)((progress/total)*100) % 5 == 0)
    {
      sprintf(log_chars,"Update in progress (Total %s/%s bytes).  %s%%",String(progress).c_str(), String(total).c_str(), String(float((float)progress/(float)total)*100,0).c_str());
      log_message(log_chars);
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    String errorName = "\0";
    if (error == OTA_AUTH_ERROR) { errorName = F("Auth Failed") ;
    } else if (error == OTA_BEGIN_ERROR) { errorName = F("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) { errorName = F("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) { errorName = F("Receive Failed");
    } else if (error == OTA_END_ERROR) {errorName = F("End Failed");
    }
    webserver.begin();
    sprintf(log_chars,"There is error upgrading by OTA: %s (%s)",String(error).c_str(), String(errorName).c_str());
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
Scheduler MeshWiFi_userScheduler;
painlessMesh  MeshWiFi;
Task MeshWiFi_taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &MeshWiFi_sendMessage );
void Setup_MeshWiFi() {
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
void MeshWiFi_sendMessage() {
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
  sprintf(log_chars,"Mesh --> startHere: New Connection, nodeId = %u", nodeId);
  log_message(log_chars);
}
void MeshWiFi_changedConnectionCallback() {
  //The changedConnectionCallback() runs whenever a connection changes on the network that is when a node joins or leaves the network.
  sprintf(log_chars,"Mesh Changed connections %s", "\0");
  log_message(log_chars);
}
void MeshWiFi_nodeTimeAdjustedCallback(int32_t offset) {
  //The nodeTimeAdjustedCallback() runs when the network adjusts the time, so that all nodes are synchronized.
  sprintf(log_chars,"Mesh Adjusted time: %u. Offset = %d", MeshWiFi.getNodeTime(), offset);
  log_message(log_chars);
}

//***********************************************************************************************************************************************************************************************
#endif
//***********************************************************************************************************************************************************************************************
void connectToWifi() {
  log_message((char*)F("Connecting to Wi-Fi..."));
  if (pass[0] == '\0') {
    WiFi.begin(ssid);
    } else {
      WiFi.begin(ssid, pass);
    }
  WiFi.enableSTA(true);
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.persistent(true);

}
//***********************************************************************************************************************************************************************************************
void Setup_WiFi() {
  sprintf(log_chars,"SetupWiFi...Connecting to: %s", String(ssid).c_str());
  log_message(log_chars);
  WiFi.setAutoReconnect(false);
  WiFi.setAutoConnect(false);
  WiFi.disconnect(true);

  #ifndef ESP32
  WiFi.hostname(String(me_lokalizacja).c_str());      //works for esp8266
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  #else
//  setCpuFrequencyMhz(80);   //STANDARD 240mHz
  WiFi.disconnect(true);
    WiFi.onEvent(onWifiConnect, ARDUINO_EVENT_WIFI_STA_GOT_IP); //SYSTEM_EVENT_STA_CONNECTED);
    WiFi.onEvent(onWifiDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);  //SYSTEM_EVENT_STA_DISCONNECTED);
  WiFi.config(((u32_t)0x0UL),((u32_t)0x0UL),((u32_t)0x0UL));//IPADDR_NONE, INADDR_NONE, INADDR_NONE); //none gives 255.255.255.255 error in libraries
  WiFi.setHostname((me_lokalizacja).c_str());  //for esp32
  #endif
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(String(me_lokalizacja).c_str());

  connectToWifi();

  sprintf(log_chars,"SSID: %s  PASS: %s", String(ssid).c_str(), String(pass).c_str());
  log_message(log_chars);


  int deadCounter = 20;
  while (WiFi.status() != WL_CONNECTED && deadCounter-- > 0)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    sprintf(log_chars,"WiFi Failed to connect to : %s", String(ssid).c_str());
    log_message(log_chars);
    while (true); //this restart
  }
  else
  {
    sprintf(log_chars,"WiFi Connected OK ");
    log_message(log_chars);
  }
  sprintf(log_chars,"Hostname: %s, LocalIP: %s", String(WiFi.getHostname()).c_str(), WiFi.localIP().toString().c_str());
  log_message(log_chars);
}

#ifdef ENABLE_INFLUX
void Setup_Influx() {
  // InfluxDB
  InfluxClient.setConnectionParamsV1(influx_server, influx_database, influx_user, influx_password);
  // Alternatively, set insecure connection to skip server certificate validation
  InfluxClient.setInsecure();
  InfluxClient.setHTTPOptions(HTTPOptions().httpReadTimeout(3000));
  InfluxClient.setWriteOptions(WriteOptions().bufferSize(1024));
  InfluxClient.setWriteOptions(WriteOptions().retryInterval(5));
  InfluxClient.setWriteOptions(WriteOptions().maxRetryInterval(300));
  InfluxClient.setWriteOptions(WriteOptions().maxRetryAttempts(3));

  // Add tags
  InfluxSensor.addTag("device", me_lokalizacja);
  // Check server connection
  if (InfluxClient.validateConnection()) {
    sprintf(log_chars, "Setup: Connected to InfluxDB: %s", String(InfluxClient.getServerUrl()).c_str());
    log_message(log_chars,0);
  } else {
    sprintf(log_chars, "Setup: InfluxDB connection failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
    log_message(log_chars,0);
  }
}
#endif
//***********************************************************************************************************************************************************************************************
void Setup_DNS() {
  log_message((char*)F("Setup DNS..."));
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.localIP());
  MDNS.begin(String(me_lokalizacja).c_str());     // start the multicast domain name server
  MDNS.addService("http", "tcp", 80);
}
//***********************************************************************************************************************************************************************************************
String getFSDir (bool html = false, String path = "/")
{ //get direcory contents
  String directory = "Actual directory list:\n";
  if (html) {
    directory += F("<table class=\"fileTable\"><tr>");
    directory += F("<th>FS File:</th><th>size:</th></tr>");
  }
  #ifndef ESP32
  Dir dir = SPIFFS.openDir(path);
  while (dir.next()) {                      // List the file system contents
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
  #else
  File root = SPIFFS.open(path);
  File dir = root.openNextFile();
  while (dir) {
    String fileName = dir.name();
    size_t fileSize = dir.size();
  #endif

  if (html) {
      directory += F("<tr><td>");
      directory += F("<a href=\"/");
      directory += String(fileName);
      directory += F("\">");
      directory += String(fileName);
      directory += F("</a>");
      directory += F("</td><td>");
      directory += formatBytes((size_t)fileSize);
      directory += F("</td></tr>");
  } else
  {
    sprintf(log_chars, "  FS File: %s, size: %s\n", fileName.c_str(), formatBytes((size_t)fileSize).c_str());
    directory += String(log_chars);
  }
  #ifdef ESP32
   dir = root.openNextFile();
  #endif
  }
  if (html) directory += F("</table>");
  return directory.c_str();
}

//***********************************************************************************************************************************************************************************************
void Setup_FileSystem() {// Start the FSand list all contents
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  #ifndef debug
  log_message((char*)F("FS started."));
  #else
  log_message((char*)F("FS started. Contents:"));
    log_message((char*)getFSDir().c_str());
  #endif
}
//***********************************************************************************************************************************************************************************************
#ifdef enableWebSocket
//***********************************************************************************************************************************************************************************************
void Setup_WebSocket() { // Start a WebSocket server
  WebSocket.onEvent(Event_WebSocket);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  webserver.addHandler(&WebSocket);             // start the websocket server
  log_message((char*)F("WebSocket server started."));
}
void notifyClients(String &Message) {
  //   if (webSocket.connectedClients() > 0) {
//     webSocket.broadcastTXT(string, strlen(string));
//   }
  if (WebSocket.availableForWriteAll() && WebSocket.enabled() && WebSocket.count()>0) WebSocket.textAll(Message);
}
//***********************************************************************************************************************************************************************************************
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    message.trim();
    String placeholdertmp = message.substring(0,message.indexOf(":",1));
    String valuetmp = message.substring(message.indexOf(":",1)+1);
    placeholdertmp.trim();
    placeholdertmp.toLowerCase();
    valuetmp.trim();
    if (placeholdertmp.indexOf("remotecommand") >= 0) {
      u_int8_t *datatmp;
      datatmp = (u_int8_t *)valuetmp.c_str();
      RemoteCommandReceived(datatmp, (size_t)(valuetmp.length()));  //received command from web ;)
    } else
    if (strcmp((char*)data, "LED") == 0) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//        notifyClients(digitalRead(LED_BUILTIN) ? "on" : "off");       //notify
    } else
    {
      handleWebSocketMessage_sensors(message);
    }
  }
}
//***********************************************************************************************************************************************************************************************
void Event_WebSocket(AsyncWebSocket       *webserver,  //
             AsyncWebSocketClient *client,  //
             AwsEventType          type,    // the signature of this function is defined
             void                 *arg,     // by the `AwsEventHandler` interface
             uint8_t              *data,    //
             size_t                len) {   // uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  //WS_EVT_CONNECT when a client has logged in,
  // WS_EVT_DISCONNECT when a client has logged out,
  // WS_EVT_DATA when a data packet is received from the client.
  // WS_EVT_PONG in response to a ping request,
  // WS_EVT_ERROR when an error is received from the client,
  switch (type) {
    case WS_EVT_DISCONNECT:             // if the websocket is disconnected
      sprintf(log_chars, "[%s] Disconnected!", String(client->id()).c_str());
      log_message(log_chars);
      break;
    case WS_EVT_CONNECT: {              // if a new websocket connection is established
        IPAddress ip = client->remoteIP();
        sprintf(log_chars,"[%u] Connected from %d.%d.%d.%d", (u_int)client->id(), ip[0], ip[1], ip[2], ip[3]);
        log_message(log_chars);
        // rainbow = false;                  // Turn rainbow off when a new connection is established
      }
      break;
    case WS_EVT_DATA:                     // if new text data is received
      sprintf(log_chars, "[%s] get Text WS_EVT_DATA.", String(client->id()).c_str()); //String(*data).c_str());
      log_message(log_chars);
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
      log_message((char*)F("WS_EVT_PONG received"));
      break;
    case WS_EVT_ERROR:
      log_message((char*)F("WS_EVT_ERROR received"));
      break;
  }
}
//***********************************************************************************************************************************************************************************************
#endif  //enableWebSocket
//***********************************************************************************************************************************************************************************************
String web_processor(const String var) {
  // sprintf(log_chars,"WebProcessor variable: %s",var.c_str());
  // log_message(log_chars);
  if (var == "ME_TITLE") {String retStrtit = String(me_lokalizacja); retStrtit += F(" v."); retStrtit += String(version); return retStrtit.c_str();
  } else
  if (var == "DIR_LIST") {
    String retval = "FS started. Contents:";
    retval = getFSDir(true);
    return retval;
  } else
  if (var == "stopkawebsite") {
    String ptr="\0";
      ptr += F("MAC: <B>");
      ptr += PrintHex8(mac, ':', sizeof(mac) / sizeof(mac[0]));
      ptr += F("</B><br>");
      ptr += F("&copy; ");
      ptr += String(MFG)+"  "+version[4]+version[5]+"-"+version[2]+version[3]+"-20"+version[0]+version[1]+" "+version[6]+version[7]+":"+version[8]+version[9];
    return ptr;
  } else
  if (var == "linkiac") {
    String ptr="\0";
      ptr += "<p>";
      #ifdef enableWebUpdate
      ptr += F("<a href='/update' target=\"_blank\">");
      ptr += String(Update_web_link);
      ptr += F("</a>");
      #endif
      #ifdef enableWebSerial
      ptr += F("&nbsp;&nbsp;&nbsp;<a href='/webserial' target=\"_blank\">");
      ptr += String(Web_Serial);
      ptr += F("</a>");
      #endif
      ptr += F("&nbsp;&nbsp;&nbsp;<a href='/edit'>Edit FileSystem</a>");
      ptr += F("</p>");
      return ptr;
  } else
  if (var == "BuildOptions") {
    String ptr="\0";
      ptr += "<p><H2>BUILD OPTIONS:</H2>";
      #ifdef enableWebSocket
      ptr += F("Async WebSocket integration with endpoint: <IP_Addr>");
      ptr += String(websocketendpoint);
      ptr += F("<br>");
      #endif
      #ifdef enableMESHNETWORK
      ptr += F("PainlessMesh network integration. MeshName: ");
      ptr += String(MESH_PREFIX);
      ptr  += F("<br>");
      #endif
      #ifdef doubleResDet
      ptr += F("ESP double Reset Detector integration<br>");
      #endif
      #ifdef ENABLE_INFLUX
      sprintf(log_chars, "InfluxDB integration to: %s  DB: %s, Measurments: %s<br>", influx_server, influx_database, influx_measurments);
      ptr += log_chars;
      #endif
      #if defined enableMQTT || defined enableMQTTAsync
        #ifdef enableMQTTAsync
        ptr += F("Async mode ");
        #endif //enableMQTTAsync
      sprintf(log_chars, "MQTT integration to: %s:%u<br>", mqtt_server, mqtt_port);
      ptr += log_chars;
      #endif
      #ifdef enableWebUpdate
      ptr += F("WebUpdate OTA<br>");
      #endif
      #ifdef enableArduinoOTA
      sprintf(log_chars, "ArduinoOTA on :%u<br>", OTA_Port);
      ptr += log_chars;
      #endif
      #ifdef debug
      ptr += F("Build with debug flag<br>");
      #endif
      #ifdef debugweb
      ptr += F("Build with debugweb flag<br>");
      #endif
      #ifdef enableDebug2Serial
      sprintf(log_chars, "Serial logging and use simple commands is enabled after connecting Serial at %ubps<br>", SerialSpeed);
      ptr += log_chars;
      #endif
      #ifdef enableWebSocketlog
      ptr += F("Send logging to WebSocket and use simple commands as webSerial but native<br>");
      #endif
      #ifdef enableWebSerial
      ptr += F("WebSerial included on <a href='/webserial'>/webserial</a><br>");
      #endif
      ptr += F("</p>");
      return ptr;
  } else
  if (var == "opcolo") { return String(opcolo);
  } else
  if (var == "opcohi") { return String(opcohi);
  } else
  if (var == "oplo") { return String(oplo);
  } else
  if (var == "ophi") { return String(ophi);
  } else
  if (var == "cutofflo") { return String(cutofflo);
  } else
  if (var == "cutoffhi") { return String(cutoffhi);
  } else
  if (var == "roomtemplo") { return String(roomtemplo);
  } else
  if (var == "roomtemphi") { return String(roomtemphi);
   } else
  #ifdef enableDebug2Serial
  if (var == "debugSerial") { return String(debugSerial ? 1 : 0);
  } else
  #endif
  #if defined enableMQTT || defined enableMQTTAsync
  if (var == "sendlogtomqtt") { return String(sendlogtomqtt ? 1 : 0);
  } else
  #endif
  #ifdef enableWebSocketlog
  if (var == "WebSocketlog") { return String(WebSocketlog ? 1 : 0);
  } else
  #endif
  if (var == "NEWS_GET_TOPIC") { return String(NEWS_GET_TOPIC);
  } else
  if (var == "NEWStemp_json") { return String(NEWStemp_json);

  } else
  if (var == "SSID_Name") { return String(ssid);
  } else
  if (var == "SSID_PAssword") { return String(pass);
  } else
  #if defined enableMQTT || defined enableMQTTAsync
  if (var == "MQTT_servername") { return String(mqtt_server);
  } else
  if (var == "MQTT_port_No") { return String(mqtt_port);
  } else
  if (var == "MQTT_username") { return String(mqtt_user);
  } else
  if (var == "MQTT_Password_data") { return String(mqtt_password);
  } else
  #endif
  #ifdef ENABLE_INFLUX
  if (var == "INFLUXDB_URL") { return String(influx_server);
  } else
  if (var == "INFLUXDB_DB_NAME") { return String(influx_database);
  } else
  if (var == "INFLUXDB_USER") { return String(influx_user);
  } else
  if (var == "INFLUXDB_PASSWORD") { return String(influx_password);
  } else
  if (var == "influx_measurments") { return String(influx_measurments);
  } else
#endif
  if (var == "STATE") { return (digitalRead(LED_BUILTIN) ? "on" : "off");
  } else
  {
    String tmpstr = getValuesToWebSocket_andWebProcessor(ValuesToWSWPforWebProcessor, var);          //get specific data of sensors
    if (tmpstr.length()>0) return tmpstr;
    return local_specific_web_processor_vars(var);

  }
  return "LSWPV_NONE";
}
//***********************************************************************************************************************************************************************************************
bool webhandleFileRead(AsyncWebServerRequest *request, String path) {
  #ifdef debug
  sprintf(log_chars,"webhandleFileRead: %s", path.c_str());
  log_message(log_chars);
  #endif

  // if (!request->authenticate(www_username, www_password)) {
  //     Serial.print(F("NOT AUTHENTICATE!"));
  //     request->requestAuthentication();
  // }

  if(path.endsWith("/") and path.length()<2) path += F("index.html");           // If a folder is requested, send the index file
  String contentType = webgetContentType(path);                // Get the MIME type
  String pathWithGz = path + F(".gz");
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){     // If the file exists, either as a compressed archive, or normal
    bool gzipped = false;

    if(SPIFFS.exists(pathWithGz)) {                         // If there's a compressed version available
      path += F(".gz");                                     // Use the compressed version
      gzipped = true;
    }

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    //request->send(SPIFFS, "/index.html", "text/html; charset=utf-8",  false, web_processor);
    //AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType,  false, web_processor); //there was problem with css and js files i think and also will be problem with gz html ;(

    if (gzipped){
        response->addHeader("Content-Encoding", "gzip");
    }
    sprintf(log_chars,"webhandleFileRead: Real file path: (%s) %s",gzipped?"gzip":"normal", String(path).c_str());
    log_message(log_chars);

    request->send(response);

    return true;
  }
  sprintf(log_chars,"webhandleFileRead: File Not Found: %s",String(path).c_str());
  log_message(log_chars);
  return false;                                             // If the file doesn't exist, return false
}
//***********************************************************************************************************************************************************************************************
String getValuesToWebSocket_andWebProcessor(u_int function, String processorAsk = "\0")  //default processorAsk = "\0"
{
    //update data true to not send after update values zbyt czesto sie uruchamia -przy kazdej zmiennej zapytania
  String toreturn = "{";
  processorAsk.trim();
  for (u_int i = 0; i < sizeof(ASS)/sizeof(ASS[0]); i++)  /////////////////////////////korekta
  {
    String placeholdername = get_PlaceholderName(i);
    if (sizeof(ASS[i].Value)>0 )
    {
      if (i>0) toreturn += F(",");
      toreturn += F("\"");
      toreturn += placeholdername;
      toreturn += F("\":");
      toreturn += F("\"");
      toreturn += String(ASS[i].Value);
      toreturn += F("\"");
    }
    if (function == ValuesToWSWPforWebProcessor && (placeholdername).indexOf(processorAsk) >= 0 ) return String(ASS[i].Value);
  }
  if (function == ValuesToWSWPinJSON) return (toreturn + "}");
  return "\0";
}
//***********************************************************************************************************************************************************************************************
void handleWebSocketMessage_sensors(String message) {
  if (!message.isEmpty() and !message.isEmpty() and message.indexOf(":",1) != -1)
  {
    String placeholdertmp = message.substring(0,message.indexOf(":",1));
    String valuetmp = message.substring(message.indexOf(":",1)+1);
    placeholdertmp.trim();
    valuetmp.trim();
    sprintf(log_chars,"Received placeholdertmp: %s, valuetmp: %s", placeholdertmp.c_str(), valuetmp.c_str());
    log_message(log_chars);
      for (u_int i = 0; i < sizeof(ASS)/sizeof(ASS[0]); i++)
      {
        if (sizeof(ASS[i].Value)>0)
        {
          String placeholdername = get_PlaceholderName(i);
          //Serial.println("Debug: "+ASS[i].placeholder+", val: "+ASS[i].Value);
          if (placeholdertmp.indexOf(placeholdername) >= 0) {
            sprintf(log_chars,"Received Websocket %s", String(message).c_str());
            log_message(log_chars);
 //           strcpy(ASS[i].Value, valuetmp.c_str());
//            valuetmp.toCharArray(ASS[i].Value, valuetmp.length());
            ASS[i].Value = valuetmp;
  //          receivedwebsocketdata = true;
            #if defined(enableMQTT) || defined(ENABLE_INFLUX) || defined (enableMQTTAsync)
            receivedmqttdata = true;

            #endif
            updateDatatoWWW_received(i);
            String tmpStr = F("{");
            tmpStr += build_JSON_Payload(placeholdername, ASS[i].Value, true, "\"");
            tmpStr += F("{");
            notifyClients(tmpStr);
  //          notifyClients(getValuesToWebSocket_andWebProcessor(ValuesToWSWPinJSON));
          }
        }
      }

  }
}
//***********************************************************************************************************************************************************************************************
const char edit_html[] = ("<!DOCTYPE html> <html> <head> <title>%ME_TITLE% Uploader</title> <meta content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0' name='viewport'> </head> <body> <center> <header> <div class=\"topnav\"> <h1>%ME_TITLE%</h1> </div> <br> <h2>Uploader</h2> </header> <div> <p style=\"text-align: center\">Use this page to upload new files to the ESP.<br/>You can use compressed (deflated) files (files with a .gz extension) to save space and bandwidth.<br/>Existing files will be replaced.</p> <form method=\"post\" enctype=\"multipart/form-data\" style=\"margin: 0px auto 8px auto\" > <input type=\"file\" name=\"Choose file\" accept=\".gz,.html,.ico,.js,.css,.cfg\"> <input class=\"button\" type=\"submit\" value=\"Upload\" name=\"submit\"> </form> </div> <br><br> %DIR_LIST% </center> </body> </html>");
//File: edit.html.gz, Size: 499
#define edit_html_gz_len 499
const uint8_t edit_html_gz[] PROGMEM = {
0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x55, 0x53, 0x4D, 0x6F, 0xDB, 0x30,
0x0C, 0xFD, 0x2B, 0x5C, 0x80, 0x20, 0x2D, 0xE0, 0xD8, 0xC9, 0x4E, 0xC3, 0x90, 0xE4, 0xD2, 0xE6,
0x10, 0xA0, 0xC3, 0x8A, 0x35, 0x3D, 0x14, 0x08, 0x50, 0xD0, 0x16, 0x63, 0x6B, 0x90, 0x25, 0xC1,
0xA2, 0x9D, 0x64, 0xBF, 0x7E, 0x94, 0x3F, 0xD6, 0xEE, 0xA2, 0x0F, 0x92, 0x7A, 0x8F, 0x8F, 0xA4,
0x36, 0x5F, 0x1E, 0x7F, 0x3E, 0x1C, 0xDF, 0x9E, 0xF7, 0x50, 0x71, 0x6D, 0x76, 0xB0, 0x99, 0x36,
0x42, 0x25, 0x1B, 0x6B, 0x36, 0xB4, 0x9B, 0xFF, 0xD8, 0xBF, 0x1F, 0x0F, 0xC7, 0xA7, 0xFD, 0x1C,
0x5E, 0xBD, 0x71, 0xA8, 0xA8, 0xD9, 0x64, 0x83, 0x0B, 0x36, 0x35, 0x31, 0x42, 0xE1, 0x2C, 0x93,
0xE5, 0xED, 0xE2, 0xA2, 0x15, 0x57, 0x5B, 0x45, 0x9D, 0x2E, 0x68, 0xD9, 0x5F, 0x12, 0xD0, 0x56,
0xB3, 0x46, 0xB3, 0x0C, 0x05, 0x1A, 0xDA, 0xAE, 0xD3, 0x55, 0x02, 0x35, 0x5E, 0x75, 0xDD, 0xD6,
0x9F, 0x4D, 0x6D, 0xA0, 0xA6, 0xBF, 0x63, 0x2E, 0xA6, 0xD5, 0x02, 0x2C, 0xD6, 0xB4, 0x5D, 0x74,
0x9A, 0x2E, 0xDE, 0x35, 0xBC, 0x10, 0xAA, 0x6C, 0xCC, 0x2A, 0x77, 0xEA, 0x26, 0x5B, 0x21, 0x8C,
0xD4, 0x8C, 0xC9, 0xF6, 0x07, 0xA5, 0x3B, 0x28, 0x0C, 0x86, 0xB0, 0x3D, 0xCD, 0xD8, 0x79, 0x8B,
0xDD, 0x69, 0x16, 0xFD, 0xEB, 0x4F, 0x12, 0x04, 0x65, 0x1D, 0xB1, 0x24, 0x36, 0x42, 0xF5, 0xEF,
0xBF, 0xEE, 0x3E, 0x74, 0xC9, 0x65, 0x64, 0x9A, 0x20, 0x65, 0xF5, 0x10, 0xF8, 0x26, 0x69, 0x09,
0x2C, 0x5D, 0x79, 0x89, 0x46, 0x97, 0xF6, 0x3B, 0x0C, 0x09, 0x08, 0xC5, 0x6B, 0x20, 0xE0, 0x4A,
0x07, 0xF0, 0x58, 0xCA, 0xC9, 0x41, 0xDB, 0xC3, 0x81, 0xA5, 0x0B, 0x9C, 0xB5, 0xA1, 0x10, 0x6D,
0x5C, 0x11, 0xEC, 0x5F, 0x9E, 0x53, 0xE1, 0xCC, 0x76, 0x6F, 0xAE, 0x85, 0x02, 0x6D, 0x54, 0x2D,
0xC5, 0xAB, 0x7D, 0x43, 0x21, 0x90, 0x82, 0x3B, 0x45, 0x67, 0x83, 0x4C, 0xEA, 0x7E, 0x7C, 0x77,
0x37, 0x6C, 0x17, 0xCD, 0x15, 0x20, 0xA4, 0xE5, 0x1F, 0x10, 0x7E, 0xB2, 0x41, 0x3B, 0x7B, 0x1F,
0x41, 0x03, 0x76, 0x04, 0xC1, 0x63, 0x41, 0x80, 0x56, 0x41, 0x2E, 0x4B, 0x5F, 0xF5, 0x81, 0x65,
0x7F, 0xD5, 0x81, 0xB5, 0x2D, 0x61, 0x42, 0x31, 0x06, 0x72, 0x82, 0x86, 0xBC, 0x91, 0x17, 0x2A,
0xDD, 0x64, 0x5E, 0xC4, 0x9D, 0x5D, 0x53, 0x83, 0xB4, 0xB1, 0x72, 0x4A, 0x04, 0x7A, 0x17, 0xF8,
0x34, 0x03, 0xB2, 0x05, 0xDF, 0x7C, 0x54, 0x5C, 0xB7, 0x86, 0xB5, 0xC7, 0x86, 0xB3, 0x18, 0xB8,
0x54, 0xC8, 0x28, 0xFE, 0xA9, 0x1E, 0x35, 0x36, 0xA5, 0x96, 0x5A, 0xAC, 0xFC, 0x15, 0xB0, 0x95,
0x8C, 0xBE, 0x8D, 0x07, 0x89, 0x11, 0x6C, 0x6D, 0x7D, 0xCB, 0x30, 0x22, 0xC5, 0x2C, 0xC4, 0xDC,
0x37, 0xF6, 0x34, 0x7B, 0xA8, 0x9C, 0x13, 0xF5, 0xA3, 0x11, 0x8B, 0x82, 0x3C, 0x8B, 0x59, 0x34,
0x26, 0x69, 0x9C, 0xC2, 0x24, 0xD5, 0x85, 0x4B, 0xD2, 0xDF, 0x21, 0x49, 0x8B, 0x10, 0x97, 0x73,
0xD9, 0x77, 0x73, 0x80, 0x9C, 0xDA, 0x9C, 0xB7, 0xCC, 0xCE, 0x0A, 0xC0, 0xC8, 0x11, 0xDA, 0xBC,
0xD6, 0x51, 0x40, 0x87, 0xA6, 0x8D, 0x86, 0xA1, 0xB3, 0x1F, 0xB4, 0x53, 0x40, 0xEC, 0x72, 0x14,
0xF4, 0xDF, 0x2C, 0xF4, 0xF3, 0x30, 0x7F, 0x3C, 0xFC, 0x7A, 0x7F, 0x3A, 0xBC, 0x1C, 0xE7, 0xE2,
0xFA, 0x37, 0x64, 0xD9, 0x38, 0x75, 0x59, 0xFF, 0x43, 0xFE, 0x02, 0xEA, 0xA0, 0x6C, 0x65, 0x38,
0x03, 0x00, 0x00
};
const char success_html[] = "<!DOCTYPE html> <html> <head> Success saved. Please click button to restart for apply new config values";
//***********************************************************************************************************************************************************************************************
void Setup_WebServer() {
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Server",String(me_lokalizacja));
  DefaultHeaders::Instance().addHeader("Title",String(me_lokalizacja));
  //DefaultHeaders::Instance().addHeader("Content-Encoding","gzip");
  //webserver.serveStatic("/", SPIFFS, "/").setTemplateProcessor(web_processor); //this prevents page loading
  #ifdef enableWebUpdate
  SetupWebUpdate();
  #endif
//  webserver.rewrite("/", "/index.html");
//  webserver.rewrite("/edit.html", "/edit");
     webserver.on("/" , HTTP_GET, [](AsyncWebServerRequest *request){
         request->send(SPIFFS, "/index.html", "text/html; charset=utf-8",  false, web_processor);
     }).setAuthentication("", "");

  webserver.serveStatic("/", SPIFFS, "/index.html")
      .setDefaultFile("index.html")
      .setCacheControl("max-age=600")
      .setTemplateProcessor(web_processor)
      .setAuthentication("\0", "\0");//no cache so can change

  if (!SPIFFS.exists("/index.html")) {
    webserver.rewrite("/", "/edit");
  }
    webserver.on("/edit" , HTTP_GET, [](AsyncWebServerRequest *request){
//       AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=utf-8; Content-Encoding; gzip", edit_html, web_processor);
//       response->addHeader("Content-Encoding", "gzip");
// //      response->addHeader("Last-Modified", htmlBuildTime);
//       request->send(response);
      request->send_P(200, "text/html; charset=utf-8",  edit_html, web_processor);
    }).setAuthentication("", "");

    // webserver.on("/post" , HTTP_GET, [](AsyncWebServerRequest *request){
    //   request->send(SPIFFS, "/success.html", "text/html; charset=utf-8",  false);
    // }).setAuthentication("", "");



  webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("RESTART"))
      {
        restart(F("Command from Webserwer get with param RESTART"));
      }else {
          //message = "No message sent PARAM_roomtempset0";
      }    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Update value");
    response->addHeader("Refresh", "2");
    response->addHeader("Location", "/");
    request->send(response);
  }).setAuthentication("", "");

  webserver.on("/post", HTTP_POST, [](AsyncWebServerRequest * request) {
    int paramsNr = request->params(); // number of params (e.g., 1)
    sprintf(log_chars,"HTTP received POST, paramsNr: %s", String(paramsNr).c_str());
    log_message(log_chars);

    int params = request->params();
    String postjson =F("{\"config\":01");
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      postjson += F(",\"");
      if (String(p->name()) == "LogOutput") { postjson += String(p->value()); } else { postjson += String(p->name()); }
      postjson += F("\"");
      postjson += F(":");
      postjson += F("\"");
      if (String(p->name()) == "LogOutput") { postjson += 1; } else { postjson += String(p->value()); }
      postjson += F("\"");
    }
    postjson += addusage_local_values_save(0);
    postjson += F("}");
    sprintf(log_chars,"Received POST: %sB", String(postjson.length()).c_str());
    log_message(log_chars);
    if(SPIFFS.exists(configfile)) {SPIFFS.remove(configfile);}
    File file = SPIFFS.open(configfile, "w");
    if (!file){
      log_message((char*)F("Error opening file for write config..."));
    } else
    if (!file.println(postjson)) {
      log_message((char*)F("File was not written with config"));
    }
      file.close();
      log_message((char*)F("File was saved. Now sending reply to success.html."));
      loadConfig();
    request->send(SPIFFS, "/success.html", "text/html; charset=utf-8",  false);
    // request->send(200);
  }).setAuthentication("", "");;



// run handleUpload function when any file is uploaded
//  webserver.addHandler(webhandleFileRead);
  webserver.onFileUpload(webhandleFileUpload);
  webserver.onNotFound([](AsyncWebServerRequest * request) {        // if someone requests any other file or page, go to function 'handleNotFound'
    log_message((char*)F("onNotFound: Check if file exist else return 404"));
    if(!webhandleFileRead(request, request->url())){                        // check if the file exists in the flash memory (SPIFFS), if so, send it
      webhandleNotFound(request);      // and check if the file exists
    }
  });

  webserver.begin();                             // start the HTTP server
#ifdef enableWebSocket
//  notifyClients(getValuesToWebSocket_andWebProcessor(ValuesToWSWPinJSON));
#endif
  log_message((char*)F("HTTP server started."));
}
//***********************************************************************************************************************************************************************************************
void webhandleNotFound(AsyncWebServerRequest *request) {
  String message = F("File Not Found...");
  message += F("URI: ");
  message += request->url();
  message += F("<br>Method: ");
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += F("<br>Arguments: ");
  message += request->args();
  message += F("<br>");
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "<br>";
  }
  request->send(404, "text/plain", message);
}
//***********************************************************************************************************************************************************************************************
void webhandleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) { // upload a new file to the SPIFFS
    struct uploadRequest {
      uploadRequest* next;
      AsyncWebServerRequest *request;
      File uploadFile;
      uint32_t fileSize;
      uploadRequest(){next = NULL; request = NULL; fileSize = 0;}
    };
    static uploadRequest uploadRequestHead;
    uploadRequest* thisUploadRequest = NULL;

    if( ! index){
      String toFile = filename;
      if(request->hasParam("dir", true)) {
        AsyncWebParameter* p = request->getParam("dir", true);
        sprintf(log_chars,"dir param: %s", String(p->value()).c_str());
        log_message(log_chars);
        toFile = p->value();
        if(!toFile.endsWith("/")) toFile += "/";
        toFile += filename;
      }
      if(!toFile.startsWith("/")) toFile = "/" + toFile ;

      if(SPIFFS.exists(toFile)) SPIFFS.remove(toFile);
      thisUploadRequest = new uploadRequest;
      thisUploadRequest->request = request;
      thisUploadRequest->next = uploadRequestHead.next;
      uploadRequestHead.next = thisUploadRequest;
      thisUploadRequest->uploadFile = SPIFFS.open(toFile, "w");
      sprintf(log_chars,"Upload: START, filename: %s", String(toFile).c_str());
      log_message(log_chars);
    }
    else {
      thisUploadRequest = uploadRequestHead.next;
      while(thisUploadRequest->request != request) thisUploadRequest = thisUploadRequest->next;
    }

    if(thisUploadRequest->uploadFile){
      for(size_t i=0; i<len; i++){
        thisUploadRequest->uploadFile.write(data[i]);
      }
      thisUploadRequest->fileSize += len;
    }

    if(final){
      thisUploadRequest->uploadFile.close();
      sprintf(log_chars,"Upload: END, Size: %s", String(formatBytes((size_t)thisUploadRequest->fileSize)).c_str());
      log_message(log_chars);
      uploadRequest* linkUploadRequest = &uploadRequestHead;
      while(linkUploadRequest->next != thisUploadRequest) linkUploadRequest = linkUploadRequest->next;
      linkUploadRequest->next = thisUploadRequest->next;
      delete thisUploadRequest;
      AsyncWebServerResponse *response = request->beginResponse(303, "text/plain", "Please wait ...OK");
      response->addHeader("Refresh", "2");
      response->addHeader("Location", "/success.html");
      request->send(response);
      if ( filename.indexOf("index.htm") >-1 ) restart(F("Restart After update main index.html file"));
    }
}

#ifdef ESP32
//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.printf("Task1 running on core %i ", String(xPortGetCoreID()).c_str());
  sprintf(log_chars, "Task1 running on core %i ", String(xPortGetCoreID()).c_str());
  log_message(log_chars);

  for(;;){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);

  }
}
#endif


//***********************************************************************************************************************************************************************************************
void MainCommonSetup()  {
  #ifndef SerialSpeed
  #define SerialSpeed 115200
  #endif
  #ifdef enableDebug2Serial
  Serial.begin(SerialSpeed);
  Serial.println(F("Starting... MainSetup..."));
  #endif
  getFreeMemory();
  #ifdef doubleResDet
  // double reset detect from start
  doubleResetDetect();
  #endif
  WiFi.setAutoReconnect(false);
  WiFi.setAutoConnect(false);
  WiFi.disconnect(true);
  int ledpin = LED_BUILTIN;
  pinMode(ledpin, OUTPUT);
  #ifdef ESP32
  btStop();   //disable bluetooth
  #endif

  Setup_FileSystem();
  if (loadConfig())
  {
    sprintf(log_chars,"Config loaded. SSID: %s, Pass: %s", ssid, pass);
    log_message(log_chars);
  }
  else
  {
    log_message((char*)F("Config not loaded!"));
    SPIFFS.format();
    SaveConfig(); // overwrite with the default settings
  }
#if defined enableMQTTAsync
  Setup_Mqtt();     //for async version move before wifi
#endif
  Setup_WiFi();
  CRTrunNumber++;
  #ifdef enableArduinoOTA
  Setup_OTA();
  #endif
  Setup_DNS();
#ifdef enableWebSerial
  WebSerial.begin(&webserver);
  WebSerial.msgCallback(RemoteCommandReceived);
#endif                //we have webserial so we can enable it now
  #ifdef enableWebSocket
  Setup_WebSocket();
  Setup_WebServer();    //new www based on spiff files
  starting = false;
  updateDatatoWWW_common();
  #else
  starthttpserver();    //old www
  #endif
  #if defined enableMQTT
  Setup_Mqtt();     //for async version move before wifi
  #endif
  #ifdef ENABLE_INFLUX
  Setup_Influx();
  #endif
  ESPlastResetReason = get_lastResetReason();
  log_message((char*)ESPlastResetReason.c_str());
  #ifdef enableMESHNETWORK
  Setup_MeshWiFi();
  #endif
  #ifdef ESP32
  //Start one core in setup and loop ;)
  xPortGetCoreID();
    //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    maxLogSize*2,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */
  delay(500);
  #endif
}



//***********************************************************************************************************************************************************************************************

char* toCharPointer (String ptrS) {
  // char ptr[ptrS.length()+1];
  // strcpy(ptr, ptrS.c_str());
  //ptrS.toCharArray(ptr, ptrS.length()+1);
  // Serial.print("tocharpntr: ");
  // Serial.println(String(*ptr));
  // Serial.println(" String: ");
  // Serial.println(*ptrS);
  char* retptr = (char*) ptrS.c_str();
  return retptr;
}
//***********************************************************************************************************************************************************************************************
void SaveAssValue(uint8_t ASSV, String source_str, bool humid = false) {
  if (!humid) ASS[ASSV].Value = source_str; else ASS[ASSV].ValueHumid = source_str;
}
//***********************************************************************************************************************************************************************************************
void updateDatatoWWW_common()
{
  //ASS[ASS_uptimedana].Value = (String(uptimedana(0) + "    CRT: <b>" + String(CRTrunNumber)));
  // strcpy(ASS[ASS_MemStats].Value, String(uptimedana(0) + "    CRT: <b>" + String(CRTrunNumber)).c_str() );
  SaveAssValue(ASS_uptimedana, String(uptimedana(0) + "    CRT: <b>" + String(CRTrunNumber)) );
  //Statusy
    String ptr = "\0";
    ptr += F("Free mem: <b>");
    ptr += String(getFreeMemory());
    ptr += F("&percnt;</b>, Heap: <b>");
    ptr += formatBytes(ESP.getFreeHeap());
    ptr += F("</b>, Wifi: <b>");
    ptr += String(getWifiQuality());
    ptr += F("&percnt;");
    ptr += F("</b>");
    #ifdef ESP32
    ptr += F(", Hall: <b>");
    ptr += String(hallRead());
    ptr += F("</b>G");
    #endif
    #if defined enableMQTT || defined enableMQTTAsync
    ptr += F("</br>MQTT");
    #ifdef enableMQTTAsync
    ptr += F("-Async ");
    #endif
    ptr += F("status: <b>");
    ptr += ((mqttclient.connected())?"Connected":"Disconnected");
    ptr += F("</b>");
    ptr += F(", reconnects: <b>");
    ptr += mqttReconnects;
    ptr += F("</b>");
    #endif
    #ifdef ENABLE_INFLUX
    ptr += F("</br>InfluxDB: <b>");
    ptr += (InfluxStatus?"Connected":"Disconnected");
    ptr += F("</b>");
    #endif
    if (ESPlastResetReason.length() > 0) {
      ptr += F("</br>Last reset: <b>");
      String ptrtmp = ESPlastResetReason;
      ptrtmp.replace("\n","<br>");
      ptr += ptrtmp;
      ptr += F("</b>");
    }
    SaveAssValue(ASS_MemStats, ptr );
    updateDatatoWWW();
  //ASS[ASS_MemStats].Value = ((ptr));
  // strcpy(ASS[ASS_MemStats].Value, String(ptr).c_str() );
}
void MainCommonLoop()
{
//  #include "configmqtttopics.h"
  #ifdef doubleResDet
  drd->loop();
  #endif
  #ifdef enableArduinoOTA
  // Handle OTA first.
  ArduinoOTA.handle();
    #ifdef debug
    if ((millis()/1000) % 5 == 0) log_message((char*)F("MainLoop: ArduinoOTA"));
    #endif
  #endif
  #ifdef debug
  log_message((char*)F("MainLoop: checkWifi"));
  #endif
  check_wifi();
  #ifdef enableMESHNETWORK
  #ifdef debug
  log_message((char*)F("MainLoop: meshnetwork"));
  #endif
  MeshWiFi.update();
  #endif
  #ifdef enableMQTT
  #ifdef debug
  log_message((char*)F("MainLoop: mqtt loop"));
  #endif
  mqttclient.loop();
  #endif
  #ifdef enableWebSocket
  #ifdef debug
  log_message((char*)F("MainLoop: Websocket cleanup"));
  #endif
  WebSocket.cleanupClients();
  #endif

  if (Serial.available() > 0) {
    u_int8_t *data;
    String test = Serial.readString();
    data = (u_int8_t *)test.c_str();
    RemoteCommandReceived(data, (size_t)(test.length() ));
    log_message((char*)F("----------------------------MainLoop: Received SerialData command --------------------------------"), logCommandResponse);

    //free(data);
  }

  if ((millis() - lastloopRunTime) > LOOP_WAITTIME and WiFi.status() == WL_CONNECTED)
  {
    lastloopRunTime = millis();
    #ifdef debug
    log_message((char*)F("MainLoopInLoop: loop begin updatedatatowww"));
    #endif
    updateDatatoWWW_common();
    #ifdef debug
    log_message((char*)F("MainLoopInLoop: notifyclients to /ws in JSON"));
    #endif
    #if defined enableMQTT || defined enableMQTTAsync
    String tmpStrmqtt = getValuesToWebSocket_andWebProcessor(ValuesToWSWPinJSON);
    notifyClients(tmpStrmqtt);
    #endif
    // check mqtt
    #ifdef enableMQTT
    #ifdef debug
    log_message((char*)F("MainLoopInLoop: mqtt pubsub"));
    #endif
      if ((WiFi.isConnected()) && (!mqttclient.connected()))
    {
      mqttclient.disconnect();
      log_message((char *)F("Lost MQTT connection! Trying Reconnect. Just .disconnected"));
      mqttReconnect();
    }
  #endif
  // #ifdef ENABLE_INFLUX
  //   #ifdef debug
  //   log_message((char*)F("MainLoopInLoop: Influx validateconnection"));
  //   #endif
  //   InfluxStatus = InfluxClient.validateConnection();
  //   if (InfluxStatus)
  //   {
  //     sprintf(log_chars, "Connected to InfluxDB: %s", String(InfluxClient.getServerUrl()).c_str());
  //     log_message(log_chars,0);
  //   }
  //   else
  //   {
  //     sprintf(log_chars, "InfluxDB connection failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
  //     log_message(log_chars,0);
  //   }
  // #endif
    #ifndef ESP32
    wdt_reset();
    #endif
    #ifdef debug
    log_message((char*)F("MainLoopInLoop: message str builder to log"));
    #endif
    // log stats
    //    #include "configmqtttopics.h"
    String payloadvalue_startend_val = F(" ");
    String message = F("stats: Uptime: ");
    message += uptimedana();
    message += F(" ## Free memory: ");
    message += String(getFreeMemory());
    message += F("% ");
    message += formatBytes(ESP.getFreeHeap());
    message += F(" bytes ## Wifi: ");
    message += String(getWifiQuality());
    #ifdef ESP32
    message += F("Hall: ");
    message += String(hallRead());
    message += F("Gauss");
    #endif
    #if defined enableMQTT || defined enableMQTTAsync
    message += F("% ## Mqtt reconnects: ");
    message += String(mqttReconnects);
    #endif

    #ifdef ENABLE_INFLUX
    message += F(", InfluxDB: ");
    message += (InfluxStatus?"Połączony":"Rozłączony");
    #endif
    log_message((char *)message.c_str());

    #if defined enableMQTT || defined enableMQTTAsync
    #ifdef debug
    log_message((char*)F("MainLoopInLoop: stats string builder to json"));
    #endif
    String stats = build_JSON_Payload( F("CRT"), String(CRTrunNumber), true, payloadvalue_startend_val);
    stats += build_JSON_Payload( F("rssi"), String(WiFi.RSSI()), false, payloadvalue_startend_val);
    stats += build_JSON_Payload( F("uptime"), String(millis()), false, payloadvalue_startend_val);
    stats += build_JSON_Payload( F("version"), String(version), false, payloadvalue_startend_val);
    stats += build_JSON_Payload( F("voltage"), String(0), false, payloadvalue_startend_val);
    stats += build_JSON_Payload( F("free memory"), String(getFreeMemory()), false, payloadvalue_startend_val);
    stats += build_JSON_Payload( F("ESP_cyclecount"), String(ESP.getCycleCount()), false, payloadvalue_startend_val);
    stats += build_JSON_Payload( F("wifi"), String(getWifiQuality()), false, payloadvalue_startend_val);

    #ifdef ESP32
    stats += build_JSON_Payload( F("Hall"), String(hallRead()), false, payloadvalue_startend_val);
    #endif
    stats += build_JSON_Payload( F("mqttReconnects"), String(mqttReconnects), false, payloadvalue_startend_val);
    // stats += F("}");
    #ifdef debug
    log_message((char*)F("MainLoopInLoop: mqttAsync send stats"));
    #endif //debug
    String topicStr ="\0";
    topicStr = String(STATS_TOPIC);
    publishMQTT(topicStr, stats, mqtt_Retain, QOS);

    // get new data
    //  if (!heishamonSettings.listenonly) send_panasonic_query();
    // Make sure the LWT is set to Online, even if the broker have marked it dead.
    #ifdef debugweb
    log_message((char*)F("MainLoopInLoop: mqttAsync send WillTopic"));
    #endif //debug
    topicStr = String(WILL_TOPIC);
    stats = String(WILL_ONLINE);
    publishMQTT(topicStr, stats, mqtt_Retain, QOS);
    #endif
    //updateDatatoWWW_common();  //send after sync struct with vars
  }

  if (((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms) or lastUpdatemqtt == 0 or (receivedmqttdata == true and ((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms/3))) // recived data ronbi co 800ms -wylacze ten sttus dla odebrania news
  {
    #ifdef debug
    log_message((char*)F("MainLoop2ndTimed: entry on mqttmessage or time"));
    #endif //debug
    sprintf(log_chars, "Update MQTT and influxDB. lastUpdatemqtt: %s receivedmqttdata: %s mqttUpdateInterval_ms: %s", String(uptimedana(lastUpdatemqtt,true,true)).c_str(), String(receivedmqttdata).c_str(), String(mqttUpdateInterval_ms).c_str());
    log_message(log_chars,0);
    receivedmqttdata = false;
    lastUpdatemqtt = millis();
    #ifdef ENABLE_INFLUX
    #ifdef debug
    log_message((char*)F("MainLoop2ndTimed: Update InfluxDB"));
    #endif //debug
    updateInfluxDB(); // i have on same server mqtt and influx so when mqtt is down influx probably also ;(  This   if (InfluxClient.isConnected())  doesn't work forme 202205
    #endif //ENABLE_INFLUX
    //mqttReconnect_subscribe_list();
    #if defined enableMQTT || defined enableMQTTAsync
    #ifdef debug
    log_message((char*)F("MainLoop2ndTimed: Update MQTT data"));
    #endif //debug
    updateMQTTData();
    #endif //defined enableMQTT || enableMQTTAsync
  }
#ifndef ESP32
  if ((millis() % 600) < 100) {
    #ifdef debug
    log_message((char*)F("MainLoop3rdTimed: Ledblonk"));
    #endif //debug
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));}
#endif
}

//***********************************************************************************************************************************************************************************************
#if defined enableMQTT || defined enableMQTTAsync
void Setup_Mqtt()
{
  #ifndef sensitive_size
  #define sensitive_size 32
  #endif  //sensitive_size
  #ifdef enableMQTT
  mqttclient.setBufferSize(2048);
  mqttclient.setSocketTimeout(10);
  mqttclient.setKeepAlive(7); // fast timeout, any slower will block the main loop too long
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(mqtt_callback);
  mqttReconnect();
  #endif //enableMQTT
  #ifdef enableMQTTAsync
  mqttclient.setMaxTopicLength(2048); //default 128
  char clientid[sensitive_size];
  strcpy(clientid, String(BASE_TOPIC).c_str() );
  mqttclient.setClientId(clientid);
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCredentials(mqtt_user, mqtt_password);
  mqttclient.onConnect(onMqttConnect);
  mqttclient.onDisconnect(onMqttDisconnect);
  //mqttclient.onSubscribe(onMqttSubscribe);
  //mqttclient.onUnsubscribe(onMqttUnsubscribe);
  mqttclient.onMessage(onMqttMessage);
  mqttclient.onPublish(onMqttPublish);
  #ifdef debug
  sprintf(log_chars,"MQTT_Setup. ClientID: %s, server: %s:%i, Uname: %s/%s",clientid, mqtt_server, mqtt_port, mqtt_user, mqtt_password);
  log_message(log_chars);
  #endif  //debug
  //mqttclient.setSecure(false); //for ssl but must be build with special flag
  //mqttclient.setKeepAlive(7);
  //mqttclient.setWill(String(WILL_TOPIC).c_str(), QOS, mqtt_Retain, String("Online").c_str());  //const char* topic, uint8_t qos, bool retain, const char* payload, size_t length)  -this block me from connecting

  #endif //enableMQTTAsync
}
#endif //defined enableMQTT || defined enableMQTTAsync
//***********************************************************************************************************************************************************************************************
#ifdef enableMQTT
void mqttReconnect()
{
  log_message((char*)("!!!!mqttReconnect"));
  // if (mqttReconnects % 10 == 0) mqttclient.disconnect();
  // if (mqttReconnects % 50 == 0) { mqttclient.disconnect(); WiFi.disconnect(); check_wifi(); }
  // if (mqttReconnects % 500 == 0) { restart(F("mqttReconnect cause soft restart")); }

  const char *clientId = String(BASE_TOPIC).c_str();

  mqttReconnects ++;
  if (mqttclient.connect(clientId, mqtt_user, mqtt_password)) {
    mqttclient.publish(String(WILL_TOPIC).c_str(), "Online");
    mqttReconnect_subscribe_list();
  }
}
#endif
//***********************************************************************************************************************************************************************************************

#ifndef ESP32
void onWifiConnect(const WiFiEventStationModeGotIP& event) {
#else
void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info) {
#endif
  log_message((char*)F("WiFi Task: Connected to Wi-Fi."));
  // if (DRDActiveStatus == false)
  #ifdef enableMQTTAsync
  connectToMqtt();
  #endif
  // sprintf(log_chars,"Wifi Connect. DRD status: %s", String(DRDActiveStatus?"Active":"Inactive").c_str());
  // log_message(log_chars);
}
//***********************************************************************************************************************************************************************************************
#ifndef ESP32
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
#else
void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
#endif
  log_message((char*)F("WiFi Task: Disconnected from Wi-Fi."));
  #ifdef enableMQTTAsync
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  #endif
  wifiReconnectTimer.once(2, connectToWifi);
}
//***********************************************************************************************************************************************************************************************
#ifdef enableMQTTAsync
void connectToMqtt() {
  log_message((char*)F("MQTT: Connecting to MQTT..."));
  mqttReconnects++;
  mqttclient.connect();
}
#endif
//***********************************************************************************************************************************************************************************************
void onMqttConnect(bool sessionPresent) {
#ifdef enableMQTTAsync
  log_message((char*)F("MQTT: Connected to MQTT."));
  sprintf(log_chars,"Session present: %s", String(sessionPresent?"Yes":"No").c_str());
  log_message(log_chars);

  // uint16_t packetIdSub = mqttclient.subscribe("test/lol", 2);
  // sprintf(log_chars,"Subscribing at QoS 2, packetId: %s", String(packetIdSub).c_str());
  // log_message(log_chars);

  // mqttclient.publish("test/lol", 0, true, "test 1");
  // sprintf(log_chars,"Publishing at QoS 0: %s", String("\0").c_str());
  // log_message(log_chars);

  // uint16_t packetIdPub1 = mqttclient.publish("test/lol", 1, true, "test 2");
  // sprintf(log_chars,"Publishing at QoS 1, packetId: %s", String(packetIdPub1).c_str());
  // log_message(log_chars);

  // uint16_t packetIdPub2 = mqttclient.publish("test/lol", 2, true, "test 3");
  // sprintf(log_chars,"Publishing at QoS 2, packetId: %s", String(packetIdPub2).c_str());
  // log_message(log_chars);

//here publish send every 2-4sec
  mqttReconnect_subscribe_list();
//  updateMQTTData();
#endif
}
//***********************************************************************************************************************************************************************************************
#ifdef enableMQTTAsync
int _retriesCount = 0;
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println((uint8_t)reason);
  String reasontxt = "\0";
  if (AsyncMqttClientDisconnectReason::TCP_DISCONNECTED == reason) reasontxt = "0. TCP_DISCONNECTED";
  if (AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION == reason) reasontxt = "1. MQTT_UNACCEPTABLE_PROTOCOL_VERSION";
  if (AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED == reason) reasontxt = "2. MQTT_IDENTIFIER_REJECTED";
  if (AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE == reason) reasontxt = "3. MQTT_SERVER_UNAVAILABLE";
  if (AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS == reason) reasontxt = "4. MQTT_MALFORMED_CREDENTIALS";
  if (AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED == reason) reasontxt = "5. MQTT_NOT_AUTHORIZED";
  if (AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE == reason) reasontxt = "6. ESP8266_NOT_ENOUGH_SPACE";
  if (AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT == reason) reasontxt = "7. TLS_BAD_FINGERPRINT";
  sprintf(log_chars,"MQTT: Disconnected from MQTT. Reason: %s. Trying to reconnect.", String(reasontxt).c_str());
  log_message(log_chars);

  if (WiFi.isConnected()) {
    int retryTimer = 2;
    _retriesCount++;
    if (_retriesCount < 5) {
        retryTimer = 2;
    } else if (_retriesCount < 10) {
        retryTimer = 15;
    } else if (_retriesCount < 20) {
        retryTimer = 30;
    } else {
        retryTimer = 60;
    }
    mqttReconnectTimer.once(retryTimer, connectToMqtt);
  }
}
#endif
//***********************************************************************************************************************************************************************************************
#ifdef enableMQTTAsync
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  sprintf(log_chars,"MQTT: Subscribe acknowledged. packetId: %s, qos: %s", String(packetId).c_str(), String(qos).c_str());
  log_message(log_chars);
}
#endif
//***********************************************************************************************************************************************************************************************
#ifdef enableMQTTAsync
void onMqttUnsubscribe(uint16_t packetId) {
  log_message((char*)F("MQTT: Unsubscribe acknowledged."));
  sprintf(log_chars," packetId: %s", String(packetId).c_str());
  log_message(log_chars);
}
#endif
//***********************************************************************************************************************************************************************************************
#ifdef enableMQTTAsync
void onMqttMessage(char* topicAMCP, char* payloadAMCP, AsyncMqttClientMessageProperties properties, size_t lenAMCP, size_t index, size_t total) {
  char cPointers[lenAMCP];
  memcpy(cPointers, payloadAMCP, lenAMCP+1);
  //return;
  cPointers[lenAMCP]='\0';
  String PayloadStrTmPAMCP = String(cPointers);
  receivedtmpString += PayloadStrTmPAMCP;
  receivedtmpIdx += lenAMCP;
  #ifdef debugx
  //sprintf(log_chars,"!!!!!!MQTT: Publish received. topic: %s, payload: %s, qos: %s, dup: %s, retain: %s, len: %s, index: %s, total: %s, PayloadLen: %s", String(topicAMCP).c_str(), String(payloadAMCP).c_str(), String(properties.qos).c_str(), String(properties.dup).c_str(), String(properties.retain).c_str(), String(lenAMCP).c_str(), String(index).c_str(), String(total).c_str(), String(PayloadStrTmPAMCP.length()).c_str());
  //log_message(log_chars);
  #else
  sprintf(log_chars,"!!!!!!MQTT: Publish received. ");
  log_message(log_chars);
  #endif
  #ifndef ESP32
  wdt_reset();
  #endif
  String topicAMCP_Str = String(topicAMCP);
  if (total == lenAMCP)  //check is starts and ends as json data and nmqttident null
  { //check if received payload is valid json if not save to var. I assume that index is same and one after another and not mixed with other payloads
    receivedtmpString = "\0";
    receivedtmpIdx = 0;
    #ifdef debugweb
    sprintf(log_chars,"                 PayloadStrTmPAMCP to callback2: %s, length: %u, len= %u, total= %u, receivedtmpString.len: %u", PayloadStrTmPAMCP.c_str(), (u_int)PayloadStrTmPAMCP.length(), (u_int)lenAMCP, \
     (u_int)total, (u_int)receivedtmpString.length());
    log_message(log_chars);
    #endif
    mqttCallbackAsString(topicAMCP_Str, PayloadStrTmPAMCP);
  } else
  {
    if (receivedtmpIdx == total) {
      #ifdef debugweb
      sprintf(log_chars,"               PayloadStrTmPAMCP to callback2 receivedtmpIdx == total: %s, length: %u, len= %u, total= %u, receivedtmpString.len: %u", PayloadStrTmPAMCP.c_str(), (u_int)PayloadStrTmPAMCP.length(), (u_int)lenAMCP, \
                (u_int)total, (u_int)receivedtmpString.length());
      log_message(log_chars);
      #endif
      mqttCallbackAsString(topicAMCP_Str, receivedtmpString);
      receivedtmpString = "\0";
      receivedtmpIdx = 0;

    }
  }
}
#endif
//***********************************************************************************************************************************************************************************************
#ifdef enableMQTTAsync
void onMqttPublish(uint16_t packetId) {
  log_message((char*)F("MQTT: Publish acknowledged."));
  sprintf(log_chars," packetId: %s", String(packetId).c_str());
  log_message(log_chars);
  _retriesCount = 0;
}
#endif
//***********************************************************************************************************************************************************************************************
String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "kB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  return "B";
}
//***********************************************************************************************************************************************************************************************
String webgetContentType(String filename) {
  if(filename=="download") return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html; charset=utf-8";
  else if(filename.endsWith(".html")) return "text/html; charset=utf-8";
  else if(filename.endsWith(".css")) return "text/css; charset=utf-8";
  else if(filename.endsWith(".js")) return "application/javascript; charset=utf-8";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml; charset=utf-8";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain; charset=utf-8";
}
//***********************************************************************************************************************************************************************************************

/*
 *  check_wifi will process wifi reconnecting managing
 */
void check_wifi()
{
  #ifndef ESP32
  wdt_reset();
  #endif
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {//|| (!WiFi.localIP()))  {
    /*
     *  if we are not connected to an AP
     *  we must be in softAP so respond to DNS
     */
    dnsServer.processNextRequest();
    /* we need to stop reconnecting to a configured wifi network if there is a hotspot user connected
     *  also, do not disconnect if wifi network scan is active
     */
    /*  only start this routine if timeout on
     *  reconnecting to AP and SSID is set
     */
  //  WiFi.disconnect();
    #ifdef enableMQTT
 ///   mqttclient.disconnect();
    #endif

    if ((ssid[0] != '\0') && ((unsigned long)(millis() - lastWifiRetryTimer) > WIFIRETRYTIMER ) )  {
      lastWifiRetryTimer = millis();
      if (WiFi.softAPSSID() == "") {
        log_message((char *)"WiFi lost, starting setup hotspot...");
//        WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255, 255, 255, 0));
//        WiFi.softAP(String(me_lokalizacja).c_str());
      }
      if ((WiFi.status() == WL_DISCONNECTED) || WiFi.status() == 0 ) {//} && (WiFi.softAPgetStationNum() == 0 )) {
        sprintf(log_chars,"Retrying configured WiFi, ... SSID: %s, pss: %s", String(ssid).c_str(), String(pass).c_str());
        log_message((log_chars));
        connectToWifi();
      } else {
        log_message((char *)"Reconnecting to WiFi failed. Waiting a few seconds before trying again.");
        WiFi.disconnect();
      }
    }
      if (ssid[0] == '\0') {
        log_message((char *)"WiFi connected without SSID and password in settings. Must come from persistent memory. Storing in settings.");
        WiFi.SSID().toCharArray(ssid, 40);
        WiFi.psk().toCharArray(pass, 40);
        SaveConfig(); //save to config file
      }
    /*
       always update if wifi is working so next time on ssid failure
       it only starts the routine above after this timeout
    */
    lastWifiRetryTimer = millis();
    sprintf(log_chars,"Adres after reconnect IP: %s",WiFi.localIP().toString().c_str() );
    log_message(log_chars);
  } else { //WiFi connected

    // Allow MDNS processing
//    MDNS.update();
  }
}

//***********************************************************************************************************************************************************************************************

#ifdef enableWebUpdate
const char htmlup[] PROGMEM = R"rawliteral(
  <form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update' accept=".bin,.bin.gz"><input type='submit' value='Update'></form>)rawliteral";
#endif
//***********************************************************************************************************************************************************************************************
#ifdef enableWebUpdate
void SetupWebUpdate() {
  webserver.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html; charset=utf-8", htmlup);
    }).setAuthentication("", "");
  webserver.on("/doUpdate", HTTP_POST,
      [&](AsyncWebServerRequest *request) {
      // the request handler is triggered after the upload has finished...
      // create the response, add header, and send response
      AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain; charset=utf-8", (Update.hasError())?"FAIL":"OK");
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
      },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
                  size_t len, bool final) {handleDoUpdate(request, filename, index, data, len, final);}
  ).setAuthentication("", "");
}
#endif
//***********************************************************************************************************************************************************************************************
#ifdef enableWebUpdate
void handleUpdate(AsyncWebServerRequest *request) {
  #ifdef enableWebSocket
   // Disable client connections
   WebSocket.enable(false);
   // Advertise connected clients what's going on
   String tmpStrnc = F("Web OTA Update Started");
   notifyClients(tmpStrnc);
   // Close them
   WebSocket.closeAll();
  #endif
  #ifdef enableWebUpdate
  request->send(200, "text/html", htmlup);
  #endif
}
#endif
#ifdef enableWebUpdate
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
//#define U_FLASH   0  //copied from ESP8266 library dla ESP32 sprawdzic
//#define U_LittleFS  100
//#define U_AUTH    200
  if (!index) {
    log_message((char*)F("Update starts"));
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "15");
    response->addHeader("Location", "/");
    request->send(response);
    content_len = request->contentLength();
    // if filename includes LittleFS, update the LittleFS partition
    int cmd = U_FLASH; //(filename.indexOf("LittleFS") > -1) ? U_LittleFS : U_FLASH;
//    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)) {
      Update.printError(Serial);
    }
  }
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
//#ifdef ESP8266
  } else {
    if ((Update.progress() * 100) / Update.size() % 5 == 0) {
    sprintf(log_chars,"Progress: %d%%", (Update.progress() * 100) / Update.size());
    log_message(log_chars);
    }
//#endif
  }
  if (final) {
    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      log_message((char*)F("Update complete"));
      Serial.flush();
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
      response->addHeader("Refresh", "20");
      response->addHeader("Location", "/");
      request->send(response);
      delay(100);
      restart("End update from WebUpdate");
    }
  }
}
#endif
//***********************************************************************************************************************************************************************************************
#ifdef enableWebUpdate
 void printProgress(size_t prg, size_t sz) {
   if ((Update.progress() * 100) / Update.size() % 5 == 0) {
   sprintf(log_chars,"Progress: %d%%", (prg * 100) / content_len);
   log_message(log_chars);
   }
 }
#endif
//***********************************************************************************************************************************************************************************************
 String PrintHex8(const uint8_t *data, char separator, uint8_t length) // prints 8-bit data in hex , uint8_t length
{
  uint8_t lensep = sizeof(separator);
  int dod = 0;
  if (separator == 0x0) {
    lensep = 0;
    dod = 1;
  }
  if (lensep > 1) dod = 1 - lensep;
  char tmp[length * (2 + lensep) + dod - lensep];
  byte first;
  byte second;
  for (int i = 0; (i + 0) < length; i++) {
    first = (data[i] >> 4) & 0x0f;
    second = data[i] & 0x0f;
    // base for converting single digit numbers to ASCII is 48
    // base for 10-16 to become lower-case characters a-f is 87
    // note: difference is 39
    tmp[i * (2 + lensep)] = first + 48;
    tmp[i * (2 + lensep) + 1] = second + 48;
    if ((i) < length and (i) + 1 != length) tmp[i * (2 + lensep) + 2] = separator;
    if (first > 9) tmp[i * (2 + lensep)] += 39;
    if (second > 9) tmp[i * (2 + lensep) + 1] += 39;

  }
  tmp[length * (2 + lensep) + 0 - lensep] = 0;
// #ifdef debug
//   Serial.print(F("MAC Addr: "));
//   Serial.println(tmp);
// #endif
  //     debugA("%s",tmp);
  return tmp;
}
//***********************************************************************************************************************************************************************************************
#if defined enableMQTT || defined enableMQTTAsync
void HADiscovery(String sensorswitchValTopic, String appendname, String nameval, String discoverytopic, String DeviceClass = "\0", String unitClass = "\0", String stateClass = "\0", String HAicon = "\0", const String payloadvalue_startend_val = "", const String payloadON = "1", const String payloadOFF = "0")
{
  const String deviceid = "\"dev\":{\"ids\":\""+String(me_lokalizacja)+"\",\"name\":\""+String(me_lokalizacja)+"\",\"sw\":\"" + String(version) + "\",\"mdl\":\""+String(me_lokalizacja)+"\",\"mf\":\"" + String(MFG) + "\"}";
  const String availabityTopic = ",\"avty_t\":\"" + String(WILL_TOPIC) + "\",\"pl_avail\":\"" + String(WILL_ONLINE) + "\",\"pl_not_avail\":\"" + String(WILL_OFFLINE) + "\"";

  String unitbuilder = "\0";
  int DCswitch = 0;
  #define tempswitch 1
  #define energyswitch 2
  #define pressureswitch 3
  #define humidityswitch 4
  #define highswitch 5
  #define coswitch 6
  #define switchswitch 7
  #define powerswitch 8
  #define voltageswitch 9
  #define currentswitch 10
  #define frequencyswitch 11
  DeviceClass.toLowerCase();
  stateClass.toLowerCase();
  HAicon.toLowerCase();
  if (unitClass.length() == 0) unitClass = " ";
  if (DeviceClass.length()>0)
  {

    if (DeviceClass.indexOf("temperature") >= 0) DCswitch = tempswitch;
    if (DeviceClass.indexOf("energy") >= 0) DCswitch = energyswitch;
    if (DeviceClass.indexOf("pressure") >= 0) DCswitch = pressureswitch;
    if (DeviceClass.indexOf("humidity") >= 0) DCswitch = humidityswitch;
    if (DeviceClass.indexOf("high") >= 0) DCswitch = highswitch;
    if (DeviceClass.indexOf("co") >= 0) DCswitch = coswitch;
    if (DeviceClass.indexOf("switch") >= 0) DCswitch = switchswitch;
    if (DeviceClass.indexOf("power") >= 0) DCswitch = powerswitch;
    if (DeviceClass.indexOf("voltage") >= 0) DCswitch = voltageswitch;
    if (DeviceClass.indexOf("current") >= 0) DCswitch = currentswitch;
    if (DeviceClass.indexOf("frequency") >= 0) DCswitch = frequencyswitch;

    switch (DCswitch) {
      case tempswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "°C";
        if (HAicon.length() == 0) HAicon = "mdi:thermometer";
        break;
      }
      case powerswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "W"; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
        if (HAicon.length() == 0) HAicon = "mdi:alpha-W-box"; //fire";
        break;
      }
      case voltageswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "V";
        if (HAicon.length() == 0) HAicon = "mdi:alpha-v-box"; //fire";
        break;
      }
      case currentswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "A";
        if (HAicon.length() == 0) HAicon = "mdi:alpha-a-box"; //fire";
        break;
      }
      case frequencyswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "Hz";
        if (HAicon.length() == 0) HAicon = "mdi:sine-wave"; //fire";
        break;
      }

      case switchswitch: {
        unitbuilder = F(",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",");
        // if (unitClass.length() == 0 || unitClass == " ") unitClass = "m";
        // if (HAicon.length() == 0) HAicon = "mdi:speedometer-medium"; //fire";
        break;
      }
      case coswitch: {
        DeviceClass = "carbon_monoxide";
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "ppm";
        if (HAicon.length() == 0) HAicon = "mdi:molecule-co"; //fire";
        break;
      }
      case highswitch: {
        DeviceClass = "\0";
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "m";
        if (HAicon.length() == 0) HAicon = "mdi:speedometer-medium"; //fire";
        break;
      }
      case energyswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "kWh";
        if (stateClass.length() == 0) stateClass = "total_increasing";
        if (HAicon.length() == 0) HAicon = "mdi:lightning-bolt-circle"; //fire";
        break;
      }
      case humidityswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "%";
        if (HAicon.length() == 0) HAicon = "mdi:mdiWavesArrowUp";
        break;
      }
      case pressureswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "hPa";  ////cbar, bar, hPa, inHg, kPa, mbar, Pa, psi
        if (HAicon.length() == 0) HAicon = "mdi:mdiCarSpeedLimiter";
        break;
      }
      default: {

        break;
      }
    }
//test/lol", 0, true, "test 1");
  }
  if (DeviceClass.length() > 0) {
    unitbuilder += build_JSON_Payload(F("dev_cla"), DeviceClass, false, "\""); }
  if (unitClass.length() > 0) {
    unitbuilder += build_JSON_Payload(F("unit_of_meas"), unitClass, false, "\"");
  }
  if (stateClass.length() > 0) {
    unitbuilder += build_JSON_Payload(F("state_class"), stateClass, false, "\"");
  }
  if (HAicon.length() > 0) unitbuilder += build_JSON_Payload(F("ic"), HAicon, false, "\"");
  if (unitbuilder.length() == 0) unitbuilder += F(",");
  char mqttTopic[maxLenMQTTTopic] = {'\0'};
  String TotalName = appendname;
  TotalName += nameval;
  sprintf(mqttTopic, "%s%s/config", discoverytopic.c_str(), TotalName.c_str());
  sprintf(log_chars, "\"name\":\"%s\",\"uniq_id\": \"%s\",\"stat_t\":\"%s\",\"val_tpl\":\"{{value_json.%s}}\"%s%s,\"qos\":%s,%s",TotalName.c_str(), TotalName.c_str(), sensorswitchValTopic.c_str(), TotalName.c_str(), unitbuilder.c_str(), availabityTopic.c_str(), String(QOS).c_str(), deviceid.c_str());
  String mqttTopicStr = String(mqttTopic);
  String mqttPayloadStr = String(log_chars);
  if (publishMQTT(mqttTopicStr, mqttPayloadStr, mqtt_Retain, QOS) > 0) log_message((char*)F("HA Discovery send OK with QOS > 0")); else log_message((char*)F("HA Discovery send OK with QOS = 0"));

}
#endif
//***********************************************************************************************************************************************************************************************
#if defined enableMQTT || defined enableMQTTAsync
uint16_t publishMQTT(String &mqttTopicxx, String &mqttPayloadxx, int mqtt_Retainv = 1, u_int qos = 0) {
    char mqttPayloadSend [maxLenMQTTPayload];
    // if (mqttPayloadxx.indexOf(",")>-1) {
      sprintf(mqttPayloadSend, "{%s}", mqttPayloadxx.c_str());
    // } else {
    //   sprintf(mqttPayloadSend, "%s", mqttPayloadxx.c_str());
    // }
    #ifdef debug2
    sprintf(log_chars, "(publishMQTT) MQTT Topic %s Size: %u, Payload Size: %u", mqttTopicxx.c_str(), (u_int)mqttTopicxx.length(), (u_int)mqttPayloadxx.length());
    log_message(log_chars);
    #endif
    uint16_t packetIdSub = 0;
    if (mqttPayloadxx.length()>0) {
      #ifdef enableMQTT
      mqttclient.publish(mqttTopicxx.c_str(), mqttPayloadxx.c_str(), mqtt_Retainv);
      #endif
      #ifdef enableMQTTAsync
      packetIdSub = mqttclient.publish(mqttTopicxx.c_str(), qos , mqtt_Retainv, mqttPayloadSend);
      log_message((char*)String(packetIdSub).c_str());
      #endif
    }
    return packetIdSub;
}
#endif
//***********************************************************************************************************************************************************************************************
String build_JSON_Payload(String MQTTname, String MQTTvalue, bool notAddSeparator = false, String Payload_StartEnd = "\0") {
    String tmpbuild = F("\0");
    if (!notAddSeparator) tmpbuild += F(",");
    tmpbuild += F("\"");
    tmpbuild += MQTTname;
    tmpbuild += F("\":");
    if (Payload_StartEnd != "\0") tmpbuild += Payload_StartEnd;
    tmpbuild += MQTTvalue;
    if (Payload_StartEnd != "\0") tmpbuild += Payload_StartEnd;
    tmpbuild.trim();
    return tmpbuild;
}
//***********************************************************************************************************************************************************************************************
#include <EEPROM.h>

// Where in EEPROM?
#define CONFIG_START 100

// load whats in EEPROM in to the local CONFIGURATION if it is a valid setting
bool loadConfig() {
  int EpromPosition = 1;
  EEPROM.begin(CONFIG_START);  //Size can be anywhere between 4 and 4096 bytes.
  EEPROM.get(EpromPosition, CRTrunNumber);
  EpromPosition += sizeof(CRTrunNumber);
  if (isnan(CRTrunNumber) || (CRTrunNumber + 1) == 0) CRTrunNumber = 0;

  if(SPIFFS.exists(configfile)) {
  String dane = "\0";
    File file = SPIFFS.open(configfile,"r");
    while (file.available()) {
      dane += (char)file.read();
    }
    file.close();
    String tmpstrval = "\0";
    #if defined enableWebSocketlog || enableWebSerial
    WebSocketlog = false;
    #endif
    #ifdef enableDebug2Serial
  //  debugSerial = false;
    #endif
    #if defined enableMQTT || defined enableMQTTAsync
    sendlogtomqtt = false;
    #endif
//this block is as param from html
    if (dane.indexOf("WebSocketlog") != -1) {
      tmpstrval = getJsonVal(dane, "WebSocketlog");
      WebSocketlog = (tmpstrval.toInt() == 1);
    }
    #ifdef enableDebug2Serial
    if (dane.indexOf("debugSerial") != -1) {
      tmpstrval = getJsonVal(dane, "debugSerial");
      debugSerial  = (tmpstrval.toInt() == 1);
    }
    #endif //enableDebug2Serial
    #if defined enableMQTT || defined enableMQTTAsync
    if (dane.indexOf("sendlogtomqtt") != -1) {
      tmpstrval = getJsonVal(dane, "sendlogtomqtt");
      sendlogtomqtt = (tmpstrval.toInt() == 1);
    }
    #endif //#if defined enableMQTT || defined enableMQTTAsync
    if (dane.indexOf("SSID_Name") != -1) {
      tmpstrval = getJsonVal(dane, "SSID_Name");
      tmpstrval.replace("\"",""); strcpy(ssid, tmpstrval.c_str());
    }
    if (dane.indexOf("SSID_PAssword") != -1) {
      tmpstrval = getJsonVal(dane, "SSID_PAssword");
      tmpstrval.replace("\"","");
      strcpy(pass, tmpstrval.c_str());
    }
    #if defined enableMQTT || defined enableMQTTAsync
    if (dane.indexOf("MQTT_servername") != -1) {
      tmpstrval = getJsonVal(dane, "MQTT_servername");
      tmpstrval.replace("\"",""); strcpy(mqtt_server, tmpstrval.c_str());
    }
    if (dane.indexOf("MQTT_port_No") > -1) {
      tmpstrval = getJsonVal(dane, "MQTT_port_No");
      tmpstrval.replace("\"","");
      mqtt_port = (int) tmpstrval.toInt();
    }
    if (dane.indexOf("MQTT_username") > -1) {
      tmpstrval = getJsonVal(dane, "MQTT_username");
      tmpstrval.replace("\"","");
      strcpy(mqtt_user, tmpstrval.c_str());
    }
    if (dane.indexOf("MQTT_Password_data") > -1) {
      tmpstrval = getJsonVal(dane, "MQTT_Password_data");
      tmpstrval.replace("\"","");
      strcpy(mqtt_password, tmpstrval.c_str());
    }
    #endif
    #ifdef ENABLE_INFLUX
    if (dane.indexOf("INFLUXDB_URL") > -1) {
      tmpstrval = getJsonVal(dane, "INFLUXDB_URL");
      tmpstrval.replace("\"","");
      strcpy(influx_server, tmpstrval.c_str());
    }
    if (dane.indexOf("INFLUXDB_DB_NAME") > -1) {
      tmpstrval = getJsonVal(dane, "INFLUXDB_DB_NAME");
      tmpstrval.replace("\"","");
      strcpy(influx_database, tmpstrval.c_str());
      }
    if (dane.indexOf("INFLUXDB_USER") > -1) {
      tmpstrval = getJsonVal(dane, "INFLUXDB_USER");
      tmpstrval.replace("\"","");
      strcpy(influx_user, tmpstrval.c_str());
    }
    if (dane.indexOf("INFLUXDB_PASSWORD") > -1) {
      tmpstrval = getJsonVal(dane, "INFLUXDB_PASSWORD");
      tmpstrval.replace("\"","");
      strcpy(influx_password, tmpstrval.c_str());
    }
    if (dane.indexOf("influx_measurments") > -1) {
      tmpstrval = getJsonVal(dane, "influx_measurments");
      tmpstrval.replace("\"","");
      strcpy(influx_measurments, tmpstrval.c_str());
    }
    #endif
    if (dane.indexOf("NEWS_GET_TOPIC") > -1) {
      tmpstrval = getJsonVal(dane, "NEWS_GET_TOPIC");
      tmpstrval.replace("\"","");
      NEWS_GET_TOPIC = tmpstrval;
    }
    if (dane.indexOf("NEWStemp_json") > -1) {
      tmpstrval = getJsonVal(dane, "NEWStemp_json");
      tmpstrval.replace("\"","");
      NEWStemp_json = tmpstrval;
    }
//this block is as param from html
//next is appender from function so i use in saveconfig this function to append rest
// sprintf(log_chars,"LOG: mqtserver: %s:%i, mqtuser: %s, mqtpass: %s, NEWS_GET_TOPIC: %s, NEWStemp_json: %s", String(mqtt_server).c_str(), mqtt_port, String(mqtt_user).c_str(), String(mqtt_password).c_str(), String(NEWS_GET_TOPIC).c_str(),String(NEWStemp_json).c_str());
// log_message(log_chars);


    addusage_local_values_load(dane, EpromPosition);

    u_int CRT = 0;
    if (dane.indexOf("CRT") > -1) {
      CRT = (u_int) getJsonVal(dane, "CRT").toInt();
      if (CRT > CRTrunNumber) CRTrunNumber = CRT;
    }
    sprintf(log_chars,"Config loaded. SSID: %s, CRT: %s",String(ssid).c_str(), String(CRT).c_str());
    log_message(log_chars);
    EEPROM.end();
    return true; // return 1 if config loaded
  }
  EEPROM.end();
  return false; // return 0 if config NOT loaded
}
//***********************************************************************************************************************************************************************************************
// save the CONFIGURATION in to EEPROM
bool SaveConfig() {
  log_message((char*)F("Saving config...........................prepare "));
  unsigned int runtmp;

  EEPROM.begin(CONFIG_START);  //Size can be anywhere between 4 and 4096 bytes.
  int EpromPosition = 1;
  EEPROM.get(EpromPosition, runtmp);
  if (runtmp != CRTrunNumber) {
    EEPROM.put(EpromPosition, CRTrunNumber);
    log_message((char*)F("SaveConfig. Saved new CRT number in EPROM"));
  }
  EpromPosition += sizeof(CRTrunNumber);

  unsigned long long check_l = 0;
  unsigned long long check_w = 0;
  if(SPIFFS.exists(configfile)) {
    String dane = "\0";
    File file = SPIFFS.open(configfile,"r");
    while (file.available()) {
      dane += (char)file.read();
    }
    file.close();
    if (dane.length() > 0) {
      for (u_int i = 0; i < dane.length(); i++){
        check_l += (unsigned int) dane[i];
      }
    //check_l = check_l;  dane.length();
    }
  }

    String configSave = F("{\"config\":99");
//this block is as param from html
    #if defined enableDebug2Serial
    configSave += build_JSON_Payload(F("debugSerial"), String(debugSerial?"1":"0"), false, "\"");
    #endif
    #if defined enableWebSocketlog || defined enableWebSerial
    configSave += build_JSON_Payload(F("WebSocketlog"), String(WebSocketlog?"1":"0"), false, "\"");
    #endif
    #if defined enableMQTT || defined enableMQTTAsync
    configSave += build_JSON_Payload(F("sendlogtomqtt"), String(sendlogtomqtt?"1":"0"), false, "\"");
    //if (sendlogtomqtt) configSave += "1"; else configSave += "0";
    #endif
    configSave += build_JSON_Payload(F("CRT"), String(CRTrunNumber), false, "\"");
    configSave += build_JSON_Payload(F("SSID_Name"), String(ssid), false, "\"");
    configSave += build_JSON_Payload(F("SSID_PAssword"), String(pass), false, "\"");

    #if defined enableMQTT || defined enableMQTTAsync
    configSave += build_JSON_Payload(F("MQTT_servername"), String(mqtt_server), false, "\"");
    configSave += build_JSON_Payload(F("MQTT_port_No"), String(mqtt_port), false, "\"");
    configSave += build_JSON_Payload(F("MQTT_username"), String(mqtt_user), false, "\"");
    configSave += build_JSON_Payload(F("MQTT_Password_data"), String(mqtt_password), false, "\"");
    #endif
    #ifdef ENABLE_INFLUX
    configSave += build_JSON_Payload(F("INFLUXDB_URL"), String(influx_server), false, "\"");
    configSave += build_JSON_Payload(F("INFLUXDB_DB_NAME"), String(influx_database), false, "\"");
    configSave += build_JSON_Payload(F("INFLUXDB_USER"), String(influx_user), false, "\"");
    configSave += build_JSON_Payload(F("INFLUXDB_PASSWORD"), String(influx_password), false, "\"");
    configSave += build_JSON_Payload(F("influx_measurments"), String(influx_measurments), false, "\"");
    #endif
    configSave += build_JSON_Payload(F("NEWS_GET_TOPIC"), String(NEWS_GET_TOPIC), false, "\"");
    configSave += build_JSON_Payload(F("NEWStemp_json"), String(NEWStemp_json), false, "\"");

//this block is as param from html
//next is appender from function so i use in saveconfig this function to append rest
    configSave += addusage_local_values_save(EpromPosition);
    configSave += F("}");

  if (configSave.length() > 0) {
    for (u_int i=0; i< configSave.length() ; i++){
      check_w += (unsigned int) configSave[i];
    }
  //check_w = check_w // configSave.length();
  }

  if (check_w != check_l) {
    sprintf(log_chars,"Saving config...........................Something changed w/l: %llu/%llu", (check_w), (check_l));
    log_message(log_chars);
    if(SPIFFS.exists(configfile)) {SPIFFS.remove(configfile);}
    File file = SPIFFS.open(configfile, "w");
    if (!file){
      log_message((char*)F("Error opening file for write config..."));
      return false;
    }
    if (!file.println(configSave)) {
      log_message((char*)F("File was not written with config"));
      file.close();
      return false;
    } else {
      file.close();
    }
    EEPROM.end();
    return true;
  }
  return false;
}
//***********************************************************************************************************************************************************************************************
void RemoteCommandReceived(uint8_t *data, size_t len)
{ // for WebSerial
#ifndef enableWebSerialorSerial
  String d = "\0";
  for (size_t i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  d.trim();
  String d_oryg = d;
  d.toUpperCase();
  log_message((char*)F("------------------------ REMOTE COMMAND RECEIVED START ------------------------------------------------------------------------"), logCommandResponse);
  sprintf(log_chars, "DirectCommands RemoteCommandReceived Received: %s (dł: %s)", String(d).c_str(), String(d.length()).c_str());
  log_message(log_chars, logCommandResponse);


  if (d.indexOf("HELP")>=0)
  {
    String d1 = d;
    d1.replace("HELP","");
    d1.trim();
    if (d1.length()==0) {
      log_message((char*)F("HELP MENU.--------------------------------------------------------------------------------------"), logCommandResponse);
      String commands = F("KOMENDY: ");
      commands += F("CLEAR, RESTART, LOAD, SAVE, DIR, DEL filename, CRTRESET");
      #ifdef enableDebug2Serial
      commands += F(", SERIALLOG");
      #endif //enableDebug2Serial
      #if defined enableWebSocketlog && defined enableWebSocket
      commands += F(", WSLOG");
      #endif //defined enableWebSocketlog && defined enableWebSocket
      #if defined enableMQTT || defined enableMQTTAsync
      commands += F(", MQTTLOG");
      #endif  //#if defined enableMQTT || defined enableMQTTAsync
      commands += F(", RESET_CONFIG");
      #ifdef enableMQTT
      commands += F(", RECONNECTMQTT");
      #endif //enableMQTT
      commands += LocalVarsRemoteCommands(d, remoteHelperMenu);
      log_message((char*)commands.c_str(), logCommandResponse);
      log_message((char*)F("Dodatkowa pomoc dot. komendy po wpisaniu jej wartości np. HELP SAVE"), logCommandResponse);
    } else
    {
      if (d.indexOf("RESTART") >=0) {
        log_message((char*)F(" RESTART  -Uruchamia ponownie układ,"), logCommandResponse);
      } else
      if (d.indexOf("CLEAR") >=0) {
        log_message((char*)F(" CLEAR  -(skrot CR) Wymazuje zawartosc okna logu na websocket (JS solution),"), logCommandResponse);
      } else
      if (d.indexOf("LOAD") >=0) {
        log_message((char*)F(" LOAD    -Wymusza odczyt konfiguracji,"), logCommandResponse);
      } else
      if (d.indexOf("SAVE") >=0) {
        log_message((char*)F(" SAVE    -Wymusza zapis konfiguracji,"), logCommandResponse);
      } else
      if (d.indexOf("RESET_CONFIG") >=0) {
        log_message((char*)F(" RESET_CONFIG    -UWAGA!!!! Resetuje konfigurację do wartości domyślnych wraz z plikami www (konieczne ponowne wgranie index.html),"), logCommandResponse);
      } else
      #ifdef enableMQTT
      if (d.indexOf(F("RECONNECTMQTT")) >=0) {
        log_message((char*)F(" RECONNECTMQTT   -Dokonuje ponownej próby połączenia z bazami mqtt,"), logCommandResponse);
      } else
      #endif
      #if defined enableWebSocketlog && defined enableWebSocket
      if (d.indexOf(F("WSLOG")) >=0) {
        log_message((char*)F(" WSLOG   -Wysyła Logi do serwera Websocket."), logCommandResponse);
      } else
      #endif  //defined enableWebSocketlog && defined enableWebSocket
      #ifdef enableDebug2Serial
      if (d.indexOf(F("SERIALLOG")) >=0) {
        log_message((char*)F(" SERIALLOG   -Wysyła Logi do portu szeregowego COM."), logCommandResponse);
      } else
      #endif   //enableDebug2Serial
      #if defined enableMQTT || defined enableMQTTAsync
      if (d.indexOf(F("MQTTLOG")) >=0) {
        log_message((char*)F(" MQTTLOG   -Wysyła Logi do serwera MQTT w topiku log."), logCommandResponse);
      } else
      #endif //#if defined enableMQTT || defined enableMQTTAsync
      {
      String x = LocalVarsRemoteCommands(d, remoteHelperMenuCommand);
      }
    }
  } else
  if (d == "RESTART")
  {
    log_message((char*)F("OK. Restarting... by command..."), logCommandResponse);
    restart(F("Initiated from Remote Command RESTART..."));
  } else
  if (d == "DIR")
  {
      log_message((char*)getFSDir().c_str(), logCommandResponse);
  } else
  if (d.indexOf("DEL")==0)
  {
    d_oryg = d_oryg.substring(4);
    d_oryg.trim();
    if (d_oryg.length()>0)
    {
      if(SPIFFS.exists("/"+d_oryg)) {
        SPIFFS.remove("/"+d_oryg);
        sprintf(log_chars," Filename %s is removed.\n%s", d_oryg.c_str(), getFSDir().c_str());
        log_message(log_chars, logCommandResponse);
      } else {
        sprintf(log_chars,"Filename %s doesn't exist.\n%s", d_oryg.c_str(), getFSDir().c_str());
        log_message(log_chars, logCommandResponse);
      }
      if (("/"+d_oryg) == configfile) {
        SaveConfig();
        sprintf(log_chars,"\nFilename %s is essential !!!. So I've created new one. ;(\n%s", d_oryg.c_str(), getFSDir().c_str());
        log_message(log_chars, logCommandResponse);
      }
    }
  } else
  #if defined enableWebSocketlog && defined enableWebSocket
  if (d.indexOf("WSLOG") == 0)
  {
    sprintf(log_chars, "Actual State of sending WebSocket log: %s, %s", String(WebSocketlog ? "ENABLED" : "DISABLED").c_str(), String(WebSocketlog).c_str());
    log_message(log_chars, logCommandResponse);
    d.replace("WSLOG","");
    d.trim();
    if (d.length()>0)
    {
      if (d=="YES" || d=="ON" || d=="1" || d == "START") {
        WebSocketlog = true;
        sprintf(log_chars,"CHANGE State of sending WebSocket log: %s, %s", String(WebSocketlog ? "ENABLED" : "DISABLED").c_str(), String(WebSocketlog).c_str());
      }
      else
        if (d=="NO" || d=="OFF" || d=="0" || d == "STOP") {
        WebSocketlog = false;
        sprintf(log_chars,"CHANGE State of sending WebSocket log: %s, %s", String(WebSocketlog ? "ENABLED" : "DISABLED").c_str(), String(WebSocketlog).c_str());
      } else {
        sprintf(log_chars, "uknown value %s", d.c_str());
      }
      log_message(log_chars, logCommandResponse);

    }
  } else
  #endif
  #if defined enableMQTT || defined enableMQTTAsync
  if (d.indexOf("MQTTLOG") == 0)
  {
    sprintf(log_chars,"Actual State of sending MQTT Log: %s, %s", String(sendlogtomqtt ? "ENABLED" : "DISABLED").c_str(), String(sendlogtomqtt).c_str());
    log_message(log_chars, logCommandResponse);
    d.replace("MQTTLOG","");
    d.trim();
    if (d.length()>0)
    {
      if (d=="YES" || d=="ON" || d=="1" || d == "START") {
        sendlogtomqtt = true;
        sprintf(log_chars,"CHANGE State of sending MQTT Log: %s, %s", String(sendlogtomqtt ? "ENABLED" : "DISABLED").c_str(), String(sendlogtomqtt).c_str());
      }
      else
        if (d=="NO" || d=="OFF" || d=="0" || d == "STOP") {
        sendlogtomqtt = false;
        sprintf(log_chars,"CHANGE State of sending MQTT Log: %s, %s", String(sendlogtomqtt ? "ENABLED" : "DISABLED").c_str(), String(sendlogtomqtt).c_str());
      } else {
        sprintf(log_chars, "uknown value %s", d.c_str());
      }
      log_message(log_chars, logCommandResponse);

    }
  } else
  #endif //#if defined enableMQTT || defined enableMQTTAsync
  #ifdef enableDebug2Serial
  if (d.indexOf("SERIALLOG") == 0)
  {
    sprintf(log_chars,"Actual State of sending Serial log: %s, %s", String(debugSerial ? "ENABLED" : "DISABLED").c_str(), String(debugSerial).c_str());
    log_message(log_chars, logCommandResponse);
    d.replace("SERIALLOG","");
    d.trim();
    if (d.length()>0)
    {
      if (d=="YES" || d=="ON" || d=="1" || d == "START") {
        debugSerial = true;
        sprintf(log_chars,"CHANGE State of sending Serial log: %s, %s", String(debugSerial ? "ENABLED" : "DISABLED").c_str(), String(debugSerial).c_str());
      }
      else
        if (d=="NO" || d=="OFF" || d=="0" || d == "STOP") {
        debugSerial = false;
        sprintf(log_chars,"CHANGE State of sending Serial log: %s, %s", String(debugSerial ? "ENABLED" : "DISABLED").c_str(), String(debugSerial).c_str());
      } else {
        sprintf(log_chars, "uknown value %s", d.c_str());
      }
      log_message(log_chars, logCommandResponse);

    }
  } else
  #endif
    #ifdef enableMQTT
  if (d == "RECONNECTMQTT")
  {
    log_message((char*)F("OK. Try reconnect mqtt"), logCommandResponse);
    mqttReconnect();
  } else
    #endif
  if (d == "LOAD")
  {
    sprintf(log_chars, "Loading config from file...  ");
    log_message(log_chars, logCommandResponse);
    if(SPIFFS.exists(configfile)) {
      String dane = "\0";
      File file = SPIFFS.open(configfile,"r");
      while (file.available()) {
        dane += (char)file.read();
      }
      file.close();
      loadConfig();
    }
  } else
  if (d == "SAVE")
  {
    sprintf(log_chars, "Saving config to EEPROM memory by command... ");
    log_message(log_chars, logCommandResponse);
    SaveConfig();
  } else
  if (d == "CRTRESET") {
    CRTrunNumber = 1;
    EEPROM.begin(CONFIG_START);
    EEPROM.put(1, CRTrunNumber);
    EEPROM.end();
    SaveConfig();
  } else
  if (d == "RESET_CONFIG")
  {
    sprintf(log_chars, "RESET ALL config to DEFAULT VALUES with all www content -make FS FORMAT and restart...  ");
    log_message(log_chars, logCommandResponse);
    SPIFFS.format();
    EEPROM.begin(CONFIG_START);
    for (u_int i=0 ; i < 100; i++){
      EEPROM.put(i, "\0");
    }
    EEPROM.end();
    CRTrunNumber = 0;
    SaveConfig();
    restart(F("Initiated from Remote Command RESET_CONFIG..."));
  } else
  if (LocalVarsRemoteCommands(d, remoteHelperCommand).length() > 0)
  { //get local specific values
  } else
  {
    log_message((char*)F("Unknown command received from serial or webserial input."), logCommandResponse);
  }
  log_message((char*)F("------------------------ REMOTE COMMAND RECEIVED END ------------------------------------------------------------------------"), logCommandResponse);

#endif
}
//***********************************************************************************************************************************************************************************************
