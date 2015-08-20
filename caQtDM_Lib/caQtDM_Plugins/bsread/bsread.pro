include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui
contains(QT_VER_MAJ, 5) {
    QT     += widgets
}

CONFIG += warn_on
CONFIG += bsread_Plugin
include (../../../caQtDM.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = bsread_Plugin.h bsreadExternals.h ../controlsinterface.h \
    bsread_decode.h
SOURCES         = bsread_Plugin.cpp md5.cc \
    bsread_decode.cpp
TARGET          = bsread_Plugin


