@echo off
set _ping_cmd=wmic path Win32_SerialPort
for /f "tokens=8" %%i in ('%_ping_cmd% ^| findstr /I /C:"STMicroelectronics STLink Virtual COM Port"') do set COMADDR=%%i
set COMADDR=%COMADDR:~1,-1%
echo %COMADDR%
set COMADDR=%COMADDR:COM=%
echo on
"C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c -me -p .\InventekBootloaderPassthrough.bin 0x08000000 -v -rst -hardrst
timeout /t 10
"C:\Program Files (x86)\STMicroelectronics\Software\Flash Loader Demo\STMFlashLoader.exe" -c --pn %COMADDR% --br 115200  --db 8 --pr EVEN --sb 1 --to 30000 -i STM32F2_1024K -e --sec 10 0 1 2 3 4 5 6 7 8 9 -d --fn .\ISM43362_M3G_L44_SPI_9dBm_C3.5.2.3.BETA9.bin --a 0x08000000 --v
