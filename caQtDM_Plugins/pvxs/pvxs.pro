include (../../caQtDM_Viewer/qtdefs.pri)
QT += core gui
contains(QT_VER_MAJ, 5) {
    QT     += widgets
    CONFIG += c++11
}
contains(QT_VER_MAJ, 6) {
    QT     += widgets
    CONFIG += c++17
}

CONFIG += warn_on
CONFIG += release
CONFIG += pvxs_plugin
include (../../caQtDM.pri)

MOC_DIR = ./moc
VPATH += ./src

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../caQtDM_Lib/src
INCLUDEPATH    += ../../caQtDM_QtControls/src

HEADERS         = pvxs_plugin.h pvxsChannel.h pvxsValueMapping.h ../controlsinterface.h ../caQtDM_Plugins_global.h
SOURCES         = pvxs_plugin.cpp pvxsValueMapping.cpp
TARGET          = pvxs_plugin

warning("pvxs was specified in qtdefs.pri, so build plugin with the pvxs EPICS pvAccess client library")
