include(qtdefs.pri)

unix {
  TEMPLATE = subdirs
  SUBDIRS = caQtDM1
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
 
 INCLUDEPATH += .
 INCLUDEPATH += $${QWTHOME}/src
 INCLUDEPATH += $${QTCONTROLS_INCLUDES}
 INCLUDEPATH += $${CAQTDM_INCLUDES}
 INCLUDEPATH += $${EPICS_INCLUDES1}
 INCLUDEPATH += $${EPICS_INCLUDES2}
      
 LIBS += $${QWTHOME}/lib/libqwt.a
 LIBS += $${QTCONTROLS_LIBS}/libqtcontrols.a
 LIBS += $${EPICS_LIBS}/ca.lib
 LIBS += $${EPICS_LIBS}/COM.lib
 LIBS += $${CAQTDM_LIBS}/libcaQtDm_Lib.a
 DEFINES += BUILDTIME=\\\"\\\"
 DEFINES += BUILDDATE=\\\"\\\"
 DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
 DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"
 DEFINES += SUPPORT=\\\"EPICS\\\"
 UI_DIR += ./
 RESOURCES +=
}
