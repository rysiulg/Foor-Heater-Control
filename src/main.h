//main.h
// extern "C" {
// //#include "user_interface.h"
// #include "Esp.h"
// }

//#include <Arduino.h>
//#include "dane.h"
#ifdef ESP32
#include <WiFi.h>

#include <AsyncTCP.h>
#include <AsyncUDP.h>
//#include <ESPmDNS.h>

//#include "esp_task_wdt.h"
#else
////#include <Updater.h>
//#include <ESP8266WiFi.h>
////#include <FS.h>
////#include <SPIFFS.h>
////#include <ESPmDNS.h>
//#include <WiFi.h>
//#include <AsyncTCP.h>
////#include <ESPAsyncUDP.h>
////#include <ESP8266mDNS.h>
////#include <ESPAsyncDNSServer.h>
#include <WiFiClient.h>
#endif

// for ota
#include <ESPAsyncWebServer.h>
//#include <AsyncTCP.h>
//#include <ESPAsyncUDP.h>
//#include <ESPAsyncDNSServer.h>

#include <DNSServer.h>
//#include <ESP8266mDNS.h>

#ifdef doubleResDet
#include <ESP_DoubleResetDetector.h>
#endif
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
//#include <ArduinoHA.h>  //HomeAssistant
#include <WebSerial.h>

#include "config.h"
#include "dane.h"

#ifdef ENABLE_INFLUX
  #ifdef ESP32
//  #include <HTTPClient.h>
  #include <InfluxDbClient.h>
  #else
  #include <ESP8266HTTPClient.h>
  #include <InfluxDbClient.h>
  #endif
#endif
#include <Update.h>
#ifdef enableDHT
#include <Adafruit_Sensor.h>
#include <DHT.h>
#endif

