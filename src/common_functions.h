
//#include <Arduino.h>

//#include "main.h"






//***********************************************************************************************************************************************************************************************
void log_message(char* string)  //         log_message((char *)"WiFi lost, but softAP station connecting, so stop trying to connect to configured ssid...");
{
  #include "configmqtttopics.h"
  String send_string = String(millis()) + F(": ") + String(string);
  #ifdef debug
    Serial.println(send_string);
  #endif
  #ifdef enableWebSerial
    if (starting == false) {WebSerial.println(send_string);}
  #endif
  if (mqttclient.connected())
  {
    if (!mqttclient.publish(LOG_TOPIC.c_str(), string)) {
      Serial.print(millis());
      Serial.print(F(": "));
      Serial.println(F("MQTT publish log message failed!"));
      mqttclient.disconnect();
    }
  }
//   if (webSocket.connectedClients() > 0) {
//     webSocket.broadcastTXT(string, strlen(string));
//   }
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
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": Json "+json+"  No mqttident contain searched value of "+tofind);
    #endif
  } else
  {
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": Inproper Json format or null: "+json+" to find: "+tofind);
    #endif
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
String convertPayloadToStr(byte *payload, unsigned int length)
{
  char s[length + 1];
  s[length] = 0;
  for (int i = 0; i < length; ++i)
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
    sprintf(log_chars, "Value is not a valid number, ignoring... payload: %s", payloadStr);
    log_message(log_chars);
    return InitTemp;
  } else
  {
    if (!withtemps_minmax)
    {
      sprintf(log_chars, "Value is valid number without minmax: %s", String(valuefromStr,2));
      log_message(log_chars);
      return valuefromStr;
    } else {
      if (valuefromStr>maxtemp and maxtemp!=InitTemp) valuefromStr = maxtemp;
      if (valuefromStr<mintemp and mintemp!=InitTemp) valuefromStr = mintemp;
      sprintf(log_chars, "Value is valid number: %s", String(valuefromStr,2));
      log_message(log_chars);
      return valuefromStr;
    }
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
    saveConfig();
    WiFi.persistent(true);
    WiFi.disconnect();
    WiFi.persistent(false);
    delay(4000);
    restart();

  }
  #endif
}
