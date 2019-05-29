@echo off
set PATH="C:\Program Files\CMake\bin\";%PATH%
mkdir vs
cd vs
cmake -G "Visual Studio 16" ../
pause