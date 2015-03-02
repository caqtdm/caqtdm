@echo off
SETLOCAL ENABLEEXTENSIONS
echo "========== Parameter ============"
call caQtDM_Env.bat

set PATH=%PATH%;%JOM%
set PATH=%PATH%;%QTHOME%\bin

echo.
echo "========== create destination directory if not exists============"
echo.

echo "package will be build in %CAQTDM_COLLECT%"
echo.

echo ============ make all =================
qmake all.pro
where /q jom.exe 
IF %ERRORLEVEL% NEQ 0 (nmake all) ELSE (jom all)

pause