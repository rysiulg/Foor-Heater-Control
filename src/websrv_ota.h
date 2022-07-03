

uint8_t mac[6] = {(uint8_t)strtol(WiFi.macAddress().substring(0,2).c_str(),0,16), (uint8_t)strtol(WiFi.macAddress().substring(3,5).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(6,8).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(9,11).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(12,14).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(15,17).c_str(),0,16)};
unsigned long  started = 0; //do mierzenia czasu uptime bez resetu

const char htmlup[] PROGMEM = R"rawliteral(
  <form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update' accept=".bin,.bin.gz"><input type='submit' value='Update'></form>)rawliteral";


String do_stopkawebsite();
//******************************************************************************************

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
#ifdef debug
  Serial.print(F("MAC Addr: "));
  Serial.println(tmp);
#endif
  //     debugA("%s",tmp);
  return tmp;
}

//*********************************************************************************************

//******************************************************************************************
//     background-color: #01DF3A;
//
//  <div class='s'><svg version='1.1' width="75px" height="75px" id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <meta http-equiv="refresh" content="3600">
  <title>%me_lokalizacja%</title>

  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">

  <style>
    %stylesectionadd%
  </style>
</head>
<body>
  <h2>%ver%</h2>
  <p>
  <sup class="units">Uptime <b><span id="%uptime%">%uptimewart%</span></B></sup>
  <br/>
<sup class="units">%dane%</sup>
  </p>
   %bodywstaw%
  <p>
    <span id="%do_stopkawebsiteS%" class="units">%stopkawebsite%</span>
    %stopkawebsite0%
  </p>
</body>
<script>
  %scriptsectionreplace%
</script>
</html>)rawliteral";
//******************************************************************************************



void handleUpdate(AsyncWebServerRequest *request) {
  request->send(200, "text/html", htmlup);
}


void handleGetTemp() {
	//digitalWrite(BUILTIN_LED, 1);
	//webserver.send(200, "text/plain", String(123));
	//digitalWrite(BUILTIN_LED, 0);
}

//#include <Update.h>
size_t content_len;
void printProgress(size_t prg, size_t sz) {
  sprintf(log_chars, "Progress: %d%%\n", (prg*100)/content_len);
  log_message(log_chars);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  //#define UPDATE_SIZE_UNKNOWN 0XFFFFFFFF
  //uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  if (!index){
    log_message((char*)F("Update starts"));
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "15");
    response->addHeader("Location", "/");
    request->send(response);
    content_len = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = U_FLASH; //(filename.indexOf("spiffs") > -1) ? U_SPIFFS : U_FLASH;
//    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)){
      Update.printError(Serial);
    }
  }
  //Serial.println(F("Write data..."));
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }
  if (final) {
    if (!Update.end(true)){
      Update.printError(Serial);
      log_message(log_chars);
    } else {
      log_message((char*)F("Update complete"));
      Serial.flush();
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
      response->addHeader("Refresh", "20");
      response->addHeader("Location", "/");
      request->send(response);
      delay(100);
      ESP.restart();
    }
  }
}

void starthttpserver() {
  webserver.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", htmlup);
    }).setAuthentication("", "");
  webserver.on("/doUpdate", HTTP_POST,
      [&](AsyncWebServerRequest *request) {
      // the request handler is triggered after the upload has finished...
      // create the response, add header, and send response
      AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
      },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
                  size_t len, bool final) {handleDoUpdate(request, filename, index, data, len, final);}
  ).setAuthentication("", "");;


  webserver.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html; charset=utf-8",  index_html, processor);
  }).setAuthentication("", "");
    // Send a GET request to <IP>/get?message=<message>
  webserver.on("/" uptimelink , HTTP_GET, [](AsyncWebServerRequest * request) {
  //  request.setAuthentication("", "");
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(started)));
  }).setAuthentication("", "");
  webserver.on("/" dallThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(temp_NEWS));
  }).setAuthentication("", "");
  webserver.on("/" tempCORETThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(retTemp,1));
  }).setAuthentication("", "");



  webserver.on("/" do_stopkawebsiteS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", do_stopkawebsite());
  }).setAuthentication("", "");

  for (u_int x=0;x<maxsensors;x++)
  {
    String fragment = String(room_temp[x].nameSensor);
    fragment.replace(sepkondname,"_");
    webserver.on( ("/" + fragment).c_str() , HTTP_GET, [x](AsyncWebServerRequest * request) { //roomtempS + getident
    request->send(200, "text/plain; charset=utf-8", String(room_temp[x].tempread==InitTemp ? "--.-" : String(room_temp[x].tempread,1)));
    }).setAuthentication("", "");
    webserver.on( ("/" + fragment + appendSet).c_str() , HTTP_GET, [x](AsyncWebServerRequest * request) {
      request->send(200, "text/plain; charset=utf-8", String(room_temp[x].tempset,1));
    }).setAuthentication("", "");
  }

