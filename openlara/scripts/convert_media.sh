#!/bin/bash

# Determine the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Check for ffmpeg in the script directory or system PATH
if [ -f "$SCRIPT_DIR/ffmpeg" ]; then
    FFMPEG="$SCRIPT_DIR/ffmpeg"
elif command -v ffmpeg &> /dev/null; then
    FFMPEG="ffmpeg"
else
    echo "ERROR: ffmpeg not found in script directory or PATH!"
    exit 1
fi

echo "Using ffmpeg: $FFMPEG"

echo "Converting TR1 Audio to 11025Hz Mono WAV..."
if [ -d "AUDIO/1" ]; then
    cd AUDIO/1 || exit
    for file in *.ogg *.mp3; do
        if [ -f "$file" ]; then
            echo "Processing $file..."
            filename="${file%.*}"
            "$FFMPEG" -y -i "$file" -ar 11025 -ac 1 "${filename}.wav"
            if [ -f "${filename}.wav" ]; then
                rm "$file"
            fi
        fi
    done
    cd ../..
fi

echo "Converting and Optimizing Backgrounds (DATA/*.PCX) to High-Quality PNG..."
if [ -d "DATA" ]; then
    cd DATA || exit
    for file in *.PCX *.pcx; do
        if [ -f "$file" ]; then
            echo "Processing $file..."
            filename="${file%.*}"
            # Lanczos scaling for high quality 320x240 conversion, keeping original PCX
            "$FFMPEG" -y -i "$file" -vf "scale=320:240:flags=lanczos" "${filename}.png"
        fi
    done
    cd ..
fi

echo "Resizing existing PNG Backgrounds to 320x240..."
if [ -d "LEVEL/1" ]; then
    cd LEVEL/1 || exit
    for file in *.png; do
        if [ -f "$file" ]; then
            echo "Processing $file..."
            filename="${file%.*}"
            "$FFMPEG" -y -i "$file" -vf "scale=320:240:flags=lanczos" "${filename}_temp.png"
            if [ -f "${filename}_temp.png" ]; then
                rm "$file"
                mv "${filename}_temp.png" "${filename}.png"
            fi
        fi
    done
    cd ../..
fi

echo "Setting high-quality Title Screen (TITLEH.png)..."
if [ -f "LEVEL/1/AMERTIT.png" ]; then
    cp "LEVEL/1/AMERTIT.png" "DATA/TITLEH.png"
fi

echo "Removing FMV folder (not supported on handheld)..."
rm -rf "FMV"

echo "Done!"
