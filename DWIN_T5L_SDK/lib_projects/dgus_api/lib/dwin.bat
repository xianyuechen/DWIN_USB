@echo off
@cd lib
@copy>nul 2>nul *.LIB ..\..\..\lib\*.lib
@srec_cat.exe>nul 2>nul *.hex -Intel -o T5L51.bin -Binary
@del>nul 2>nul *.obj
@del>nul 2>nul *.lnp
@del>nul 2>nul *.SBR
@del>nul 2>nul *.__b
echo !!!!lib copy finished and file clean done!!!!