#ifdef enableDHT
    webserver.on( ("/"+String(dhumidcor)).c_str(), HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(humiditycor,1));
      }).setAuthentication("", "");

    webserver.on( ("/"+String(dtempcor)).c_str(), HTTP_GET, [](AsyncWebServerRequest * request) {
        String tempw = "\0";
        if (millis()-dhtreadtime > (60*1000)) tempw="<span class='units'>  Last update was "+uptimedana(dhtreadtime)+" ago</span>";
        request->send(200, "text/plain; charset=utf-8",  String(tempcor,1)+tempw);
      }).setAuthentication("", "");
#endif

  webserver.on("/" NEWS_lastTimeS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(lastNEWSSet)));
  }).setAuthentication("", "");



  webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
    for (u_int x=0;x<maxsensors;x++)
    {
      String fragment = String(room_temp[x].nameSensor)+ appendSet;
      fragment.replace(sepkondname,"_");
      if (request->hasParam((fragment ).c_str()))
      {
        message = request->getParam((fragment).c_str())->value();
        log_message((char*)message.c_str());
        receivedmqttdata = getRemoteTempHumid(message, x, action_tempset);
      }else {
          //message = "No message sent PARAM_roomtempset0";
      }
    }
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Update value");
    response->addHeader("Refresh", "1");
    response->addHeader("Location", "/");
    request->send(response);
  });
    // Send a POST request to <IP>/post with a form field message set to <message>
/*     webserver.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });
 */
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Server",me_lokalizacja);
  DefaultHeaders::Instance().addHeader("Title",me_lokalizacja);

	webserver.begin();
  log_message((char*)F("HTTP server started"));
}

