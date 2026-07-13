include (../../caQtDM_Viewer/qtdefs.pri)
# network/concurrent for the config file download (fileFunctions)
QT += core gui network
contains(QT_VER_MAJ, 5) {
    QT     +=  widgets concurrent
}
contains(QT_VER_MAJ, 6) {
    QT     +=  widgets concurrent
}

CONFIG += warn_on
CONFIG += release
CONFIG += bleacon_plugin
include (../../caQtDM.pri)

MOC_DIR = ./moc
VPATH += ./src

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../caQtDM_Lib/src
INCLUDEPATH    += ../../caQtDM_QtControls/src
HEADERS         = bleacon_plugin.h beaconscanner.h beaconscanner_sim.h ../controlsinterface.h ../caQtDM_Plugins_global.h
SOURCES         = bleacon_plugin.cpp beaconscanner.cpp beaconscanner_sim.cpp

ios {
    # ios strips iBeacon frames from CoreBluetooth: ranging goes through CoreLocation
    HEADERS           += beaconscanner_ios.h
    OBJECTIVE_SOURCES += beaconscanner_ios.mm
    LIBS              += -framework CoreLocation -framework Foundation
} else {
    QT      += bluetooth
    HEADERS += beaconscanner_qtble.h
    SOURCES += beaconscanner_qtble.cpp
}

TARGET          = bleacon_plugin
