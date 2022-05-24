String BASE_TOPIC = me_lokalizacja; //jest niepelna -brakuje kondygnacji
const String BASE_HA_TOPIC = "homeassistant";
const String ROOM_TEMP = "room";
const String TEMPERATURE = "_temperature";
const String SET_LAST = "/set";
const String SENSOR = "/sensor/";
const int mqtt_Retain = 1;
const String QOS = "0";
const String OT = "FL"+String(kondygnacja)+"_";

const String ROOM_TEMPERATURE = ROOM_TEMP + TEMPERATURE;
const String ROOM_TEMPERATURE_SETPOINT = ROOM_TEMPERATURE + "_setpoint";
const String ROOM_TEMPERATURE_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + ROOM_TEMPERATURE_SETPOINT; //+SET_LAST + "/set"; // t

const String ROOMS_TOPIC_SENSOR = BASE_TOPIC + SENSOR + ROOM_TEMP + "/attributes";
const String ROOMS_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/" + ROOM_TEMP;     //+"/state"
const String ROOMS_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + ROOM_TEMP; //+"/state"

String COPUMP_GET_TOPIC = "COWoda0/switch/boilerroom/attributes";                      // temperatura outside avg NEWS
#define COPumpStatus_json "CO0_boilerroom_pump2CO"
#define WaterPumpStatus_json "CO0_boilerroom_pump1Water"

String NEWS_GET_TOPIC = "COWoda0/sensor/boilerroom/attributes";          // pompa CO status     IF CHANGE TOPIC -CHANGE CONFIGURATION VERSION !!!!!
#define NEWStemp_json "CO0_outside_temperature_Averange"

String BOILER_FLAME_STATUS_TOPIC = "opentherm-thermostat/boiler/attributes";                              //flme status of co gaz boiler
String BOILER_FLAME_STATUS_ATTRIBUTE = "ot_flame_state";                              //boiler flame status of co gaz boiler
String BOILER_COPUMP_STATUS_ATTRIBUTE = "ot_boiler_ch_state";                          //boiler pump status