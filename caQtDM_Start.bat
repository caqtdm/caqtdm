@echo off
Setlocal EnableDelayedExpansion
echo "========== Parameter ============"
call caQtDM_Env.bat

set "CAQTDM_COLLECT=%CAQTDM_COLLECT:/=\%"

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


echo =============================================================================================
echo ============ Parameter =================
echo Enter Parameter  
set /P PARAMETER=Select: 
echo =============================================================================================




echo ============ start caqtdm =================
set PATH=%PATH%;%JOM%
set PATH=%PATH%;%QTHOME%\bin
set PATH=%PATH%;%QWTLIB%




IF %SELCTION%==1 GOTO :SELECTRelease
IF %SELCTION%==2 GOTO :SELECTDebug



:SELECTDebug
set QT_PLUGIN_PATH=%CAQTDM_COLLECT%\debug\designer
set PATH=%PATH%;%EPICS_BASE%\bin\windows-x64
set PATH=%CAQTDM_COLLECT%\debug;%PATH%
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTHOME%\plugins\platforms
set "QT_QPA_PLATFORM_PLUGIN_PATH=%QT_QPA_PLATFORM_PLUGIN_PATH:/=\%"
echo %QT_QPA_PLATFORM_PLUGIN_PATH%
caqtdm %PARAMETER%

GOTO:eof

:SELECTRelease
set QT_PLUGIN_PATH=%CAQTDM_COLLECT%\designer
set PATH=%PATH%;%EPICS_BASE%\bin\windows-x64
set PATH=%CAQTDM_COLLECT%;%PATH%
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTHOME%\plugins\platforms
set "QT_QPA_PLATFORM_PLUGIN_PATH=%QT_QPA_PLATFORM_PLUGIN_PATH:/=\%"
echo %QT_QPA_PLATFORM_PLUGIN_PATH%
caqtdm %PARAMETER%

GOTO:eof


GOTO:eof