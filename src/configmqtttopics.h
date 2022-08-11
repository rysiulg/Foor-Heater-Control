#include "Arduino.h"


//common
String BASE_TOPIC = me_lokalizacja + String(loctmp);
String LOG_TOPIC = String(BASE_TOPIC) + "/log";
String WILL_TOPIC = String(BASE_TOPIC) + "/Will";
String IP_TOPIC = String(BASE_TOPIC) + "/IP";
String STATS_TOPIC = String(BASE_TOPIC) + "/stats";
#define WILL_ONLINE "Online"
#define WILL_OFFLINE "Offline"

#define BASE_HA_TOPIC "homeassistant"
//Homeassistant Autodiscovery topics
String HA_SENSORS_TOPIC = String(BASE_HA_TOPIC) + "/sensor/" + String(BASE_TOPIC) + "/";
String HA_SWITCH_TOPIC = String(BASE_HA_TOPIC) + "/switch/" + String(BASE_TOPIC) + "/";
String HA_BINARY_TOPIC = String(BASE_HA_TOPIC) + "/binary_sensor/" + String(BASE_TOPIC) + "/";
String HA_CLIMATE_TOPIC = String(BASE_HA_TOPIC) + "/climate/" + String(BASE_TOPIC) + "/";

#define ROOM_TEMP "room"
#define QOS 0
String OT = "FL" + String(kondygnacja) + "_";
//endcommon

const String SET_LAST = F("/set");

const String ROOM_TEMPERATURE = String(ROOM_TEMP) + "_temperature";
const String ROOM_HUMIDITY = String(ROOM_TEMP) + "_humidity";
const String ROOM_PUMPSTATE = String(ROOM_TEMP) + "_pumpstate";
const String ROOM_TEMPERATURE_SETPOINT = ROOM_TEMPERATURE + "_setpoint";
String ROOM_TEMPERATURE_SETPOINT_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + ROOM_TEMPERATURE_SETPOINT; //+SET_LAST + "/set"; // t


String ROOMS_TOPIC_SENSOR = String(BASE_TOPIC) + "/" + String(ROOM_TEMP) + "/attr";


String COPUMP_GET_TOPIC = F("COWODA0/switch/boilerroom/attr");                      // temperatura outside avg NEWS
#define COPumpStatus_json "boilerroom_pump2CO"
#define WaterPumpStatus_json "boilerroom_pump1Water"

String NEWS_GET_TOPIC = F("COWODA0/sensor/boilerroom/attributes");          // pompa CO status     IF CHANGE TOPIC -CHANGE CONFIGURATION VERSION !!!!!
String NEWStemp_json = F("outside_temperature_Averange");

String BOILER_FLAME_STATUS_TOPIC = F("opentherm-thermostat/boiler/attr");                              //flme status of co gaz boiler
String BOILER_FLAME_STATUS_ATTRIBUTE = F("ot_flame_state");                              //boiler flame status of co gaz boiler
String BOILER_COPUMP_STATUS_ATTRIBUTE = F("ot_boiler_ch_state");                          //boiler pump status

//String json_temp = F("Temperature");
//String json_humid = F("Humidity");

String FLOOR1_ROOM_TOPICS [8][3] ={ {("aaa1"), ("Temperature"), ("Humidity")},
                                {("aaa2"), ("Temperature"), ("Humidity")},
                                {("aaa3"), ("Temperature"), ("Humidity")},
                                {("aaa4"), ("Temperature"), ("Humidity")},
                                {("tele/tasmota_BB04B4-34/SENSOR"), ("Temperature"), ("Humidity")},
                                {("aaa6"), ("Temperature"), ("Humidity")},
                                {("aaa7"), ("Temperature"), ("Humidity")},
                                {("aaa8"), ("Temperature"), ("Humidity")}};
String FLOOR2_ROOM_TOPICS [8][3] ={ {("bbb1"), ("Temperature"), ("Humidity")},
                                {("bbb2"), ("Temperature"), ("Humidity")},
                                {("bbb3"), ("Temperature"), ("Humidity")},
                                {("bbb4"), ("Temperature"), ("Humidity")},
                                {("bbb5"), ("Temperature"), ("Humidity")},
                                {("bbb6"), ("Temperature"), ("Humidity")},
                                {("bbb8"), ("Temperature"), ("Humidity")},
                                {("bbb8"), ("Temperature"), ("Humidity")},};

#define action_temp 0
#define action_tempset 1

void updateMQTT_Topics() {
    //me_lokalizacja was updated by kondygnacja so update all with base_topic
    BASE_TOPIC = me_lokalizacja + String(loctmp);
    OT = "FL" + String(kondygnacja) + "_";
    LOG_TOPIC = String(BASE_TOPIC) + "/log";
    WILL_TOPIC = String(BASE_TOPIC) + "/Will";
    IP_TOPIC = String(BASE_TOPIC) + "/IP";
    STATS_TOPIC = String(BASE_TOPIC) + "/stats";
    HA_SENSORS_TOPIC = String(BASE_HA_TOPIC) + "/sensor/" + String(BASE_TOPIC) + "/";
    HA_SWITCH_TOPIC = String(BASE_HA_TOPIC) + "/switch/" + String(BASE_TOPIC) + "/";
    HA_BINARY_TOPIC = String(BASE_HA_TOPIC) + "/binary_sensor/" + String(BASE_TOPIC) + "/";
    HA_CLIMATE_TOPIC = String(BASE_HA_TOPIC) + "/climate/" + String(BASE_TOPIC) + "/";
    ROOM_TEMPERATURE_SETPOINT_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + ROOM_TEMPERATURE_SETPOINT;
    ROOMS_TOPIC_SENSOR = String(BASE_TOPIC) + "/" + String(ROOM_TEMP) + "/attributes";
}