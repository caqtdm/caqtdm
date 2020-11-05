@echo off
Setlocal EnableDelayedExpansion
echo "========== Parameter ============"
call caQtDM_Env.bat
if "%CAQTDM_GENERAL_COMPILATION%"=="1" GOTO :GENERAL_COMPILATION
echo.
echo "========== create destination directory if not exists============"
echo.
IF NOT EXIST %CAQTDM_COLLECT% (mkdir %CAQTDM_COLLECT%)
echo "package will be build in %CAQTDM_COLLECT%"
echo.
call :compile
pause
GOTO:eof


:GENERAL_COMPILATION

SETLOCAL
REM =============================================================================================
REM GENERAL_COMPILATION SELECT1
REM =============================================================================================
call caQtDM_Env.bat 2
call caQtDM_CleanAll.bat
REM call :compile
REM call caQtDM_CleanAll.bat
ENDLOCAL
SETLOCAL
REM =============================================================================================
REM GENERAL_COMPILATION SELECT2
REM =============================================================================================
call caQtDM_Env.bat 2
call :compile
call caQtDM_CleanAll.bat
ENDLOCAL
SETLOCAL
REM =============================================================================================
REM GENERAL_COMPILATION SELECT3
REM =============================================================================================
call caQtDM_Env.bat 3
call :compile
call caQtDM_CleanAll.bat
ENDLOCAL
GOTO:eof

:compile
echo ============ make all =================
set PATH=%PATH%;%JOM%
set PATH=%QTHOME%\bin;%PATH%
set PATH=%PATH%;%GITPATH%
set /a timerstart=(((1%time:~0,2%-100)*60*60)+((1%time:~3,2%-100)*60)+(1%time:~6,2%-100)^)
qmake all.pro
where /q jom.exe 
IF %ERRORLEVEL% NEQ 0 (nmake release) ELSE (jom release)
set /a timerstop=(((1%time:~0,2%-100)*60*60)+((1%time:~3,2%-100)*60)+(1%time:~6,2%-100)^)
set /a timeseks=(%timerstop%-%timerstart%)
set /a timemins=(%timerstop%-%timerstart%)/60
echo caQtDM compile time seconds: %timeseks% -- minutes: %timemins%
GOTO:eof