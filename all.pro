_QTHOME=$$(QTHOME)
_QWTHOME=$$(QWTHOME)
_EPICSBASE=$$(EPICS_BASE)

isEmpty(_QTHOME){
 message("QTHOME must be defined in order to locate QT")
 error(please define QTHOME.)
}else{
 message("QT located in $$_QTHOME")
}

isEmpty(_QWTHOME){
 message("QWTHOME must be defined in order to locate QWT")
 error(please define QWTHOME.)
}else{
 message("qwt located in $$_QWTHOME")
}

isEmpty(_EPICSBASE){
 message("EPICS_BASE must be defined in order to use EPICS")
 error(please define EPICS_BASE.)
}else{
 message("EPICS located in $$_EPICSBASE")
}

#version check qt
contains(QT_VERSION, ^4\\.[0-7]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.8.")
}

TEMPLATE = subdirs
SUBDIRS = caQtDM_QtControls caQtDM_Lib caQtDM 
win32{
 CONFIG(debug, debug | release) 
}

contains(QMAKE_HOST.name, pc2401) {
SUBDIRS += QtTunes probeQtProscan HFdispQt
}
