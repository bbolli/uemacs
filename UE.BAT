@echo off
if not exist emacs.sym goto ERR
if not exist emacs.exe goto ERR
symdeb -s -"g _main" emacs.sym emacs.exe %1 %2 %3 %4 %5 %6 %7 %8 %9
goto END
:ERR
echo Weder .SYM noch .EXE gefunden
:END
