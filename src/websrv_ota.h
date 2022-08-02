//websrv_ota.h

#include "Arduino.h"


String get_PlaceholderName(u_int i)
{ //get names by number to match www placeholders
  switch(i) {
    case ASS_uptimedana: return PSTR(ASS_uptimedanaStr); break;
    case ASS_Statusy: return PSTR(ASS_StatusyStr); break;
    case ASS_MemStats: return PSTR(ASS_MemStatsStr); break;

    case ASS_temp_NEWS: return PSTR(ASS_temp_NEWSStr); break;
    case ASS_lastNEWSSet: return PSTR(ASS_lastNEWSSetStr); break;
    case ASS_opcohi: return PSTR("opcohi"); break;

    case ASS_room1temp: return PSTR(ASS_room1tempStr); break;
    case ASS_room1tempSet: return PSTR(ASS_room1tempSetStr); break;
    case ASS_RoomHumid1: return PSTR(ASS_RoomHumid1Str); break;
    case ASS_Roomstate1: return PSTR(ASS_Roomstate1Str); break;

    case ASS_room2temp: return PSTR(ASS_room2tempStr); break;
    case ASS_room2tempSet: return PSTR(ASS_room2tempSetStr); break;
    case ASS_RoomHumid2: return PSTR(ASS_RoomHumid2Str); break;
    case ASS_Roomstate2: return PSTR(ASS_Roomstate2Str); break;

    case ASS_room3temp: return PSTR(ASS_room3tempStr); break;
    case ASS_room3tempSet: return PSTR(ASS_room3tempSetStr); break;
    case ASS_RoomHumid3: return PSTR(ASS_RoomHumid3Str); break;
    case ASS_Roomstate3: return PSTR(ASS_Roomstate3Str); break;

    case ASS_room4temp: return PSTR(ASS_room4tempStr); break;
    case ASS_room4tempSet: return PSTR(ASS_room4tempSetStr); break;
    case ASS_RoomHumid4: return PSTR(ASS_RoomHumid4Str); break;
    case ASS_Roomstate4: return PSTR(ASS_Roomstate4Str); break;

    case ASS_room5temp: return PSTR(ASS_room5tempStr); break;
    case ASS_room5tempSet: return PSTR(ASS_room5tempSetStr); break;
    case ASS_RoomHumid5: return PSTR(ASS_RoomHumid5Str); break;
    case ASS_Roomstate5: return PSTR(ASS_Roomstate5Str); break;

    case ASS_room6temp: return PSTR(ASS_room6tempStr); break;
    case ASS_room6tempSet: return PSTR(ASS_room6tempSetStr); break;
    case ASS_RoomHumid6: return PSTR(ASS_RoomHumid6Str); break;
    case ASS_Roomstate6: return PSTR(ASS_Roomstate6Str); break;

    case ASS_room7temp: return PSTR(ASS_room7tempStr); break;
    case ASS_room7tempSet: return PSTR(ASS_room7tempSetStr); break;
    case ASS_RoomHumid7: return PSTR(ASS_RoomHumid7Str); break;
    case ASS_Roomstate7: return PSTR(ASS_Roomstate7Str); break;

    case ASS_room8temp: return PSTR(ASS_room8tempStr); break;
    case ASS_room8tempSet: return PSTR(ASS_room8tempSetStr); break;
    case ASS_RoomHumid8: return PSTR(ASS_RoomHumid8Str); break;
    case ASS_Roomstate8: return PSTR(ASS_Roomstate8Str); break;

    case ASS_room9temp: return PSTR(ASS_room9tempStr); break;
    case ASS_room9tempSet: return PSTR(ASS_room9tempSetStr); break;
    case ASS_Roomstate9: return PSTR(ASS_Roomstate9Str); break;

    case ASS_room10temp: return PSTR(ASS_room10tempStr); break;
    case ASS_room10tempSet: return PSTR(ASS_room10tempSetStr); break;
    case ASS_Roomstate10: return PSTR(ASS_Roomstate10Str); break;

    case ASS_room11temp: return PSTR(ASS_room11tempStr); break;
    case ASS_room11tempSet: return PSTR(ASS_room11tempSetStr); break;
    case ASS_Roomstate11: return PSTR(ASS_Roomstate11Str); break;

    case ASS_room12temp: return PSTR(ASS_room12tempStr); break;
    case ASS_room12tempSet: return PSTR(ASS_room12tempSetStr); break;
    case ASS_Roomstate12: return PSTR(ASS_Roomstate12Str); break;


    case ASS_cutOffTemp: return PSTR(ASS_cutOffTempStr); break;
    case ASS_retTemp: return PSTR(ASS_retTempStr); break;

    case ASS_roomDHTtemp: return PSTR(ASS_roomDHTtempStr); break;
    case ASS_roomDHThum: return PSTR(ASS_roomDHThumStr); break;

    case ASS_modeState: return PSTR(ASS_modeStateStr); break;


    // case ASS_tempBoilerSet: return PSTR("sliderValue1"); break;
    // case ASS_dhwTarget: return PSTR("sliderValue2"); break;
    // case ASS_roomtempSet: return PSTR("sliderValue4"); break;  //Room Target sp


    // case ASS_tempBoiler: return PSTR("tempBoiler"); break;
    // case ASS_tempCWU: return PSTR("tempCWU"); break;
    // case ASS_roomtemp: return PSTR("roomtemp"); break;
    // case ASS_AutoMode: return PSTR("boilermodewww"); break;
    // case ASS_EnableHeatingCO: return PSTR("boilerwww"); break;
    // case ASS_statusWaterActive: return PSTR("statusWaterActive"); break;  //pump for water cwu active
    // case ASS_statusCHActive: return PSTR("statusCHActive"); break;
    // case ASS_statusFlameOn: return PSTR("statusFlameOn"); break;
    // case ASS_statusFault: return PSTR("statusFault"); break;
    // case ASS_EnableHotWater: return PSTR("boilerhwwww"); break;
     case ASS_UsedMedia: return PSTR("UsedMedia"); break;
    // case ASS_ecoMode: return PSTR("ecoMode"); break;
  }
  return "\0";
}


