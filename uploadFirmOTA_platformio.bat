@echo off
setlocal EnableDelayedExpansion
set BINFILE=.pio/build/esp32doit-devkit-v1/firmware.bin
set LITTLEFS=.pio\build\esp32doit-devkit-v1\littlefs.bin
set LOCALPORT=COM4
set OTAPORT=10.48.18.40

:startowanie
echo "PORT %OTAPORT%"
"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\espota.py" "-p 8266" --debug --progress -i %OTAPORT%  -f "%BINFILE%"
echo %errorlevel%
echo !errorlevel!
if  errorlevel 1 goto startowanie
:startowanie0
IF EXIST %LITTLEFS% (
    "%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\espota.py" "-p 8266" --debug --progress -i %OTAPORT%  -f "%LITTLEFS%" --spiffs --timeout=20
) ELSE (
    echo "No FS image -ignoring"
    goto startowanie2
)

echo %errorlevel%
echo !errorlevel!
if  errorlevel 1 goto startowanie0

set OTAPORT=10.48.18.30
:startowanie2
echo "PORT %OTAPORT%"
    "%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\espota.py" "-p 8266" --debug --progress -i %OTAPORT%  -f "%BINFILE%"
echo %errorlevel%
echo !errorlevel!
if  errorlevel 1 goto startowanie2
:startowanie2a
IF EXIST %LITTLEFS% (
    "%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\espota.py" "-p 8266" --debug --progress -i %OTAPORT%  -f "%LITTLEFS%" --spiffs --timeout=20
) ELSE (
    echo "No FS image -ignoring"
    goto startowanie2a
)

pause
exit


set BINFILE=.pio/build/d1_mini/firmw
set LOCALPORT=COM
set OTAPORTx=10.48.18.0
echo platformio ota
"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif8266\tools\espota.py" --debug --progress -i %OTAPORT% -f "%BINFILE%"
"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\espota.py" "-p 8266" --debug --progress -i 10.48.18.30 -f S:\EPROM\!Projekty\Floor_Heater_Control\.pio\build\esp32doit-devkit-v1\firmware.bin

echo arduino path:
echo "%USERPROFILE%\AppData\Local\Arduino15\packages\esp8266\tools\python3\3.7.2-post1/python3" -I "%USERPROFILE%\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2/tools/upload.py" --chip esp8266 --port "%LOCALPORT%" --baud 921600 --before default_reset --after hard_reset erase_flash write_flash 0x0 "%BINFILE%"

pause
