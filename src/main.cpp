//Main.cpp



#include "main.h"

//to read bus voltage in stats
//ADC_MODE(ADC_VCC);


void ReadTemperatures()
{
  #ifdef enableDHT
  dht.read(true);
  #endif

  bool assignedsensor=false;
  UnassignedTempSensor="";
   String addrstr = "";
  //read temperatures
  sensors.setWaitForConversion(true); //
  sensors.requestTemperatures();        //Send the command to get temperatures
  //sensors.setWaitForConversion(false); // switch to async mode
  uint8_t  addr[8];
  float temp1w;
  int count = sensors.getDS18Count();
  if (count == 0) count = sensors.getDeviceCount();
  sprintf(log_chars, "Reading 1wire 18B20 and DHT sensors... Sensors Count: %s ...", String(count,0).c_str());
  log_message(log_chars);
  if (count==0) count = maxsensors;
  String temptmp = "";
   log_message(log_chars);
  for (int j = 0; j < count; j++)
  {
    temp1w = sensors.getTempCByIndex(j);
    addrstr="\0";
    sensors.getAddress(addr, j);
    for (int i1 = 0; i1 < 8; i1++) if (String(addr[i1], HEX).length() == 1) { addrstr += F("0"); addrstr += String(addr[i1], HEX); } else { addrstr += String(addr[i1], HEX); } //konwersja HEX2StringHEX
      //zapisanie do zmiennej addr[8], addrstr, aktualiozacja index wskazany przez j i aktualozacja temp1w.
    for (int z = 0; z < maxsensors; z++)
    {
      if (array_cmp_8(room_temp[z].addressHEX, addr, sizeof(room_temp[z].addressHEX) / sizeof(room_temp[z].addressHEX[0]),sizeof(addr) / sizeof(addr[0])) == true)
      {
        assignedsensor = true;
        if (check_isValidTemp(temp1w)) room_temp[z].tempread = temp1w;
      }
    }
    if (!assignedsensor and UnassignedTempSensor.indexOf(addrstr)==-1 and check_isValidTemp(temp1w)) {sprintf(log_chars, ";%s %f", addrstr, temp1w); UnassignedTempSensor += log_chars;}
    assignedsensor=false;
    if (check_isValidTemp(temp1w)) {
      sprintf(log_chars, "       %i: 18B20 ROM= %s, temp: %f\n", j, addrstr, temp1w);
      temptmp += log_chars;
    }
  }
  sprintf(log_chars,"Collecting 18B20 ROMS and temps:%s ...", temptmp.c_str());
  log_message(log_chars);

  if (UnassignedTempSensor.length() > 0)
  {
    sprintf(log_chars, "Unassigned Sensors: %s ...", UnassignedTempSensor);
    log_message(log_chars);
  }

  #ifdef enableDHT
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) or isnan(t)) {
    log_message((char*)F("Failed to read from DHT sensor!"));
    //if (isnan(t)) t=InitTemp;
    //if (isnan(h)) h=0;
    } else {

      dhtreadtime = millis();
      if (check_isValidTemp(t) and t != 0 and h != 0) {
        tempcor = t; //to check ds18b20 also ;)
        humiditycor = h;
      }
    }

  sprintf(log_chars, "Get DHT values t= %f, humid: %f", t, h);
  log_message(log_chars);

  #endif
}


void setup()
{
  pinMode(choosepin, INPUT_PULLUP);
  if (digitalRead(choosepin)==1) {
    kondygnacja = "2";
  } else {
    kondygnacja = "1";
  }
  AssignSensors();  //po tym ginie mi me_lokalizacja
  me_lokalizacja += kondygnacja;//+"_mqqt_MARM";
  updateMQTT_Topics();
  MainCommonSetup();

  // pinMode(lampPin, OUTPUT );
  // pinMode(choosepin,INPUT_PULLUP);    // sets the digital pin 13 as output
  //get Configuration floor 1 or 2 -if set is 2
  sprintf(log_chars,"choosePin: %i,  status: %i,  czyli: %s,  - %s  : %s", (int)choosepin, (int)digitalRead(choosepin), String(digitalRead(choosepin)? "Kondygnacja 2":"Kondygnacja 1").c_str(), kondygnacja.c_str(), me_lokalizacja.c_str());
  log_message(log_chars);
 // pinMode(choosepin, DISABLED );


  ts = millis();
  lastTempSet = -extTempTimeout_ms;
  log_message((char*)F("18B20 begin"));
  Serial.println("18B20 begin");
   // Init DS18B20 sensor
  sensors.begin();
  #ifdef enableDHT
  dht.begin();
  #endif
  sensors.setWaitForConversion(true); //
    //sensors.setWaitForConversion(true); //
  sensors.requestTemperatures();
  log_message((char*)F("Setup Finished...."));
}

  #define abs(x) ((x)>0?(x):-(x))

