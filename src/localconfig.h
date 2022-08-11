#include "Arduino.h"
String kondygnacja = "0";  //This changes in setup automatically -depends on pin connection to recognize floor
String me_lokalizacja = "FLOORH"  + String(loctmp); //base name


#define choosepin 18 //15 must be low during boot 34 have no internal pullup -only input         //pin for choose if low than kond=1 else kond=2

//#if kondygnacja == 1
#define dzawor1addr1   "281234760400008f"  //+0-1-mama -o+13 -odczyty z dawnej powrotu podlogi teraz 1w
#define dzawor2addr1   "281d8e7604000059"  //+0-8-piotrek -pok 8
#define dzawor3addr1   "28a375e103000086"  //0-2: Podloga 2 lazienka+wc
#define dzawor4addr1   "284f61e1030000b1"  //+0-7-maja
#define dzawor5addr1   "287845e10300003e"  //0-3: Podloga 3 Kuchnia
#define dzawor6addr1   "2847375d040000e0"  //+0-6-salon
#define dzawor7addr1   "28875076040000db"  //0-4: Podloga 4 kuchnia
#define dzawor8addr1   "28c39a5d04000009"  //0-5
#define dzawor9addr1   "283b4c64040000c3"   //+zas,we pumpoff
#define dzawor10addr1  "281a67630400004c"  //listwa góra    tempwename                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         00004c"   //+listwa we
#define dzawor11addr1  "283b4c6404000000"   //Zas.4D dol--
#define dzawor12addr1  "28d13fe103000024"   //+Zasilanie-powrot (od zaworu 4D lewo)
#define dzawor13addr1  "28ff256d80140266"  //komora temp -czujnik weryfikacyjny na PBA

//#endif
//#if kondygnacja==2
#define dzawor1addr2   "28ffe13f80140284"    //0-1-mama -o+13 -odczyty z dawnej powrotu podlogi teraz 1w
#define dzawor2addr2   "28ff684280140289"    //0-8-piotrek -pok 8 28ffd06480140289
#define dzawor3addr2   "28ff256d80100000" //"28ff056e80140216"  //0-2: Podloga 2 lazienka+wc
#define dzawor4addr2   "28ffd06480140281"    //0-7-maja 28ff684280140281 /podmienilem z 21 bo ten ma wysoki odczyt
#define dzawor5addr2   "28ff556580140217"  //0-3: Podloga 3 Kuchnia
#define dzawor6addr2   "28ffcd43801402e7"    //0-6-salon
#define dzawor7addr2   "28ffb7658014024e"  //0-4: Podloga 4 kuchnia
#define dzawor8addr2   "28ff2e658014020a"  //0-5
#define dzawor9addr2   "28ffca5224170362"     //pumpoff
#define dzawor10addr2  "28ff5867801402fc"   //tempwename
#define dzawor11addr2  "28ff2f57241703eb"  //b Zas.4D dol--   tttttttttttyyyyyyyyyyyyyyyyyyyymmmmmmmmmmmmmmmmmmmmmmmmmccccccccccccccccccczzzzzzzzzzzzzzzzzzaaaaaaaaaaaaaaaasssssssssssssssss
#define dzawor12addr2  "28ff347080140272"  //Zasilanie-powrot (od zaworu 4D lewo)
#define dzawor13addr2  "28ff266b80140221"  //komora temp -czujnik weryfikacyjny na PBA

//#endif

// Temperature sensor pin
const int DALLAS_SENSOR_PIN = 15; //18b20pin 0; //for Arduino, 14 for ESP8266 (D5), 18 for ESP32

#define zawor1 13
#define zawor2 19 //12 must be low during boot
#define zawor3 14
#define zawor4 27
#define zawor5 26
#define zawor6 25
#define zawor7 33
#define zawor8 32
#define zawor9 21 //pump
#define zawor10 22
#define zawor11 23 //reserved pompa_alter_pin
#define zawor12 -1 //reserved pompa_alter_pin
#define zawor13 -1 //reserved pompa_alter_pin

#ifdef enableDHT
#define DHTPIN 5     // Digital pin connected to the DHT sensor
// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)
#endif

#define tempwename "WEgora_listwa"     //nazwa czujnika temp jako odniesienie ciepla grzania
#define tempcutoff "CUTOFF-WE_z4Ddol"  //glowne wylaczanie pompy i wykorzystany tempset do wylaczania papy zawor 4D dol zasilanie
//remember to give name of max namelength variable -default 15 chars
#define dzawor1name "MAMA"                   //1 original
#define dzawor2name "PIOTREK"                //8 original
#define dzawor3name "LAZIENKA_WC"            //2 original
#define dzawor4name "MAJA"                   //7 original
#define dzawor5name "KUCHNIA1"               //3 original
#define dzawor6name "SALON"                  //6 original
#define dzawor7name "KUCHNIA2"               //4 original
#define dzawor8name "GANEK"                  //5 original
#define dzawor9name tempcutoff  //zawor 4d dol
#define dzawor10name tempwename    //listwa we -temp odcinania pompy
#define dzawor11name "unknown"
#define dzawor12name "RETURN_zaw.4D_lewo"      //wyjscie z podlogi
#define dzawor13name "temp_komora"

#define appendSet "_Set"


const String sepkondname = ":"; //separator between kondygnacja number and sensor name
#define pumpofftemp 24.3  //24.3 temperatura odlaczenia papy z czujnika 1W na dole zaworu 4d
// #define DS18B20nodata -127
// #define DS18B20nodata2 -255
#define limitsetsensor 9 //limitacja wskazania na web suwaka ustawienia tempset -czujniki + prog pompy
#define temptoheat 8
#define temptocool 25


#define uptimelink "uptimelink"
#define dallThermometerS "dallThermometerS"

#define tempCORETThermometerS "tempCORETThermometerS"

#define stopkawebsite "stopkawebsite"

#define cutOffTempS "cutOffTempS"
#define cutOffTempSet "cutOffTempSet"
#define cutOffTempVAL "tempCutOffset"
#define flameprocS "flameprocS"
#define flameprocSVAL "flameprocSVAL"
#define NEWS_lastTimeS "NEWS_lastTimeS"
#define NEWS_lastTime "NEWS_lastTime"
#define do_stopkawebsiteS "do_stopkawebsiteS"
#define dhumidcor "dhumidcor"
#define dtempcor "dtempcor"


#define dhwTargetStart 51           // domyslna temperatura docelowa wody uzytkowej
#define cutoffstartval 2
#define roomtempinitial 21
#define ecoModeMaxTemp 39           //max temp pracy kondensacyjnej
#define ecoModeDisabledMaxTemp 60
