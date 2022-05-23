

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
  <div class='s'><svg version='1.1' width="75px" height="75px" id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>
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
  Serial.printf("Progress: %d%%\n", (prg*100)/content_len);
  #ifdef enableWebSerial
  WebSerial.println("Progress: "+String((prg*100)/content_len));
  #endif
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  //#define UPDATE_SIZE_UNKNOWN 0XFFFFFFFF
  //uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  if (!index){
    Serial.println(String(millis())+": "+F("Update"));
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
    } else {
      Serial.println(String(millis())+": "+"Update complete");
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

void WebServers() {
  #ifdef debug
    Serial.println(F("subWerbServers..."));
  #endif
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
  // webserver.on("/" cutOffTempS, HTTP_GET, [](AsyncWebServerRequest * request) {
  //   request->send(200, "text/plain; charset=utf-8", String(cutOffTemp,1));
  // }).setAuthentication("", "");
  webserver.on("/" cutOffTempS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(temp_NEWS_count*temp_NEWS_interval_reduction_time_ms+lastNEWSSet)));
  }).setAuthentication("", "");



  webserver.on("/" do_stopkawebsiteS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", do_stopkawebsite());
  }).setAuthentication("", "");



//0  for (int room_no=0;room_no<8;room_no++) { for is not caputured;(
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(0)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[0].tempread==InitTemp ? "--.-" : String(room_temp[0].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(0)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[0].tempset,1));
      }).setAuthentication("", "");
//1  for (int room_no=0;room_no<8;room_no++) { for is not caputured;(
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(1)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[1].tempread==InitTemp ? "--.-" : String(room_temp[1].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(1)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[1].tempset,1));
      }).setAuthentication("", "");
//2  for (int room_no=0;room_no<8;room_no++) { for is not caputured;(
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(2)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[2].tempread==InitTemp ? "--.-" : String(room_temp[2].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(2)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[2].tempset,1));
      }).setAuthentication("", "");
//3  for (int room_no=0;room_no<8;room_no++) { for is not caputured;(
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(3)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[3].tempread==InitTemp ? "--.-" : String(room_temp[3].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(3)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[3].tempset,1));
      }).setAuthentication("", "");
//4  for (int room_no=0;room_no<8;room_no++) { for is not caputured;(
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(4)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[4].tempread==InitTemp ? "--.-" : String(room_temp[4].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(4)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[4].tempset,1));
      }).setAuthentication("", "");
//5  for (int room_no=0;room_no<8;room_no++) { for is not caputured;(
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(5)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[5].tempread==InitTemp ? "--.-" : String(room_temp[5].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(5)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[5].tempset,1));
      }).setAuthentication("", "");
//6  for (int room_no=0;room_no<8;room_no++) { for is not caputured;(
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(6)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[6].tempread==InitTemp ? "--.-" : String(room_temp[6].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(6)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[6].tempset,1));
      }).setAuthentication("", "");
//7  for (int room_no=0;room_no<8;room_no++) { for is not caputured;(
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(7)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[7].tempread==InitTemp ? "--.-" : String(room_temp[7].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(7)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[7].tempset,1));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(8)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[8].tempread==InitTemp ? "--.-" : String(room_temp[8].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempSetS)+getIdentyfikator(8)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8", String(room_temp[8].tempset,1));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(9)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[9].tempread==InitTemp ? "--.-" : String(room_temp[9].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(10)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[10].tempread==InitTemp ? "--.-" : String(room_temp[10].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(11)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[11].tempread==InitTemp ? "--.-" : String(room_temp[11].tempread,1)));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(roomtempS)+getIdentyfikator(12)).c_str() , HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(room_temp[12].tempread==InitTemp ? "--.-" : String(room_temp[12].tempread,1)));
      }).setAuthentication("", "");

#ifdef enableDHT
    webserver.on( ("/"+String(dhumidcor)).c_str(), HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(humiditycor,1));
      }).setAuthentication("", "");
    webserver.on( ("/"+String(dtempcor)).c_str(), HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/plain; charset=utf-8",  String(tempcor,1));
      }).setAuthentication("", "");
