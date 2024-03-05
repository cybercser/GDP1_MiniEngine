@echo off
setlocal

set asset_directory=%1
set bin_directory=%2

set "processed_file=%asset_directory%\.processed_files.txt"

if not exist "%asset_directory%" (
    echo Assets directory does not exist.
    exit /b 1
)

if not exist "%bin_directory%" (
    echo Tools directory not found.
    exit /b 1
)

if not exist "%processed_file%" (
    type nul > "%processed_file%"
)

for /r "%asset_directory%" %%I in (*.png) do (
    findstr /i "%%~nxI" "%processed_file%" > nul
    if errorlevel 1 (
        "%bin_directory%\pngquant" --speed 5 --skip-if-larger --quality=65-80 --force --ext .png "%%~fI"
        "%bin_directory%\optipng" -o3 "%%~fI"
        "%bin_directory%\PVRTexTool" -i "%%~fI" -o "%%~dpnI.dds"
        echo %%~nxI >> "%processed_file%"
    )
)

echo Processing complete.
exit /b 0