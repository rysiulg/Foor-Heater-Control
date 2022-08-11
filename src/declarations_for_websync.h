#define ASScount 0
//defined in common_functions 0,1,2
#define ASS_uptimedana ASScount
#define ASS_uptimedanaStr "uptimedana"

#define ASS_Statusy 1
#define ASS_StatusyStr "Statusy"
#define ASS_MemStats 2
#define ASS_MemStatsStr "MemStats"
#define ASS_temp_NEWS  3      // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South
#define ASS_temp_NEWSStr "temp_NEWS"
#define ASS_lastNEWSSet 4      //last time NEWS get updated from mqtt
#define ASS_lastNEWSSetStr "lastNEWSSet"
#define ASS_opcohi 5
#define ASS_opcohiStr "opcohi"

#define ASS_room1temp 6
#define ASS_room1tempStr "Roomtemp1"
#define ASS_room1tempSet 7
#define ASS_room1tempSetStr "sliderValue1"
#define ASS_RoomHumid1 8
#define ASS_RoomHumid1Str "RoomHumid1"
#define ASS_Roomstate1 9
#define ASS_Roomstate1Str "Roomstate1"

#define ASS_room2temp 10
#define ASS_room2tempStr "Roomtemp2"
#define ASS_room2tempSet 11
#define ASS_room2tempSetStr "sliderValue2"
#define ASS_RoomHumid2 12
#define ASS_RoomHumid2Str "RoomHumid2"
#define ASS_Roomstate2 13
#define ASS_Roomstate2Str "Roomstate2"

#define ASS_room3temp 14
#define ASS_room3tempStr "Roomtemp3"
#define ASS_room3tempSet 15
#define ASS_room3tempSetStr "sliderValue3"
#define ASS_RoomHumid3 16
#define ASS_RoomHumid3Str "RoomHumid3"
#define ASS_Roomstate3 17
#define ASS_Roomstate3Str "Roomstate3"

#define ASS_room4temp 18
#define ASS_room4tempStr "Roomtemp4"
#define ASS_room4tempSet 19
#define ASS_room4tempSetStr "sliderValue4"
#define ASS_RoomHumid4 20
#define ASS_RoomHumid4Str "RoomHumid4"
#define ASS_Roomstate4 21
#define ASS_Roomstate4Str "Roomstate4"

#define ASS_room5temp 22
#define ASS_room5tempStr "Roomtemp5"
#define ASS_room5tempSet 23
#define ASS_room5tempSetStr "sliderValue5"
#define ASS_RoomHumid5 24
#define ASS_RoomHumid5Str "RoomHumid5"
#define ASS_Roomstate5 25
#define ASS_Roomstate5Str "Roomstate5"

#define ASS_room6temp 26
#define ASS_room6tempStr "Roomtemp6"
#define ASS_room6tempSet 27
#define ASS_room6tempSetStr "sliderValue6"
#define ASS_RoomHumid6 28
#define ASS_RoomHumid6Str "RoomHumid6"
#define ASS_Roomstate6 29
#define ASS_Roomstate6Str "Roomstate6"

#define ASS_room7temp 30
#define ASS_room7tempStr "Roomtemp7"
#define ASS_room7tempSet 31
#define ASS_room7tempSetStr "sliderValue7"
#define ASS_RoomHumid7 32
#define ASS_RoomHumid7Str "RoomHumid7"
#define ASS_Roomstate7 33
#define ASS_Roomstate7Str "Roomstate7"

#define ASS_room8temp 34
#define ASS_room8tempStr "Roomtemp8"
#define ASS_room8tempSet 35
#define ASS_room8tempSetStr "sliderValue8"
#define ASS_RoomHumid8 36
#define ASS_RoomHumid8Str "RoomHumid8"
#define ASS_Roomstate8 37
#define ASS_Roomstate8Str "Roomstate8"

#define ASS_room9temp 38
#define ASS_room9tempStr "Roomtemp9"
#define ASS_Roomstate9 39
#define ASS_Roomstate9Str "Roomstate9"
#define ASS_room9tempSet 40
#define ASS_room9tempSetStr "sliderValue9"

//pump

#define ASS_room10temp 41
#define ASS_room10tempStr "Roomtemp10"
#define ASS_Roomstate10 42
#define ASS_Roomstate10Str "Roomstate10"
#define ASS_room10tempSet 43
#define ASS_room10tempSetStr "sliderValue10"

#define ASS_room11temp 44
#define ASS_room11tempStr "Roomtemp11"
#define ASS_Roomstate11 45
#define ASS_Roomstate11Str "Roomstate11"
#define ASS_room11tempSet 46
#define ASS_room11tempSetStr "sliderValue11"

#define ASS_room12temp 47
#define ASS_room12tempStr "Roomtemp12"
#define ASS_Roomstate12 48
#define ASS_Roomstate12Str "Roomstate12"
#define ASS_room12tempSet 49
#define ASS_room12tempSetStr "sliderValue12"


#define ASS_cutOffTemp 50        // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
#define ASS_cutOffTempStr "sliderValue9"
#define ASS_retTemp  51        //Return temperature
#define ASS_retTempStr "retTemp"

#define ASS_roomDHTtemp 52
#define ASS_roomDHTtempStr "RoomDHTtemp"
#define ASS_roomDHThum 53
#define ASS_roomDHThumStr "RoomDHTHumid"

// #define ASS_dhwTarget 5         //hot water temp set
// #define ASS_dhwTargetStr "sliderValue2"
// #define ASS_roomtemp 7
// #define ASS_roomtempStr "roomtemp"
// #define ASS_roomtempSet 8
// #define ASS_roomtempSetStr "sliderValue4"
// #define ASS_AutoMode 9     //boiler mode NEWS_Temp, auto:NewsTemp+sp based on room tempset
// #define ASS_AutoModeStr "boilermodewww"
// #define ASS_EnableHeatingCO 10        //boiler heat for co heat/not
// #define ASS_EnableHeatingCOStr "boilerwww"
// #define ASS_EnableHotWater 11      //hotWater heat active/not
// #define ASS_EnableHotWaterStr "boilerhwwww"
// #define ASS_statusWaterActive 12
// #define ASS_statusWaterActiveStr "statusWaterActive"
// #define ASS_statusCHActive 13
// #define ASS_statusCHActiveStr "statusCHActive"
// #define ASS_statusFlameOn 14
// #define ASS_statusFlameOnStr "statusFlameOn"
// #define ASS_statusFault 15
// #define ASS_statusFaultStr "statusFault"
// #define ASS_tempBoiler 18
// #define ASS_tempBoilerStr "tempBoiler"
 #define ASS_UsedMedia 54           //Statistical data of used media
 #define ASS_UsedMediaStr "UsedMedia"
// #define ASS_tempBoilerSet 20     // boiler tempset on heat modetemp boiler set -mainly used in auto mode and for display on www actual temp
// #define ASS_tempBoilerSetStr "sliderValue1"
// #define ASS_ecoMode 21          //tryb pracy kondensacyjny -eco -temp grzania CO max 40st
// #define ASS_ecoModeStr "ecoMode"
#define ASS_modeState 55
#define ASS_modeStateStr "modeState"

#define ASS_Num (55 + 1)              //number of ASS definitions
#define numDecimalsWWW 1
#define cutoff_histereza 0.7        //histereza cutoff and roomtemp/roomtempset