#endif

  webserver.on("/" NEWS_lastTimeS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(temp_NEWS_count*temp_NEWS_interval_reduction_time_ms+lastNEWSSet)));
  }).setAuthentication("", "");

  webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
    //0
    if (request->hasParam(PARAM_roomtempset0)) { //tempCOset
    message = request->getParam(PARAM_roomtempset0)->value();
    String ident = String(millis())+": Fromweb room_temp[0].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[0].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset0";
    }
//1
    if (request->hasParam(PARAM_roomtempset1)) { //tempCOset
    message = request->getParam(PARAM_roomtempset1)->value();
    String ident = String(millis())+": Fromweb room_temp[1].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[1].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset1";
    }
//2
    if (request->hasParam(PARAM_roomtempset2)) { //tempCOset
    message = request->getParam(PARAM_roomtempset2)->value();
    String ident = String(millis())+": Fromweb room_temp[2].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[2].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset2";
    }
//3
    if (request->hasParam(PARAM_roomtempset3)) { //tempCOset
    message = request->getParam(PARAM_roomtempset3)->value();
    String ident = String(millis())+": Fromweb room_temp[3].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[3].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset3";
    }
//4
    if (request->hasParam(PARAM_roomtempset4)) { //tempCOset
    message = request->getParam(PARAM_roomtempset4)->value();
    String ident = String(millis())+": Fromweb room_temp[4].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[4].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset4";
    }
//5
    if (request->hasParam(PARAM_roomtempset5)) { //tempCOset
    message = request->getParam(PARAM_roomtempset5)->value();
    String ident = String(millis())+": Fromweb room_temp[5].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[5].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset5";
    }
//6
    if (request->hasParam(PARAM_roomtempset6)) { //tempCOset
    message = request->getParam(PARAM_roomtempset6)->value();
    String ident = String(millis())+": Fromweb room_temp[6].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[6].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset6";
    }
//7
    if (request->hasParam(PARAM_roomtempset7)) { //tempCOset
    message = request->getParam(PARAM_roomtempset7)->value();
    String ident = String(millis())+": Fromweb room_temp[7].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[7].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset7";
    }
//8
    if (request->hasParam(PARAM_roomtempset8)) { //tempCOset
    message = request->getParam(PARAM_roomtempset8)->value();
    String ident = String(millis())+": Fromweb room_temp[8].tempset ";
    if (PayloadtoValidFloatCheck(message))
    {
      #ifdef debug
      Serial.print(ident);
      #endif
      #ifdef enableWebSerial
      WebSerial.print(ident);
      #endif
      room_temp[8].tempset = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
      receivedmqttdata = true;
      } else {
        #ifdef debug
        Serial.println(ident + " is not a valid number, ignoring...");
        #endif
        #ifdef enableWebSerial
        WebSerial.println(ident + " is not a valid number, ignoring...");
        #endif
        }
    } else {
          //message = "No message sent PARAM_roomtempset8";
    }
      if (request->hasParam(PARAM_MESSAGE_cutOffTempSet)) { //tempCOset
          message = request->getParam(PARAM_MESSAGE_cutOffTempSet)->value();
          String ident = String(millis())+": Fromweb cutOffTemp ";
          if (PayloadtoValidFloatCheck(message))
          {
            #ifdef debug
            Serial.print(ident);
            #endif
            #ifdef enableWebSerial
            WebSerial.print(ident);
            #endif
            cutOffTemp = PayloadtoValidFloat(message, true, cutofflo, cutoffhi);
            receivedmqttdata = true;
          } else {
            #ifdef debug
            Serial.println(ident + " is not a valid number, ignoring...");
            #endif
            #ifdef enableWebSerial
            WebSerial.println(ident + " is not a valid number, ignoring...");
            #endif
          }
      } else {
        //message = "No message sent PARAM_MESSAGE_cutOffTempSet";
      }

    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Update value");
    response->addHeader("Refresh", "1");
    response->addHeader("Location", "/");
    request->send(response);
