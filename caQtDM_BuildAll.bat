@echo off
SETLOCAL ENABLEEXTENSIONS
echo "========== Parameter ============"
call caQtDM_Env.bat

set PATH=%PATH%;%JOM%
set PATH=%PATH%;%QTHOME%\bin

echo.
echo "========== create destination directory if not exists============"
echo.






if exist ".\caQtDM_Binaries" (
 echo "directory .\caQtDM_Binaries exists already"
) else (
 echo "create directory ./caQtDM_Binaries"
 md .\caQtDM_Binaries
)

if exist .\caQtDM_Binaries\designer ( 
echo "directory .\caQtDM_Binaries\designer exists already"
) else (
 echo "create directory ./caQtDM_Binaries/designer"
 md .\caQtDM_Binaries\designer
)

echo "package will be build in ./caQtDM_Binaries"
echo.
echo "Press [Enter] key to start build "
pause

echo ============ make all =================
qmake all.pro
where /q jom.exe 
IF %ERRORLEVEL% NEQ 0 (nmake all) ELSE (jom all)

pause