String processor(const String var) {


  #ifdef debug
//    Serial.print(F("Start processor: "));
//    Serial.println(var);
  #endif
  if (var == "ver") {
    String a = F("</B>ESP CO Server dla: <B>");
    a += String(me_lokalizacja);
    a += F("</B><BR>v. ");
    a += me_version;
    a += F("<br><font size=\"2\" color=\"DarkGreen\">");
    a += mqttclient.connected()? "MQTT "+String(msg_Connected)+": "+String(mqtt_server)+":"+String(mqtt_port) : "MQTT "+String(msg_disConnected)+": "+String(mqtt_server)+":"+String(mqtt_port) ;  //1 conn, 0 not conn
    #ifdef ENABLE_INFLUX
    a += " + INFLUXDB: "+String(INFLUXDB_DB_NAME)+"/"+String(InfluxMeasurments);
    #endif
    a += F("</font>");
    return a;
  }

  if (var == "dane") {
    String a = F("MAC: <B>");
    #ifdef debug
      Serial.println(F("Raport Hosta "));
    #endif
    a += PrintHex8(mac, ':', sizeof(mac) / sizeof(mac[0]));
    a += F("</b>&nbsp;&nbsp;<B>");
    a += F("</B>WiFi (RSSI): <B>");
    a += WiFi.RSSI();
    a += F("dBm</b> CRT:");
    a += String(runNumber);
    a += F(", Hall: ");
    a += String(hallRead());
    a += F("Gauss");
    a += F("<br>");
    a += LastboilerResponseError;
    a += F(" Unassigned: ");
    a += UnassignedTempSensor;
    a += F("<br></B>");

    #ifdef debug
      Serial.println(F("Raport Hosta po read_eprom: "));
    #endif
    return String(a);
  }

  if (var == "uptimewart") {
    return String(uptimedana(started));
  }
  if (var == "me_lokalizacja") {
    return String(me_lokalizacja);
  }

  if (var == "uptime") {
    return String(uptimelink);
  }
  if (var == "do_stopkawebsiteS") return String(do_stopkawebsiteS);

  if (var=="stylesectionadd") {
    String ptr;
    ptr=F("html{font-family:Arial;display:inline-block;margin:0 auto;text-align:center}\
    h2{font-size:2.1rem}\
    p{font-size:2.2rem}\
    .units{font-size:1.1rem}\
    .dht-labels{font-size:1.3rem;vertical-align:middle;padding-bottom:8px}\
    .dht-labels-temp{font-size:3.3rem;font-weight:700;vertical-align:middle;padding-bottom:8px}\
    table,td,th{border-color:green;border-collapse:collapse;border-style:outset;margin-left:auto;margin-right:auto;border:0;text-align:center;padding-left: 5px;padding-right: 10px;padding-top: 5px;padding-bottom: 10px;}\
    input{margin:.4rem}\
    td{height:auto;width:auto}");
    if (kondygnacja=="1") ptr+=F("body{background-color:LightSalmon;}"); else ptr+=F("body{background-color:NavajoWhite;}");

    ptr+="#blink{font-weight:bold;font-size:20px;font-family:sans-serif;transition: 0.5s;}";//         color: #2d38be;   font-size: 20px; font-weight: bold; color: #2d38be;
    #ifdef debug2
    Serial.println(var+": "+ptr.length());
    #endif
    return String(ptr);
  }

  if (var == "stopkawebsite") {
    return do_stopkawebsite();
  }
  if (var == "stopkawebsite0") {
    String ptr;
      ptr = "<br><span class='units'><a href='"+String(update_path)+"' target=\"_blank\">"+String(Update_web_link)+"</a> &nbsp; &nbsp;&nbsp;";
      #ifdef enableWebSerial
      ptr += "<a href='/webserial' target=\"_blank\">"+String(Web_Serial)+"</a>&nbsp;";
      #endif
      ptr += "<br>&copy; ";
      ptr += stopka;
      ptr += "<br>";
    #ifdef debug2
    Serial.println(var+": "+ptr.length());
    #endif
    return String(ptr);
  }


  if (var=="bodywstaw") {
    String ptr;
    const float tempstep=0.5;
    ptr="<form action=\"/get\">";

    ptr+="<p>"+tempicon+"<span class=\"dht-labels\">"+String(Temp_NEWS)+"</span><B><span class=\"dht-labels-temp\" id=\""+String(dallThermometerS)+"\">"+String(temp_NEWS)+"</span><sup class=\"units\">&deg;C</sup></B>";
    ptr+="<font size=\"4\" color=\"blue\">"+String(ActualFrom)+"<B><span id=\""+String(NEWS_lastTimeS)+"\">"+String(uptimedana(lastNEWSSet))+"</span></B> </font></p>";

    ptr+="<p><table><tr>";
    // ptr+="<td><B><LABEL FOR=\"BOILMOD\">"+String(Boler_mode)+"</LABEL></B><br><INPUT TYPE=\"Radio\" ID=\"BOILMOD\" Name=\"boilermodewww\" Value=\"2\" "+String(automodeCO?"Checked":"")+">"+String(Automatic_mode)+"</td>";
    // ptr+="<td><B><LABEL FOR=\"HWMOD\">"+String(DHW_Mode)+"</LABEL></B><br><INPUT TYPE=\"Radio\" ID=\"HWMOD\" Name=\"boilerhwwww\" Value=\"1\" "+String(enableHotWater?"Checked":"")+">"+String(Heating)+"</td>";
    // ptr+="</tr><tr><td><INPUT TYPE=\"Radio\" ID=\"BOILMOD\" Name=\"boilermodewww\" Value=\"1\" "+String(automodeCO?"":"Checked")+">"+String(Heating)+"/"+String(Off)+"</td>";
    // ptr+="<td><INPUT TYPE=\"Radio\" ID=\"HWMOD\" Name=\"boilerhwwww\" Value=\"0\" "+String(enableHotWater?"":"Checked")+">"+String(Off)+"</td>";
    ptr+="</tr>";


    for (int x=0;x<maxsensors;x++) {
      if ((x % 2) == 0) {ptr+=F("<tr>");}//else ptr+="<td>";
      ptr+=F("<td>");
      ptr+=tempicon+"<span class=\"dht-labels\">"+String(room_temp[x].nameSensor)+F(" id")+getIdentyfikator(x)+F("</span>");
      ptr+=F("<br><B>");
      if (modestate == MODEHEAT and x<8 and room_temp[x].switch_state==startrun)  ptr += F("<font color=\"Red\">"); //room_temp[x].tempread < room_temp[x].tempset
      if (modestate == MODECOOL and x<8 and room_temp[x].switch_state==startrun)  ptr += F("<font color=\"Green\">"); //room_temp[x].tempread > (room_temp[x].tempset + histereza)
      String fragment = String(room_temp[x].nameSensor);
      fragment.replace(sepkondname,"_");
      if (room_temp[x].tempread>room_temp[x].tempset and x>=8) ptr += F("<font color=\"Blue\">");
      if (room_temp[x].switch_state) ptr += F("<p id=\"blink\">"); else ptr+=F("<p>");
      ptr+="<span class=\"dht-labels-temp\" id=\"" + fragment + F("\">")+String(room_temp[x].tempread==InitTemp ? "--.-" : String(room_temp[x].tempread,1))+F("</span><sup class=\"units\">&deg;C</sup></B>");
      if (room_temp[x].switch_state) ptr += F("</p>"); else ptr += F("</p>"); //end blink
      ptr += F("</font>");
      ptr+=F("<br>");//</td><td>";
      if (room_temp[x].humidityread >0) ptr +="<B>Humid: "+String(room_temp[x].humidityread,1)+"%</B<br>";
      //ptr+=tempicon+"<span class=\"dht-labels\">"+String(room4tempset)+"</span>";
      if ( x < limitsetsensor) //room_temp[x].idpinout!=-1 and room_temp[x].tempset != 0 and room_temp[x].tempread != InitTemp and
      {
        ptr+=F("<br>");
        ptr+="<font size=\"4\" color=\"blue\"><input type=\"number\" id=\"T" + fragment + appendSet+F("\" min=\"")+String(roomtemplo,1)+F("\" max=\"")+String(roomtemphi,1)+F("\" step=\"")+String(tempstep,1)+F("\" value=\"")+String(room_temp[x].tempset,1)+F("\" style=\"width:auto\" onchange=\"uTI(this.value, '")+fragment +appendSet+F("');\"><sup class=\"units\">&deg;C</sup></B><input id=\"")+fragment+appendSet+F("\" type=\"range\" min=\"")+String(roomtemplo,1)+F("\" max=\"")+String(roomtemphi,1)+F("\" step=\"")+String(tempstep,1)+F("\" name=\"")+ fragment +appendSet+F("\" value=\"")+String(room_temp[x].tempset,1)+F("\" style=\"width:50px\" onchange=\"uTI(this.value, 'T")+fragment+appendSet+F("');\">");
        ptr+=F("<input type=\"submit\" style=\"width:45px\"></font>");
        ptr+=F("</td>");
      }
      if ((x % 2) == 1) {ptr+="</tr>"; }//else ptr+="</td>";
    }


    #ifdef enableDHT
    ptr+=F("<tr><td>");
    ptr+="<p>"+humidicon+F("<span class=\"dht-labels\">")+String(humidcorstr)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dhumidcor)+F("\">&nbsp;<font color=\"Blue\">")+String(humiditycor,1)+F("</font></span><sup class=\"units\">&#37;</sup></B></p>");
    ptr+=F("<td></td");
    ptr+="<p>"+tempicon+F("<span class=\"dht-labels\">")+String(tempcorstr)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dtempcor)+F("\">&nbsp;<font color=\"blue\">")+String(tempcor,1)+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
    if (millis()-dhtreadtime > (60*1000)) ptr+="<span class='units'>  Last update was "+uptimedana(dhtreadtime)+" ago</span>";
    ptr+=F("</td></tr>");
    #endif


  //  ptr+="<tr><td>";

  //   ptr+=tempicon+"<span class=\"dht-labels\">"+String(Outside_Cutoff_Below)+"</span>";
  //   ptr+="<br>";
  //   ptr+="<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T"+String(cutOffTempS)+"\" min=\""+String(cutofflo,1)+"\" max=\""+String(cutoffhi,1)+"\" step=\""+String(tempstep,1)+"\" value=\""+String(cutOffTemp,1)+"\" style=\"width:auto\" onchange=\"uTI(this.value, '"+String(cutOffTempS)+"');\"><sup class=\"units\">&deg;C</sup></B><input id=\""+String(cutOffTempS)+"\" type=\"range\" min=\""+String(cutofflo,1)+"\" max=\""+String(cutoffhi,1)+"\" step=\""+String(tempstep,1)+"\" name=\""+String(PARAM_MESSAGE_cutOffTempSet)+"\" value=\""+String(cutOffTemp,1)+"\" style=\"width:50px\" onchange=\"uTI(this.value, 'T"+String(cutOffTempS)+"');\">";
  //   ptr+="<input type=\"submit\" style=\"width:45px\"></font>";
  //   ptr+="</td></tr>";

    ptr+=F("</table></form><br>");

    ptr += F("<p id=\"blink\"></p>"); //to avoid error of no finded
    #ifdef debug2
    Serial.println(var+": "+ptr.length());
    #endif
    return ptr;
  }


  if (var=="scriptsectionreplace") {
    String ptr;
    String tmp;
    unsigned long int step=125;
    unsigned long int refreshtime = 9100;
    const String autoref0 = F("setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\"");
    const String autoref1 = F("\").innerHTML=this.responseText)},e.open(\"GET\",\"/");
    const String autoref1a = F("\",!0),e.send()},");
    const String autoref2 = F(");\n");

    ptr = F("function uTI(e,n){document.getElementById(n).value=e};\n");
    tmp = String(uptimelink);  //spanid
    refreshtime += step; ptr += autoref0 + tmp + autoref1 + tmp + autoref1a + String(refreshtime/2) + autoref2;
    for (int x=0; x<maxsensors; x++) {  ///was 8
      // if (room_temp[x].idpinout != -1 and room_temp[x].tempset!=0)
      // {
        String fragment = String(room_temp[x].nameSensor);
        fragment.replace(sepkondname,"_");
        tmp = fragment; refreshtime+=step; ptr+=autoref0 + tmp + autoref1 + tmp + autoref1a + String(refreshtime) + autoref2;
        if (x < limitsetsensor) {tmp = fragment+appendSet; refreshtime+=step; ptr+=autoref0 + tmp + autoref1 + tmp + autoref1a + String(refreshtime) + autoref2;}
      // }
    }

    tmp = String(dallThermometerS); refreshtime += step; ptr += autoref0 + tmp + autoref1 + tmp + autoref1a + String(refreshtime) + autoref2;
    tmp = String(NEWS_lastTimeS); refreshtime += step; ptr += autoref0 + tmp + autoref1 + tmp + autoref1a + String(refreshtime) + autoref2;
    tmp = String(do_stopkawebsiteS); refreshtime += step; ptr += autoref0 + tmp + autoref1 + tmp + autoref1a + String(refreshtime) + autoref2;
    tmp = String(dhumidcor); refreshtime += step; ptr += autoref0 + tmp + autoref1 + tmp + autoref1a + String(refreshtime) + autoref2;
    tmp = String(dtempcor); refreshtime += step; ptr += autoref0 + tmp + autoref1 + tmp + autoref1a + String(refreshtime) + autoref2;

    //ptr+="if(document.getElementById(\"blink\")){var blink=document.getElementById('blink');setInterval(function(){blink.style.color=(blink.style.color=='red'?'blue':'red');}, 1200);}"; //blink
    ptr += F("function bt(){document.querySelectorAll('.blink').forEach(e =>{setInterval(()=>{console.log(e);e.style.color=(blink.style.color=='red'?'blue':'red')},500);});}document.addEventListener('DOMContentLoaded',()=>{bt();});"); //classList.toggle('hide');

    #ifdef debug2
    Serial.println(var+": "+ptr.length());
    #endif
    return String(ptr);
  }
  #ifdef debug
