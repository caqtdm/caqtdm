include (../caQtDM/qtdefs.pri)

unix {
  TEMPLATE = subdirs
  SUBDIRS = caQtDM_Lib1
  exists("/home/ACS/Control/Lib/libDEV.so") {
         SUBDIRS += caQtDM_Lib2
  }
}

win32 {


win32-msvc* {
        DEFINES +=_CRT_SECURE_NO_WARNINGS
        DEFINES += CAQTDM_LIB_LIBRARY
        
        DebugBuild {
                EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86
                DESTDIR = debug
                OBJECTS_DIR = debug/obj
                LIBS += $$(QWTHOME)/lib/qwtd.lib
                LIBS += $${EPICS_LIBS}/ca.lib
                LIBS += $${EPICS_LIBS}/COM.lib
                LIBS += $$(QTCONTROLS_LIBS)/debug/qtcontrols.lib
        }
        ReleaseBuild {
                EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-debug
                DESTDIR = release
                OBJECTS_DIR = release/obj
                LIBS += $$(QWTHOME)/lib/qwt.lib
                LIBS += $${EPICS_LIBS}/ca.lib
                LIBS += $${EPICS_LIBS}/COM.lib
                LIBS += $$(QTCONTROLS_LIBS)/release/qtcontrols.lib
                QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM_Lib.dll ..\caQtDM_Binaries
        }
 }
     win32-g++ {
        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
	LIBS += $$(QWTLIB)/libqwt.a
	LIBS += $$(QTCONTROLS_LIBS)/release//libqtcontrols.a
	LIBS += $${EPICS_LIBS}/ca.lib
	LIBS += $${EPICS_LIBS}/COM.lib
	QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM_Lib.dll ..\caQtDM_Binaries
     }
MOC_DIR = ./moc
VPATH += ./src
INCLUDEPATH += ./src
UI_DIR += ./
INCLUDEPATH += ../caQtDM_QtControls/src
INCLUDEPATH += $$(EPICS_BASE)/include
INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
include (./caQtDM_Lib.pri)




}