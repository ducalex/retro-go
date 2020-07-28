@ECHO OFF

echo ""
echo "Building the launcher"
cd retro-go
del /q build\main\* build\odroid\*
idf.py build || goto :error

echo ""
echo "Building the NES emulator"
cd ../nofrendo-go
del /q build\main\* build\odroid\*
idf.py build || goto :error

echo ""
echo "Building the Gameboy emulator"
cd ../gnuboy-go
del /q build\main\* build\odroid\*
idf.py build || goto :error

echo ""
echo "Building the PC Engine emulator"
cd ../huexpress-go
del /q build\main\* build\odroid\*
idf.py build || goto :error

echo ""
echo "Building the SMS/Gamegear/Coleco emulator"
cd ../smsplusgx-go
del /q build\main\* build\odroid\*
idf.py build || goto :error

echo ""
echo "Building the Lynx emulator"
cd ../handy-go
del /q build\main\* build\odroid\*
idf.py build || goto :error

echo ""
echo "All done!"
cd ..

exit /b

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
