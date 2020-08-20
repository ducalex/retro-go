@ECHO OFF

echo ""
echo "Building the launcher"
cd retro-go
idf.py fullclean
idf.py app || goto :error

echo ""
echo "Building the NES emulator"
cd ../nofrendo-go
idf.py fullclean
idf.py app || goto :error

echo ""
echo "Building the Gameboy emulator"
cd ../gnuboy-go
idf.py fullclean
idf.py app || goto :error

echo ""
echo "Building the PC Engine emulator"
cd ../huexpress-go
idf.py fullclean
idf.py app || goto :error

echo ""
echo "Building the SMS/Gamegear/Coleco emulator"
cd ../smsplusgx-go
idf.py fullclean
idf.py app || goto :error

echo ""
echo "Building the Lynx emulator"
cd ../handy-go
idf.py fullclean
idf.py app || goto :error

echo ""
echo "All done!"
cd ..

exit /b

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
