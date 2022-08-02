//Main.cpp



#include "main.h"

//to read bus voltage in stats
//ADC_MODE(ADC_VCC);


void ReadTemperatures()
{
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
  sprintf(log_chars, "Reading 1wire 18B20 and DHT sensors... Count: %i", count);
  log_message(log_chars);
  if (count==0) count = maxsensors;
  String temptmp =" Collecting 18B20 ROMS and temps:\n";
  for (int j = 0; j < count; j++)
  {
    temp1w = sensors.getTempCByIndex(j);
    addrstr="";
    sensors.getAddress(addr, j);
    for (int i1 = 0; i1 < 8; i1++) if (String(addr[i1], HEX).length() == 1) addrstr += "0" + String(addr[i1], HEX); else addrstr += String(addr[i1], HEX); //konwersja HEX2StringHEX
      //zapisanie do zmiennej addr[8], addrstr, aktualiozacja index wskazany przez j i aktualozacja temp1w.
    for (int z = 0; z < maxsensors; z++)
    {
      if (array_cmp_8(room_temp[z].addressHEX, addr, sizeof(room_temp[z].addressHEX) / sizeof(room_temp[z].addressHEX[0]),sizeof(addr) / sizeof(addr[0])) == true)
      {
        assignedsensor=true;
        if (check_isValidTemp(temp1w)) room_temp[z].tempread = temp1w;
      }
    }
    if (!assignedsensor and UnassignedTempSensor.indexOf(addrstr)==-1) UnassignedTempSensor += ";"+String(addrstr)+" "+String(temp1w);
    assignedsensor=false;
    temptmp += "       " + String(j) + ": 18B20 ROM= " + addrstr + ", temp: " + String(temp1w,2) + "\n";
  }
  log_message((char*)temptmp.c_str());

  if (UnassignedTempSensor!= "")
  {
    sprintf(log_chars, "Unassigned Sensors: %s", UnassignedTempSensor);
    log_message(log_chars);
  }

  #ifdef enableDHT
  dht.read(true);
  delay(200);
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) or isnan(t)) {
    log_message((char*)F("Failed to read from DHT sensor!"));
    //if (isnan(t)) t=InitTemp;
    //if (isnan(h)) h=0;
    } else {
      dhtreadtime = millis();
    }

  sprintf(log_chars, "Get DHT values t= %s, humid: %s", String(t,2), String(h,1));
  log_message(log_chars);
  if (!isnan(h)) humiditycor = h;
  if (!isnan(t)) tempcor = t; //to check ds18b20 also ;)
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
  sprintf(log_chars,"choosePin: %s,  status: %s,  czyli: %s,  - %s  : %s",String(choosepin).c_str(), String(digitalRead(choosepin)).c_str(), String(digitalRead(choosepin)? "Kondygnacja 2":"Kondygnacja 1").c_str(), String(kondygnacja).c_str(), String(me_lokalizacja).c_str());
  log_message(log_chars);
 // pinMode(choosepin, DISABLED );


  ts = millis();
  lastTempSet = -extTempTimeout_ms;

  Serial.println("18B20 begin");
   // Init DS18B20 sensor
  sensors.begin();
  #ifdef enableDHT
  dht.begin();
  #endif
  sensors.setWaitForConversion(true); //
    //sensors.setWaitForConversion(true); //
  sensors.requestTemperatures();

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
    ReadTemperatures();
    Pump_Activate_Control();
//    opentherm_update_data(lastUpdatemqtt); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }


}


void Pump_Activate_Control () {
  log_message((char *)F("Pump loop..."));
  if (temp_NEWS < temptoheat or min_temp < (max_temp - histereza)) modestate = MODEHEAT;   //min i max co drugie wykonanie ;()
  if (temp_NEWS > temptocool or min_temp > (max_temp + histereza)) modestate = MODECOOL;
  if ((tempwe > 21 or tempwe != InitTemp) and modestate == MODECOOL) modestate = MODEOFF;
  min_max_state = stoprun;
  GetSpecificSensorData();
  String tmpval="\r";

  for (int o = 0; o < maxsensors; o++) {
    if (room_temp[o].tempread == InitTemp) room_temp[o].switch_state = stoprun; //-wylacz gdy temp na wartosci inicjalnej
    //aktywacja przekaznika low, moja plyta HIGH
    if (o<8 and room_temp[o].tempread == InitTemp and temp_NEWS < temptoheat) {room_temp[o].switch_state=startrun;}
    if (o<8 and room_temp[o].tempread != InitTemp) {
      if (room_temp[o].tempread > room_temp[o].tempset and modestate == MODEHEAT) {room_temp[o].switch_state=stoprun;}
      if (room_temp[o].tempread > room_temp[o].tempset and modestate == MODECOOL) {room_temp[o].switch_state=startrun;}
      if (room_temp[o].tempread < (room_temp[o].tempset + histereza) and modestate == MODEHEAT) {room_temp[o].switch_state=startrun;}
      if (room_temp[o].tempread < (room_temp[o].tempset + histereza) and modestate == MODECOOL) {room_temp[o].switch_state=stoprun;}
      if (room_temp[o].switch_state == startrun) min_max_state = startrun;
    }  //stop if tempread is > tempset HIGH disable, LOW enable
    if (o == 8 and min_max_state == stoprun) pump = stoprun;


  //  if (o>=8) if (room_temp[o].tempread<room_temp[o].tempset) room_temp[o].switch_state=stop; else room_temp[o].switch_state=start;  //stop if tempread is > tempset HIGH disable, LOW enable
    if (room_temp[o].idpinout>0) {
      if (pump==stoprun) room_temp[o].switch_state = stoprun;  //turnoff floor switches when pump is off

//room_temp[o].switch_state = 1;

      tmpval += String("    ")+String(o)+": Switching: "+String(digitalRead(room_temp[o].idpinout)?"OFF":"ON")+" -> "+String((room_temp[o].switch_state)?"OFF":"ON")+" pump: "+String(pump?"OFF":"ON") + "\r";
      digitalWrite(room_temp[o].idpinout, room_temp[o].switch_state); //chyba raczej do 8 bo tyle czujnikow w pokoju
    }
  }
  log_message((char*)tmpval.c_str());
  GetSpecificSensorData();

//  if (pump == stop) digitalWrite(pompa_pin, stop_digital); else digitalWrite(pompa_pin, start_digital);

}

