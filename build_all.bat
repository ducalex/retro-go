@ECHO OFF

echo ""
echo "Building the launcher"
cd retro-go
rmdir /S /Q build
idf.py build || goto :error

echo ""
echo "Building the NES emulator"
cd ../nofrendo-go
rmdir /S /Q build
idf.py build || goto :error

echo ""
echo "Building the Gameboy emulator"
cd ../gnuboy-go
rmdir /S /Q build
idf.py build || goto :error

echo ""
echo "Building the PC Engine emulator"
cd ../huexpress-go
rmdir /S /Q build
idf.py build || goto :error

echo ""
echo "Building the SMS/Gamegear/Coleco emulator"
cd ../smsplusgx-go
rmdir /S /Q build
idf.py build || goto :error

echo ""
echo "Building the Lynx emulator"
cd ../handy-go
rmdir /S /Q build
idf.py build || goto :error

echo ""
echo "All done!"
cd ..

exit /b

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
