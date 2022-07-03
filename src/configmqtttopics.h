

String BASE_TOPIC = me_lokalizacja;
const String LOG_TOPIC = BASE_TOPIC + "/log";
const String WILL_TOPIC = BASE_TOPIC + "/Will";
const String IP_TOPIC = BASE_TOPIC + "/IP";
const String STATS_TOPIC = BASE_TOPIC + "/stats";



const String BASE_HA_TOPIC = F("homeassistant");
const String ROOM_TEMP = F("room");
const String TEMPERATURE = F("_temperature");
const String SET_LAST = F("/set");
const String SENSOR = F("/sensor/");
const int mqtt_Retain = 1;
const String QOS = F("0");
const String OT = "FL"+String(kondygnacja)+"_";

const String ROOM_TEMPERATURE = ROOM_TEMP + TEMPERATURE;
const String ROOM_HUMIDITY = ROOM_TEMP + "_humidity";
const String ROOM_PUMPSTATE = ROOM_TEMP + "_pumpstate";
const String ROOM_TEMPERATURE_SETPOINT = ROOM_TEMPERATURE + "_setpoint";
const String ROOM_TEMPERATURE_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + ROOM_TEMPERATURE_SETPOINT; //+SET_LAST + "/set"; // t

const String ROOMS_TOPIC_SENSOR = BASE_TOPIC + SENSOR + ROOM_TEMP + "/attributes";
const String ROOMS_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/" + ROOM_TEMP;     //+"/state"
const String ROOMS_HA_TOPIC_SWITCH = BASE_HA_TOPIC + "/switch/" + BASE_TOPIC + "/" + ROOM_TEMP;     //+"/state"
const String ROOMS_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + ROOM_TEMP; //+"/state"

String COPUMP_GET_TOPIC = F("COWoda0/switch/boilerroom/attributes");                      // temperatura outside avg NEWS
#define COPumpStatus_json "CO0_boilerroom_pump2CO"
#define WaterPumpStatus_json "CO0_boilerroom_pump1Water"

String NEWS_GET_TOPIC = F("COWoda0/sensor/boilerroom/attributes");          // pompa CO status     IF CHANGE TOPIC -CHANGE CONFIGURATION VERSION !!!!!
#define NEWStemp_json "CO0_outside_temperature_Averange"

String BOILER_FLAME_STATUS_TOPIC = F("opentherm-thermostat/boiler/attributes");                              //flme status of co gaz boiler
String BOILER_FLAME_STATUS_ATTRIBUTE = F("ot_flame_state");                              //boiler flame status of co gaz boiler
String BOILER_COPUMP_STATUS_ATTRIBUTE = F("ot_boiler_ch_state");                          //boiler pump status

//String json_temp = F("Temperature");
//String json_humid = F("Humidity");

const String FLOOR1_ROOM_TOPICS [8][3] ={ {("aaa1"), ("Temperature"), ("Humidity")},
                                {("aaa2"), ("Temperature"), ("Humidity")},
                                {("aaa3"), ("Temperature"), ("Humidity")},
                                {("aaa4"), ("Temperature"), ("Humidity")},
                                {("tele/tasmota_BB04B4-34/SENSOR"), ("Temperature"), ("Humidity")},
                                {("aaa6"), ("Temperature"), ("Humidity")},
                                {("aaa7"), ("Temperature"), ("Humidity")},
                                {("aaa8"), ("Temperature"), ("Humidity")}};
const String FLOOR2_ROOM_TOPICS [8][3] ={ {("bbb1"), ("Temperature"), ("Humidity")},
                                {("bbb2"), ("Temperature"), ("Humidity")},
                                {("bbb3"), ("Temperature"), ("Humidity")},
                                {("bbb4"), ("Temperature"), ("Humidity")},
                                {("bbb5"), ("Temperature"), ("Humidity")},
                                {("bbb6"), ("Temperature"), ("Humidity")},
                                {("bbb8"), ("Temperature"), ("Humidity")},
                                {("bbb8"), ("Temperature"), ("Humidity")},};

#define action_temp 0
#define action_tempset 1