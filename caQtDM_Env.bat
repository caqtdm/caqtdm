  
  IF "%QTHOME%"==""          set QTHOME=X:/qt/4.8.5
  IF "%QWTHOME%"==""         set QWTHOME=X:/qt/Qwt-6.0.1
  IF "%QWTINCLUDE%"==""      set QWTINCLUDE=X:/qt/Qwt-6.0.1/include
  IF "%QWTLIB%"==""          set QWTLIB=X:/qt/Qwt-6.0.1/lib
  IF "%EPICS_BASE%"==""      set EPICS_BASE=X:/epics/base-3.14.12.4
  IF "%EPICS_HOST_ARCH%"=="" set EPICS_HOST_ARCH=win32-x86
  IF "%EPICSINCLUDE%"==""    set EPICSINCLUDE=X:/epics/base-3.14.12.4/include
  IF "%EPICSEXTENSIONS%"=="" set EPICSEXTENSIONS=X:/Qt/4.8.5
  IF "%QTCONTROLS_LIBS%"=="" set QTCONTROLS_LIBS=X:/Qt/caqtdm_project/caQtDM_QtControls
  IF "%CAQTDM_COLLECT%"==""  set CAQTDM_COLLECT=X:/Qt/caqtdm_project/caQtDM_Binaries
  IF "%JOM%"==""             set JOM=X:\qt\jom
  
  
  
  set QTBASE=%QTCONTROLS_LIBS%
  
  
  IF "%QTDM_LIBINSTALL%"=="" 		set QTDM_LIBINSTALL=X:\Qt\4.8.5\lib
  IF "%QTDM_BININSTALL%"=="" 		set QTDM_BININSTALL=X:\qt\4.8.5\bin
  IF "%WIXHOME%"=="" 			set WIXHOME=C:\Program Files (x86)\WixEdit\wix-3.0.5419.0

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
echo CAQTDM_COLLECT       now defined as %CAQTDM_COLLECT%	for target path compile 
echo.
echo for install:
echo.
echo EPICSEXTENSIONS      	now defined as %EPICSEXTENSIONS%	for locating epics extensions
echo QTDM_LIBINSTALL      	now defined as %QTDM_LIBINSTALL%	for libraries install 
echo QTDM_BININSTALL      	now defined as %QTDM_BININSTALL%	for binaries install
echo WIXHOME      		now defined as %WIXHOME%		for package generation 
echo ============================================================================================
 