const char version[12+1] =
{
   // YY year
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
const String me_version = String(version);
const String  stopka = String(MFG)+" "+version[4]+version[5]+"-"+version[2]+version[3]+"-20"+version[0]+version[1]+" "+version[6]+version[7]+":"+version[8]+version[9];
String me_lokalizacja = "FLOORH";//pozniej dopisywane +String(kondygnacja);//+"_mqqt_MARM";
String mqttident = "CO_"; //dopisane w setup + String(kondygnacja) + "_";
//to testowo tutaj -zobacze czy to zmieni w trakcie me_lokalizacja z dopisaniem kondygnacji -jak nie to trzeba bedzie to przeniesc
String mqttdeviceid = "\"dev\":{\"ids\":\""+String("me_lokalizacja")+"\",\"name\":\""+String("me_lokalizacja")+"\",\"sw\":\"" + String(me_version) + "\",\"mdl\": \""+String("me_lokalizacja")+"\",\"mf\":\"" + String(MFG) + "\"}";






#define startrun LOW //HIGH     //active state
#define stoprun HIGH //LOW   //deactive state
#define start_digital LOW     //active state physically digitalwrite
#define stop_digital HIGH   //deactive state physically digitalwrite
#define MODEOFF 0
#define MODEHEAT 1
#define MODECOOL 2
#define MODEAUTO 3

char log_chars[256];
u_int modestate = MODEOFF;            //tryb pracy
const unsigned long extTempTimeout_ms = 180 * 1000,
                    statusUpdateInterval_ms = 60 * 1 * 1000,    //update pumps i dallas  -i check mqtt for active boiler ch pump and co pump and if active one or both change time to next value
                    statusUpdateShortenInterval_s = 60,          //update pumps i dallas  -i check mqtt for active boiler ch pump and co pump and if active one or both change time to this value
                    spOverrideTimeout_ms = 180 * 1000,
                    mqttUpdateInterval_ms = 1 * 60 * 1000,      //send data to mqtt and influxdb
                    temp_NEWS_interval_reduction_time_ms = 30 * 60 * 1000, // time to laps between temp_NEWS reduction by 5%
                    MQTTRECONNECTTIMER = 30 * 1000,      //it takes 30 secs for each mqtt server reconnect attempt usxes lastMqttReconnectAttempt
                    WIFIRETRYTIMER = 10*1000,           // switch between hotspot and configured SSID each 10 secs if SSID is lost uses lastWifiRetryTimer
                    LOOP_WAITTIME = 30*1000,
                    save_config_every = 5 * 60 * 1000,                    // time saveing config values in eeprom (15minutes) -changed to shorter time -implemented save only if value are changed
                    start_pump_delay = 120 * 1000,        //delay to start pump
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
      retTemp = InitTemp,                         // return temperature
      temp_NEWS = InitTemp,                       // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South
      cutOffTemp = 2,                      // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
      op_override = noCommandSpOverride, // boiler tempset on heat mode
      room_temp_default = 20.5,     //domyslna temp pokoju
      pressure = 0,
      humiditycor = 0,              //DHT humidity
      tempcor = 0,                  //DHT temp and mayby dual with 18b20 onboard
      tempwe = InitTemp,                  //temp. wejscia co
      histereza = 1,              //histereza dla pompy
      min_temp = 0,                    //minmalna temp poziomu
      max_temp = 0,                    //maksymalna temp poziomu -setpoint
      pumpOffVal = pumpofftemp;


String LastboilerResponseError,
       dana,
       UnassignedTempSensor,
       kondygnacja = "0";          //ident pietra

int temp_NEWS_count = 0,
    mqttReconnects = 0,
    mqtt_offline_retries = 15, // retries to mqttconnect before timewait
    pompa_pin;

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
              lastWifiRetryTimer =0,
              dhtreadtime = 0,
              start_pump =0,
              lastloopRunTime = 0;

bool receivedmqttdata = false,
     CO_PumpWorking = false,
     CO_BoilerPumpWorking = false,
     tmanual = false,
     starting = true,
     firstConnectSinceBoot = true, //if this is true there is no first connection made yet
     pump = startrun,        //stan pompy
     min_max_state = stoprun;  //stan otwarcia zamkniecia przeplywu podlogi

const int lampPin = LED_BUILTIN;

struct tempsensor {
  uint8_t addressHEX[8];
  uint8_t addressHEXnew[8];
  float tempread;                 //actual temperature
  float tempset;                  //temperature set
  float humidityread;             //actual humidity
  char nameSensor[namelength];
  int idpinout;                    //numer portu digital wyjscia sterowania.
  bool switch_state;               //is pump or other device active
  String mqtt_temp_floor_topic;
  String mqtt_temp_json;
  String mqtt_humid_json;
  String mqtt_tempset_floor_topic;
};
tempsensor room_temp[maxsensors];







AsyncWebServer webserver(wwwport);
// for ota webs

OneWire oneWire(DALLAS_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient mqttclient(espClient);

// ESP8266WebServer server(80);

#ifdef ENABLE_INFLUX
InfluxDBClient InfluxClient(INFLUXDB_URL, INFLUXDB_DB_NAME);
Point InfluxSensor(InfluxMeasurments);
#endif
#ifdef enableDHT
DHT dht(DHTPIN, DHTTYPE);
#endif
//doule reset detection
// maximum number of seconds between resets that
// counts as a double reset
#ifdef doubleResDet
  #define DRD_TIMEOUT 0.1

  // address to the block in the RTC user memory
  // change it if it collides with another usageb
  // of the address block
  #define DRD_ADDRESS 0x00
  DoubleResetDetector* drd;
#endif
DNSServer dnsServer;



//common_functions.h
void log_message(char* string);
String uptimedana(unsigned long started_local);
String getJsonVal(String json, String tofind);
bool isValidNumber(String str);
String convertPayloadToStr(byte *payload, unsigned int length);
int dBmToQuality(int dBm);
int getWifiQuality();
int getFreeMemory();
bool PayloadStatus(String payloadStr, bool state);
bool PayloadtoValidFloatCheck(String payloadStr);
float PayloadtoValidFloat(String payloadStr,bool withtemps_minmax=false, float mintemp=InitTemp, float maxtemp=InitTemp);
void restart();
String getIdentyfikator(int x);



String processor(const String var);






//main

void mqtt_callback(char *topic, byte *payload, unsigned int length);
void mqtt_reconnect();
void ReadTemperatures();
String getIdentyfikator(int x);

void setup();
void loop();
void Pump_Activate_Control ();
void GetSpecificSensorData ();

//other
bool loadConfig();
void saveConfig();
void Assign_Name_Addr_Pinout(int i, String name, String addresss, int outpin);
int convertCharToHex(char ch);
void AssignSensors();
boolean array_cmp_8( uint8_t *a, uint8_t *b);
int convertCharToHex(char ch);
void recvMsg(uint8_t *data, size_t len);
void updateInfluxDB();
void updateMQTTData();
void getRemoteTempHumid(String payloadStr, u_int roomnotmp, String json_temp, String json_humid);





//#define debugweb   //to debug getJsonVal but it clips if too much data ;(