void GetSpecificSensorData () {
  min_temp = room_temp[0].tempread, max_temp = room_temp[0].tempset;
  String tmpval="\0";
  for (int o = 0; o < maxsensors; o++) {
    if (!check_isValidTemp(room_temp[o].tempread)) room_temp[o].tempread = InitTemp;
    if (o<8) {
      if (check_isValidTemp(room_temp[o].tempread)) min_temp = room_temp[o].tempread;
      if (check_isValidTemp(room_temp[o].tempread)) max_temp = room_temp[o].tempset;
      if (min_temp < roomtemplo) min_temp = roomtemplo;
      if (max_temp > roomtemphi) max_temp = roomtemphi;
      if (room_temp[o].switch_state == startrun) min_max_state = startrun;

    }
    if (String(room_temp[o].nameSensor) == (String(kondygnacja)+sepkondname+String(tempcutoff)).substring(0,namelength) and room_temp[o].tempread != InitTemp) {
      if (modestate == MODEHEAT or modestate == MODEOFF) pumpOffVal = room_temp[o].tempset;
      if (modestate == MODECOOL and min_max_state == startrun) {pumpOffVal = max_temp - histereza; pump = startrun;}


      if (room_temp[o].tempread > pumpOffVal and modestate == MODEHEAT and min_max_state == startrun) pump = startrun;  //raczej na wejsciu z 4d dol
      if (room_temp[o].tempread < (pumpOffVal + histereza) and modestate == MODEHEAT) pump = stoprun;   //raczej na wejsciu z 4d dol
      //if (room_temp[o].tempread > pumpOffVal and modestate == MODECOOL) pump = stop;  //raczej na wejsciu z 4d dol
      //if (room_temp[o].tempread < (pumpOffVal + histereza) and modestate == MODECOOL) pump = start;  //raczej na wejsciu z 4d dol

      if (min_max_state = stoprun or modestate == MODEOFF) {pump=stoprun; start_pump=0;}  //turn off pump if all switches are disabled

      if (start_pump == 0 and pump == startrun) {start_pump=millis(); pump = stoprun;}
      if ((millis()-start_pump) > (start_pump_delay*1.2) and pump == stoprun) start_pump = 0;
      if ((millis()-start_pump) > start_pump_delay and min_max_state == startrun) pump = startrun;

      sprintf(log_chars, "------------------------------ %i: SetPUMP: %s, to pump: %s ----> modestate: %s", o, room_temp[o].switch_state?"OFF":"ON", pump?"OFF":"ON", modestate==MODEHEAT?"heating":modestate==MODECOOL?"cooling":String(modestate));
      log_message(log_chars);

      room_temp[o].switch_state = pump;
      digitalWrite(room_temp[o].idpinout, pump);
    }
    if (String(room_temp[o].nameSensor) == (String(kondygnacja)+sepkondname+String(tempwename)).substring(0,namelength) and room_temp[o].tempread != InitTemp) {
      tempwe = room_temp[o].tempread;
      room_temp[o].switch_state = stoprun;
    }

      //tempwe = 34;
      //room_temp[o].tempset = pumpofftemp;
      //if ((tempwe > min) and (tempwe > pumpOffVal)) pump = start; else pump = stop; //"Wejscie podloga -gora"
      //room_temp[o].switch_state = pump;
  }
    sprintf(log_chars, " Pump: %s, tempwe: %f, MIN: %f, PUMPoffTemp: %f", pump?"OFF":"ON", tempwe, min_temp, pumpOffVal);
    log_message(log_chars);
}