void loop()
{
  MainCommonLoop();

    if ((millis() - lastSaveConfig) > save_config_every)
    {
      lastSaveConfig = millis();
      log_message((char*)F("Saving config to EEPROM memory..."));
      SaveConfig(); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
    }
  if (((millis() - lastUpdateTempPump) > statusUpdateInterval_ms) or lastUpdateTempPump==0 ) //or (((millis() < statusUpdateInterval_ms) or CO_PumpWorking or CO_BoilerPumpWorking) and (int(millis()/1000)%statusUpdateShortenInterval_s==0)) )   //after start for first defauylt 600s every 60s refresh
  {
    sprintf(log_chars, "temps+pumps lastUpdate: %d, statusUpdateInterval_ms: %d", lastUpdateTempPump, statusUpdateInterval_ms);
    log_message(log_chars);
    lastUpdateTempPump = millis();
    #ifndef ESP32
    ReadTemperatures();  //moved to 2nd core
    #endif
    Pump_Activate_Control();
//    opentherm_update_data(lastUpdatemqtt); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }


}


void Pump_Activate_Control () {
  //aktywacja przekaznika low, moja plyta HIGH
  log_message((char *)F("Pump loop..."));
  GetSpecificSensorData();
  for (int o = 0; o < maxsensors; o++) {
    //if (o < 8 and pump == stoprun) room_temp[o].switch_state = stoprun;  //turnoff floor switches when pump is off
    sprintf(log_chars,"%i. Switching: %s -> %s, pump: %s, startpump: %s temp: %f, tempset: %f, min_max_state: %s\r", o, digitalRead(room_temp[o].idpinout)?"OFF":"ON", (room_temp[o].switch_state)?"OFF":"ON", pump?"OFF":"ON", String(start_pump).c_str(), room_temp[o].tempread, room_temp[o].tempset, min_max_state?"OFF":"ON");
    log_message((char*)log_chars);
    digitalWrite(room_temp[o].idpinout, room_temp[o].switch_state); //chyba raczej do 8 bo tyle czujnikow w pokoju

  }
}

void GetSpecificSensorData () {

  if (check_isValidTemp(temp_NEWS)) {
    if (temp_NEWS < temptoheat ) modestate = MODEHEAT;   //min i max co drugie wykonanie ;()     or min_temp < (max_temp - histereza)
    if (temp_NEWS > temptocool ) modestate = MODECOOL;  //or min_temp > (max_temp + histereza))
    if ((temp_NEWS > temptoheat and check_isValidTemp(tempwe) and temp_NEWS < temptocool) and modestate == MODECOOL) modestate = MODEOFF;
  } else {
    modestate = MODEHEAT;
  }
  min_max_state = stoprun;

  min_temp = room_temp[0].tempread, max_temp = room_temp[0].tempset;
  String tmpval="\r";
  for (int o = 0; o < maxsensors; o++) {
    if (!check_isValidTemp(room_temp[o].tempread)) {
      room_temp[o].tempread = InitTemp;
    }
      if (modestate == MODEHEAT) {
        if (room_temp[o].tempread > (room_temp[o].tempset + histereza)) {
          room_temp[o].switch_state=stoprun;
        }
        if (room_temp[o].tempread < (room_temp[o].tempset)) {
          room_temp[o].switch_state = startrun;
        }
      }

      if (modestate == MODECOOL) {
        if (room_temp[o].tempread < (room_temp[o].tempset + histereza)) {
          room_temp[o].switch_state=stoprun;
        }
        if (room_temp[o].tempread > room_temp[o].tempset) {
          room_temp[o].switch_state = startrun;
        }
      }
      if (modestate == MODEOFF) {
        room_temp[o].switch_state=stoprun;
      }
    if (o<8) {
      if (check_isValidTemp(room_temp[o].tempread)) {
        min_temp = room_temp[o].tempread;
      } else {
        room_temp[o].switch_state = stoprun; //-wylacz gdy temp na wartosci inicjalnej
      }
      if (check_isValidTemp(room_temp[o].tempset)) max_temp = room_temp[o].tempset;
      if (min_temp < roomtemplo) min_temp = roomtemplo;
      if (max_temp > roomtemphi) max_temp = roomtemphi;

      if (room_temp[o].switch_state == startrun) min_max_state = startrun;
      if (min_max_state == stoprun) pump = stoprun; else pump = startrun;
    } else
    {
//      sprintf(log_chars,"%i. nameSensor: %s == %s  %f",o, room_temp[o].nameSensor, ((String(kondygnacja)+sepkondname+String(tempcutoff)).substring(0,namelength)).c_str(), room_temp[o].tempread);
//      log_message((char*)log_chars);
      room_temp[o].tempset = max_temp;
      if (String(room_temp[o].nameSensor) == (String(kondygnacja)+sepkondname+String(tempcutoff)).substring(0,namelength) ){//and check_isValidTemp(room_temp[o].tempread)) {
        pumpOffVal = room_temp[o].tempset;

        if (start_pump == 0 and min_max_state == startrun) {
          start_pump = millis();
          pump = startrun;
        }
        if ((millis() - start_pump) > start_pump_delay) {
          room_temp[o].switch_state = startrun;
        }
        if (pump == stoprun and min_max_state == stoprun) {
          start_pump = 0;
          room_temp[o].switch_state = stoprun;
        }

       // if (room_temp[o].tempread < (room_temp[o].tempset + histereza)) {


        sprintf(log_chars, "------------------------------ %i: SetPUMP: %s, to pump: %s ----> modestate: %s", o, room_temp[o].switch_state?"OFF":"ON", pump?"OFF":"ON", modestate==MODEHEAT?"heating":modestate==MODECOOL?"cooling":String(modestate));
        log_message(log_chars);

//        digitalWrite(room_temp[o].idpinout, pump);
      }
      if (String(room_temp[o].nameSensor) == (String(kondygnacja)+sepkondname+String(tempwename)).substring(0,namelength) and check_isValidTemp(room_temp[o].tempread)) {
        tempwe = room_temp[o].tempread;
      }
    }


      //tempwe = 34;
      //room_temp[o].tempset = pumpofftemp;
      //if ((tempwe > min) and (tempwe > pumpOffVal)) pump = start; else pump = stop; //"Wejscie podloga -gora"
      //room_temp[o].switch_state = pump;
  }
    sprintf(log_chars, " Pump: %s, tempwe: %f, MIN: %f, PUMPoffTemp: %f", pump?"OFF":"ON", tempwe, min_temp, pumpOffVal);
    log_message(log_chars);
}