
//main.h


//#include <Arduino.h>
//#include "dane.h"
#ifdef ESP32
#include <WiFi.h>
#include <Update.h>
#include <AsyncTCP.h>
#include <AsyncUDP.h>
//#include <ESPmDNS.h>
//#include <AsyncDNSServer.h>
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


#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
//#include <ArduinoHA.h>  //HomeAssistant
#include <WebSerial.h>

#include "config.h"

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






#define start LOW //HIGH     //active state
#define stop HIGH //LOW   //deactive state
#define start_digital LOW     //active state physically digitalwrite
#define stop_digital HIGH   //deactive state physically digitalwrite




String UnassignedTempSensor;

struct tempsensor {
  uint8_t addressHEX[8];
  uint8_t addressHEXnew[8];
  float tempread;                 //actual temperature
  float tempset;                  //temperature set
  float humidityread;             //actual humidity
  char nameSensor[namelength];
  int idpinout;                    //numer portu digital wyjscia sterowania.
  bool switch_state;               //is pump or other device active
};

tempsensor room_temp[maxsensors];




String uptimedana(unsigned long started_local);
String processor(const String var);
bool isValidNumber(String str);



String convertPayloadToStr(byte *payload, unsigned int length);
bool isValidNumber(String str);
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void ReadTemperatures();
String getIdentyfikator(int x);

void setup();
void loop();
void Pump_Activate_Control ();
void GetSpecificSensorData ();

bool loadConfig();
void saveConfig();

void restart();
bool PayloadStatus(String payloadStr, bool state);
bool PayloadtoValidFloatCheck(String payloadStr);
float PayloadtoValidFloat(String payloadStr,bool withtemps_minmax=false, float mintemp=InitTemp, float maxtemp=InitTemp);
String getIdentyfikator(int x);

void Assign_Name_Addr_Pinout(int i, String name, String addresss, int outpin);
int convertCharToHex(char ch);
void AssignSensors();
boolean array_cmp_8( uint8_t *a, uint8_t *b);
int convertCharToHex(char ch);
void recvMsg(uint8_t *data, size_t len);
void updateMQTTData();

//#define debugweb   //to debug getJsonVal but it clips if too much data ;(
String getJsonVal(String json, String tofind);
