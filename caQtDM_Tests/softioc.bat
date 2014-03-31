@echo on
set PATH=X:\epics\base-3.14.12.4\bin\win32-x86;%PATH%
cd X:\qt\caqtdm\caQtDM_Tests\
softioc.exe -D X:\epics\base-3.14.12.4\dbd\softioc.dbd st.cmd
cd ..\..

pause