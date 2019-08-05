@echo off
mkdir vs
cd vs
cmake -A Win32 -G "Visual Studio 16 2019" ../
pause
