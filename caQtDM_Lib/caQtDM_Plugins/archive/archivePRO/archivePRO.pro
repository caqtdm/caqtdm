include (../../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui network
contains(QT_VER_MAJ, 5) {
    QT     += widgets
}

CONFIG += warn_on
CONFIG += release
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
HEADERS         = ../../controlsinterface.h archivePRO_plugin.h ../archiverCommon.h \
    proRetrieval.h
SOURCES         =  archivePRO_plugin.cpp ../archiverCommon.cpp \
    proRetrieval.c
TARGET          = archivePRO_plugin

INCLUDEPATH    += ../../../../../Include

QMAKESPEC = $$(QMAKESPEC)
X64 = $$find(QMAKESPEC, 64)

isEmpty(X64) {
  message( "Building for 32 bit")
  LIBS += /afs/psi.ch/user/m/mezger/workarea/ACS/mezger/Libs/libNewLogRPC.a
} else {
  message( "Building for 64 bit")
  LIBS += /afs/psi.ch/user/m/mezger/workarea/ACS/mezger/Libs/libNewLogRPC_64.a
}
