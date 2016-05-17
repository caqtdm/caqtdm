@echo off
call caQtDM_Env.bat
echo =============================================================================================
echo.

echo binaries will be installed in %QTDM_BININSTALL%
echo libraries will be installed in %QTDM_LIBINSTALL%
echo plugins will be installed in %QTDM_LIBINSTALL%/designer
echo.

set PATH=%PATH%;%WIXHOME%
set WINCAQTDM_COLLECT=%CAQTDM_COLLECT:/=\%

cd %WINCAQTDM_COLLECT%
signtool sign /fd SHA256 /n %CAQTDM_SIGNER% /t %TIMESTAPER% caQtDM.exe caQtDM_Lib.dll qtcontrols.dll adl2ui.exe .\controlsystems\demo_plugin.dll .\controlsystems\epics3_plugin.dll .\controlsystems\epics4_plugin.dll .\designer\qtcontrols_controllers_plugin.dll .\designer\qtcontrols_graphics_plugin.dll .\designer\qtcontrols_monitors_plugin.dll
GOTO FINISHED



:FINISHED
cd..


pause