//      request->send(200, "text/plain", "<HEAD> <meta http-equiv=\"refresh\" content=\"0;url=/\"> </head>");
//    saveConfig();
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
  #ifdef debug
	Serial.println("HTTP server started");
  #endif
}


String processor(const String var) {


  #ifdef debug
//    Serial.print(F("Start processor: "));
//    Serial.println(var);
  #endif
  if (var == "ver") {
    String a = "</B>ESP CO Server dla: <B>" + String(me_lokalizacja) + "</B><BR>v. ";
    a += me_version;
    a += "<br><font size=\"2\" color=\"DarkGreen\">";
    a += client.connected()? "MQTT "+String(msg_Connected)+": "+String(mqtt_server)+":"+String(mqtt_port) : "MQTT "+String(msg_disConnected)+": "+String(mqtt_server)+":"+String(mqtt_port) ;  //1 conn, 0 not conn
    #ifdef ENABLE_INFLUX
    a += " + INFLUXDB: "+String(INFLUXDB_DB_NAME)+"/"+String(InfluxMeasurments);
    #endif
    a += "</font>";
    return a;
  }

  if (var == "dane") {
    String a = "MAC: <B>";
    #ifdef debug
      Serial.println(F("Raport Hosta "));
    #endif
    a += PrintHex8(mac, ':', sizeof(mac) / sizeof(mac[0]));
    a += "</b>&nbsp;&nbsp;<B>";
    a += "</B>WiFi (RSSI): <B>";
    a += WiFi.RSSI();
    a += "dBm</b> CRT:";
    a += String(runNumber);
    a += "<br>";
    a += LastboilerResponseError;
    a += " Unassigned: "+UnassignedTempSensor;
    a += "<br></B>";

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
    if (kondygnacja=="1") ptr+=F("body{background-color:lightorange;}"); else ptr+=F("body{background-color:white;}");

    ptr+="#blink{font-weight:bold;font-size:20px;font-family:sans-serif;transition: 0.5s;}";//         color: #2d38be;   font-size: 20px; font-weight: bold; color: #2d38be;

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
    return String(ptr);
  }


  if (var=="bodywstaw") {
    String ptr;
    const float tempstep=0.5;
    ptr="<form action=\"/get\">";

    ptr+="<p>"+tempicon+"<span class=\"dht-labels\">"+String(Temp_NEWS)+"</span><B><span class=\"dht-labels-temp\" id=\""+String(dallThermometerS)+"\">"+String(temp_NEWS)+"</span><sup class=\"units\">&deg;C</sup></B>";
    ptr+="<font size=\"4\" color=\"blue\">"+String(ActualFrom)+"<B><span id=\""+String(NEWS_lastTimeS)+"\">"+String(uptimedana(temp_NEWS_count*temp_NEWS_interval_reduction_time_ms+lastNEWSSet))+"</span></B> </font></p>";

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
      if (room_temp[x].tempread<room_temp[x].tempset and x<8)  ptr += F("<font color=\"Red\">");
      if (room_temp[x].tempread>room_temp[x].tempset and x>=8) ptr += F("<font color=\"Blue\">");
      if (room_temp[x].switch_state) ptr += F("<p id=\"blink\")>"); else ptr+=F("<p>");
      ptr+="<span class=\"dht-labels-temp\" id=\""+String(roomtempS)+getIdentyfikator(x)+F("\">")+String(room_temp[x].tempread==InitTemp ? "--.-" : String(room_temp[x].tempread,1))+F("</span><sup class=\"units\">&deg;C</sup></B>");
      if (room_temp[x].switch_state) ptr += F("</p>"); else ptr += F("</p>"); //end blink
      if ((room_temp[x].tempread<room_temp[x].tempset and x<8) or (room_temp[x].tempread>room_temp[x].tempset and x>=8)) ptr += F("</font>");
      ptr+=F("<br>");//</td><td>";
      //ptr+=tempicon+"<span class=\"dht-labels\">"+String(room4tempset)+"</span>";
      if (room_temp[x].idpinout!=0 and room_temp[x].tempset!=0 and room_temp[x].tempread!=InitTemp)
      {
        ptr+=F("<br>");
        ptr+="<font size=\"4\" color=\"blue\"><input type=\"number\" id=\"T"+String(roomtempSetS)+getIdentyfikator(x)+F("\" min=\"")+String(roomtemplo,1)+F("\" max=\"")+String(roomtemphi,1)+F("\" step=\"")+String(tempstep,1)+F("\" value=\"")+String(room_temp[x].tempset,1)+F("\" style=\"width:auto\" onchange=\"uTI(this.value, '")+String(roomtempSetS)+getIdentyfikator(x)+F("');\"><sup class=\"units\">&deg;C</sup></B><input id=\"")+String(roomtempSetS)+getIdentyfikator(x)+F("\" type=\"range\" min=\"")+String(roomtemplo,1)+F("\" max=\"")+String(roomtemphi,1)+F("\" step=\"")+String(tempstep,1)+F("\" name=\"")+String(roomtempset)+getIdentyfikator(x)+F("\" value=\"")+String(room_temp[x].tempset,1)+F("\" style=\"width:50px\" onchange=\"uTI(this.value, 'T")+String(roomtempSetS)+getIdentyfikator(x)+F("');\">");
        ptr+=F("<input type=\"submit\" style=\"width:45px\"></font>");
        ptr+=F("</td>");
      }
      if ((x % 2) == 1) {ptr+="</tr>"; }//else ptr+="</td>";
    }
    #ifdef enableDHT
    ptr+=F("<tr><td>");
    ptr+="<p>"+humidicon+F("<span class=\"dht-labels\">")+String(humidcorstr)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dhumidcor)+F("\">&nbsp;<font color=\"Blue\">")+String(humiditycor,1)+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("<td></td");
    ptr+="<p>"+tempicon+F("<span class=\"dht-labels\">")+String(tempcorstr)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dtempcor)+F("\">&nbsp;<font color=\"blue\">")+String(tempcor,1)+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
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
    return ptr;
  }


  if (var=="scriptsectionreplace") {
    String ptr;
    String tmp;
    unsigned long int step=125;
    unsigned long int refreshtime = 9100;
    const String autoref0 = F("setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\"");
    const String autoref1 = "\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},";
    const String autoref2 = ");\n";

    ptr=F("function uTI(e,n){document.getElementById(n).value=e};\n");
    tmp=String(uptimelink);  //spanid
    refreshtime+=step; ptr+=autoref0+tmp+autoref1+String(refreshtime/2)+autoref2;
    for (int x=0;x<13;x++) {  ///was 8
      if (room_temp[x].idpinout!=0)
      {
        tmp=String(roomtempS)+getIdentyfikator(x); refreshtime+=step; ptr+=autoref0+tmp+autoref1+String(refreshtime)+autoref1;
        tmp=String(roomtempSetS)+getIdentyfikator(x); refreshtime+=step; ptr+=autoref0+tmp+autoref1+String(refreshtime)+autoref2;
      }
    }

    tmp=String(dallThermometerS); refreshtime+=step; ptr+=autoref0+tmp+autoref1+String(refreshtime)+autoref2;
    tmp=String(NEWS_lastTimeS); refreshtime+=step; ptr+=autoref0+tmp+autoref1+String(refreshtime)+autoref2;
    tmp=String(do_stopkawebsiteS); refreshtime+=step; ptr+=autoref0+tmp+autoref1+String(refreshtime)+autoref2;
    tmp=String(dhumidcor); refreshtime+=step; ptr+=autoref0+tmp+autoref1+String(refreshtime)+autoref2;
    tmp=String(dtempcor); refreshtime+=step; ptr+=autoref0+tmp+autoref1+String(refreshtime)+autoref2;

    //ptr+="if(document.getElementById(\"blink\")){var blink=document.getElementById('blink');setInterval(function(){blink.style.color=(blink.style.color=='red'?'blue':'red');}, 1200);}"; //blink
    ptr+=F("function bt(){document.querySelectorAll('.blink').forEach(e =>{setInterval(()=>{console.log(e);e.style.color=(blink.style.color=='red'?'blue':'red')},500);});}document.addEventListener('DOMContentLoaded',()=>{bt();});"); //classList.toggle('hide');


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
      if (room_temp[8].switch_state) {
        ptr += F("<i class='fas fa-fire' style='color: red'></i>"); ptr += "<span class='dht-labels'>"+String(Flame_Active_Flame_level)+F("</span><B>")+ String("flame_level",0)+F("<sup class=\"units\">&#37;</sup></B>");
        ptr += F("<br>");
      }
      if (CO_BoilerPumpWorking) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(BOILER_IS_HEATING)+F("<br></B></span></font>");
      if (CO_PumpWorking) ptr += "<font color=\"blue\"><span class='dht-labels'><B>"+String(Second_Engine_Heating_PompActive)+F("<br></B><br></span></font>");


      // if (status_Fault) ptr += "<span class='dht-labels'><B>!!!!!!!!!!!!!!!!! status_Fault !!!!!!!<br></B></span>";

      // if (status_CHActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(BOILER_IS_HEATING)+"<br></B></span></font>";
      // if (enableHotWater) ptr += "<span class='dht-labels'><B>"+String(DHW_HEAT_ON)+"<br></B></span>";
      // if (status_WaterActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(Boiler_Active_heat_DHW)+"<br></B></span></font>";
      // if (status_Cooling) ptr += "<font color=\"orange\"><span class='dht-labels'><B>"+String(CoolingMode)+"<br></B></span></font>";
      // if (status_Diagnostic) ptr += "<font color=\"darkred\"><span class='dht-labels'><B>"+String(DiagMode)+"<br></B></span></font>";

//      if (flame_time>0) ptr+= "<font color=\"green\"><span class='dht-labels'>"+String(Flame_time)+"<B>"+uptimedana(millis()-flame_time)+"<br></B><br></span></font>";
//      ptr += "<br>"+String(Flame_total)+"<B>"+String(flame_used_power_kwh,4)+"kWh</B>";
    return String(ptr);
}
//******************************************************************************************
String uptimedana(unsigned long started_local) {
  String wynik = " ";
  unsigned long  partia = millis() - started_local;
  if (partia<1000) return "< 1 "+String(t_sek)+" ";
  #ifdef debug
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
  #ifdef debug
    Serial.println(wynik);
  #endif
  //wynik += (String)partia + "/1000";  //pomijam to wartosci <1sek
  return wynik;
}