//    Serial.print(F("End processor "));
//    Serial.println(var);
  #endif
  return String();

}

String do_stopkawebsite() {
      String ptr;
      ptr = "&nbsp;";
      if (pump==startrun) {
        ptr += F("<i class='fas fa-fire' style='color: red'></i>"); ptr += "<span class='dht-labels'>"+String(FloorPumpActive)+F("</span><B>")+F("<sup class=\"units\"> </sup></B>");
        ptr += F("<br>");
      }
      if (CO_BoilerPumpWorking) {ptr += F("<font color=\"red\"><span class='dht-labels'><B>"); ptr += String(BOILER_IS_HEATING); ptr += F("<br></B></span></font>");}
      if (CO_PumpWorking) {ptr += F("<font color=\"blue\"><span class='dht-labels'><B>"); ptr += String(Second_Engine_Heating_PompActive); ptr += F("<br></B><br></span></font>");}

      ptr += F("<font color=\"blue\"><span class='dht-labels'><B>"); ptr += String(sMODESTATE)+" "+String(modestate==MODEHEAT?sMODESTATE_HEAT:modestate==MODECOOL?sMODESTATE_COOL:modestate==MODEOFF?sMODESTATE_OFF:String(modestate)); ptr += F("<br></B><br></span></font>");

      // if (status_Fault) ptr += "<span class='dht-labels'><B>!!!!!!!!!!!!!!!!! status_Fault !!!!!!!<br></B></span>";

      // if (status_CHActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(BOILER_IS_HEATING)+"<br></B></span></font>";
      // if (enableHotWater) ptr += "<span class='dht-labels'><B>"+String(DHW_HEAT_ON)+"<br></B></span>";
      // if (status_WaterActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(Boiler_Active_heat_DHW)+"<br></B></span></font>";
      // if (status_Cooling) ptr += "<font color=\"orange\"><span class='dht-labels'><B>"+String(CoolingMode)+"<br></B></span></font>";
      // if (status_Diagnostic) ptr += "<font color=\"darkred\"><span class='dht-labels'><B>"+String(DiagMode)+"<br></B></span></font>";

//      if (flame_time>0) ptr+= "<font color=\"green\"><span class='dht-labels'>"+String(Flame_time)+"<B>"+uptimedana(millis()-flame_time)+"<br></B><br></span></font>";
//      ptr += "<br>"+String(Flame_total)+"<B>"+String(flame_used_power_kwh,4)+"kWh</B>";
      ptr += F("<span class='dht-labels'>Temperatura progu włączenia/wyłączenia pompy: <B>"); ptr += String(pumpOffVal); ptr += F("</B></span><br>");

          ptr += F("<br><br> ## Free memory: ");
    ptr += getFreeMemory();
    ptr += F("% ");
    ptr += ESP.getFreeHeap();
    ptr += F(" bytes ## Wifi: ");
    ptr += getWifiQuality();
    ptr += F("% ## Mqtt reconnects: ");
    ptr += mqttReconnects;
    return String(ptr);
}
