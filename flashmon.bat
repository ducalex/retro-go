@ECHO OFF

set COMPORT=%1
set OFFSET=%2
for /f "delims=" %%i in ('dir /b build\*.bin') do set BINFILE="build\%%i"

python.exe %IDF_PATH%\components\esptool_py\esptool\esptool.py --chip esp32 --port %COMPORT% --baud 1152000 --before default_reset write_flash -u --flash_mode qio --flash_freq 80m --flash_size detect %OFFSET% %BINFILE% || goto :error

idf.py monitor -p %COMPORT%

exit /b

:error
echo Flash failed
