/*
  Topic structure is built with get/set mechanizm for
  compatibility with home assistant and to allow external
  device control
  _GET_ topics are used to publish current thermostat state
  _SET_ topics are used to control the thermostat

  Works also offline.

*/
//#define smallercode


// v.2.0 Initial after move from Arduino Mega to ESP32

//#define debug		//Serial Debug
//#define debug1
//#define debug2
//#define debugweb

#define enableWebSerial   //1115101 - 1033185 = 81 916
#define enableArduinoOTA

#define ATOMIC_FS_UPDATE
#define MFG "MARM.pl Sp. z o.o."
#define wwwport 80
#define PL_lang
#ifndef smallercode
#define  ENABLE_INFLUX        //if defined sending to influx database is performed at time when mqtt messages is send 1313937-1115101=198836
#endif

#define enableDHT

//#boiler_gas_conversion_to_m3  1

#include "sensivity-config-data.h" //it have definitions of sensivity data
#include "config-translate.h" //definitions polish/english translate
#ifndef SSID_Name
#define SSID_Name "SSID_Name"
#endif
#ifndef SSID_PAssword
#define SSID_PAssword "SSID_PAssword"
#endif
#ifndef MQTT_username
#define MQTT_username "MQTT_username"
#endif
#ifndef MQTT_Password_data
#define MQTT_Password_data "MQTT_Password_data"
#endif
#ifndef MQTT_port_No
#define MQTT_port_No 1883     //default mqtt port
#endif
#ifndef MQTT_servername
#define MQTT_servername "MQTT_servername"     //default mqtt port
#endif


#ifdef ENABLE_INFLUX
#ifndef INFLUXDB_URL
#define INFLUXDB_URL "http://localhost:8086"
#endif
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>) but I use only version 1 as default in HomeAssistant
#ifndef INFLUXDB_DB_NAME
#define INFLUXDB_DB_NAME "test"
#endif
#ifndef INFLUXDB_USER
#define INFLUXDB_USER "test"
#endif
#ifndef INFLUXDB_PASSWORD
#define INFLUXDB_PASSWORD "test"
#endif
#define InfluxMeasurments "MARMpl_Measurments"
#endif

#define InitTemp -127.00
#define maxsensors 13            //maksymalna liczba czujnikow w tabeli
#define namelength 15 //ilosc znakow z nazwy czunika
#define update_path "/update"
// Your WiFi credentials.
// Set password to "" for open networks.
#define sensitive_size 32
#define sensitive_sizeS "32"
char ssid[sensitive_size] = SSID_Name;
char pass[sensitive_size] = SSID_PAssword;

// Your MQTT broker address and credentials
char mqtt_server[sensitive_size*2] = MQTT_servername;
char mqtt_user[sensitive_size] = MQTT_username;
char mqtt_password[sensitive_size] = MQTT_Password_data;
int mqtt_port = MQTT_port_No;
String kondygnacja = "0";          //ident pietrac -initial -replaced in setup
String me_lokalizacja = "FLOORH";//pozniej dopisywane +String(kondygnacja);//+"_mqqt_MARM";
 //jest niepelna -brakuje kondygnacji
const int mqtt_Retain = 1;

// Master OpenTherm Shield pins configuration
//const int OT_IN_PIN = 26;  // for Arduino, 4 for ESP8266 (D2), 21 for ESP32
//const int OT_OUT_PIN = 26; // for Arduino, 5 for ESP8266 (D1), 22 for ESP32



/*
   current temperature topics
   if setter is used - thermostat works with external values, bypassing built-in sensor
   if no values on setter for more than 1 minute - thermostat falls back to built-in sensor
*/






// const String BOILER = "boiler";
// const String HOT_WATER = "domestic_hot_water";
// const String ROOM_OTHERS = "room_other";

// const String BOILER_TEMPERATURE = BOILER + TEMPERATURE;
// const String BOILER_MOD = BOILER+"-mode";   //tryb pracy
// const String BOILER_TEMPERATURE_RET = BOILER + TEMPERATURE + "_return";
// const String BOILER_TEMPERATURE_SETPOINT = BOILER + TEMPERATURE + "_setpoint";
// const String BOILER_CH_STATE = BOILER + "_ch_state";
// const String BOILER_SOFTWARE_CH_STATE_MODE = BOILER + "_software_ch_state_and_mode";
// const String FLAME_STATE = "flame_state";
// const String FLAME_LEVEL = "flame_level";
// const String TEMP_CUTOFF = "temp_cutoff";
// const String FLAME_W = "flame_used_energy";
// const String FLAME_W_TOTAL = "flame_used_energy_total";

// const String HOT_WATER_TEMPERATURE = HOT_WATER + TEMPERATURE;
// const String HOT_WATER_TEMPERATURE_SETPOINT = HOT_WATER + TEMPERATURE + "_setpoint";
// const String HOT_WATER_CH_STATE = HOT_WATER + "_dhw_state";
// const String HOT_WATER_SOFTWARE_CH_STATE = HOT_WATER + "_software_dhw_state";


// const String ROOM_OTHERS_PRESSURE = ROOM_OTHERS + "_pressure";

// const String BOILER_TOPIC = BASE_TOPIC + "/" + BOILER + "/attributes";
// const String HOT_WATER_TOPIC = BASE_TOPIC + "/" + HOT_WATER + "/attributes";



// const String TEMP_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_TEMPERATURE_SETPOINT + "/set";  // sp
// const String TEMP_CUTOFF_SET_TOPIC = BASE_TOPIC + "/SET/" + TEMP_CUTOFF + "/set";                    // cutOffTemp
// const String STATE_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_SOFTWARE_CH_STATE + "/set";      // enableHotWater
// const String MODE_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_SOFTWARE_CH_STATE_MODE + "/set";         // 012 auto, heat, off ch
// const String TEMP_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_TEMPERATURE_SETPOINT + "/set";    // dhwTarget


// logs topic
// const String DIAGS = "diag";
// const String DIAG_TOPIC = BASE_TOPIC + "/" + DIAGS + "/attributes";
// const String DIAG_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";
// const String DIAG_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";

// const String LOGS = "log";
// const String LOG_GET_TOPIC = BASE_TOPIC + "/" + DIAGS + "/" + LOGS;
// const String INTEGRAL_ERROR_GET_TOPIC = DIAGS + "_" + "interr";
// const String DIAGS_OTHERS_FAULT = DIAGS + "_" + "fault";
// const String DIAGS_OTHERS_DIAG = DIAGS + "_" + "diagnostic";

//Homeassistant Autodiscovery topics
// const String BOILER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";              //+"/state"
// const String BOILER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";     //+"/state"
// const String BOILER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + BOILER; //+"/state"

// const String HOT_WATER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";                 //+"/state"
// const String HOT_WATER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";        //+"/state"
// const String HOT_WATER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + HOT_WATER; //+"/state"





// setpoint topic
// const String SETPOINT_OVERRIDE = "setpoint-override";
// const String SETPOINT_OVERRIDE_SET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/set";     // op_override
// const String SETPOINT_OVERRIDE_RESET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/reset"; //





#define CONFIG_VERSION "V02" sensitive_sizeS

// Where in EEPROM?
#define CONFIG_START 32

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

const String
        tempicon=F("<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        presicon=F("<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        attiicon=F("<i class=\"fas fa-water\" style=\"color:#90add6;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        ppmicon=F("<i class=\"fas fa-tint\" style=\"color:#50ad00;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        pumpicon=F("<i class=\"fas fa-tint\" style=\"color:#500d00;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;"),
        humidicon=F("<i class=\"fas fa-humidity\" style=\"color:blue;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;");
