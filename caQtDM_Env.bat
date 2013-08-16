  
  IF "%QTHOME%"==""          set QTHOME=C:/Qt/4.8.2
  IF "%QWTHOME%"==""         set QWTHOME=C:/Qt/Qwt-6.0.1
  IF "%QWTINCLUDE%"==""      set QWTINCLUDE=C:/Qt/Qwt-6.0.1/include
  IF "%QWTLIB%"==""          set QWTLIB=C:/Qt/Qwt-6.0.1/lib
  IF "%EPICS_BASE%"==""      set EPICS_BASE=C:/epics/base-3.14.12.2
  IF "%EPICSINCLUDE%"==""    set EPICSINCLUDE=C:/epics/base-3.14.12.2/include
  IF "%EPICSEXTENSIONS%"=="" set EPICSEXTENSIONS=C:/Qt/4.8.2
  IF "%QTCONTROLS_LIBS%"=="" set QTCONTROLS_LIBS=C:/qt/4.8.2/src/caQtDM_QtControls
  
  set QTBASE=%QTCONTROLS_LIBS%
  
  
  IF "%QTDM_LIBINSTALL%"=="" 		set QTDM_LIBINSTALL=C:\Qt\4.8.2\lib
  IF "%QTDM_BININSTALL%"=="" 		set QTDM_BININSTALL=C:\Qt\4.8.2\bin
  IF "%QTDM_BININSTALLPLUGIN%"=="" 	set QTDM_BININSTALL_PLUGIN=C:\qt\4.8.2\plugins\designer
  IF "%QTDM_RPATH%"==""      		set QTDM_RPATH=C:/Qt/4.8.2/bin

echo =============================================================================================
echo in order to build this package you will eventually have to redefine following variables in 
echo this file, they are taken from your environment if they exist, otherwise define them yourself:
echo.
echo for building:
echo.
echo QTHOME               now defined as %QTHOME%		for locating Qt
echo QWTHOME              now defined as %QWTHOME%		for locating qwt
echo QWTINCLUDE           now defined as %QWTINCLUDE%		for locating the include files of qwt
echo QWTLIB               now defined as %QWTLIB%		for locating the libraries of qwt
echo EPICS_BASE           now defined as %EPICS_BASE%		for locating epics 
echo EPICSINCLUDE         now defined as %EPICSINCLUDE%		for locating epics include files
echo EPICSLIB             now defined as %EPICSLIB%		for locating epics libraries
echo QTBASE               now defined as %QTBASE%		for building the package locally, pointing to caQtDM_Binaries
echo QTDM_RPATH           now defined as %QTDM_RPATH%		for runtime search path 
echo.
echo for install:
echo.
echo EPICSEXTENSIONS      	now defined as %EPICSEXTENSIONS%	for locating epics extensions
echo QTDM_LIBINSTALL      	now defined as %QTDM_LIBINSTALL%	for libraries install 
echo QTDM_BININSTALL      	now defined as %QTDM_BININSTALL%	for binaries install
echo QTDM_BININSTALLPLUGIN      now defined as %QTDM_BININSTALLPLUGIN%	for libraries install 
echo ============================================================================================
 




