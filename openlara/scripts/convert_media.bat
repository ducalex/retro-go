@echo off
setlocal enabledelayedexpansion

:: Get the absolute path to ffmpeg.exe in the current directory
set FFMPEG_PATH=%~dp0ffmpeg.exe

:: Check if ffmpeg exists in current directory
if not exist "%FFMPEG_PATH%" (
    echo ERROR: ffmpeg.exe not found in the current directory!
    echo Please ensure ffmpeg.exe is in: %~dp0
    pause
    exit /b 1
)

echo Converting TR1 Audio to 11025Hz Mono WAV...
if exist "AUDIO\1" (
    cd AUDIO\1
    for %%F in (*.ogg *.mp3) do (
        echo Processing %%F...
        "%FFMPEG_PATH%" -y -i "%%F" -ar 11025 -ac 1 "%%~nF.wav"
        if exist "%%~nF.wav" del "%%F"
    )
    cd ..\..
)

echo Converting and Optimizing Backgrounds (DATA/*.PCX) to High-Quality PNG...
if exist "DATA" (
    cd DATA
    for %%F in (*.PCX *.pcx) do (
        echo Processing %%F...
        :: Using Lanczos scaling for high quality 320x240 conversion, keeping original PCX intact
        "%FFMPEG_PATH%" -y -i "%%F" -vf "scale=320:240:flags=lanczos" "%%~nF.png"
    )
    cd ..
)

echo Resizing existing PNG Backgrounds to 320x240...
if exist "LEVEL\1" (
    cd LEVEL\1
    for %%F in (*.png) do (
        echo Processing %%F...
        "%FFMPEG_PATH%" -y -i "%%F" -vf "scale=320:240:flags=lanczos" "%%~nF_temp.png"
        if exist "%%~nF_temp.png" (
            del "%%F"
            move "%%~nF_temp.png" "%%~nF.png"
        )
    )
    cd ..\..
)

echo Setting high-quality Title Screen (TITLEH.png)...
if exist "LEVEL\1\AMERTIT.png" (
    copy "LEVEL\1\AMERTIT.png" "DATA\TITLEH.png"
)

echo Removing FMV folder (not supported on handheld)...
if exist "FMV" rd /s /q "FMV"

echo Done!
pause
