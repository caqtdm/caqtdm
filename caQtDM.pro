include(qtdefs.pri)

unix {
  TEMPLATE = subdirs
  SUBDIRS = parser caQtDM1 
  exists("/home/ACS/Control/Lib/libDEV.so") {
         SUBDIRS += caQtDM2
  }
}

win32 {
QT       += core gui svg
CONFIG   += qt warn_on thread uitools console

TARGET = caQtDM
TEMPLATE = app

SOURCES +=\
    caQtDM.cpp \
    fileopenwindow.cpp \
    messagebox.cpp

HEADERS  +=  \
    messagebox.h \
    fileopenwindow.h

 FORMS    += \
     main.ui

 MOC_DIR = moc
 OBJECTS_DIR = obj
 DESTDIR = .
 VPATH += ./src
 UI_DIR += ./
 
RESOURCES += ./caQtDM.qrc
OTHER_FILES += ./caQtDM.ico

win32-msvc* {
        DebugBuild {
        	RC_FILE = caQtDM.rc
                EPICS_LIBS=$${EPICS_BASE}/lib/win32-x86
                OBJECTS_DIR = debug/obj
        }
        ReleaseBuild {
        	RC_FILE = caQtDM.rc
                EPICS_LIBS=$${EPICS_BASE}/lib/win32-x86-debug
                OBJECTS_DIR = release/obj
        }
}
 
 INCLUDEPATH += .
 INCLUDEPATH += $(QWTHOME)/src
 INCLUDEPATH += $${EPICS_BASE}/include
 INCLUDEPATH += $${EPICS_BASE}/include/os/win32
      
 DEFINES += BUILDTIME=\\\"\\\"
 DEFINES += BUILDDATE=\\\"\\\"
 DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
 DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"
 DEFINES += SUPPORT=\\\"EPICS\\\"

     win32-g++ {
	 LIBS += $${QWTHOME}/lib/libqwt.a
	 LIBS += $${QTCONTROLS_LIBS}/libqtcontrols.a
	 LIBS += $${EPICS_LIBS}/ca.lib
	 LIBS += $${EPICS_LIBS}/COM.lib
	 LIBS += $${CAQTDM_LIBS}/libcaQtDm_Lib.a
     }



     win32-msvc* {
        DebugBuild {
                LIBS += $${CAQTDM_LIB}/debug/caQtDM_Lib.lib
                LIBS += $${QWTHOME}/lib/qwtd.lib
                LIBS += $${EPICS_BASE}/lib/win32-x86-debug/ca.lib
                LIBS += $${EPICS_BASE}/lib/win32-x86-debug/COM.lib
                LIBS += $${QTCONTROLS}/debug/qtcontrols.lib

        }

        ReleaseBuild {
                LIBS += $${CAQTDM_LIB}/release/caQtDM_Lib.lib
                LIBS += $${QWTHOME}/lib/qwt.lib
                LIBS += $${EPICS_BASE}/lib/win32-x86/ca.lib
                LIBS += $${EPICS_BASE}/lib/win32-x86/COM.lib
                LIBS += $${QTCONTROLS}/release/qtcontrols.lib
                QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM.exe $${BINARY_LOCATION}
        }
     }

}
