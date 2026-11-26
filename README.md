# 7-Zip Plugin for .zar ZArchive file

Easily create and extract **`.zar`** / **`.wua`** archives in 7-Zip.

Based on [Exzap/ZArchive](https://github.com/Exzap/ZArchive).

## Install

1. Download `ZArchive.dll` from the [latest release](https://github.com/victorator/7zipzar/releases/latest).
2. Copy it to:
   ```
   C:\Program Files\7-Zip\Formats\ZArchive.dll
   ```
   Create the `Formats` folder if it does not exist.
3. Restart 7-Zip File Manager.

Requires **7-Zip 64-bit (x64)**.

## Usage

- **Extract / browse:** open any `.zar` or `.wua` file in 7-Zip.
- **Create:** *Add to archive* → format **ZArchive** (or CLI: `7z a -tZArchive archive.zar folder\`).

## Features

| Feature              | Supported |
|----------------------|-----------|
| List files           | Yes       |
| Extract files        | Yes       |
| Create new archive   | Yes       |
| Update existing .zar | No        |

## Build (optional)

Windows x64, CMake + MSVC + Ninja:

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: `build\ZArchive.dll`

## Credits

- [ZArchive](https://github.com/Exzap/ZArchive) by Exzap (MIT No Attribution)
- [zstd](https://github.com/facebook/zstd) by Facebook (BSD / GPLv2)
- 7-Zip plugin API / headers by Igor Pavlov
