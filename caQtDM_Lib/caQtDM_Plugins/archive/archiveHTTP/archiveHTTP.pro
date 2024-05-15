include (../../../../caQtDM_Viewer/qtdefs.pri)

QT += core gui network

contains(QT_VER_MAJ, 4) {
      CONFIG += designer
}
contains(QT_VER_MAJ, 5) {
      QT += uitools
}
contains(QT_VER_MAJ, 6) {
      QT += uitools
}

CONFIG += archive_plugin

include (../../../../caQtDM.pri)

MOC_DIR = ./moc
VPATH += ./src

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../
INCLUDEPATH    += ../../../src
INCLUDEPATH    += ../../../../caQtDM_QtControls/src/
INCLUDEPATH    += $(QWTINCLUDE)

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}

HEADERS         = ../../controlsinterface.h \
    archivehttp_plugin.h \
	httpretrieval.h \
	../archiverGeneral.h \
	httpperformancedata.h \
    urlhandlerhttp.h \
    workerHttp.h \
    workerHttpThread.h
SOURCES         =  archivehttp_plugin.cpp \
    httpretrieval.cpp \
	../archiverGeneral.cpp \
    httpperformancedata.cpp \
    urlhandlerhttp.cpp \
    workerHttp.cpp \
    workerHttpThread.cpp

TARGET          = archiveHTTP_plugin

DISTFILES +=


