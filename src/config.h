/*
  Topic structure is built with get/set mechanizm for
  compatibility with home assistant and to allow external
  device control
  _GET_ topics are used to publish current thermostat state
  _SET_ topics are used to control the thermostat

  Works also offline.

*/
#include <Arduino.h>
//#define smallercode


// v.2.0 Initial after move from Arduino Mega to ESP32


//#define loctmp "_TMP"     //appender for temporary name in influx and mqtt to not make trash in testing


//#define debug		//Serial Debug
//#define debug1
//#define debug2
//#define debugweb



#define MFG "MARM.pl Sp. z o.o."
#define wwwport 80
#define PL_lang
#ifndef smallercode
#define  ENABLE_INFLUX        //if defined sending to influx database is performed at time when mqtt messages is send 1313937-1115101=198836
#endif

#define enableDebug2Serial    //send log_message to websocket
#define enableWebSocketlog  //send log to websocket AND enableWebSocket MUST BE ENABLED -this replaces me webserial -files on web to build ... to get it
#define enableArduinoOTA
#define enableWebSocket      //ESPAsyncWebServer
#define doubleResDet        //  khoih-prog/ESP_DoubleResetDetector @ ^1.3.1)  check if double reset occurs -if yes stay on OTA ready
#define ENABLE_INFLUX        // tobiasschuerg/ESP8266 Influxdb @ ^3.12.0   if defined sending to influx database is performed at time when mqtt messages is send  -about 130kB of code
#define enableInfluxClient     // works as client -uses include InfluxdbClient.h not Influxdb.h
#define enableMQTTAsync     //Async MQTT   ottowinter/AsyncMqttClient-esphome @ ^0.8.6  -implements also reconnects based by wifi
//#define enableMQTT        //knolleary/PubSubClient@^2.8  --problem with connected status ---
//#define enableWebUpdate   //not implemented -not working well
//#define enableMESHNETWORK
//#define enableWebSerial     //not fully implemented and abandoned  1115101 - 1033185 = 81 916
#define enableDHT



#if defined enableWebSocketlog && not defined enableWebSocket
#undef enableWebSocketlog
#endif



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


#define maxsensors 13            //maksymalna liczba czujnikow w tabeli
#define namelength 15 //ilosc znakow z nazwy czunika
#define update_path "/update"
// Your WiFi credentials.
// Set password to "" for open networks.
#define sensitive_size 32
#define sensitive_sizeS "32"

#ifndef loctmp
#define loctmp "\0"
#endif




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
