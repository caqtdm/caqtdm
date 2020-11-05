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
HEADERS         = ../../controlsinterface.h archiveHIPA_plugin.h ../archiverCommon.h \
    hipaRetrieval.h
SOURCES         = archiveHIPA_plugin.cpp ../archiverCommon.cpp \
    hipaRetrieval.c
TARGET          = archiveHIPA_plugin

INCLUDEPATH    += $(CAQTDM_LOGGING_ARCHIVELIBS)


QMAKESPEC = $$(QMAKESPEC)
X64 = $$find(QMAKESPEC, 64)

isEmpty(X64) {
  message( "Building for 32 bit")
  LIBS += $(CAQTDM_LOGGING_ARCHIVELIBS)/libNewLogRPC.a
} else {
  message( "Building for 64 bit")
  LIBS +=  $(CAQTDM_LOGGING_ARCHIVELIBS)/libNewLogRPC_64.a
}