void updateDatatoWWW_received(u_int i) {

  sprintf(log_chars, "Received data nr: %s", String(i).c_str());
  log_message(log_chars);
  switch (i) {
    case ASS_room1tempSet: room_temp[0].tempset = PayloadtoValidFloat(String(ASS[ASS_room1tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_room2tempSet: room_temp[1].tempset = PayloadtoValidFloat(String(ASS[ASS_room2tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_room3tempSet: room_temp[2].tempset = PayloadtoValidFloat(String(ASS[ASS_room3tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_room4tempSet: room_temp[3].tempset = PayloadtoValidFloat(String(ASS[ASS_room4tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_room5tempSet: room_temp[4].tempset = PayloadtoValidFloat(String(ASS[ASS_room5tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_room6tempSet: room_temp[5].tempset = PayloadtoValidFloat(String(ASS[ASS_room6tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_room7tempSet: room_temp[6].tempset = PayloadtoValidFloat(String(ASS[ASS_room7tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_room8tempSet: room_temp[7].tempset = PayloadtoValidFloat(String(ASS[ASS_room8tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_room9tempSet: room_temp[8].tempset = PayloadtoValidFloat(String(ASS[ASS_room9tempSet].Value), true, roomtemplo, roomtemphi); break;
    case ASS_modeState: {
      String modestate_tmp = String(ASS[ASS_room9tempSet].Value);
      if (modestate_tmp == "modeAuto") {
        modestate = MODEAUTO;
      } else
      if (modestate_tmp == "modeCool") {
        modestate = MODECOOL;
      } else
      if (modestate_tmp == "modeHeat") {
        modestate = MODEHEAT;
      } else {
        modestate = MODEOFF;
      }
    }
     // tempBoilerSet = PayloadtoValidFloat(String(ASS[ASS_tempBoilerSet].Value), true, oplo, ophi);
      break;
//egfor dla set values


      //     String message;
//     for (u_int x=0;x<maxsensors;x++)
//     {
//       String fragment = String(room_temp[x].nameSensor)+ appendSet;
//       fragment.replace(sepkondname,"_");
//       if (request->hasParam((fragment ).c_str()))
//       {
//         message = request->getParam((fragment).c_str())->value();
//         log_message((char*)message.c_str());
//         receivedmqttdata = getRemoteTempHumid(message, x, action_tempset);
//       }else {
//           //message = "No message sent PARAM_roomtempset0";
//       }
//     }

  }
}
String return_noval_if_invalid(float val2check)
{
  if (check_isValidTemp(val2check)) {
    return String(val2check, decimalPlaces);
    } else
    {
      if (decimalPlaces == 1)
      {
        return F("--.-");
      } else
      if (decimalPlaces == 2)
      {
        return F("--.--");
      } else
      {
        return F("--");
      }
    }
}

void updateDatatoWWW() //default false so if true than update
{

  sprintf(log_chars, "Update Data to www ");
  log_message(log_chars);
#ifdef enableWebSocket
  //if (receivedwebsocketdata and dont_send_after_sync) return;
  //String dana = {"DHWTemp",DHW_Temp}
  String ptrS = "\0";

    SaveAssValue(ASS_lastNEWSSet,       uptimedana(lastNEWSSet) );
    SaveAssValue(ASS_temp_NEWS,         return_noval_if_invalid(temp_NEWS) );

    SaveAssValue(ASS_room1temp,         return_noval_if_invalid(room_temp[0].tempread) );
    SaveAssValue(ASS_RoomHumid1,        return_noval_if_invalid(room_temp[0].humidityread), true );
    SaveAssValue(ASS_room1tempSet,      String(room_temp[0].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate1,        String(room_temp[0].switch_state?1:0 ) );

    SaveAssValue(ASS_room2temp,         return_noval_if_invalid(room_temp[1].tempread) );
    SaveAssValue(ASS_RoomHumid2,        return_noval_if_invalid(room_temp[1].humidityread), true );
    SaveAssValue(ASS_room2tempSet,      String(room_temp[1].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate2,        String(room_temp[1].switch_state?1:0 ) );

    SaveAssValue(ASS_room3temp,         return_noval_if_invalid(room_temp[2].tempread) );
    SaveAssValue(ASS_RoomHumid3,        return_noval_if_invalid(room_temp[2].humidityread), true );
    SaveAssValue(ASS_room3tempSet,      String(room_temp[2].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate3,        String(room_temp[2].switch_state?1:0 ) );

    SaveAssValue(ASS_room4temp,         return_noval_if_invalid(room_temp[3].tempread) );
    SaveAssValue(ASS_RoomHumid4,        return_noval_if_invalid(room_temp[3].humidityread), true );
    SaveAssValue(ASS_room4tempSet,      String(room_temp[3].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate4,        String(room_temp[3].switch_state?1:0 ) );

    SaveAssValue(ASS_room5temp,         return_noval_if_invalid(room_temp[4].tempread) );
    SaveAssValue(ASS_RoomHumid5,        return_noval_if_invalid(room_temp[4].humidityread), true );
    SaveAssValue(ASS_room5tempSet,      String(room_temp[4].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate5,        String(room_temp[4].switch_state?1:0 ) );

    SaveAssValue(ASS_room6temp,         return_noval_if_invalid(room_temp[5].tempread) );
    SaveAssValue(ASS_RoomHumid6,        return_noval_if_invalid(room_temp[5].humidityread), true );
    SaveAssValue(ASS_room6tempSet,      String(room_temp[5].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate6,        String(room_temp[5].switch_state?1:0 ) );

    SaveAssValue(ASS_room7temp,         return_noval_if_invalid(room_temp[6].tempread) );
    SaveAssValue(ASS_RoomHumid7,        return_noval_if_invalid(room_temp[6].humidityread), true );
    SaveAssValue(ASS_room7tempSet,      String(room_temp[6].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate7,        String(room_temp[6].switch_state?1:0 ) );

    SaveAssValue(ASS_room8temp,         return_noval_if_invalid(room_temp[7].tempread) );
    SaveAssValue(ASS_RoomHumid8,        return_noval_if_invalid(room_temp[7].humidityread), true );
    SaveAssValue(ASS_room8tempSet,      String(room_temp[7].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate8,        String(room_temp[7].switch_state?1:0 ) );

    SaveAssValue(ASS_room9temp,         return_noval_if_invalid(room_temp[8].tempread) );
    SaveAssValue(ASS_room9tempSet,      String(room_temp[8].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate9,        String(room_temp[8].switch_state?1:0 ) );

    SaveAssValue(ASS_room10temp,         return_noval_if_invalid(room_temp[9].tempread) );
    SaveAssValue(ASS_room10tempSet,      String(room_temp[9].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate10,        String(room_temp[9].switch_state?1:0 ) );

    SaveAssValue(ASS_room11temp,         return_noval_if_invalid(room_temp[10].tempread) );
    SaveAssValue(ASS_room11tempSet,      String(room_temp[10].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate11,        String(room_temp[10].switch_state?1:0 ) );

    SaveAssValue(ASS_room12temp,         return_noval_if_invalid(room_temp[11].tempread) );
    SaveAssValue(ASS_room12tempSet,      String(room_temp[11].tempset, decimalPlaces) );
    SaveAssValue(ASS_Roomstate12,        String(room_temp[11].switch_state?1:0 ) );

    SaveAssValue(ASS_modeState,        String((modestate == MODEAUTO)? "modeAuto": ((modestate == MODECOOL)?"modeCool":((modestate == MODEHEAT)?"modeHeat":"modeOFF"))) );
//modestate == MODEHEAT and x<8 and room_temp[x].switch_state==startrun


    SaveAssValue(ASS_roomDHTtemp, return_noval_if_invalid(tempcor) );
    SaveAssValue(ASS_roomDHThum, return_noval_if_invalid(humiditycor) );


//tempCORETThermometerS    String(retTemp,1)
for (u_int x=0;x<maxsensors;x++)
{ String fragment = String(room_temp[x].nameSensor);
  fragment.replace(sepkondname,"_");
  //fragment      String(room_temp[x].tempread==InitTemp ? "--.-" : String(room_temp[x].tempread,1);
  //fragment + appendSet    String(room_temp[x].tempset,1)

}
#ifdef enableDHT
  //dhumidcor     String(humiditycor,1)
  //dtempcor      String(tempcor,1)+tempw)
#endif
    SaveAssValue(ASS_retTemp,             return_noval_if_invalid(retTemp) );
    SaveAssValue(ASS_cutOffTemp,          return_noval_if_invalid(cutOffTemp) );
    // SaveAssValue(ASS_tempBoiler,          String(tempBoiler, decimalPlaces) );
    // SaveAssValue(ASS_tempBoilerSet,       String(tempBoilerSet, decimalPlaces) );
    // SaveAssValue(ASS_tempCWU,             String(tempCWU, decimalPlaces) );
    // SaveAssValue(ASS_dhwTarget,           String(dhwTarget, decimalPlaces) );
    // SaveAssValue(ASS_roomtemp,            String(roomtemp, decimalPlaces) );
    // SaveAssValue(ASS_roomtempSet,         String(roomtempSet, decimalPlaces) );


    ptrS = "\0";
      if (pump==startrun) {
        ptrS += F("<i class='fas fa-fire'></i>"); ptrS += "<span id='StatusRedNormal'>"+String(FloorPumpActive)+F("</span><B>")+F("<sup class=\"units\"> </sup></B></br>");
      }
      if (CO_BoilerPumpWorking) {ptrS += F("<span id='StatusRedNormal'><B>"); ptrS += String(BOILER_IS_HEATING); ptrS += F("</span></br>");}
      if (CO_PumpWorking) {ptrS += "<span id='StatusBlue'>" + String(Second_Engine_Heating_PompActive_Disable_heat) + "</span>></br>";}

      ptrS += F("<span id='StatusBlue'><B>"); ptrS += String(sMODESTATE)+" "+String(modestate==MODEHEAT?sMODESTATE_HEAT:modestate==MODECOOL?sMODESTATE_COOL:modestate==MODEOFF?sMODESTATE_OFF:String(modestate)); ptrS += F("</B></span></br>");
      ptrS += F("<span id='StatusBlue'>Temperatura progu włączenia/wyłączenia pompy: <B>"); ptrS += String(pumpOffVal); ptrS += F("</B></span><br>");


    // if (status_FlameOn) {
    //   ptrS += "<i class='fas fa-fire' id='StatusRed'></i>"; ptrS += "<span id='StatusRedNormal'>" + String(Flame_Active_Flame_level) + "</span><b>" + String(flame_level, 0) + "<sup class=\"units\">&#37;</sup></b></br>";
    // }
    // if (status_Fault) ptrS += "<span id='StatusRed'>!!!!!!!!!!!!!!!!! status_Fault !!!!!!!</span></br>";
    // if (heatingEnabled) {
    //   ptrS += "<span is='StatusBlack'>" + String(BOILER_HEAT_ON);
    //   if (automodeCO) ptrS += F(" (AUTO)"); else ptrS += F(" (Standard)");
    //   ptrS += ("</span></br>");
    // }
    // if (status_CHActive) ptrS += "<span id='StatusRed'>" + String(BOILER_IS_HEATING) + "</span></br>";
    // if (enableHotWater) ptrS += "<span id='StatusBlack'>" + String(DHW_HEAT_ON) + "</span></br>";
    // if (status_WaterActive) ptrS += "<span id='StatusRed'>" + String(Boiler_Active_heat_DHW) + "</span></br>";
    // if (status_Cooling) ptrS += "<span id='StatusOrange'>" + String(CoolingMode) + "</span><</br>";
    // if (status_Diagnostic) ptrS += "<span id='StatusDarkRed'>" + String(DiagMode) + "</span></br>";
    // if (Water_PumpWorking) ptrS += "<span id='StatusBlue'>" + String(Second_Engine_Heating_Water_PompActive) + "</span></br>";
    // if (status_FlameOn) ptrS += "<span id='StatusGreen'>" + String(Flame_time) + "<b>" + uptimedana(start_flame_time_fordisplay) + "</b></span></br>";
//    ASS[ASS_Statusy].Value = toCharPointer(String(ptrS));
    //strcpy(ASS[ASS_Statusy].Value, toCharPointer(ptrS) );
    SaveAssValue(ASS_Statusy, ptrS );

    ptrS = "\0";
    ptrS += F(" Unassigned Dallas: ");
    ptrS += UnassignedTempSensor;
    // ptrS += "<p id='StatusBlackNormal'>" + String(Flame_total) + "<b>" + String(flame_used_power_kwh, 4) + "kWh</b>";
    // ptrS += String(" : ") + "<b>" + String(uptimedana((flame_time_total), true)+"</b>");
    // ptrS += "</br>w tym woda: <b>" + String(flame_used_power_waterTotal, 4) + "kWh</b>";
    // ptrS += String(" : ") + "<b>" + String(uptimedana((flame_time_waterTotal), true)+"</b>");
    // ptrS += "</br>w tym CO: <b>" + String(flame_used_power_CHTotal, 4) + "kWh</b>";
    // ptrS += String(" : ") + "<b>" + String(uptimedana((flame_time_CHTotal), true)+"</b></p>");
// //    ASS[ASS_UsedMedia].Value = toCharPointer(String(ptrS));
//     strcpy(ASS[ASS_UsedMedia].Value, toCharPointer( ptrS) );
    SaveAssValue(ASS_UsedMedia, ptrS );
    #ifdef debug
    sprintf(log_chars,"Flame_Total: %s (%s), CO: %s (%s), DHW: %s (%s)", String(flame_used_power_kwh).c_str(), String(uptimedana((flame_time_total), true)).c_str(), String(flame_used_power_CHTotal).c_str(), String(uptimedana((flame_time_CHTotal), true)).c_str(), String(flame_used_power_waterTotal).c_str(), String(uptimedana((flame_time_waterTotal), true)).c_str());
    log_message(log_chars);
    #endif
#endif
}

String local_specific_web_processor_vars(String var) {
  //Update config page
  if (var == "COPUMP_GET_TOPIC") { return String(COPUMP_GET_TOPIC);
  } else
  if (var == "COPumpStatus_json") { return String(COPumpStatus_json);
  } else
  if (var == "WaterPumpStatus_json") { return String(WaterPumpStatus_json);
  // } else
  // if (var == "ROOMS_F1_GET_TOPIC") { return String(ROOMS_F1_GET_TOPIC);
  // } else
  // if (var == "roomF1temp_json") { return String(roomF1temp_json);
  // } else
  // if (var == "roomF1tempset_json") { return String(roomF1tempset_json);
  // } else
  // if (var == "ROOMS_F2_GET_TOPIC") { return String(ROOMS_F2_GET_TOPIC);
  // } else
  // if (var == "roomF2temp_json") { return String(roomF2temp_json);
  // } else
  // if (var == "roomF2tempset_json") { return String(roomF2tempset_json);
  }
  return "\0";
}


//******************************************************************************************
//     background-color: #01DF3A;
//
//  <div class='s'><svg version='1.1' width="75px" height="75px" id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>
#ifdef enableWebSocket
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

#endif

// void handleUpdate(AsyncWebServerRequest *request) {
//   request->send(200, "text/html", htmlup);
// }


void handleGetTemp() {
	//digitalWrite(BUILTIN_LED, 1);
	//webserver.send(200, "text/plain", String(123));
	//digitalWrite(BUILTIN_LED, 0);
}

// void starthttpserver() {
//   webserver.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
//     request->send(200, "text/html", htmlup);
//     }).setAuthentication("", "");
//   webserver.on("/doUpdate", HTTP_POST,
//       [&](AsyncWebServerRequest *request) {
//       // the request handler is triggered after the upload has finished...
//       // create the response, add header, and send response
//       AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
//       response->addHeader("Connection", "close");
//       response->addHeader("Access-Control-Allow-Origin", "*");
//       request->send(response);
//       },
//     [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
//                   size_t len, bool final) {handleDoUpdate(request, filename, index, data, len, final);}
//   ).setAuthentication("", "");;


//   webserver.onNotFound([](AsyncWebServerRequest * request) {
//     request->send(404);
//   });
//   webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//     request->send_P(200, "text/html; charset=utf-8",  index_html, processor);
//   }).setAuthentication("", "");
//     // Send a GET request to <IP>/get?message=<message>
//   webserver.on("/" uptimelink , HTTP_GET, [](AsyncWebServerRequest * request) {
//   //  request.setAuthentication("", "");
//     request->send(200, "text/plain; charset=utf-8", String(uptimedana(starting)));
//   }).setAuthentication("", "");
//   webserver.on("/" dallThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
//     request->send(200, "text/plain; charset=utf-8", String(temp_NEWS));
//   }).setAuthentication("", "");
//   webserver.on("/" tempCORETThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
//     request->send(200, "text/plain; charset=utf-8", String(retTemp,1));
//   }).setAuthentication("", "");



//   webserver.on("/" do_stopkawebsiteS, HTTP_GET, [](AsyncWebServerRequest * request) {
//     request->send(200, "text/plain; charset=utf-8", do_stopkawebsite());
//   }).setAuthentication("", "");

//   for (u_int x=0;x<maxsensors;x++)
//   {
//     String fragment = String(room_temp[x].nameSensor);
//     fragment.replace(sepkondname,"_");
//     webserver.on( ("/" + fragment).c_str() , HTTP_GET, [x](AsyncWebServerRequest * request) { //roomtempS + getident
//     request->send(200, "text/plain; charset=utf-8", String(room_temp[x].tempread==InitTemp ? "--.-" : String(room_temp[x].tempread,1)));
//     }).setAuthentication("", "");
//     webserver.on( ("/" + fragment + appendSet).c_str() , HTTP_GET, [x](AsyncWebServerRequest * request) {
//       request->send(200, "text/plain; charset=utf-8", String(room_temp[x].tempset,1));
//     }).setAuthentication("", "");
//   }

// #ifdef enableDHT
//     webserver.on( ("/"+String(dhumidcor)).c_str(), HTTP_GET, [](AsyncWebServerRequest * request) {
//         request->send(200, "text/plain; charset=utf-8",  String(humiditycor,1));
//       }).setAuthentication("", "");

//     webserver.on( ("/"+String(dtempcor)).c_str(), HTTP_GET, [](AsyncWebServerRequest * request) {
//         String tempw = "\0";
//         if (millis()-dhtreadtime > (60*1000)) tempw="<span class='units'>  Last update was "+uptimedana(dhtreadtime)+" ago</span>";
//         request->send(200, "text/plain; charset=utf-8",  String(tempcor,1)+tempw);
//       }).setAuthentication("", "");
// #endif

//   webserver.on("/" NEWS_lastTimeS, HTTP_GET, [](AsyncWebServerRequest * request) {
//     request->send(200, "text/plain; charset=utf-8", String(uptimedana(lastNEWSSet)));
//   }).setAuthentication("", "");



//   webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
//     String message;
//     for (u_int x=0;x<maxsensors;x++)
//     {
//       String fragment = String(room_temp[x].nameSensor)+ appendSet;
//       fragment.replace(sepkondname,"_");
//       if (request->hasParam((fragment ).c_str()))
//       {
//         message = request->getParam((fragment).c_str())->value();
//         log_message((char*)message.c_str());
//         receivedmqttdata = getRemoteTempHumid(message, x, action_tempset);
//       }else {
//           //message = "No message sent PARAM_roomtempset0";
//       }
//     }
//     AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Update value");
//     response->addHeader("Refresh", "1");
//     response->addHeader("Location", "/");
//     request->send(response);
//   });
//     // Send a POST request to <IP>/post with a form field message set to <message>
// /*     webserver.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
//         String message;
//         if (request->hasParam(PARAM_MESSAGE, true)) {
//             message = request->getParam(PARAM_MESSAGE, true)->value();
//         } else {
//             message = "No message sent";
//         }
//         request->send(200, "text/plain", "Hello, POST: " + message);
//     });
//  */
//   DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
//   DefaultHeaders::Instance().addHeader("Server",me_lokalizacja);
//   DefaultHeaders::Instance().addHeader("Title",me_lokalizacja);

// 	webserver.begin();
//   log_message((char*)F("HTTP server starting"));
// }

String processor(const String var) {

#ifdef enableWebSocket

  #ifdef debug
//    Serial.print(F("Start processor: "));
//    Serial.println(var);
  #endif
  if (var == "ver") {
    String a = F("</B>ESP CO Server dla: <B>");
    a += String(me_lokalizacja);
    a += F("</B><BR>v. ");
    //a += me_version;
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
    a += String(CRTrunNumber);
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
    return String(uptimedana(starting));
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
    //return do_stopkawebsite();
  }
  if (var == "stopkawebsite0") {
    String ptr;
      ptr = "<br><span class='units'><a href='"+String(update_path)+"' target=\"_blank\">"+String(Update_web_link)+"</a> &nbsp; &nbsp;&nbsp;";
      #ifdef enableWebSerial
      ptr += "<a href='/webserial' target=\"_blank\">"+String(Web_Serial)+"</a>&nbsp;";
      #endif
      ptr += "<br>&copy; ";
      // const String  stopka = String(MFG)+" "+version[4]+version[5]+"-"+version[2]+version[3]+"-20"+version[0]+version[1]+" "+version[6]+version[7]+":"+version[8]+version[9];
      ptr += String(MFG)+" "+version[4]+version[5]+"-"+version[2]+version[3]+"-20"+version[0]+version[1]+" "+version[6]+version[7]+":"+version[8]+version[9];
      ptr += "<br>";
    #ifdef debug2
    Serial.println(var+": "+ptr.length());
    #endif
    return String(ptr);
  }
#endif

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
#ifdef enableWebSocket

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
#endif
}

String do_stopkawebsite() {
  #ifndef enableWebSocket
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
    #else
    return "\0";
    #endif

}
