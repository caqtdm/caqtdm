@echo off
set PATH=C:\epics\base-3.14.12.2\bin\win32-x86;%PATH%
cd C:\epics\softioc_win32
set EPICS_DRIVER_PATH=%~dp0%\bin\win32-x86\
cd C:\epics\softioc_win32\iocBoot\softioc\
..\..\bin\win32-x86\ioc.exe C:\Qt\4.8.2\src\caQtDM\tests\st.cmd
cd ..\..

pause