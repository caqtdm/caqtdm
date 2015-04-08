@echo off
Setlocal EnableDelayedExpansion
echo "========== Parameter ============"
call caQtDM_Env.bat
if "%CAQTDM_GENERAL_COMPILATION%"=="1" GOTO :GENERAL_COMPILATION
echo.
echo "========== create destination directory if not exists============"
echo.

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
call caQtDM_Env.bat 1
call caQtDM_CleanAll.bat
call :compile
call caQtDM_CleanAll.bat
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
set PATH=%PATH%;%QTHOME%\bin
qmake all.pro
where /q jom.exe 
IF %ERRORLEVEL% NEQ 0 (nmake all) ELSE (jom all)
GOTO:eof