#include <EEPROM.h>



// load whats in EEPROM in to the local CONFIGURATION if it is a valid setting
bool loadConfig() {
  // is it correct?
  if (sizeof(CONFIGURATION)<1024) EEPROM.begin(1024); else EEPROM.begin(sizeof(CONFIGURATION)+128); //Size can be anywhere between 4 and 4096 bytes.
  EEPROM.get(1,runNumber);
  if (isnan(runNumber)) runNumber=0;
  runNumber++;
  //EEPROM.get(1+sizeof(runNumber),flame_used_power_kwh);
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){

  // load (overwrite) the local configuration struct
    for (unsigned int i=0; i<sizeof(configuration_type); i++){
      *((char*)&CONFIGURATION + i) = EEPROM.read(CONFIG_START + i);
    }
    room_temp[0].tempset = CONFIGURATION.roomtempset1;
    room_temp[1].tempset = CONFIGURATION.roomtempset2;
    room_temp[2].tempset = CONFIGURATION.roomtempset3;
    room_temp[3].tempset = CONFIGURATION.roomtempset4;
    room_temp[4].tempset = CONFIGURATION.roomtempset5;
    room_temp[5].tempset = CONFIGURATION.roomtempset6;
    room_temp[6].tempset = CONFIGURATION.roomtempset7;
    room_temp[7].tempset = CONFIGURATION.roomtempset8;
    room_temp[8].tempset = CONFIGURATION.roomtempset9;

    //roomtemp = CONFIGURATION.roomtemp;
    temp_NEWS = CONFIGURATION.temp_NEWS;
    strcpy(ssid, CONFIGURATION.ssid);
    strcpy(pass, CONFIGURATION.pass);
    strcpy(mqtt_server, CONFIGURATION.mqtt_server);
    strcpy(mqtt_user, CONFIGURATION.mqtt_user);
    strcpy(mqtt_password, CONFIGURATION.mqtt_password);
    mqtt_port = CONFIGURATION.mqtt_port;
    COPUMP_GET_TOPIC=(CONFIGURATION.COPUMP_GET_TOPIC);
    NEWS_GET_TOPIC=(CONFIGURATION.NEWS_GET_TOPIC);
    BOILER_FLAME_STATUS_TOPIC=(CONFIGURATION.BOILER_FLAME_STATUS_TOPIC);
    BOILER_FLAME_STATUS_ATTRIBUTE=(CONFIGURATION.BOILER_FLAME_STATUS_ATTRIBUTE);
    BOILER_COPUMP_STATUS_ATTRIBUTE=(CONFIGURATION.BOILER_COPUMP_STATUS_ATTRIBUTE);

    return true; // return 1 if config loaded
  }
  //try get only my important values

  return false; // return 0 if config NOT loaded
}

