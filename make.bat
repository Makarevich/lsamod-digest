@echo off

set CLROOT=d:\cl
set CLBIN=%CLROOT%\bin
set INCLUDE=%CLROOT%\include
set LIB=%CLROOT%\lib

%CLBIN%\nmake.exe /NOLOGO /S %1 %2 %3
