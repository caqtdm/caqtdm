CAQTDM_TOP=../../..
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
QT += core gui
contains(QT_VER_MAJ, 5) {
    QT     += widgets
}
contains(QT_VER_MAJ, 6) {
    QT     += widgets
}

CONFIG += warn_on
CONFIG += epics3_plugin
CONFIG += caQtDM_Installation
INSTALLTIONSUBDIR = controlsystems
CAQTDM_INSTALL_LIB = epics3_plugin
include($$CAQTDM_TOP/caQtDM.pri)

MOC_DIR = ./moc
VPATH += ./src

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src


HEADERS         = epics3_plugin.h ../controlsinterface.h
SOURCES         = epics3_plugin.cpp epicsSubs.c
TARGET          = epics3_plugin
android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}



