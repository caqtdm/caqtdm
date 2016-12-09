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
HEADERS         = ../../controlsinterface.h archiveSF_plugin.h sfRetrieval.h ../archiverCommon.h
SOURCES         =  archiveSF_plugin.cpp sfRetrieval.cpp ../archiverCommon.cpp
TARGET          = archiveSF_plugin

#INCLUDEPATH    += ../../../../../Include

#LIBS += /afs/psi.ch/user/m/mezger/workarea/ACS/mezger/Libs/libNewLogRPC.a