// save the CONFIGURATION in to EEPROM
void saveConfig() {

  EEPROM.put(1, runNumber);
  //EEPROM.put(1+sizeof(runNumber), flame_used_power_kwh);

  strcpy(CONFIGURATION.version,CONFIG_VERSION);
  CONFIGURATION.roomtempset1 = room_temp[0].tempset;
  CONFIGURATION.roomtempset2 = room_temp[1].tempset;
  CONFIGURATION.roomtempset3 = room_temp[2].tempset;
  CONFIGURATION.roomtempset4 = room_temp[3].tempset;
  CONFIGURATION.roomtempset5 = room_temp[4].tempset;
  CONFIGURATION.roomtempset6 = room_temp[5].tempset;
  CONFIGURATION.roomtempset7 = room_temp[6].tempset;
  CONFIGURATION.roomtempset8 = room_temp[7].tempset;
  CONFIGURATION.roomtempset9 = room_temp[8].tempset;

  CONFIGURATION.temp_NEWS = temp_NEWS;
  strcpy(CONFIGURATION.ssid,ssid);
  strcpy(CONFIGURATION.pass,pass);
  strcpy(CONFIGURATION.mqtt_server,mqtt_server);
  strcpy(CONFIGURATION.mqtt_user,mqtt_user);
  strcpy(CONFIGURATION.mqtt_password,mqtt_password);
  CONFIGURATION.mqtt_port = mqtt_port;
  strcpy(CONFIGURATION.COPUMP_GET_TOPIC,COPUMP_GET_TOPIC.c_str());
  strcpy(CONFIGURATION.NEWS_GET_TOPIC,NEWS_GET_TOPIC.c_str());
  strcpy(CONFIGURATION.BOILER_FLAME_STATUS_TOPIC,BOILER_FLAME_STATUS_TOPIC.c_str());
  strcpy(CONFIGURATION.BOILER_FLAME_STATUS_ATTRIBUTE,BOILER_FLAME_STATUS_ATTRIBUTE.c_str());
  strcpy(CONFIGURATION.BOILER_COPUMP_STATUS_ATTRIBUTE,BOILER_COPUMP_STATUS_ATTRIBUTE.c_str());

  for (unsigned int i=0; i<sizeof(configuration_type); i++)
    EEPROM.write(CONFIG_START + i, *((char*)&CONFIGURATION + i));
  EEPROM.commit();
}

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