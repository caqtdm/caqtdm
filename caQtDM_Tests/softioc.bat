@echo on
set PATH=X:\epics\Package\base\bin\win32-x86;%PATH%
cd X:\qt\caqtdm\caQtDM_Tests\
softioc.exe -D X:\epics\Package\base\dbd\softioc.dbd st.cmd
cd ..\..

pause