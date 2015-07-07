@echo off
Setlocal EnableDelayedExpansion
echo "========== Parameter ============"
call caQtDM_Env.bat
echo.
echo "========== create destination directory if not exists============"
echo.
echo.
call :run_caqtdm
pause
GOTO:eof


:run_caqtdm
echo =============================================================================================
echo ============ Type Release/Debug =================
echo 1) Release  
echo 2) Debug  
set /P SELCTION=Select: 
echo =============================================================================================


echo ============ start caqtdm =================
set PATH=%PATH%;%JOM%
set PATH=%PATH%;%QTHOME%\bin
set PATH=%PATH%;%QWTLIB%

IF %SELCTION%==1 GOTO :SELECTRelease
IF %SELCTION%==2 GOTO :SELECTDebug



:SELECTDebug
set QT_PLUGIN_PATH=X:\qt\caqtdm_project\caQtDM_Binaries_64Bit\debug\designer
set PATH=%PATH%;%EPICS_BASE%\bin\windows-x64
set PATH=%PATH%;%CAQTDM_COLLECT%\debug
caqtdm

GOTO:eof

:SELECTRelease
set QT_PLUGIN_PATH=X:\qt\caqtdm_project\caQtDM_Binaries_64Bit\designer
set PATH=%PATH%;%EPICS_BASE%\bin\windows-x64
set PATH=%PATH%;%CAQTDM_COLLECT%
caqtdm

GOTO:eof


GOTO:eof