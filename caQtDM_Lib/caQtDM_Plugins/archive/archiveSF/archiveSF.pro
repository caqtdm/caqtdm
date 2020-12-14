CAQTDM_TOP=../../../..
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)

QT += core gui network

contains(QT_VER_MAJ, 4) {
      CONFIG += designer
}
contains(QT_VER_MAJ, 5) {
      QT += uitools
}

CONFIG += archive_plugin
CONFIG += caQtDM_Installation
INSTALLTIONSUBDIR = controlsystems
include ($$CAQTDM_TOP/caQtDM.pri)

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

HEADERS         = ../../controlsinterface.h archiveSF_plugin.h sfRetrieval.h ../archiverCommon.h
SOURCES         =  archiveSF_plugin.cpp sfRetrieval.cpp ../archiverCommon.cpp
TARGET          = archiveSF_plugin


