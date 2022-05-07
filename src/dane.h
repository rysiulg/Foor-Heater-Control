
#if kondygnacja == 1
#define dzawor1addr   "281234760400008f"  //+0-1-mama -o+13 -odczyty z dawnej powrotu podlogi teraz 1w
#define dzawor2addr   "281d8e7604000059"  //+0-8-piotrek -pok 8
#define dzawor3addr   "28a375e103000086"  //0-2: Podloga 2 lazienka+wc
#define dzawor4addr   "284f61e1030000b1"  //+0-7-maja
#define dzawor5addr   "287845e10300003e"  //0-3: Podloga 3 Kuchnia
#define dzawor6addr   "2847375d040000e0"  //+0-6-salon
#define dzawor7addr   "28875076040000db"  //0-4: Podloga 4 kuchnia
#define dzawor8addr   "28c39a5d04000009"  //0-5
#define dzawor9addr   "283b4c64040000c3"   //+zas,we pumpoff
#define dzawor10addr  "281a67630400004c"   //+listwa we
#define dzawor11addr  "283b4c6404000000"   //Zas.4D dol--
#define dzawor12addr  "28d13fe103000024"   //+Zasilanie-powrot (od zaworu 4D lewo)
#define dzawor13addr  "28ff5867801302fd"  //nie wiem bzy

#endif
#if kondygnacja==2
#define dzawor1addr   "28ffe13f80140284"    //0-1-mama -o+13 -odczyty z dawnej powrotu podlogi teraz 1w
#define dzawor2addr   "28ff684280140289"    //0-8-piotrek -pok 8 28ffd06480140289
#define dzawor3addr   "28ff256d80140266" //"28ff056e80140216"  //0-2: Podloga 2 lazienka+wc
#define dzawor4addr   "28ffd06480140281"    //0-7-maja 28ff684280140281 /podmienilem z 21 bo ten ma wysoki odczyt
#define dzawor5addr   "28ff556580140217"  //0-3: Podloga 3 Kuchnia
#define dzawor6addr   "28ffcd43801402e7"    //0-6-salon
#define dzawor7addr   "28ffb7658014024e"  //0-4: Podloga 4 kuchnia
#define dzawor8addr   "28ff2e658014020a"  //0-5
#define dzawor9addr   "28ffca5224170362"     //pumpoff
#define dzawor10addr  "28ff5867801402fc"   //tempwename
#define dzawor11addr  "28ff2f57241703eb"  //b Zas.4D dol--   tttttttttttyyyyyyyyyyyyyyyyyyyymmmmmmmmmmmmmmmmmmmmmmmmmccccccccccccccccccczzzzzzzzzzzzzzzzzzaaaaaaaaaaaaaaaasssssssssssssssss
#define dzawor12addr  "28ff347080140272"  //Zasilanie-powrot (od zaworu 4D lewo)
#define dzawor13addr  "28ff586780130200"  //nie wiem bzy
#endif

// Temperature sensor pin
const int ROOM_TEMP_SENSOR_PIN = 15; // 0; //for Arduino, 14 for ESP8266 (D5), 18 for ESP32

#define zawor1 13
#define zawor2 32 //35 34x
#define zawor3 12
#define zawor4 33
#define zawor5 14
#define zawor6 25
#define zawor7 27
#define zawor8 26
#define zawor9 21 //pump
#define zawor10 22 //reserved pompa_alter_pin
#define zawor11 23 //reserved pompa_alter_pin
#define zawor12 0 //reserved pompa_alter_pin
#define zawor13 0 //reserved pompa_alter_pin


#define tempwename "WEgora listwa"     //nazwa czujnika temp jako odniesienie ciepla grzania
#define tempcutoff "CUTOFF-WE(z4Ddol)"  //glowne wylaczanie pompy i wykorzystany tempset do wylaczania papy zawor 4D dol zasilanie
//remember to give name of max namelength variable -default 15 chars
#define dzawor1name "MAMA"                   //1 original
#define dzawor2name "PIOTREK"                //8 original
#define dzawor3name "LAZIENKA+WC"            //2 original
#define dzawor4name "MAJA"                   //7 original
#define dzawor5name "KUCHNIA1"               //3 original
#define dzawor6name "SALON"                  //6 original
#define dzawor7name "KUCHNIA2"               //4 original
#define dzawor8name "GANEK"                  //5 original
#define dzawor9name tempcutoff  //zawor 4d dol
#define dzawor10name tempwename    //listwa we -temp odcinania pompy
#define dzawor11name "unknown"
#define dzawor12name "RETURN(zaw.4D lewo)"      //wyjscie z podlogi
#define dzawor13name "nie wiem bzy"


const String zawor1name = dzawor1name,
     zawor2name = dzawor2name,
     zawor3name = dzawor3name,
     zawor4name = dzawor4name,
     zawor5name = dzawor5name,
     zawor6name = dzawor6name,
     zawor7name = dzawor7name,
     zawor8name = dzawor8name,
     zawor9name = dzawor9name,
     zawor10name = dzawor10name,
     zawor11name = dzawor11name,
     zawor12name = dzawor12name,
     zawor13name = dzawor13name,
     zawor1addr = dzawor1addr,
     zawor2addr = dzawor2addr,
     zawor3addr = dzawor3addr,
     zawor4addr = dzawor4addr,
     zawor5addr = dzawor5addr,
     zawor6addr = dzawor6addr,
     zawor7addr = dzawor7addr,
     zawor8addr = dzawor8addr,
     zawor9addr = dzawor9addr,
     zawor10addr = dzawor10addr,
     zawor11addr = dzawor11addr,
     zawor12addr = dzawor12addr,
     zawor13addr = dzawor13addr;


const String sepkondname = ":"; //separator between kondygnacja number and sensor name
#define pumpofftemp 24.3  //24.3 temperatura odlaczenia papy z czujnika 1W na dole zaworu 4d
#define InitTemp 85             //temperatura inicjalna gdy brak odczuty
#define DS18B20nodata -127.00

#define part_room  "room"
#define part_room_after "tempS"
#define part_set "etS"
#define roomtempS part_room part_room_after
#define roomtempSetS roomtempS part_set
#define roomtempset part_room"tempset"
const char* PARAM_roomtempset0 = roomtempset"_1";
const char* PARAM_roomtempset1 = roomtempset"_2";
const char* PARAM_roomtempset2 = roomtempset"_3";
const char* PARAM_roomtempset3 = roomtempset"_4";
const char* PARAM_roomtempset4 = roomtempset"_5";
const char* PARAM_roomtempset5 = roomtempset"_6";
const char* PARAM_roomtempset6 = roomtempset"_7";
const char* PARAM_roomtempset7 = roomtempset"_8";
const char* PARAM_roomtempset8 = roomtempset"_9";

#define roomtempS4 part_room part_room_after "4"
#define roomtempSetS4 part_room part_room_after "4"


bool pump;
int pompa_pin;
float  pumpOffVal;
String dana;
