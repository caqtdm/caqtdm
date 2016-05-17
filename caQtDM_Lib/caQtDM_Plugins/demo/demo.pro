include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui
contains(QT_VER_MAJ, 5) {
    QT     += widgets
}
CONFIG += warn_on
CONFIG += release
CONFIG += demo_plugin
include (../../../caQtDM.pri)

MOC_DIR = ./moc
VPATH += ./src

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = demo_plugin.h ../controlsinterface.h
SOURCES         = demo_plugin.cpp
TARGET          = demo_plugin

