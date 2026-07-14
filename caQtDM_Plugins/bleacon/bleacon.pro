include (../../caQtDM_Viewer/qtdefs.pri)
# network/concurrent for the config file download (fileFunctions),
# bluetooth for ibeacon/eddystone reception (on ios eddystone only)
QT += core gui network bluetooth
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
HEADERS         = bleacon_plugin.h beaconscanner.h beaconscanner_sim.h beaconscanner_qtble.h ../controlsinterface.h ../caQtDM_Plugins_global.h
SOURCES         = bleacon_plugin.cpp beaconscanner.cpp beaconscanner_sim.cpp beaconscanner_qtble.cpp

ios {
    # ios strips iBeacon frames from CoreBluetooth: iBeacon ranging goes through
    # CoreLocation, eddystone (service data) still through Qt Bluetooth in parallel
    HEADERS           += beaconscanner_ios.h
    OBJECTIVE_SOURCES += beaconscanner_ios.mm
    LIBS              += -framework CoreLocation -framework Foundation
}

TARGET          = bleacon_plugin
