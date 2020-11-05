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
INCLUDEPATH    += $(CAQTDM_CA_ARCHIVELIBS)/Tools
INCLUDEPATH    += $(CAQTDM_CA_ARCHIVELIBS)/Storage
INCLUDEPATH    += $(CAQTDM_CA_ARCHIVELIBS)/xerces-c-3.1.4/src
INCLUDEPATH    += $(EPICSINCLUDE)
INCLUDEPATH    += $(EPICSINCLUDE)/os/Linux
#for epics 3.15 and gcc we need this
INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc

INCLUDEPATH    += $(QWTINCLUDE)
HEADERS         = ../../controlsinterface.h archiveCA_plugin.h ../archiverCommon.h
SOURCES         =  archiveCA_plugin.cpp ../archiverCommon.cpp
TARGET          = archiveCA_plugin

LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom

QMAKESPEC = $$(QMAKESPEC)
X64 = $$find(QMAKESPEC, 64)

isEmpty(X64) {
    message( "Building for 32 bit")
    LIBS += $(CAQTDM_CA_ARCHIVELIBS)/Storage/libStorage_32.a
    LIBS += $(CAQTDM_CA_ARCHIVELIBS)/Tools/libTools_32.a
    LIBS += $(CAQTDM_CA_ARCHIVELIBS)/xerces-c-3.1.4/libxerces-c_32.a
} else {
    message( "Building for 64 bit")
    LIBS += $(CAQTDM_CA_ARCHIVELIBS)/Storage/libStorage_64.a
    LIBS += $(CAQTDM_CA_ARCHIVELIBS)/Tools/libTools_64.a
    LIBS += $(CAQTDM_CA_ARCHIVELIBS)/xerces-c-3.1.4/libxerces-c_64.a
    LIBS +=  -L/lib64/ -lcurl
}
