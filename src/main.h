//main.h
// extern "C" {
// //#include "user_interface.h"
// #include "Esp.h"
// }


#include "Common_symlinkFiles\common_functions.h"

//#include <Arduino.h>
//#include "dane.h"

// #include "config.h"
// #include "localconfig.h"

//



// #include "configmqtttopics.h"


#include <OneWire.h>
#include <DallasTemperature.h>



#ifdef enableDHT
#include <Adafruit_Sensor.h>
#include <DHT.h>
#endif




#define startrun LOW //HIGH     //active state
#define stoprun HIGH //LOW   //deactive state
#define start_digital LOW     //active state physically digitalwrite
#define stop_digital HIGH   //deactive state physically digitalwrite
#define MODEOFF 0
#define MODEHEAT 1
#define MODECOOL 2
#define MODEAUTO 3


u_int modestate = MODEOFF;            //tryb pracy
const unsigned long extTempTimeout_ms = 180 * 1000,
                    statusUpdateInterval_ms = 60 * 1 * 1000,    //update pumps i dallas  -i check mqtt for active boiler ch pump and co pump and if active one or both change time to next value
                    statusUpdateShortenInterval_s = 60,          //update pumps i dallas  -i check mqtt for active boiler ch pump and co pump and if active one or both change time to this value
                    spOverrideTimeout_ms = 180 * 1000,
                    temp_NEWS_interval_reduction_time_ms = 30 * 60 * 1000, // time to laps between temp_NEWS reduction by 5%
                    MQTTRECONNECTTIMER = 30 * 1000,      //it takes 30 secs for each mqtt server reconnect attempt usxes lastMqttReconnectAttempt
                    save_config_every = 5 * 60 * 1000,                    // time saveing config values in eeprom (15minutes) -changed to shorter time -implemented save only if value are changed
                    start_pump_delay = 120 * 1000,        //delay to start pump
                    WiFiinterval = 30 * 1000;


// upper and lower bounds on heater level
//const float  noCommandSpOverride = 32; //heating water temperature for fail mode (no external temp provided) for co

float roomtemp_last = 0, // prior temperature
      ierr = 25,         // integral error
      dt = 0,            // time between measurements
      //       op = 0, //PID controller output
      cutOffTemp = 2,                      // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
//      op_override = noCommandSpOverride, // boiler tempset on heat mode
      room_temp_default = 20.5,     //domyslna temp pokoju
      pressure = 0,
      humiditycor = 0,              //DHT humidity
      tempcor = 0,                  //DHT temp and mayby dual with 18b20 onboard
      histereza = 1,              //histereza dla pompy
      min_temp = 0,                    //minmalna temp poziomu
      max_temp = 0,                    //maksymalna temp poziomu -setpoint
      pumpOffVal = pumpofftemp;


String LastboilerResponseError,
       dana,
       UnassignedTempSensor;


int
    pompa_pin;

unsigned long ts = 0, new_ts = 0, // timestamp
              lastUpdateTempPump = 0,
              lastTempSet = 0,
              lastcutOffTempSet = 0,
              lastNEWSSet = 0,
              lastmqtt_reconnect = 0,
              lastSaveConfig = 0,
              lastSpSet = 0,
              WiFipreviousMillis = 0,
              dhtreadtime = 0,
              start_pump =0;

bool CO_PumpWorking = false,
     CO_BoilerPumpWorking = false,
     tmanual = false,
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
  char nameSensor[sensitive_size];
  int idpinout;                    //numer portu digital wyjscia sterowania.
  bool switch_state;               //is pump or other device active
  String mqtt_temp_floor_topic;
  String mqtt_temp_json;
  String mqtt_humid_json;
  String mqtt_tempset_floor_topic;
};
tempsensor room_temp[maxsensors];








OneWire oneWire(DALLAS_SENSOR_PIN);
DallasTemperature sensors(&oneWire);


#ifdef enableDHT
DHT dht(DHTPIN, DHTTYPE);
#endif


const String
        tempicon=F("<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        presicon=F("<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        attiicon=F("<i class=\"fas fa-water\" style=\"color:#90add6;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        ppmicon=F("<i class=\"fas fa-tint\" style=\"color:#50ad00;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        pumpicon=F("<i class=\"fas fa-tint\" style=\"color:#500d00;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        humidicon=F("<i class=\"fas fa-humidity\" style=\"color:blue;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;");

#define CONFIG_VERSION "V02" sensitive_sizeS

// Where in EEPROM?
//#define CONFIG_START 32

typedef struct
{
  char version[6]; // place to detect if settings actually are written
  char ssid[sensitive_size];
  char pass[sensitive_size];
  char mqtt_server[sensitive_size*2];
  char mqtt_user[sensitive_size];
  char mqtt_password[sensitive_size];
  int mqtt_port;
  float roomtempset1=0,
        roomtempset2=0,
        roomtempset3=0,
        roomtempset4=0,
        roomtempset5=0,
        roomtempset6=0,
        roomtempset7=0,
        roomtempset8=0,
        roomtempset9=0,
        roomtempset10=0,
  //float roomtemp;        //now is static sensor so for while save last value
        temp_NEWS =0;
  char COPUMP_GET_TOPIC[255],  //temperatura outside avg NEWS
       NEWS_GET_TOPIC[255],   //pompa CO status
       BOILER_FLAME_STATUS_TOPIC[255],   //pompa CO status for 1st temp room sensor
       BOILER_FLAME_STATUS_ATTRIBUTE[255],   //pompa CO status for 2nd temp room sensor
       BOILER_COPUMP_STATUS_ATTRIBUTE[255];
} configuration_type;

// with DEFAULT values!
configuration_type CONFIGURATION;
configuration_type CONFTMP;





//common_functions.h

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
void Assign_Name_Addr_Pinout(int i, String name, String addresss, int outpin);
int convertCharToHex(char ch);
void AssignSensors();
boolean array_cmp_8( uint8_t *a, uint8_t *b);
int convertCharToHex(char ch);
void recvMsg(uint8_t *data, size_t len);
void getRemoteTempHumid(String payloadStr, u_int roomnotmp, String json_temp, String json_humid);



//#define debugweb   //to debug getJsonVal but it clips if too much data ;(


float tempwe = InitTemp,                  //temp. wejscia co
      retTemp = InitTemp,                         // return temperature
      temp_NEWS = InitTemp;                       // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South

#include "websrv_ota.h"
#include "mqttinflux.h"

void updateMQTT_Topics();
