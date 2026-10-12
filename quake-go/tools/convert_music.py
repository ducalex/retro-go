#!/usr/bin/env python3
"""Convert numbered MP3/OGG music to Retro-Go Quake PCM WAV (all targets).

Examples:
  python quake-go/tools/convert_music.py
  python quake-go/tools/convert_music.py /path/to/music --output /path/to/wav
With no arguments, scan this script's directory and its subdirectories.
Put FFmpeg in the ffmpeg subdirectory next to this script, or use --ffmpeg.
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import wave

SAMPLE_RATE = 22050
TOOLS_DIR = Path(__file__).resolve().parent


def sources(directory):
    tracks = {}
    for path in sorted(directory.iterdir()):
        if not path.is_file() or path.suffix.lower() not in (".ogg", ".mp3"):
            continue
        match = re.fullmatch(r"track(\d{2,3})", path.stem, re.IGNORECASE)
        if not match or not 2 <= int(match[1]) <= 255:
            continue
        track = int(match[1])
        if track not in tracks or path.suffix.lower() == ".ogg":
            tracks[track] = path
    return sorted(tracks.items())


def soundtrack_sources(directory, output=None):
    """Keep duplicate selection local to each album; never descend into outputs."""
    for current, directories, _ in os.walk(directory, followlinks=False):
        current = Path(current)
        directories[:] = sorted(name for name in directories
                                if name.lower() not in ("wav", "ffmpeg", "__pycache__")
                                and not (current / name).is_symlink()
                                and not ((current / name).stat().st_file_attributes & 0x400
                                         if os.name == "nt" else False)
                                and (output is None or (current / name).resolve() != output))
        tracks = sources(current)
        if tracks:
            yield current, tracks


def find_ffmpeg(explicit):
    if explicit:
        found = shutil.which(explicit)
        if found:
            return Path(found).resolve()
        path = Path(explicit).resolve()
        if path.is_file():
            return path
        raise ValueError("FFmpeg not found: " + explicit)
    names = ("ffmpeg.exe", "ffmpeg") if os.name == "nt" else ("ffmpeg",)
    for name in names:
        path = TOOLS_DIR / "ffmpeg" / name
        if path.is_file():
            return path.resolve()
    raise ValueError("FFmpeg not found; put your OS's executable in " +
                     str(TOOLS_DIR / "ffmpeg") + " or use --ffmpeg PATH")


def convert_one(ffmpeg, source, destination):
    # Always generate separately, then replace only after validating the result.
    fd, name = tempfile.mkstemp(prefix="." + destination.stem + "-", suffix=".wav", dir=destination.parent)
    os.close(fd)
    temporary = Path(name)
    try:
        command = [str(ffmpeg), "-hide_banner", "-loglevel", "error", "-nostdin", "-y",
                   "-i", str(source), "-map", "0:a:0", "-vn", "-ac", "2", "-ar", str(SAMPLE_RATE),
                   "-c:a", "pcm_s16le", "-map_metadata", "-1", "-f", "wav", str(temporary)]
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise ValueError(result.stderr.strip() or "FFmpeg failed")
        with wave.open(str(temporary), "rb") as wav:
            if (wav.getframerate(), wav.getnchannels(), wav.getsampwidth()) != (SAMPLE_RATE, 2, 2) or not wav.getnframes():
                raise ValueError(f"FFmpeg did not produce a nonempty {SAMPLE_RATE} Hz stereo PCM16 WAV")
        os.replace(temporary, destination)
    finally:
        temporary.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", nargs="?", type=Path, default=TOOLS_DIR,
                        help="tree containing track02/track002.ogg/mp3 (default: this script's directory)")
    parser.add_argument("--output", type=Path,
                        help="output root preserving source subfolders (default: each soundtrack's wav subfolder)")
    parser.add_argument("--ffmpeg", help="FFmpeg executable path")
    parser.add_argument("--overwrite", action="store_true", help="replace existing WAVs after successful conversion")
    args = parser.parse_args()
    try:
        source = args.source.resolve()
        if not source.is_dir():
            raise ValueError("Input directory not found: " + str(source))
        output = args.output.resolve() if args.output else None
        if output == source:
            raise ValueError("Output directory must differ from the input root")
        soundtracks = list(soundtrack_sources(source, output))
        if not soundtracks:
            raise ValueError("No trackNN.ogg or trackNN.mp3 files found (track02 through track255)")
        ffmpeg = find_ffmpeg(args.ffmpeg)
        failed = 0
        for folder, tracks in soundtracks:
            destination_dir = output / folder.relative_to(source) if output else folder / "wav"
            destination_dir.mkdir(parents=True, exist_ok=True)
            for track, path in tracks:
                destination = destination_dir / f"track{track:02d}.wav"
                if destination.exists() and not args.overwrite:
                    print(f"Keeping existing {destination}")
                    continue
                print(f"Converting {path} -> {destination}", flush=True)
                try:
                    convert_one(ffmpeg, path, destination)
                except (OSError, ValueError, wave.Error, EOFError) as error:
                    failed += 1
                    print(f"Failed {path}: {error}", file=sys.stderr)
        if failed:
            return 1
        print(f"Ready: {len(soundtracks)} soundtrack folder(s)\nFormat: {SAMPLE_RATE} Hz, stereo, 16-bit PCM WAV")
        print("Copy these WAVs to the appropriate quake/id1/music, hipnotic/music or rogue/music directory.")
        return 0
    except (OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
