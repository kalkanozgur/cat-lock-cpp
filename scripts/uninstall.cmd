@echo off
rem Cift tikla: yonetici izni ister, uninstall.ps1'i calistirir. Ayar ve log kalir.
powershell -NoProfile -Command "Start-Process powershell -Verb RunAs -ArgumentList '-NoProfile','-NoExit','-ExecutionPolicy','Bypass','-File','\"%~dp0uninstall.ps1\"'"
