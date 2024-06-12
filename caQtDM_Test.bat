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
set SELCTION=1
set PARAMETER=D:\qt\caqtdm_project\caQtDM_Tests\tests.ui



echo ============ start caqtdm =================
set PATH=%PATH%;%JOM%
set PATH=%QTHOME%\bin;%PATH%
set PATH=%QWTLIB%;%PATH%




IF %SELCTION%==1 GOTO :SELECTRelease
IF %SELCTION%==2 GOTO :SELECTDebug



:SELECTDebug
set QT_PLUGIN_PATH=%CAQTDM_COLLECT%\debug\designer
set PATH=%PATH%;%EPICS_BASE%\bin\%EPICS_HOST_ARCH%
set PATH=%CAQTDM_COLLECT%\debug;%PATH%
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTHOME%\plugins\platforms
set "QT_QPA_PLATFORM_PLUGIN_PATH=%QT_QPA_PLATFORM_PLUGIN_PATH:/=\%"

caqtdm %PARAMETER%

GOTO:eof

:SELECTRelease
set QT_PLUGIN_PATH=%CAQTDM_COLLECT%\designer
set PATH=%PATH%;%EPICS_BASE%\bin\%EPICS_HOST_ARCH%:"C:\Program Files (x86)\Dr. Memory\bin64\"
set PATH=%CAQTDM_COLLECT%;%SSLLIB%;%PATH%
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTHOME%\plugins\platforms
set "QT_QPA_PLATFORM_PLUGIN_PATH=%QT_QPA_PLATFORM_PLUGIN_PATH:/=\%"

drmemory.exe -leaks_only -- caqtdm %PARAMETER%

GOTO:eof


GOTO:eof