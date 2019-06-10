@echo off
set PATH="C:\Program Files (x86)\CMake\bin\";%PATH%
mkdir vs
cd vs
cmake -G "Visual Studio 15" ../
pause