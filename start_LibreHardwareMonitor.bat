@echo off

set processo=LibreHardwareMonitor.exe

tasklist | findstr /i %processo% > nul

if %errorlevel% equ 0 (
	echo Ja esta aberto %processo%
) else (
	echo Vai abrir %processo%
    start /min LibreHardwareMonitor\LibreHardwareMonitor.exe
)
